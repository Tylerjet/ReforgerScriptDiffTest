[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class SCR_ScenarioFrameworkLayerBaseClass : ScriptComponentClass
{
	// prefab properties here
};

enum SCR_EScenarioFrameworkSpawnChildrenType
{
	ALL,
	RANDOM_ONE,
	RANDOM_MULTIPLE,
	RANDOM_BASED_ON_PLAYERS_COUNT
};

class SCR_ScenarioFrameworkLayerBase : ScriptComponent
{	
	[Attribute(defvalue: "0", UIWidgets.ComboBox, desc: "Spawn all children, only random one or random multiple ones?", "", ParamEnumArray.FromEnum(SCR_EScenarioFrameworkSpawnChildrenType), category: "Children")];
	protected SCR_EScenarioFrameworkSpawnChildrenType			m_SpawnChildren;
	
	[Attribute(desc: "Faction key that corresponds with the SCR_Faction set in FactionManager", category: "Asset")]
 	protected FactionKey 		m_sFactionKey;
	
	[Attribute(defvalue: "100", desc: "If the RANDOM_MULTIPLE option is selected, what's the percentage? ", UIWidgets.Graph, "0 100 1", category: "Children")];
	protected int 							m_iRandomPercent;
	
	[Attribute(desc: "When enabled, it will repeatedly spawn childern according to other parameters set", category: "Children")];
	protected bool		m_bEnableRepeatedSpawn;
	
	[Attribute(defvalue: "-1", desc: "If Repeated Spawn is enabled, how many times can children be spawned? If set to -1, it is unlimited", category: "Children")];
	protected int 							m_iRepeatedSpawnNumber;
	
	[Attribute(defvalue: "-1", UIWidgets.Slider, desc: "If Repeated Spawn is enabled, how frequently it will spawn next wave of children? Value -1 means disabled, thus children won't be spawned by the elapsed time.", params: "-1 86400 1", category: "Children")]
	protected float 	m_fRepeatedSpawnTimer;	
		
	[Attribute(desc: "Show the debug shapes during runtime", category: "Debug")];
	protected bool							m_bShowDebugShapesDuringRuntime;
		
	[Attribute("0", uiwidget: UIWidgets.ComboBox, "", "", ParamEnumArray.FromEnum(SCR_ScenarioFrameworkEActivationType), category: "Activation")]
	protected SCR_ScenarioFrameworkEActivationType			m_eActivationType;
	
	[Attribute(UIWidgets.Auto, category: "Plugins")];
	protected ref array<ref SCR_ScenarioFrameworkPlugin> m_aPlugins;
	
	protected ref array<SCR_ScenarioFrameworkLayerBase>		m_aChildren = {};
	protected ref array<SCR_ScenarioFrameworkLayerBase>		m_aRandomlySpawnedChildren = {};
	protected ref array<SCR_ScenarioFrameworkLogic>			m_aLogic = {};
		
	protected ref array<IEntity>			m_aSpawnedEntities = {};
	protected IEntity						m_Entity;
	protected SCR_ScenarioFrameworkArea						m_Area;
	protected SCR_ScenarioFrameworkLayerBase				m_ParentLayer;
	protected float							m_fDebugShapeRadius = 0.25;	
	protected float 						m_fRepeatSpawnTimeStart;
	protected float 						m_fRepeatSpawnTimeEnd;	
	protected int							m_iDebugShapeColor = ARGB(32, 0xFF, 0x00, 0x12);
	protected bool							m_bInitiated = false;
	protected bool							m_bIsRegistered = false;
	protected bool 							m_bRepeatedSpawningSet;
	protected bool							m_bIsTerminated; //Marks if this was terminated - either by death or deletion
	
	//------------------------------------------------------------------------------------------------
	string GetName()
	{
		return GetOwner().GetName();
	}
	
	//------------------------------------------------------------------------------------------------
	void SetEntity(IEntity entity)
	{
		m_Entity = entity;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetIsTerminated()
	{
		return m_bIsTerminated;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetIsTerminated(bool state)
	{
		m_bIsTerminated = state;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetRandomlySpawnedChildren(array<string> randomlySpawnedChildren)
	{
		IEntity entity;
		SCR_ScenarioFrameworkLayerBase layer;
		m_aRandomlySpawnedChildren.Clear();
		foreach (string child : randomlySpawnedChildren)
		{
			entity = GetGame().GetWorld().FindEntityByName(child);
			if (!entity)
				continue;
			
			layer = SCR_ScenarioFrameworkLayerBase.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerBase));
			if (!layer)
				continue;
			
			m_aRandomlySpawnedChildren.Insert(layer);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	array<SCR_ScenarioFrameworkLayerBase> GetRandomlySpawnedChildren()
	{
		return m_aRandomlySpawnedChildren;
	}
	
	//------------------------------------------------------------------------------------------------
	protected int GetPlayersCount(FactionKey factionName = "")
	{
		if (factionName.IsEmpty())
			return GetGame().GetPlayerManager().GetPlayerCount();
			
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return -1;
				
		int iCnt = 0;
		array<int> aPlayerIDs = {};
		SCR_PlayerController playerController;
		GetGame().GetPlayerManager().GetPlayers(aPlayerIDs);
		foreach (int iPlayerID: aPlayerIDs)
		{
			playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(iPlayerID));
			if (!playerController)
				continue;

			if (playerController.GetLocalControlledEntityFaction() == factionManager.GetFactionByKey(factionName))
				iCnt++;
		}
		return iCnt;
	}
	
	//------------------------------------------------------------------------------------------------
	protected int GetMaxPlayersForGameMode(FactionKey factionName = "")
	{
		//TODO: separate players by faction (attackers / defenders)
		SCR_MissionHeader header = SCR_MissionHeader.Cast(GetGame().GetMissionHeader());
		
		if (!header)
			return 4;	//TODO: make a constant

		return header.m_iPlayerCount;		
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get parent area the object is nested into
	SCR_ScenarioFrameworkArea GetParentArea() 
	{ 
		SCR_ScenarioFrameworkArea layer;
		IEntity entity = GetOwner().GetParent();
		while (entity)
		{
			layer = SCR_ScenarioFrameworkArea.Cast(entity.FindComponent(SCR_ScenarioFrameworkArea));
			if (layer)
				return layer;
			
			entity = entity.GetParent();
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetFactionKey(FactionKey factionKey)
	{
		m_sFactionKey = factionKey;
	}
	
	//------------------------------------------------------------------------------------------------
	protected FactionKey GetFactionKey()
	{
		return m_sFactionKey;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetParentLayer(SCR_ScenarioFrameworkLayerBase parentLayer)
	{
		m_ParentLayer = parentLayer;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_ScenarioFrameworkLayerBase GetParentLayer()
	{
		return m_ParentLayer;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_EScenarioFrameworkSpawnChildrenType GetSpawnChildrenType()
	{
		return m_SpawnChildren;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetEnableRepeatedSpawn()
	{
		return m_bEnableRepeatedSpawn;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetEnableRepeatedSpawn(bool value)
	{
		m_bEnableRepeatedSpawn = value;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_ScenarioFrameworkEActivationType GetActivationType()
	{
		return m_eActivationType;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetActivationType(SCR_ScenarioFrameworkEActivationType activationType) 
	{ 
		m_eActivationType = activationType;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetIsInitiated()
	{
		return m_bInitiated;
	}
	
	//------------------------------------------------------------------------------------------------
	array<IEntity> GetSpawnedEntities()
	{
		return m_aSpawnedEntities;
	}
	
	//------------------------------------------------------------------------------------------------
	array<SCR_ScenarioFrameworkLayerBase> GetChildrenEntities()
	{
		return m_aChildren;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the random Slot
	SCR_ScenarioFrameworkLayerBase GetRandomChildren()
	{
		if (m_aChildren.IsEmpty())
			return null;

		Math.Randomize(-1);
		SCR_ScenarioFrameworkLayerBase selectedChild = m_aChildren[Math.RandomInt(0, m_aChildren.Count())];
		return selectedChild;
	}
	
	//------------------------------------------------------------------------------------------------
	//! 
	protected void GetChildren()
	{
		SCR_ScenarioFrameworkLayerBase slotComponent;
		IEntity child = GetOwner().GetChildren();
		while (child)	
		{
			slotComponent = SCR_ScenarioFrameworkLayerBase.Cast(child.FindComponent(SCR_ScenarioFrameworkLayerBase));
			if (slotComponent)
			{
				m_aChildren.Insert(slotComponent);
			}
			child = child.GetSibling();			
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! 
	protected void GetLogics()
	{
		IEntity child = GetOwner().GetChildren();
		SCR_ScenarioFrameworkLogic logic;
		while (child)	
		{
			//PrintFormat("Entity name: %1", child.GetName());
			logic = SCR_ScenarioFrameworkLogic.Cast(child);
			if (logic)
				m_aLogic.Insert(logic);
			child = child.GetSibling();			
		}
	}
	
	//------------------------------------------------------------------------------------------------
	array<SCR_ScenarioFrameworkLogic> GetSpawnedLogics()
	{
		return m_aLogic;
	}
	
	//------------------------------------------------------------------------------------------------
	array<ref SCR_ScenarioFrameworkPlugin> GetSpawnedPlugins()
	{
		return m_aPlugins;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetRepeatedSpawnNumber()
	{
		return m_iRepeatedSpawnNumber;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetRepeatedSpawnNumber(int number)
	{
		m_iRepeatedSpawnNumber = number;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void RepeatedSpawn()
	{
		if (!m_bEnableRepeatedSpawn || m_iRepeatedSpawnNumber <= 1)
		{
			if (!m_bShowDebugShapesDuringRuntime)
				GetOwner().ClearFlags(EntityFlags.ACTIVE);
			
			return;
		}
		
		if (m_fRepeatedSpawnTimer >= 0)
		{
			m_fRepeatSpawnTimeStart = Replication.Time();
			m_fRepeatSpawnTimeEnd = m_fRepeatSpawnTimeStart + (m_fRepeatedSpawnTimer * 1000);
			m_bRepeatedSpawningSet = true;
		}	
		
		m_iRepeatedSpawnNumber--;
		
		SpawnChildren(true);
	
	}
	
	//------------------------------------------------------------------------------------------------
	void SpawnChildren(bool bInit = true, bool previouslyRandomized = false)
	{
		if (m_SpawnChildren == SCR_EScenarioFrameworkSpawnChildrenType.ALL)
		{
			foreach (SCR_ScenarioFrameworkLayerBase child : m_aChildren)	//spawn all assets from the set
			{
				InitChild(child, bInit);
			}
		}
		else if (m_SpawnChildren == SCR_EScenarioFrameworkSpawnChildrenType.RANDOM_ONE)
		{
			SpawnRandomOneChild(bInit, previouslyRandomized);
		}
		else 
		{
			SpawnRandomMultipleChildren(bInit, previouslyRandomized);
		}
		
	}
	
	//------------------------------------------------------------------------------------------------
	void SpawnPreviouslyRandomizedChildren(bool bInit = true)
	{
		foreach (SCR_ScenarioFrameworkLayerBase child : m_aRandomlySpawnedChildren)
		{
			InitChild(child, bInit);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SpawnRandomOneChild(bool bInit = true, bool previouslyRandomized = false)
	{
		if (previouslyRandomized)
			SpawnPreviouslyRandomizedChildren(bInit);
		else
		{
			SCR_ScenarioFrameworkLayerBase child = GetRandomChildren();
			m_aRandomlySpawnedChildren.Insert(child);
			InitChild(child, bInit);
		}
	
	}
	
	//------------------------------------------------------------------------------------------------
	void SpawnRandomMultipleChildren(bool bInit = true, bool previouslyRandomized = false)
	{
		if (previouslyRandomized)
			SpawnPreviouslyRandomizedChildren(bInit);
		else
		{
			array<SCR_ScenarioFrameworkLayerBase> aRandomChildren = {};
			SCR_ScenarioFrameworkLayerBase child;
			int iCnt; 
				
			if (m_SpawnChildren == SCR_EScenarioFrameworkSpawnChildrenType.RANDOM_BASED_ON_PLAYERS_COUNT)
				m_iRandomPercent = Math.Ceil(GetPlayersCount() / GetMaxPlayersForGameMode() * 100);
						
			iCnt = Math.Round(m_aChildren.Count() / 100 * m_iRandomPercent);
				
			for (int i = 1; i <= iCnt ; i++)
			{
				child = GetRandomChildren();
				if (!aRandomChildren.Contains(child))
				{
					aRandomChildren.Insert(child);
					m_aRandomlySpawnedChildren.Insert(child);
					InitChild(child, bInit);
				}
				else
				{
					i--;
				}
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void InitChild(SCR_ScenarioFrameworkLayerBase child, bool bInit)
	{
		if (!child)
			return;
		
		if (child.GetIsInitiated())
			return;
		
		
		//if (!SCR_ScenarioFrameworkArea.Cast(this) || child.GetActivationType() == SCR_ScenarioFrameworkEActivationType.SAME_AS_PARENT)
		if (child.GetActivationType() == SCR_ScenarioFrameworkEActivationType.SAME_AS_PARENT)
		// If parent of this child is not Area, set the same activation type as the parent has
		// Only 1st level layer can have different activation type than its parent
			child.SetActivationType(m_eActivationType);			
	
				
		if (child.GetActivationType() == SCR_ScenarioFrameworkEActivationType.ON_AREA_TRIGGER_ACTIVATION)
		{
			if (bInit)
			{
				ScriptInvoker invoker;
				invoker = m_Area.GetOnAreaTriggerActivated();
				if (invoker)
					invoker.Insert(child.Init);
				return;
			}
		}
		
		child.SetParentLayer(this);
		child.Init(m_Area, m_eActivationType, bInit);
		IEntity entity = child.GetSpawnedEntity();
	 	if (entity)
			m_aSpawnedEntities.Insert(entity);
	}
	
	//------------------------------------------------------------------------------------------------
	IEntity GetSpawnedEntity()
	{
		return m_Entity;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ActivateLogic()
	{
		GetLogics();
		foreach (SCR_ScenarioFrameworkLogic logic : m_aLogic)
		{
			logic.Init();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void Init(SCR_ScenarioFrameworkArea area = null, SCR_ScenarioFrameworkEActivationType activation = SCR_ScenarioFrameworkEActivationType.SAME_AS_PARENT, bool bInit = true)
	{
		if (m_bInitiated)
			return;
		
		if (m_bIsTerminated)
		{
			if (!m_ParentLayer)
				return;
			
			SCR_ScenarioFrameworkLayerTask layerTask = SCR_ScenarioFrameworkLayerTask.Cast(m_ParentLayer);
			if (!layerTask && !GetParentArea().GetLayerTask())
				return;
		}

		if (m_eActivationType != activation)
			return;

		if (!area)
		{
			SCR_GameModeSFManager gameModeComp = SCR_GameModeSFManager.Cast(GetGame().GetGameMode().FindComponent(SCR_GameModeSFManager));
			if (gameModeComp) 
				area = gameModeComp.GetParentArea(GetOwner());
		}
		m_Area = area;
		
		bool previouslyRandomized;
		if (!m_aRandomlySpawnedChildren.IsEmpty())
			previouslyRandomized = true;

		// Handles inheritance of faction settings from parents
		if (m_sFactionKey.IsEmpty() && m_ParentLayer && !m_ParentLayer.GetFactionKey().IsEmpty())
			SetFactionKey(m_ParentLayer.GetFactionKey());
		
		GetChildren();
		
		SpawnChildren(bInit, previouslyRandomized);
		ActivateLogic();
		
		foreach(SCR_ScenarioFrameworkPlugin plugin : m_aPlugins)
		{
			plugin.Init(this);
		}
		
		m_bInitiated = true;		//used to let know this slot was already initiated (to prevent spawning objects twice)
		
		if (m_fRepeatedSpawnTimer >= 0)
		{
			m_fRepeatSpawnTimeStart = Replication.Time();
			m_fRepeatSpawnTimeEnd = m_fRepeatSpawnTimeStart + (m_fRepeatedSpawnTimer * 1000);
			m_bRepeatedSpawningSet = true;
		}	
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		super.EOnFrame(owner, timeSlice);
		if (m_bShowDebugShapesDuringRuntime)
			DrawDebugShape(true);
		
		if (m_bRepeatedSpawningSet && m_fRepeatedSpawnTimer >= 0 && (Replication.Time() >= m_fRepeatSpawnTimeEnd))
			 RepeatedSpawn();
	}
	
	
		
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		if (m_bShowDebugShapesDuringRuntime || m_fRepeatedSpawnTimer >= 0)
		{
			//TODO: deactivate once the slots are not needed (after entity was spawned)
			SetEventMask(owner, EntityEvent.INIT | EntityEvent.FRAME);	
		}
	}
	//------------------------------------------------------------------------------------------------
	void SetDebugShapeSize(float fSize)
	{
		m_fDebugShapeRadius = fSize;
	}

	//------------------------------------------------------------------------------------------------
	protected void DrawDebugShape(bool draw)
	{
		Shape dbgShape = null;
		if (!draw)
			return;
		
		dbgShape = Shape.CreateSphere(	
										m_iDebugShapeColor,
										ShapeFlags.TRANSP | ShapeFlags.DOUBLESIDE | ShapeFlags.NOZWRITE | ShapeFlags.ONCE | ShapeFlags.NOOUTLINE, 
										GetOwner().GetOrigin(), 
										m_fDebugShapeRadius 
								);
	}
	
#ifdef WORKBENCH	
	//------------------------------------------------------------------------------------------------
	override void _WB_OnCreate(IEntity owner, IEntitySource src)
	{
		GenericEntity genericEntity = GenericEntity.Cast(owner);
			
		WorldEditorAPI api = genericEntity._WB_GetEditorAPI();
			
		string uniqueName = string.Empty;
		uniqueName = api.GetEntityNiceName(api.EntityToSource(owner));
	
		api.RenameEntity(owner, uniqueName);
	}

	//------------------------------------------------------------------------------------------------
	void SCR_ScenarioFrameworkLayerBase(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_iDebugShapeColor = ARGB(32, 0x99, 0xF3, 0x12);;
		foreach(SCR_ScenarioFrameworkPlugin plugin : m_aPlugins)
		{
			plugin.OnWBKeyChanged(this);
		}
	}
#endif	
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkPlugin : ScriptAndConfig
{
	protected SCR_ScenarioFrameworkLayerBase m_Object;
	
	//------------------------------------------------------------------------------------------------
	SCR_ScenarioFrameworkLayerBase GetObject()
	{
		return m_Object;
	}
	
	//------------------------------------------------------------------------------------------------
	void Init(SCR_ScenarioFrameworkLayerBase object) 
	{
		m_Object = object;
	}
	
	//------------------------------------------------------------------------------------------------
	void OnWBKeyChanged(SCR_ScenarioFrameworkLayerBase object)  
	{
	}
};

[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkPluginTrigger: SCR_ScenarioFrameworkPlugin
{
	[Attribute(defvalue: "5.0", UIWidgets.Slider, params: "1.0 1000.0 0.5", desc: "Radius of the trigger if selected", category: "Trigger")];
	protected float						m_fAreaRadius;
	
	[Attribute("0", UIWidgets.ComboBox, "By whom the trigger is activated", "", ParamEnumArray.FromEnum(TA_EActivationPresence), category: "Trigger Activation")]
	protected TA_EActivationPresence	m_eActivationPresence;
	
	[Attribute(desc: "If SPECIFIC_CLASS is selected, fill the class name here.", category: "Trigger")]	//TODO: do array of classes
	protected string 		m_sSpecificClassName;
	
	[Attribute(desc: "If SPECIFIC_PREFAB_NAME is selected, fill the class name here.", category: "Trigger")]	//TODO: do array of classes
	protected ResourceName 	m_sSpecificPrefabName;
	
	[Attribute("", category: "Trigger Activation")]
 	protected FactionKey 		m_sActivatedByThisFaction;
	
	[Attribute(desc: "Here you can input custom trigger conditions that you can create by extending the SCR_CustomTriggerConditions", uiwidget: UIWidgets.Object)]
	protected ref array <ref SCR_CustomTriggerConditions> m_aCustomTriggerConditions;
	
	[Attribute(defvalue: "1", UIWidgets.CheckBox, desc: "Activate the trigger once or everytime the activation condition is true?", category: "Trigger")];
	protected bool		m_bOnce;
	
	[Attribute(defvalue: "0", UIWidgets.Slider, desc: "Minimum players needed to activate this trigger when PLAYER Activation presence is selected", params: "0 1 0.01", precision: 2, category: "Trigger")]
	protected float		m_fMinimumPlayersNeededPercentage;
	
	[Attribute(defvalue: "0", UIWidgets.Slider, desc: "For how long the trigger conditions must be true in order for the trigger to activate. If conditions become false, timer resets", params: "0 86400 1", category: "Trigger")]
	protected float 	m_fActivationCountdownTimer;
	
	[Attribute(defvalue: "0", UIWidgets.CheckBox, desc: "Whether or not the notification is allowed to be displayed", category: "Trigger")]
	protected bool		m_bNotificationEnabled;
	
	[Attribute(desc: "Notification title text that will be displayed when the PLAYER Activation presence is selected", category: "Trigger")]
	protected string 	m_sPlayerActivationNotificationTitle;
	
	[Attribute(defvalue: "0", UIWidgets.CheckBox, desc: "Whether or not the audio sound is played and affected by the trigger", category: "Trigger")]
	protected bool		m_bEnableAudio;
	
	[Attribute(desc: "Audio sound that will be playing when countdown is active.", category: "Trigger")]
	protected string 	m_sCountdownAudio;
	
	//------------------------------------------------------------------------------------------------
	override void Init(SCR_ScenarioFrameworkLayerBase object)
	{
		if (!object)
			return;

		super.Init(object);
		IEntity entity = object.GetSpawnedEntity();
		if (!BaseGameTriggerEntity.Cast(entity))
		{
			Print("ScenarioFramework: SlotTrigger - The selected prefab is not trigger!", LogLevel.ERROR);
			return;
		}
		BaseGameTriggerEntity.Cast(entity).SetSphereRadius(m_fAreaRadius);
		SCR_CharacterTriggerEntity trigger = SCR_CharacterTriggerEntity.Cast(entity);
		if (trigger)
		{
			trigger.SetActivationPresence(m_eActivationPresence);
			trigger.SetOwnerFaction(m_sActivatedByThisFaction);
			trigger.SetSpecificClass(m_sSpecificClassName);
			trigger.SetSpecificPrefabName(m_sSpecificPrefabName);
			trigger.SetCustomTriggerConditions(m_aCustomTriggerConditions);
			trigger.SetOnce(m_bOnce);
			trigger.SetNotificationEnabled(m_bNotificationEnabled);
			trigger.SetEnableAudio(m_bEnableAudio);
			trigger.SetMinimumPlayersNeeded(m_fMinimumPlayersNeededPercentage);
			trigger.SetPlayerActivationNotificationTitle(m_sPlayerActivationNotificationTitle);
			trigger.SetActivationCountdownTimer(m_fActivationCountdownTimer);
			trigger.SetCountdownAudio(m_sCountdownAudio);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnWBKeyChanged(SCR_ScenarioFrameworkLayerBase object) 
	{
		super.OnWBKeyChanged(object);
		object.SetDebugShapeSize(m_fAreaRadius);
		//src.Set("m_sAreaName", m_fAreaRadius);
	}
};
