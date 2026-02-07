[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class CP_SlotBaseClass : CP_LayerBaseClass
{
	// prefab properties here
}

//------------------------------------------------------------------------------------------------
class CP_SlotBase : CP_LayerBase
{
	[Attribute("", category: "Asset")]
	protected ResourceName 		m_rObjectToSpawn;
	
	[Attribute("This won't spawn new object, but it will rather use the object already existing in the world", category: "Asset")]
 	protected bool 		m_bUseExistingWorldAsset;
	
	[Attribute("",desc: "Overrides display name of the spawned object for task purposes", category: "Asset")]
	protected string 	m_sOverrideObjectDisplayName;
	
	[Attribute("", category: "Asset")]
 	protected FactionKey 		m_sFaction;

		
	//TODO: not yet implemented
	//[Attribute(defvalue: "100", desc: "Spawn probability", category: "Asset")]
 	//protected float 			m_fProbability;
	
	[Attribute(defvalue: "", desc: "Name of the entity used for identification", category: "Asset")];
	protected string					m_sID;
		
	protected bool						m_bSelected = false;
	protected ref EntitySpawnParams 	m_pSpawnParams = new EntitySpawnParams;
	vector 								m_size; 	
	
	protected IEntitySource m_pSrc;
	
	#ifdef WORKBENCH
		protected WorldEditorAPI	m_API;
	#endif
		

	//------------------------------------------------------------------------------------------------
	//! Get objects of type defined in m_rObjectToSpawn in the range
	protected void QueryObjectsInRange(float fRange = 2.5)
	{
		BaseWorld pWorld = GetGame().GetWorld();
		if (!pWorld)
			return;
		pWorld.QueryEntitiesBySphere(GetOwner().GetOrigin(), fRange, GetEntity, null, EQueryEntitiesFlags.ALL);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get object of type defined in m_rObjectToSpawn
	protected bool GetEntity(notnull IEntity entity)
	{
		IEntity pParent = SCR_EntityHelper.GetMainParent(entity, true);
		EntityPrefabData pPrefabData = pParent.GetPrefabData();
		if (!pPrefabData)
			return true;

		ResourceName pResource = pPrefabData.GetPrefabName();
		if (pResource.IsEmpty())
		{
			pResource = pPrefabData.GetPrefab().GetAncestor().GetName();
			if (pResource.IsEmpty())
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
				if (m_rObjectToSpawn.IsEmpty())
					return true;
				
				TStringArray strs = new TStringArray;
				pResource.Split("/", strs, true);
				string pResourceName = strs[strs.Count() - 1];
				
				TStringArray strsObject = new TStringArray;
				m_rObjectToSpawn.Split("/", strsObject, true);
				string pResourceObject = strsObject[strsObject.Count() - 1];
				
				if (pResourceName == pResourceObject)
				{
					m_pEntity = entity;
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
			if (pResource != m_rObjectToSpawn)
				return true;
		}
		
		m_pEntity = entity;
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get the Layer Task which is parent of this Slot 
	CP_Area GetAreaWB() 
	{ 
		CP_Area pArea;
		IEntity entity = GetOwner().GetParent();
		while (entity)
		{
			pArea = CP_Area.Cast(entity.FindComponent(CP_Area));
			if (pArea)
				return pArea;
			
			entity = entity.GetParent();
		}
		
		return pArea;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetSpawnedObjectName() { return m_sID; }
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetObjectToSpawn() { return m_rObjectToSpawn; }
	
	//------------------------------------------------------------------------------------------------
	override void Init(CP_Area pArea = null, CP_EActivationType EActivation = CP_EActivationType.SAME_AS_PARENT, bool bInit = true)
	{
	
		if (m_EActivationType != EActivation)
			return;
		if (m_pEntity && !m_bEnableRepeatedSpawn)
		{
			IEntity entity = GetOwner().GetParent();
			if (!entity)
				return;
				
			CP_LayerBase pLayerBase = CP_LayerBase.Cast(entity.FindComponent(CP_LayerBase));
			if (!pLayerBase)
				return;
				
			if (!pLayerBase.GetEnableRepeatedSpawn())
			{
				PrintFormat("CP: Object %1 already exists and won't be spawned, exiting...", m_pEntity);
				return;
			}
		}
		
		SelectRandomSlot();		//if subSlots exists select one
		
		if (!m_bUseExistingWorldAsset)
		{
			m_pEntity = SpawnAsset();
		}
		else
		{
			QueryObjectsInRange();	//sets the m_pEntity in subsequent callback
		}
		
		if (!m_pEntity)
			return;
		m_pEntity.SetName(m_sID);	
		
		foreach(CP_Plugin pPlugin : m_aPlugins)
			pPlugin.Init(this);
		
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnAllAreasInitiatedInvoker()
	{
		BaseGameMode pGameMode = GetGame().GetGameMode();
		if (!pGameMode)
			return null;
		
		SCR_GameModeSFManager pGameModeMgr = SCR_GameModeSFManager.Cast(pGameMode.FindComponent(SCR_GameModeSFManager));
		if (!pGameModeMgr)
			return null;
		
		return pGameModeMgr.GetOnAllAreasInitiated();
	}
	
	//------------------------------------------------------------------------------------------------
	// Slot cannot have children
	override void SpawnChildren(bool bInit = true) {}
	
	//------------------------------------------------------------------------------------------------
	IEntity SpawnAsset()
	{
		Resource resource = Resource.Load(m_rObjectToSpawn);
		if (!resource)
			return null;
		
		//--- Get slot transformation
		vector mat[4];
		GetOwner().GetWorldTransform(mat);
		
		m_pSpawnParams.TransformMode = ETransformMode.WORLD;
		//m_pSpawnParams.Transform = mat;
		
		//--- Apply rotation
		vector angles = Math3D.MatrixToAngles(m_pSpawnParams.Transform);
		Math3D.AnglesToMatrix(angles, m_pSpawnParams.Transform);
		
		//--- Spawn the prefab
		BaseResourceObject pResourceObject = resource.GetResource();
		if (!pResourceObject)
			return null;
		string resourceName = pResourceObject.GetResourceName();
		IEntity entity = GetGame().SpawnEntityPrefab(resource, GetGame().GetWorld(), m_pSpawnParams);
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
			GetOwner().GetWorldTransform(m_pSpawnParams.Transform);
		}
		else
		{
			Math.Randomize(-1);
			m_pSpawnParams = aPosOut[ Math.RandomInt(0, aPosOut.Count())];
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void GetSubSlots(out notnull array<ref EntitySpawnParams> aPosOut)
	{
		GenericEntity pSlot;
		IEntity child = GetOwner().GetChildren();
		
		while (child)	
		{
			ref EntitySpawnParams pParams = new EntitySpawnParams;
			child.GetWorldTransform(pParams.Transform);
			aPosOut.Insert(pParams);
			child = child.GetSibling();			
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		//autoptr PhysicsGeomDef geoms[] = {PhysicsGeomDef("", PhysicsGeom.CreateBox(m_size), "material/default", EPhysicsLayerDefs.Vehicle | EPhysicsLayerDefs.Character)};
		//Physics.CreateGhostEx(this.GetOwner(), geoms);
	}

				
	//------------------------------------------------------------------------------------------------
	void CP_SlotBase(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		#ifdef WORKBENCH
			m_fDebugShapeColor = ARGB(100, 0x99, 0x10, 0xF2);
		#endif
	}

	//------------------------------------------------------------------------------------------------
	void ~CP_SlotBase()
	{
	}
	
	
	
}

[BaseContainerProps()];
class SCR_WaypointSet	
{
	[Attribute("Name")]
 	string m_sName ;	
	
	[Attribute("Use random order")]
	bool m_bUseRandomOrder;
	
	[Attribute("Cycle the waypoints")]
	bool m_bCycleWaypoints;	
}
