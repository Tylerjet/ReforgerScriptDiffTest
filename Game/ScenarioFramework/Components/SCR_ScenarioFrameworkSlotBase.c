[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class SCR_ScenarioFrameworkSlotBaseClass : SCR_ScenarioFrameworkLayerBaseClass
{
	// prefab properties here
};

//------------------------------------------------------------------------------------------------
class SCR_ScenarioFrameworkSlotBase : SCR_ScenarioFrameworkLayerBase
{
	[Attribute(desc: "Resource name of the object to be spawned", category: "Asset")]
	protected ResourceName 		m_sObjectToSpawn;
	
	[Attribute(desc: "Name of the entity used for identification", category: "Asset")];
	protected string					m_sID;
	
	[Attribute(desc: "This won't spawn new object, but it will rather use the object already existing in the world", category: "Asset")]
 	protected bool 		m_bUseExistingWorldAsset;
	
	[Attribute(desc: "Overrides display name of the spawned object for task purposes", category: "Asset")]
	protected string 	m_sOverrideObjectDisplayName;
	
	[Attribute(desc: "Randomize spawned asset(s) per Faction Attribute which needs to be filled as well. Will override Object To Spawn Attribute.", category: "Randomization")]
	protected bool						m_bRandomizePerFaction;
	
	[Attribute("0", UIWidgets.ComboBox, "Select Entity Catalog type for random spawn", "", ParamEnumArray.FromEnum(EEntityCatalogType), category: "Randomization")]
	protected EEntityCatalogType	m_eEntityCatalogType;
	
	[Attribute("0", UIWidgets.ComboBox, "Select Entity Labels which you want to optionally include to random spawn. If you want to spawn everything, you can leave it out empty and also leave Include Only Selected Labels attribute to false.", "", ParamEnumArray.FromEnum(EEditableEntityLabel), category: "Randomization")]
	protected ref array<EEditableEntityLabel> 		m_aIncludedEditableEntityLabels;
	
	[Attribute("0", UIWidgets.ComboBox, "Select Entity Labels which you want to exclude from random spawn", "", ParamEnumArray.FromEnum(EEditableEntityLabel), category: "Randomization")]
	protected ref array<EEditableEntityLabel> 		m_aExcludedEditableEntityLabels;
	
	[Attribute(desc: "If true, it will spawn only the entities that are from Included Editable Entity Labels and also do not contain Label to be Excluded.", category: "Randomization")]
	protected bool										m_bIncludeOnlySelectedLabels;
		
	//TODO: not yet implemented
	//[Attribute(defvalue: "100", desc: "Spawn probability", category: "Asset")]
 	//protected float 			m_fProbability;
		
	protected bool						m_bSelected = false;
	protected ref EntitySpawnParams 	m_SpawnParams = new EntitySpawnParams;
	vector 								m_Size;
	protected ResourceName 				m_sRandomlySpawnedObject; 	
	
#ifdef WORKBENCH
	[Attribute(defvalue: "1", desc: "Show the debug shapes in Workbench", category: "Debug")];
	protected bool							m_bShowDebugShapesInWorkbench;
	
	protected WorldEditorAPI	m_API;
	protected IEntity			m_PreviewEntity;
#endif

	//------------------------------------------------------------------------------------------------
	//! Get objects of type defined in m_sObjectToSpawn in the range
	protected void QueryObjectsInRange(float fRange = 2.5)
	{
		BaseWorld pWorld = GetGame().GetWorld();
		if (!pWorld)
			return;
			
		pWorld.QueryEntitiesBySphere(GetOwner().GetOrigin(), fRange, GetEntity, null, EQueryEntitiesFlags.ALL);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnObjectDamage(EDamageState state)
	{
		if (state != EDamageState.DESTROYED || !m_Entity)
			return;
		
		ScriptedDamageManagerComponent objectDmgManager = ScriptedDamageManagerComponent.Cast(m_Entity.FindComponent(ScriptedDamageManagerComponent));
		if (objectDmgManager)
	 		objectDmgManager.GetOnDamageStateChanged().Remove(OnObjectDamage);

		SetIsTerminated(true);				
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get object of type defined in m_sObjectToSpawn
	protected bool GetEntity(notnull IEntity entity)
	{
		IEntity pParent = SCR_EntityHelper.GetMainParent(entity, true);
		EntityPrefabData prefabData = pParent.GetPrefabData();
		if (!prefabData)
			return true;

		ResourceName resource = prefabData.GetPrefabName();
		if (resource.IsEmpty())
		{
			resource = prefabData.GetPrefab().GetAncestor().GetName();
			if (resource.IsEmpty())
			{
				if (!m_sID.IsEmpty())
				{
					if (entity.GetName() != m_sID)
						return true;
				}
				else
				{
					return true;
				}			
			}
			else
			{
				if (m_sObjectToSpawn.IsEmpty())
					return true;
				
				TStringArray strs = new TStringArray;
				resource.Split("/", strs, true);
				string resourceName = strs[strs.Count() - 1];
				
				TStringArray strsObject = new TStringArray;
				m_sObjectToSpawn.Split("/", strsObject, true);
				string resourceObject = strsObject[strsObject.Count() - 1];
				
				if (resourceName == resourceObject)
				{
					m_Entity = entity;
					return false;
				}
				else
				{
					return true;
				}
			}
		}
		else
		{	
			if (resource != m_sObjectToSpawn)
				return true;
		}
		
		m_Entity = entity;
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get the Layer Task which is parent of this Slot 
	SCR_ScenarioFrameworkArea GetAreaWB() 
	{ 
		SCR_ScenarioFrameworkArea area;
		IEntity entity = GetOwner().GetParent();
		while (entity)
		{
			area = SCR_ScenarioFrameworkArea.Cast(entity.FindComponent(SCR_ScenarioFrameworkArea));
			if (area)
				return area;
			
			entity = entity.GetParent();
		}
		
		return area;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetSpawnedObjectName()
	{
		return m_sID;
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetObjectToSpawn()
	{
		return m_sObjectToSpawn;
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetRandomlySpawnedObject()
	{
		return m_sRandomlySpawnedObject;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetRandomlySpawnedObject(ResourceName name)
	{
		m_sRandomlySpawnedObject = name;
	}
	
	//------------------------------------------------------------------------------------------------
	override void Init(SCR_ScenarioFrameworkArea area = null, SCR_ScenarioFrameworkEActivationType activation = SCR_ScenarioFrameworkEActivationType.SAME_AS_PARENT, bool bInit = true)
	{
		if (m_bIsTerminated)
			return;
	
		if (m_eActivationType != activation)
			return;

		if (m_Entity && !m_bEnableRepeatedSpawn)
		{
			IEntity entity = GetOwner().GetParent();
			if (!entity)
				return;
				
			SCR_ScenarioFrameworkLayerBase layerBase = SCR_ScenarioFrameworkLayerBase.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerBase));
			if (!layerBase)
				return;
				
			if (!layerBase.GetEnableRepeatedSpawn())
			{
				PrintFormat("ScenarioFramework: Object %1 already exists and won't be spawned, exiting...", m_Entity, LogLevel.ERROR);
				return;
			}
		}
		
		// Handles inheritance of faction settings from parents
		if (m_sFactionKey.IsEmpty() && m_ParentLayer && !m_ParentLayer.GetFactionKey().IsEmpty())
			SetFactionKey(m_ParentLayer.GetFactionKey());
		
		SelectRandomSlot();		//if subSlots exists select one
		
		if (!m_bUseExistingWorldAsset)
		{
			m_Entity = SpawnAsset();
		}
		else
		{
			QueryObjectsInRange();	//sets the m_Entity in subsequent callback
		}
		
		if (!m_Entity)
			return;
		
		if (!m_sID.IsEmpty())
			m_Entity.SetName(m_sID);	
		
		ScriptedDamageManagerComponent objectDmgManager = ScriptedDamageManagerComponent.Cast(m_Entity.FindComponent(ScriptedDamageManagerComponent));
		if (objectDmgManager)
			objectDmgManager.GetOnDamageStateChanged().Insert(OnObjectDamage);
		
		foreach(SCR_ScenarioFrameworkPlugin plugin : m_aPlugins)
		{
			plugin.Init(this);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnAllAreasInitiatedInvoker()
	{
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return null;
		
		SCR_GameModeSFManager pGameModeMgr = SCR_GameModeSFManager.Cast(gameMode.FindComponent(SCR_GameModeSFManager));
		if (!pGameModeMgr)
			return null;
		
		return pGameModeMgr.GetOnAllAreasInitiated();
	}
	
	//------------------------------------------------------------------------------------------------
	// Slot cannot have children
	override void SpawnChildren(bool bInit = true, bool previouslyRandomized = false)
	{
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetRandomAsset(out ResourceName prefab)
	{
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
		{
			Print("ScenarioFramework Randomized Spawn: Faction manager not found.", LogLevel.ERROR);
			return string.Empty;
		}

		SCR_Faction faction = SCR_Faction.Cast(factionManager.GetFactionByKey(m_sFactionKey));
		if (!faction)
		{
			Print("ScenarioFramework Randomized Spawn: Selected faction not found.", LogLevel.ERROR);
			return string.Empty;
		}
			
		SCR_EntityCatalog entityCatalog = faction.GetFactionEntityCatalogOfType(m_eEntityCatalogType);
		if (!entityCatalog)
		{
			Print("ScenarioFramework Randomized Spawn: Faction Entity Catalog not found for selected type.", LogLevel.ERROR);
			return string.Empty;
		}
						
		array<SCR_EntityCatalogEntry> aFactionEntityEntry = {};
		entityCatalog.GetFullFilteredEntityListWithLabels(aFactionEntityEntry, m_aIncludedEditableEntityLabels, m_aExcludedEditableEntityLabels, m_bIncludeOnlySelectedLabels);
		if (aFactionEntityEntry.IsEmpty())
		{
			Print("ScenarioFramework Randomized Spawn: Applied labels resulted in no viable prefabs to be randomly selected.", LogLevel.ERROR);
			return string.Empty;
		}
			
		prefab = aFactionEntityEntry.GetRandomElement().GetPrefab();
		m_sRandomlySpawnedObject = prefab;
		return prefab;
	}
	
	//------------------------------------------------------------------------------------------------
	IEntity SpawnAsset()
	{
		//If Randomization is enabled, it will try to apply settings from Attributes.
		//If it fails anywhere, original m_sObjectToSpawn will be used.
		if (m_bRandomizePerFaction)
		{
			ResourceName randomAsset; 
			if (SCR_StringHelper.IsEmptyOrWhiteSpace(GetRandomlySpawnedObject()))
			{
				GetRandomAsset(randomAsset);
				if (!SCR_StringHelper.IsEmptyOrWhiteSpace(randomAsset)) 
					m_sObjectToSpawn = randomAsset;
			}
			else
			{
				randomAsset = GetRandomlySpawnedObject();
			}
		}
		
		Resource resource = Resource.Load(m_sObjectToSpawn);
		if (!resource)
			return null;
		
		//--- Get slot transformation
		vector mat[4];
		GetOwner().GetWorldTransform(mat);
		
		m_SpawnParams.TransformMode = ETransformMode.WORLD;
		//m_SpawnParams.Transform = mat;
		
		//--- Apply rotation
		vector angles = Math3D.MatrixToAngles(m_SpawnParams.Transform);
		Math3D.AnglesToMatrix(angles, m_SpawnParams.Transform);
		
		//--- Spawn the prefab
		BaseResourceObject resourceObject = resource.GetResource();
		if (!resourceObject)
			return null;

		string resourceName = resourceObject.GetResourceName();
		IEntity entity = GetGame().SpawnEntityPrefab(resource, GetGame().GetWorld(), m_SpawnParams);
		SCR_AIWorld aiWorld = SCR_AIWorld.Cast(GetGame().GetAIWorld());
		if(aiWorld)
			aiWorld.RequestNavmeshRebuildEntity(entity);
	
		return entity;
	}
		
	//------------------------------------------------------------------------------------------------
	void SelectRandomSlot()
	{
		array<ref EntitySpawnParams> aPosOut = {};
		GetSubSlots(aPosOut);
		
		if (aPosOut.IsEmpty())
		{
			GetOwner().GetWorldTransform(m_SpawnParams.Transform);
		}
		else
		{
			Math.Randomize(-1);
			m_SpawnParams = aPosOut[Math.RandomInt(0, aPosOut.Count())];
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void GetSubSlots(out notnull array<ref EntitySpawnParams> aPosOut)
	{
		IEntity child = GetOwner().GetChildren();
		
		while (child)	
		{
			ref EntitySpawnParams params = new EntitySpawnParams;
			child.GetWorldTransform(params.Transform);
			aPosOut.Insert(params);
			child = child.GetSibling();			
		}
	}
	
#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	override void _WB_AfterWorldUpdate(IEntity owner, float timeSlice)
	{
		DrawDebugShape(m_bShowDebugShapesInWorkbench);
	}
	
	//------------------------------------------------------------------------------------------------
	void SpawnEntityPreview(IEntity owner, Resource resource)
	{
		EntitySpawnParams spawnParams = new EntitySpawnParams;
		spawnParams.TransformMode = ETransformMode.WORLD;
		owner.GetWorldTransform(spawnParams.Transform);
			
		m_PreviewEntity = GetGame().SpawnEntityPrefab(resource, owner.GetWorld(), spawnParams);
	}
	
	//------------------------------------------------------------------------------------------------
	override void _WB_OnInit(IEntity owner, inout vector mat[4], IEntitySource src)
	{
		SCR_EntityHelper.DeleteEntityAndChildren(m_PreviewEntity);
			
		Resource resource = Resource.Load(m_sObjectToSpawn);
		if (!resource)
			return;
			
		SpawnEntityPreview(owner, resource);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool _WB_OnKeyChanged(IEntity owner, BaseContainer src, string key, BaseContainerList ownerContainers, IEntity parent)
	{
		if (key == "m_bShowDebugShapesInWorkbench")
			DrawDebugShape(m_bShowDebugShapesInWorkbench);
		
		if (key == "coords")
		{
			SCR_EntityHelper.DeleteEntityAndChildren(m_PreviewEntity);
			
			Resource resource = Resource.Load(m_sObjectToSpawn);
			if (!resource)
				return false;
			
			SpawnEntityPreview(owner, resource);
			return true;
		}
		else if (key == "m_sObjectToSpawn")
		{
			SCR_EntityHelper.DeleteEntityAndChildren(m_PreviewEntity);
			return false;
		}
		return false;
	}
			
	//------------------------------------------------------------------------------------------------
	void SCR_ScenarioFrameworkSlotBase(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_iDebugShapeColor = ARGB(100, 0x99, 0x10, 0xF2);
	}
#endif	
};

[BaseContainerProps()];
class SCR_WaypointSet	
{
	[Attribute("Name")]
 	string m_sName ;	
	
	[Attribute("Use random order")]
	bool m_bUseRandomOrder;
	
	[Attribute("Cycle the waypoints")]
	bool m_bCycleWaypoints;	
};