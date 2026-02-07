[EntityEditorProps(category: "GameScripted/ScenarioFramework/Layer", description: "")]
class SCR_ScenarioFrameworkLayerBaseClass : ScriptComponentClass
{
}

// SCR_ScenarioFrameworkLayerBase Invoker
void ScriptInvokerScenarioFrameworkLayerMethod(SCR_ScenarioFrameworkLayerBase layer);
typedef func ScriptInvokerScenarioFrameworkLayerMethod;
typedef ScriptInvokerBase<ScriptInvokerScenarioFrameworkLayerMethod> ScriptInvokerScenarioFrameworkLayer;

enum SCR_EScenarioFrameworkSpawnChildrenType
{
	ALL,
	RANDOM_ONE,
	RANDOM_MULTIPLE,
	RANDOM_BASED_ON_PLAYERS_COUNT
}

class SCR_ScenarioFrameworkLayerBase : ScriptComponent
{
	[Attribute(defvalue: "0", UIWidgets.ComboBox, desc: "Spawn all children, only random one or random multiple ones?", "", ParamEnumArray.FromEnum(SCR_EScenarioFrameworkSpawnChildrenType), category: "Children")]
	SCR_EScenarioFrameworkSpawnChildrenType	m_SpawnChildren;

	[Attribute(desc: "Faction key that corresponds with the SCR_Faction set in FactionManager", category: "Asset")]
	FactionKey m_sFactionKey;

	[Attribute(defvalue: "100", desc: "If the RANDOM_MULTIPLE option is selected, what's the percentage? ", UIWidgets.Graph, "0 100 1", category: "Children")]
	int m_iRandomPercent;

	[Attribute(desc: "When enabled, it will repeatedly spawn childern according to other parameters set", category: "Children")]
	bool m_bEnableRepeatedSpawn;

	[Attribute(defvalue: "-1", desc: "If Repeated Spawn is enabled, how many times can children be spawned? If set to -1, it is unlimited", params: "-1 inf 1", category: "Children")]
	int m_iRepeatedSpawnNumber;

	[Attribute(defvalue: "-1", UIWidgets.Slider, desc: "If Repeated Spawn is enabled, how frequently it will spawn next wave of children? Value -1 means disabled, thus children won't be spawned by the elapsed time.", params: "-1 86400 1", category: "Children")]
	float m_fRepeatedSpawnTimer;

	[Attribute(desc: "Show the debug shapes during runtime", category: "Debug")]
	bool m_bShowDebugShapesDuringRuntime;

	[Attribute("0", uiwidget: UIWidgets.ComboBox, "", "", ParamEnumArray.FromEnum(SCR_ScenarioFrameworkEActivationType), category: "Activation")]
	SCR_ScenarioFrameworkEActivationType m_eActivationType;

	[Attribute(desc: "Conditions that will be checked upon init and based on the result it will let this to finish init or not", category: "Activation")]
	ref array<ref SCR_ScenarioFrameworkActivationConditionBase> m_aActivationConditions;

	[Attribute(desc: "Actions that will be activated when this gets initialized", category: "OnInit")]
	ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActivationActions;

	[Attribute(desc: "", category: "Activation")]
	bool m_bExcludeFromDynamicDespawn;

	[Attribute(UIWidgets.Auto, category: "Plugins")]
	ref array<ref SCR_ScenarioFrameworkPlugin> m_aPlugins;

	ref array<SCR_ScenarioFrameworkLayerBase> m_aChildren = {};
	ref array<SCR_ScenarioFrameworkLayerBase> m_aRandomlySpawnedChildren = {};
	ref array<SCR_ScenarioFrameworkLogic> m_aLogic = {};

	ref ScriptInvokerBase<ScriptInvokerScenarioFrameworkLayerMethod> m_OnAllChildrenSpawned;
	ref array<IEntity> m_aSpawnedEntities = {};
	IEntity	m_Entity;
	SCR_ScenarioFrameworkArea m_Area;
	SCR_ScenarioFrameworkLayerBase m_ParentLayer;
	float m_fDebugShapeRadius = 0.25;
	WorldTimestamp m_fRepeatSpawnTimeStart;
	WorldTimestamp	m_fRepeatSpawnTimeEnd;
	int	m_iDebugShapeColor = ARGB(32, 0xFF, 0x00, 0x12);
	int m_iCurrentlySpawnedChildren;
	int m_iSupposedSpawnedChildren;
	bool m_bInitiated;
	bool m_bIsRegistered;
	bool m_bDynamicallyDespawned;
	bool m_bIsTerminated; //Marks if this was terminated - either by death or deletion
	
