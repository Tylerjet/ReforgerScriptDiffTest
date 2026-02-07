//------------------------------------------------------------------------------------------------
[WorkbenchToolAttribute("Destruction indices assign tool", "Automatically assigns response indices to destructible objects", "", awesomeFontCode: 0xf7e4)]
class SCR_DestructionIndicesAssignTool : WorldEditorTool
{
	[Attribute()]
	protected ref array<ref SCR_MassResponseIndexPair> m_aPairs;
	
	const ref array<string> EXTENSIONS = {"et"};
	const ref array<string> DESTRUCTIBLE_COMPONENT_CLASSES = {"SCR_DestructionBaseComponent", "SCR_DestructionMultiPhaseComponent"};
	const string PHYSICS_COMPONENT = "RigidBody";
	
	protected ref SCR_IndicesAssignToolHandler m_Handler;
	
	//------------------------------------------------------------------------------------------------
	protected float CalculateDensity(notnull IEntity entity)
	{
		Physics physics = entity.GetPhysics();
		if (!physics)
			return 0;
		
		array<SurfaceProperties> surfaces = new array<SurfaceProperties>();
		GameMaterial gameMaterial;
		BallisticInfo ballisticInfo;
		
		float totalDensity;
		int j, densitiesCount;
		
		for (int i = physics.GetNumGeoms() - 1; i >= 0; i--)
		{
			surfaces.Clear();
			physics.GetGeomSurfaces(i, surfaces);
			
			for (j = surfaces.Count() - 1; i >= 0; i--)
			{
				gameMaterial = surfaces[j];
				if (!gameMaterial)
					continue;
				
				ballisticInfo = gameMaterial.GetBallisticInfo();
				if (!ballisticInfo)
					continue;
				
				totalDensity += ballisticInfo.GetDensity();
				densitiesCount++;
			}
		}
		
		if (densitiesCount != 0)
			return totalDensity / densitiesCount;
		else
			return 0;
	}
	
	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Show Mass")]
	protected void ShowMass()
	{
		IEntity entity = m_API.GetSelectedEntity();
		float volume = MeshObjectVolumeCalculator.GetVolumeFromColliders(entity, EPhysicsLayerDefs.FireGeometry);
		float density = CalculateDensity(entity);
		Print(density * 1000 * volume);
	}
	
	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Assign Indices")]
	protected void AssignIndices()
	{
		Debug.BeginTimeMeasure();
		Workbench.SearchResources(m_Handler.Callback, EXTENSIONS);
		
		array<ResourceName> resourceNames = m_Handler.GetResourceNames();
		
		Resource resource;
		BaseResourceObject baseResource;
		IEntitySource source;
		IEntitySource runtimeSource;
		string componentClassName;
		IEntity entity;
		int componentCount;
		int j, k, l;
		bool hasDestruction, hasPhysics;
		int componentClassesCount = DESTRUCTIBLE_COMPONENT_CLASSES.Count();
		float density;
		float volume;
		float mass;
		int pairsCount = m_aPairs.Count();
		string indexName;
		
		IEntityComponentSource physicsSource;
		
		array<ref ContainerIdPathEntry> entryPath = {ContainerIdPathEntry(PHYSICS_COMPONENT)};
		
		m_API.BeginEntityAction();
		
		for (int i = resourceNames.Count() - 1; i >= 0; i--)
		{
			hasDestruction = false;
			hasPhysics = false;
			resource = Resource.Load(resourceNames[i]);
			baseResource = resource.GetResource();
			if (!baseResource)
			{
				resourceNames.Remove(i);
				continue;
			}
			
			source = baseResource.ToEntitySource();
			componentCount = source.GetComponentCount();
			for (j = componentCount - 1; j >= 0; j--)
			{
				componentClassName = source.GetComponent(j).GetClassName();
				if (!hasPhysics && componentClassName == PHYSICS_COMPONENT)
					hasPhysics = true;
				
				for (k = componentClassesCount - 1; k >= 0; k--)
				{
					if (componentClassName == DESTRUCTIBLE_COMPONENT_CLASSES[k])
					{
						physicsSource = source.GetComponent(j);
						hasDestruction = true;
						break;
					}
				}
				
				if (hasPhysics && hasDestruction)
					break;
			}
			
			if (!hasDestruction || !hasPhysics)
				continue;
			
			entity = m_API.CreateEntity(resourceNames[i], "", 0, null, vector.Zero, vector.Zero);
			if (!entity)
				continue;
			
			runtimeSource = m_API.EntityToSource(entity);
			
			volume = MeshObjectVolumeCalculator.GetVolumeFromColliders(entity, EPhysicsLayerDefs.FireGeometry);
			
			density = CalculateDensity(entity);
			
			mass = density * 1000 * volume; // 1000 g/cm3 -> kg/m3
			
			for (l = 0; l < pairsCount; l++)
			{
				if (mass < m_aPairs[l].GetMass())
				{
					indexName = m_aPairs[l].GetIndexName();
					break;
				}
				else if (l == pairsCount - 1) //last pair, mass is bigger than last entry -> set max response index
					indexName = ScriptedDamageManagerComponent.MAX_DESTRUCTION_RESPONSE_INDEX_NAME;
			}
			
			//Edit prefab here
			m_API.SetVariableValue(source, entryPath, "ResponseIndex", indexName);
			entity = m_API.SourceToEntity(runtimeSource); //Refresh pointer after changes above
			
			m_API.DeleteEntity(entity);
		}
		
		m_API.EndEntityAction();
		
		Debug.EndTimeMeasure("Assigning done");
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_DestructionIndicesAssignTool()
	{
		m_Handler = new SCR_IndicesAssignToolHandler();
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_MassResponseIndexPair
{
	[Attribute(desc: "Objects with smaller weight will have this index. [kg]")]
	protected float m_fMass;
	
	[Attribute(desc: "Refer to physics settings of the project.")]
	protected int m_iIndex;
	
	[Attribute(desc: "Refer to physics settings of the project.")]
	protected string m_sIndexName;
	
	//------------------------------------------------------------------------------------------------
	float GetMass()
	{
		return m_fMass;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetIndexName()
	{
		return m_sIndexName;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetIndex()
	{
		return m_iIndex;
	}
};

//------------------------------------------------------------------------------------------------
class SCR_IndicesAssignToolHandler
{
	protected SCR_DestructionIndicesAssignTool m_Tool;
	protected ref array<ResourceName> m_aResourceNames = {};
	
	//------------------------------------------------------------------------------------------------
	array<ResourceName> GetResourceNames()
	{
		return m_aResourceNames;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetTool(SCR_DestructionIndicesAssignTool tool)
	{
		m_Tool = tool;
	}
	
	//------------------------------------------------------------------------------------------------
	void Callback(ResourceName resName, string filePath = "")
	{
		m_aResourceNames.Insert(resName);
	}
};