	//Default values we need to store
	int m_iRepeatedSpawnNumberDefault = m_iRepeatedSpawnNumber;
	SCR_ScenarioFrameworkEActivationType m_eActivationTypeDefault = m_eActivationType;

	static const int SPAWN_DELAY = 200;

	//------------------------------------------------------------------------------------------------
	//! \return
	string GetName()
	{
		return GetOwner().GetName();
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] entity
	void SetEntity(IEntity entity)
	{
		m_Entity = entity;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	bool GetIsTerminated()
	{
		return m_bIsTerminated;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] state
	void SetIsTerminated(bool state)
	{
		m_bIsTerminated = state;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] randomlySpawnedChildren
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
	//! \return
	array<SCR_ScenarioFrameworkLayerBase> GetRandomlySpawnedChildren()
	{
		return m_aRandomlySpawnedChildren;
	}

	//------------------------------------------------------------------------------------------------
	int GetPlayersCount(FactionKey factionName = "")
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
		foreach (int iPlayerID : aPlayerIDs)
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
	int GetMaxPlayersForGameMode(FactionKey factionName = "")
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
		if (m_Area)
			return m_Area;

		IEntity entity = GetOwner().GetParent();
		while (entity)
		{
			m_Area = SCR_ScenarioFrameworkArea.Cast(entity.FindComponent(SCR_ScenarioFrameworkArea));
			if (m_Area)
				return m_Area;

			entity = entity.GetParent();
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Get layer task the object is nested into if there is some
	SCR_ScenarioFrameworkLayerTask GetLayerTask()
	{
		SCR_ScenarioFrameworkLayerTask layer;
		IEntity entity = GetOwner().GetParent();
		while (entity)
		{
			layer = SCR_ScenarioFrameworkLayerTask.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerTask));
			if (layer)
				return layer;

			entity = entity.GetParent();
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Get SlotTask from array of LayerBases if there is any
	SCR_ScenarioFrameworkSlotTask GetSlotTask(array<SCR_ScenarioFrameworkLayerBase> aLayers)
	{
		SCR_ScenarioFrameworkSlotTask slotTask;
		foreach (SCR_ScenarioFrameworkLayerBase layer : aLayers)
		{
			IEntity child = layer.GetOwner();
			slotTask = SCR_ScenarioFrameworkSlotTask.Cast(child.FindComponent(SCR_ScenarioFrameworkSlotTask));
			if (slotTask)
				return slotTask;

			child = GetOwner().GetChildren();
			while (child)
			{
				slotTask = SCR_ScenarioFrameworkSlotTask.Cast(child.FindComponent(SCR_ScenarioFrameworkSlotTask));
				if (slotTask)
					return slotTask;

				child = child.GetSibling();
			}
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
	//! \param[in] parentLayer
	void SetParentLayer(SCR_ScenarioFrameworkLayerBase parentLayer)
	{
		m_ParentLayer = parentLayer;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_ScenarioFrameworkLayerBase GetParentLayer()
	{
		if (m_ParentLayer)
			return m_ParentLayer;

		IEntity entity = GetOwner().GetParent();
		if (!entity)
			return null;

		m_ParentLayer = SCR_ScenarioFrameworkLayerBase.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerBase));
		return m_ParentLayer;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_EScenarioFrameworkSpawnChildrenType GetSpawnChildrenType()
	{
		return m_SpawnChildren;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	bool GetEnableRepeatedSpawn()
	{
		return m_bEnableRepeatedSpawn;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] value
	void SetEnableRepeatedSpawn(bool value)
	{
		m_bEnableRepeatedSpawn = value;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_ScenarioFrameworkEActivationType GetActivationType()
	{
		return m_eActivationType;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] activationType
	void SetActivationType(SCR_ScenarioFrameworkEActivationType activationType)
	{
		m_eActivationType = activationType;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	bool GetIsInitiated()
	{
		return m_bInitiated;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	bool GetDynamicDespawnExcluded()
	{
		return m_bExcludeFromDynamicDespawn;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] excluded
	void SetDynamicDespawnExcluded(bool excluded)
	{
		m_bExcludeFromDynamicDespawn = excluded;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	array<IEntity> GetSpawnedEntities()
	{
		return m_aSpawnedEntities;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
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
		return m_aChildren.GetRandomElement();
	}

	//------------------------------------------------------------------------------------------------
	//! Goes through the hierarchy and returns all the child entities of LayerBase type
	//! \param[out] children
	void GetChildren(out array<SCR_ScenarioFrameworkLayerBase> children)
	{
		children = {};
		array<SCR_ScenarioFrameworkLayerBase> childrenReversed = {};
		SCR_ScenarioFrameworkLayerBase slotComponent;
		IEntity child = GetOwner().GetChildren();
		while (child)
		{
			slotComponent = SCR_ScenarioFrameworkLayerBase.Cast(child.FindComponent(SCR_ScenarioFrameworkLayerBase));
			if (slotComponent)
				childrenReversed.Insert(slotComponent);
			
			child = child.GetSibling();
		}
		
		for (int i = childrenReversed.Count() - 1; i >= 0; i--)
		{
			if (!children.Contains(childrenReversed[i]))
				children.Insert(childrenReversed[i]);
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	void GetLogics(out array<SCR_ScenarioFrameworkLogic> logics)
	{
		IEntity child = GetOwner().GetChildren();
		SCR_ScenarioFrameworkLogic logic;
		while (child)
		{
			logic = SCR_ScenarioFrameworkLogic.Cast(child);
			if (logic && !logics.Contains(logic))
				logics.Insert(logic);

			child = child.GetSibling();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	array<SCR_ScenarioFrameworkLogic> GetSpawnedLogics()
	{
		return m_aLogic;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	array<ref SCR_ScenarioFrameworkPlugin> GetSpawnedPlugins()
	{
		return m_aPlugins;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	int GetRepeatedSpawnNumber()
	{
		return m_iRepeatedSpawnNumber;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] number
	void SetRepeatedSpawnNumber(int number)
	{
		m_iRepeatedSpawnNumber = number;
	}

	//------------------------------------------------------------------------------------------------
	protected void RepeatedSpawn()
	{
		if (!m_bEnableRepeatedSpawn || (m_iRepeatedSpawnNumber != -1 && m_iRepeatedSpawnNumber <= 0))
			return;

		//This calls the RepeatedSpawnCalled with set delay and is set in a way that it
		//Can be both queued that way or called manually from different place (pseudo-looped CallLater)
		GetGame().GetCallqueue().CallLater(RepeatedSpawnCalled, 1000 * m_fRepeatedSpawnTimer);
	}

	//------------------------------------------------------------------------------------------------
	protected void RepeatedSpawnCalled()
	{
		if (m_iRepeatedSpawnNumber != -1)
			m_iRepeatedSpawnNumber--;
		
		SpawnChildren(true);

		if (!m_bEnableRepeatedSpawn || (m_iRepeatedSpawnNumber != -1 && m_iRepeatedSpawnNumber <= 0))
			return;

		RepeatedSpawn();
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	ScriptInvokerScenarioFrameworkLayer GetOnAllChildrenSpawned()
	{
		if (!m_OnAllChildrenSpawned)
			m_OnAllChildrenSpawned = new ScriptInvokerBase<ScriptInvokerScenarioFrameworkLayerMethod>();

		return m_OnAllChildrenSpawned;
	}

	//------------------------------------------------------------------------------------------------
	//!
	void InvokeAllChildrenSpawned()
	{
		if (m_OnAllChildrenSpawned)
			m_OnAllChildrenSpawned.Invoke(this);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] layer
	void CheckAllChildrenSpawned(SCR_ScenarioFrameworkLayerBase layer = null)
	{
		if (m_SpawnChildren == SCR_EScenarioFrameworkSpawnChildrenType.ALL)
		{
			m_iCurrentlySpawnedChildren = 0;
			m_iSupposedSpawnedChildren = 0;
			foreach (SCR_ScenarioFrameworkLayerBase child : m_aChildren)
			{
				int activationType = child.GetActivationType();
				if (activationType == SCR_ScenarioFrameworkEActivationType.ON_TRIGGER_ACTIVATION || activationType == SCR_ScenarioFrameworkEActivationType.ON_AREA_TRIGGER_ACTIVATION
					|| activationType == SCR_ScenarioFrameworkEActivationType.ON_TASKS_INIT)
					continue;

				if (child.GetIsInitiated())
					m_iCurrentlySpawnedChildren++;

				m_iSupposedSpawnedChildren++;
			}

			if (m_iCurrentlySpawnedChildren == m_iSupposedSpawnedChildren)
				InvokeAllChildrenSpawned();
		}
		else if (m_SpawnChildren == SCR_EScenarioFrameworkSpawnChildrenType.RANDOM_ONE)
		{
			InvokeAllChildrenSpawned();
		}
		else
		{
			m_iCurrentlySpawnedChildren = 0;
			foreach (SCR_ScenarioFrameworkLayerBase child : m_aChildren)
			{
				int activationType = child.GetActivationType();
				if (activationType == SCR_ScenarioFrameworkEActivationType.ON_TRIGGER_ACTIVATION || activationType == SCR_ScenarioFrameworkEActivationType.ON_AREA_TRIGGER_ACTIVATION)
					continue;

				if (child.GetIsInitiated())
					m_iCurrentlySpawnedChildren++;
			}

			if (m_iCurrentlySpawnedChildren == m_iSupposedSpawnedChildren)
				InvokeAllChildrenSpawned();
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] previouslyRandomized
	void SpawnChildren(bool previouslyRandomized = false)
	{
		if (m_aChildren.IsEmpty())
		{
			InvokeAllChildrenSpawned();
			return;
		}
		
		if (!previouslyRandomized && !m_aRandomlySpawnedChildren.IsEmpty())
			previouslyRandomized = true;

		if (m_SpawnChildren == SCR_EScenarioFrameworkSpawnChildrenType.ALL)
		{
			int slotCount;
			foreach (SCR_ScenarioFrameworkLayerBase child : m_aChildren)
			{
				if (SCR_ScenarioFrameworkSlotBase.Cast(child))
				{
					GetGame().GetCallqueue().CallLater(InitChild, SPAWN_DELAY * slotCount, false, child);
					slotCount++;
				}
				else
				{
					InitChild(child);
				}
			}
		}
		else if (m_SpawnChildren == SCR_EScenarioFrameworkSpawnChildrenType.RANDOM_ONE)
		{
			//We need to introduce slight delay for the randomization by time seed to occur
			GetGame().GetCallqueue().CallLater(SpawnRandomOneChild, Math.RandomInt(200, 400), false, previouslyRandomized);
		}
		else
		{
			//We need to introduce slight delay for the randomization by time seed to occur
			GetGame().GetCallqueue().CallLater(SpawnRandomMultipleChildren, Math.RandomInt(200, 400), false, previouslyRandomized);
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	void SpawnPreviouslyRandomizedChildren()
	{
		foreach (int i, SCR_ScenarioFrameworkLayerBase child : m_aRandomlySpawnedChildren)
		{
			GetGame().GetCallqueue().CallLater(InitChild, 200 * i, false, child);
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] previouslyRandomized
	void SpawnRandomOneChild(bool previouslyRandomized = false)
	{
		if (previouslyRandomized)
			SpawnPreviouslyRandomizedChildren();
		else
		{
			SCR_ScenarioFrameworkLayerBase child = GetRandomChildren();
			m_aRandomlySpawnedChildren.Insert(child);
			InitChild(child);
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] previouslyRandomized
	void SpawnRandomMultipleChildren(bool previouslyRandomized = false)
	{
		if (previouslyRandomized)
			SpawnPreviouslyRandomizedChildren();
		else
		{
			array<SCR_ScenarioFrameworkLayerBase> aChildren = {};
			aChildren.Copy(m_aChildren);

			if (aChildren.IsEmpty())
				return;

			if (m_SpawnChildren == SCR_EScenarioFrameworkSpawnChildrenType.RANDOM_BASED_ON_PLAYERS_COUNT)
				m_iRandomPercent = Math.Ceil(GetPlayersCount() / GetMaxPlayersForGameMode() * 100);

			m_iSupposedSpawnedChildren = Math.Round(m_aChildren.Count() / 100 * m_iRandomPercent);
			SCR_ScenarioFrameworkLayerBase child;
			for (int i = 1; i <= m_iSupposedSpawnedChildren; i++)
			{
				if (aChildren.IsEmpty())
					continue;

				Math.Randomize(-1);
				child = aChildren.GetRandomElement();
				m_aRandomlySpawnedChildren.Insert(child);
				InitChild(child);
				aChildren.RemoveItem(child);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] child
	void InitChild(SCR_ScenarioFrameworkLayerBase child)
	{
		if (!child)
			return;

		child.SetParentLayer(this);
		child.Init(GetParentArea(), SCR_ScenarioFrameworkEActivationType.SAME_AS_PARENT);
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	IEntity GetSpawnedEntity()
	{
		return m_Entity;
	}

	//------------------------------------------------------------------------------------------------
	protected void ActivateLogic()
	{
		GetLogics(m_aLogic);
		foreach (SCR_ScenarioFrameworkLogic logic : m_aLogic)
		{
			logic.Init();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void RestoreToDefault(bool includeChildren = false, bool reinitAfterRestoration = false)
	{
		m_Entity = null;
		m_iRepeatedSpawnNumber = m_iRepeatedSpawnNumberDefault;
		m_eActivationType = m_eActivationTypeDefault;
		m_bInitiated = false;
		m_bIsRegistered = false;
		m_bDynamicallyDespawned = false;
		m_bIsTerminated = false;
		
		foreach (SCR_ScenarioFrameworkActionBase activationAction : m_aActivationActions)
		{
			activationAction.m_iNumberOfActivations = 0;
		}
		
		if (includeChildren)
		{
			foreach (SCR_ScenarioFrameworkLayerBase child : m_aChildren)
			{
				child.RestoreToDefault(includeChildren, false);
			}
		}

		m_aChildren.Clear();

		foreach (IEntity entity : m_aSpawnedEntities)
		{
			SCR_EntityHelper.DeleteEntityAndChildren(entity);
		}
		
		m_aSpawnedEntities.Clear();
		
		if (reinitAfterRestoration)
			Init(m_Area);
	}

	//------------------------------------------------------------------------------------------------
	//!
	void DynamicDespawn(SCR_ScenarioFrameworkLayerBase layer)
	{
		GetOnAllChildrenSpawned().Remove(DynamicDespawn);
		if (!m_bInitiated)
		{
			GetOnAllChildrenSpawned().Insert(DynamicDespawn);
			return;
		}
		
		if (m_bExcludeFromDynamicDespawn)
			return;

		m_bInitiated = false;
		m_bDynamicallyDespawned = true;
		m_aChildren.RemoveItem(null);
		foreach (SCR_ScenarioFrameworkLayerBase child : m_aChildren)
		{
			child.DynamicDespawn(this);
		}

		m_aChildren.Clear();

		foreach (IEntity entity : m_aSpawnedEntities)
		{
			SCR_EntityHelper.DeleteEntityAndChildren(entity);
		}

		m_aSpawnedEntities.Clear();
	}

	//------------------------------------------------------------------------------------------------
	//!
	void DynamicReinit()
	{
		Init(GetParentArea(), SCR_ScenarioFrameworkEActivationType.SAME_AS_PARENT);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	bool InitAlreadyHappened()
	{
		return m_bInitiated;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	bool InitParentLayer()
	{
		if (m_ParentLayer)
			return true;
		
		m_ParentLayer = GetParentLayer();
		return (m_ParentLayer != null);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	bool InitNotTerminated()
	{
		if (!m_bIsTerminated)
			return true;
		
		if (m_ParentLayer)
			m_ParentLayer.CheckAllChildrenSpawned(this);
			
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	bool InitDynDespawnAndActivation(SCR_ScenarioFrameworkEActivationType activation)
	{
		if (!m_bDynamicallyDespawned && activation != m_eActivationType)
		{
			if (m_ParentLayer)
				m_ParentLayer.CheckAllChildrenSpawned(this);
			
			return false;
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	bool InitActivationConditions()
	{
		IEntity owner = GetOwner();
		foreach (SCR_ScenarioFrameworkActivationConditionBase activationCondition : m_aActivationConditions)
		{
			//If just one condition is false, we don't continue and interrupt the init
			if (!activationCondition.Init(owner))
			{
				if (m_ParentLayer)
					m_ParentLayer.CheckAllChildrenSpawned(this);
			
				return false;
			}
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	bool InitArea(SCR_ScenarioFrameworkArea area)
	{
		if (area)
		{
			m_Area = area;
			return true;		
		}
	
		m_Area = GetParentArea();
		if (!m_Area)
		{
			if (m_ParentLayer)
				m_ParentLayer.CheckAllChildrenSpawned(this);
			
			return false;
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Handles inheritance of faction settings from parents
	void InitFactionSettings()
	{
		if (!m_ParentLayer)
			return;
		
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(m_sFactionKey))
		{
			FactionKey parentFactionKey = m_ParentLayer.GetFactionKey();
			if (!SCR_StringHelper.IsEmptyOrWhiteSpace(parentFactionKey))
				SetFactionKey(parentFactionKey);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! For situations where some other logic is to be appended in these checks and is to be performed before FinishInit
	bool InitOtherThings()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! For situations where some other logic is needed to be performed before or after this Insert
	void FinishInitChildrenInsert()
	{
		GetOnAllChildrenSpawned().Insert(AfterAllChildrenSpawned);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void FinishInit()
	{
		FinishInitChildrenInsert();
		InitFactionSettings();
		GetChildren(m_aChildren);
		SpawnChildren();
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] area
	//! \param[in] activation
	void Init(SCR_ScenarioFrameworkArea area = null, SCR_ScenarioFrameworkEActivationType activation = SCR_ScenarioFrameworkEActivationType.SAME_AS_PARENT)
	{
		if (InitAlreadyHappened())
			return;
		
		if (!InitParentLayer())
			return;

		if (!InitNotTerminated())
			return;

		if (!InitDynDespawnAndActivation(activation))
			return;
		
		if (!InitActivationConditions())
			return;

		if (!InitArea(area))
			return;
		
		if (!InitOtherThings())
			return;
		
		FinishInit();
	}

	//------------------------------------------------------------------------------------------------
	//!
	void AfterAllChildrenSpawned(SCR_ScenarioFrameworkLayerBase layer)
	{
		m_bInitiated = true;

		ActivateLogic();
		foreach (SCR_ScenarioFrameworkPlugin plugin : m_aPlugins)
		{
			plugin.Init(this);
		}

		foreach (SCR_ScenarioFrameworkActionBase activationAction : m_aActivationActions)
		{
			activationAction.Init(GetOwner());
		}

		if (m_ParentLayer)
			m_ParentLayer.CheckAllChildrenSpawned(this);

		GetOnAllChildrenSpawned().Remove(AfterAllChildrenSpawned);

		if (m_fRepeatedSpawnTimer >= 0)
			RepeatedSpawn();
	}

	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		super.EOnFrame(owner, timeSlice);
		
		if (m_bShowDebugShapesDuringRuntime)
			DrawDebugShape(true);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		if (m_bShowDebugShapesDuringRuntime || m_fRepeatedSpawnTimer >= 0)
		{
			//TODO: deactivate once the slots are not needed (after entity was spawned)
			SetEventMask(owner, EntityEvent.INIT | EntityEvent.FRAME);
			
			if (SCR_Global.IsEditMode())
				return;
			
			BaseGameMode gameMode = GetGame().GetGameMode();
			if (!gameMode)
				return;
			
			SCR_GameModeSFManager gameModeComp = SCR_GameModeSFManager.Cast(gameMode.FindComponent(SCR_GameModeSFManager));
			if (!gameModeComp)
				return;
			
			gameModeComp.ManageLayerDebugShape(GetOwner().GetID(), m_bShowDebugShapesDuringRuntime, m_fDebugShapeRadius, true);
		}
	}
	//------------------------------------------------------------------------------------------------
	//! \param[in] fSize
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
		RenameOwnerEntity(owner);

		IEntity child = GetOwner().GetChildren();
		while (child)
		{
			RenameOwnerEntity(child);
			child = child.GetSibling();
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] owner
	void RenameOwnerEntity(IEntity owner)
	{
		GenericEntity genericEntity = GenericEntity.Cast(owner);

		WorldEditorAPI api = genericEntity._WB_GetEditorAPI();
		if (!api.UndoOrRedoIsRestoring())
			api.RenameEntity(api.EntityToSource(owner), api.GenerateDefaultEntityName(api.EntityToSource(owner)));
	}
#endif

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_ScenarioFrameworkLayerBase(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_iDebugShapeColor = ARGB(32, 0x99, 0xF3, 0x12);
#ifdef WORKBENCH
		foreach (SCR_ScenarioFrameworkPlugin plugin : m_aPlugins)
		{
			plugin.OnWBKeyChanged(this);
		}
#endif
	}
	
	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_ScenarioFrameworkLayerBase()
	{
		if (SCR_Global.IsEditMode())
			return;
		
		DynamicDespawn(this);
	}
}