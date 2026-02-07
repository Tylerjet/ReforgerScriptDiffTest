[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class CP_LayerBaseClass : ScriptComponentClass
{
	// prefab properties here
}

enum CP_ESpawnChildrenType
{
	ALL,
	RANDOM_ONE,
	RANDOM_MULTIPLE,
	RANDOM_BASED_ON_PLAYERS_COUNT
}

class CP_LayerBase : ScriptComponent
{	
	[Attribute(defvalue: "0", UIWidgets.ComboBox, desc: "Spawn all children, only random one or random multiple ones?", "", ParamEnumArray.FromEnum(CP_ESpawnChildrenType), category: "Children")];
	protected CP_ESpawnChildrenType			m_SpawnChildren;
	
	[Attribute(defvalue: "100", desc: "If the RANDOM_MULTIPLE option is selected, what's the percentage? ", UIWidgets.Graph, "0 100 1", category: "Children")];
	protected int 							m_iRandomPercent;
	
	[Attribute(defvalue: "0", UIWidgets.EditBox, desc: "When enabled, it will repeatedly spawn childern according to other parameters set", category: "Children")];
	protected bool		m_bEnableRepeatedSpawn;
	
	[Attribute(defvalue: "-1", desc: "If Repeated Spawn is enabled, how many times can children be spawned? If set to -1, it is unlimited", category: "Children")];
	protected int 							m_iRepeatedSpawnNumber;
	
	[Attribute(defvalue: "-1", UIWidgets.Slider, desc: "If Repeated Spawn is enabled, how frequently it will spawn next wave of children? Value -1 means disabled, thus children won't be spawned by the elapsed time.", params: "-1 86400 1", category: "Children")]
	protected float 	m_fRepeatedSpawnTimer;	
		
	[Attribute(defvalue: "0", desc: "Show the debug shapes during runtime", category: "Debug")];
	protected bool							m_bShowDebugShapesDuringRuntime;
		
	[Attribute("0", uiwidget: UIWidgets.ComboBox, "", "", ParamEnumArray.FromEnum(CP_EActivationType), category: "Activation")]
	protected CP_EActivationType			m_EActivationType;
	
	[Attribute(UIWidgets.Auto, category: "Plugins")];
	protected ref array<ref CP_Plugin> m_aPlugins;
	
	protected ref array<CP_LayerBase>		m_aChildren = {};
	protected ref array<CP_Logic>			m_aLogics = {};
		
	protected ref array<IEntity>			m_aSpawnedEntities = {};
	protected IEntity						m_pEntity;
	protected CP_Area						m_pArea;
	protected float							m_fDebugShapeRadius = 2.0;	
	protected float 						m_fRepeatSpawnTimeStart;
	protected float 						m_fRepeatSpawnTimeEnd;	
	protected int							m_fDebugShapeColor = ARGB(32, 0xFF, 0x00, 0x12);
	protected bool							m_bInitiated = false;
	protected bool							m_bIsRegistered = false;
	protected bool 							m_bRepeatedSpawningSet;

						
	//------------------------------------------------------------------------------------------------
	void SetEntity(IEntity pEnt)
	{
		m_pEntity = pEnt;
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
		SCR_PlayerController pPlayerCtrl;
		GetGame().GetPlayerManager().GetPlayers(aPlayerIDs);
		foreach (int iPlayerID: aPlayerIDs)
		{
			pPlayerCtrl = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(iPlayerID));
			if (!pPlayerCtrl)
				continue;
			if (pPlayerCtrl.GetLocalControlledEntityFaction() == factionManager.GetFactionByKey(factionName))
				iCnt++;
		}
		return iCnt;
	}
	
	//------------------------------------------------------------------------------------------------
	protected int GetMaxPlayersForGameMode(FactionKey factionName = "")
	{
		//TODO: separate players by faction (attackers / defenders)
		SCR_MissionHeader pHeader = SCR_MissionHeader.Cast(GetGame().GetMissionHeader());
		
		if (!pHeader)
			return 4;	//TODO: make a constant
		else
			return pHeader.m_iPlayerCount;		
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get parent area the object is nested into
	protected CP_Area GetParentArea() 
	{ 
		CP_Area pLayer;
		IEntity pEnt = GetOwner().GetParent();
		while (pEnt)
		{
			pLayer = CP_Area.Cast(pEnt.FindComponent(CP_Area));
			if (pLayer)
				return pLayer;
			
			pEnt = pEnt.GetParent();
		}
		
		return pLayer;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetEnableRepeatedSpawn()
	{
		return m_bEnableRepeatedSpawn;
	}
	
	//------------------------------------------------------------------------------------------------
	CP_EActivationType GetActivationType() { return m_EActivationType; }
	
	//------------------------------------------------------------------------------------------------
	void SetActivationType(CP_EActivationType activationType) 
	{ 
		m_EActivationType = activationType;
	}
	
	bool GetIsInitiated() { return m_bInitiated; }
	
	//------------------------------------------------------------------------------------------------
	array<IEntity> GetSpawnedEntities()
	{
		return m_aSpawnedEntities;
	}
	
	//------------------------------------------------------------------------------------------------
	array<CP_LayerBase> GetChildrenEntities()
	{
		return m_aChildren;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the random Slot
	CP_LayerBase GetRandomChildren()
	{
		if (m_aChildren.IsEmpty())
			return null;
		Math.Randomize(-1);
		CP_LayerBase pSelectedChild = m_aChildren[ Math.RandomInt(0, m_aChildren.Count())];
		return pSelectedChild;
	}
	
	//------------------------------------------------------------------------------------------------
	//! 
	protected void GetChildren()
	{
		CP_LayerBase pSlotComponent;
		IEntity child = GetOwner().GetChildren();
		while (child)	
		{
			pSlotComponent = CP_LayerBase.Cast(child.FindComponent(CP_LayerBase));
			if (pSlotComponent)
			{
				m_aChildren.Insert(pSlotComponent);
			}
			child = child.GetSibling();			
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! 
	protected void GetLogics()
	{
		IEntity child = GetOwner().GetChildren();
		CP_Logic pLogic;
		while (child)	
		{
			//PrintFormat("Entity name: %1", child.GetName());
			pLogic = CP_Logic.Cast(child);
			if (pLogic)
				m_aLogics.Insert(pLogic);
			child = child.GetSibling();			
		}
	}
	
	protected void RepeatedSpawn()
	{
		if (!m_bEnableRepeatedSpawn || m_iRepeatedSpawnNumber <= 0)
		{
			if (!m_bShowDebugShapesDuringRuntime)
					GetOwner().ClearFlags(EntityFlags.ACTIVE, true);
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
	void SpawnChildren(bool bInit = true)
	{
		if (m_SpawnChildren == CP_ESpawnChildrenType.ALL)
		{
			foreach (CP_LayerBase pChild : m_aChildren)	//spawn all assets from the set
				InitChild(pChild, bInit);
		}
		else if (m_SpawnChildren == CP_ESpawnChildrenType.RANDOM_ONE)
		{
			InitChild(GetRandomChildren(), bInit);
		}
		else 
		{
			array<CP_LayerBase> aRandomChildren = {};
			CP_LayerBase pChild;
			int iCnt; 
			
			if (m_SpawnChildren == CP_ESpawnChildrenType.RANDOM_BASED_ON_PLAYERS_COUNT)
				m_iRandomPercent = Math.Ceil(GetPlayersCount() / GetMaxPlayersForGameMode() * 100);
					
			iCnt = Math.Round(m_aChildren.Count() / 100 * m_iRandomPercent);
			
			for (int i = 1; i <= iCnt ; i++)
			{
				pChild = GetRandomChildren();
				if (!aRandomChildren.Contains(pChild))
				{
					aRandomChildren.Insert(pChild);
					InitChild(pChild, bInit);
				}
				else
				{
					i--;
				}
			}
		}
		
	}
	
	//------------------------------------------------------------------------------------------------
	void InitChild(CP_LayerBase pChild, bool bInit)
	{
		if (!pChild)
			return;
		
		if (pChild.GetIsInitiated())
			return;
		
		
		//if (!CP_Area.Cast(this) || pChild.GetActivationType() == CP_EActivationType.SAME_AS_PARENT)
		if (pChild.GetActivationType() == CP_EActivationType.SAME_AS_PARENT)
		// If parent of this child is not Area, set the same activation type as the parent has
		// Only 1st level layer can have different activation type than its parent
			pChild.SetActivationType(m_EActivationType);			
	
				
		if (pChild.GetActivationType() == CP_EActivationType.ON_AREA_TRIGGER_ACTIVATION)
		{
			if (bInit)
			{
				ScriptInvoker pInvoker;
				pInvoker = m_pArea.GetOnAreaTriggerActivated();
				if (pInvoker)
					pInvoker.Insert(pChild.Init);
				return;
			}
		}
	
		pChild.Init(m_pArea, m_EActivationType, bInit);
		IEntity pEnt = pChild.GetSpawnedEntity();
	 	if (pEnt)
			m_aSpawnedEntities.Insert(pEnt);
	}
	
	//------------------------------------------------------------------------------------------------
	IEntity GetSpawnedEntity() { return m_pEntity; }
	
	//------------------------------------------------------------------------------------------------
	protected void ActivateLogics()
	{
		GetLogics();
		foreach (CP_Logic pLogic : m_aLogics)
			pLogic.Init();
	}
	
	//------------------------------------------------------------------------------------------------
	void Init(CP_Area pArea = null, CP_EActivationType EActivation = CP_EActivationType.SAME_AS_PARENT, bool bInit = true)
	{
		if (m_bInitiated)
			return;
		if (m_EActivationType != EActivation)
			return;
		if (!pArea)
		{
			SCR_GameModeSFManager pGameModeComp = SCR_GameModeSFManager.Cast(GetGame().GetGameMode().FindComponent(SCR_GameModeSFManager));
			if (pGameModeComp) 
				pArea = pGameModeComp.GetParentArea(GetOwner());
		}
		m_pArea = pArea;
		GetChildren();
		SpawnChildren(bInit);
		ActivateLogics();
		
		foreach(CP_Plugin pPlugin : m_aPlugins)
			pPlugin.Init(this);
		
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
			DrawDebugShape();
		
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
			GetOwner().SetFlags(EntityFlags.ACTIVE, true);	
		}
	}
	//------------------------------------------------------------------------------------------------
	void SetDebugShapeSize(float fSize)
	{
		m_fDebugShapeRadius = fSize;
	}

	//------------------------------------------------------------------------------------------------
	protected void DrawDebugShape()
	{
		Shape dbgShape = null;
		dbgShape = Shape.CreateSphere(	
										m_fDebugShapeColor,
										ShapeFlags.TRANSP | ShapeFlags.DOUBLESIDE | ShapeFlags.NOZWRITE | ShapeFlags.ONCE | ShapeFlags.NOOUTLINE, 
										GetOwner().GetOrigin(), 
										m_fDebugShapeRadius 
								);
	}
	
#ifdef WORKBENCH	
	//------------------------------------------------------------------------------------------------
	/*
	override bool _WB_OnKeyChanged(IEntity owner, BaseContainer src, string key, BaseContainerList ownerContainers, IEntity parent)
	{
		if (SCR_Global.IsEditMode())
		{
			foreach(CP_Plugin pPlugin : m_aPlugins)
				pPlugin.OnWBKeyChanged(this);
		}
		DrawDebugShape();
		return false;
	}
	*/
	//------------------------------------------------------------------------------------------------
	override void _WB_AfterWorldUpdate(IEntity owner, float timeSlice)
	{
		if (SCR_Global.IsEditMode())
		{
			DrawDebugShape();
		}
	}
	
	
	
#endif	


	
	//------------------------------------------------------------------------------------------------
	void CP_LayerBase(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		#ifdef WORKBENCH
			m_fDebugShapeColor = ARGB(32, 0x99, 0xF3, 0x12);;
			m_fDebugShapeRadius = 1.0;
			foreach(CP_Plugin pPlugin : m_aPlugins)
				pPlugin.OnWBKeyChanged(this);			
		#endif
	}
	
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class CP_Plugin : ScriptAndConfig
{
	protected CP_LayerBase m_pObj;
	
	CP_LayerBase GetObject()
	{
		return m_pObj;
	}
	
	void Init(CP_LayerBase pObj) 
	{
		m_pObj = pObj;
	}

	void OnWBKeyChanged(CP_LayerBase pObj )  
	{
	}
}

[BaseContainerProps(), SCR_ContainerActionTitle()]
class CP_PluginTrigger: CP_Plugin
{
	[Attribute(defvalue: "5.0", UIWidgets.Slider, params: "1.0 1000.0 0.5", desc: "Radius of the trigger if selected", category: "Trigger")];
	protected float						m_fAreaRadius;
	
	[Attribute("0", UIWidgets.ComboBox, "By whom the trigger is activated", "", ParamEnumArray.FromEnum(TA_EActivationPresence), category: "Trigger Activation") ]
	protected TA_EActivationPresence	m_EActivationPresence;
	
	[Attribute(desc: "If SPECIFIC_CLASS is selected, fill the class name here.", category: "Trigger")]	//TODO: do array of classes
	protected string 		m_sSpecificClassName;
	
	[Attribute(desc: "If SPECIFIC_PREFAB_NAME is selected, fill the class name here.", category: "Trigger")]	//TODO: do array of classes
	protected ResourceName 	m_sSpecificPrefabName;
	
	[Attribute("", category: "Trigger Activation")]
 	protected FactionKey 		m_sActivatedByThisFaction;
	
	[Attribute(defvalue: "1", UIWidgets.CheckBox, desc: "Activate the trigger once or everytime the activation condition is true?", category: "Trigger")];
	protected bool		m_bOnce;
	
	[Attribute(defvalue: "0", UIWidgets.CheckBox, desc: "Whether or not the notification is allowed to be displayed", category: "Trigger")]
	protected bool		m_bNotificationEnabled;
	
	[Attribute(defvalue: "0", UIWidgets.Slider, desc: "Minimm players needed to activate this trigger when PLAYER Activation presence is selected", params: "0 1 0.01", precision: 2, category: "Trigger")]
	protected float		m_fMinimumPlayersNeededPercentage;
	
	[Attribute(desc: "Notification title text that will be displayed when the PLAYER Activation presence is selected", category: "Trigger")]
	protected string 	m_sPlayerActivationNotificationTitle;
	
	[Attribute(desc: "Notification subtitle text that will be displayed when the PLAYER Activation presence is selected", category: "Trigger")]
	protected string 	m_sPlayerActivationNotificationSubtitle;
	
	[Attribute(defvalue: "0", UIWidgets.Slider, desc: "For how long the trigger conditions must be true in order for the trigger to activate. If conditions become false, timer resets", params: "0 86400 1", category: "Trigger")]
	protected float 	m_iActivationCountdownTimer;
	
	[Attribute(desc: "Notification text that will be displayed when Activation Countdown Timer is set to value higher than 0", category: "Trigger")]
	protected string 	m_sActivationCountdownTimerNotification;
	
	[Attribute(defvalue: "0", UIWidgets.CheckBox, desc: "Whether or not the audio sound is played and affected by the trigger", category: "Trigger")]
	protected bool		m_bEnableAudio;
	
	[Attribute(desc: "Audio sound that will be playing when countdown is active.", category: "Trigger")]
	protected string 	m_sCountdownAudio;
	
	
	override void Init(CP_LayerBase pObj)
	{
		if (!pObj)
			return;
		super.Init(pObj);
		IEntity pEnt = pObj.GetSpawnedEntity();
		if (!BaseGameTriggerEntity.Cast(pEnt))
		{
			Print("CP: SlotTrigger - The selected prefab is not trigger!");
			return;
		}
		BaseGameTriggerEntity.Cast(pEnt).SetSphereRadius(m_fAreaRadius);
		SCR_CharacterTriggerEntity pTrig = SCR_CharacterTriggerEntity.Cast(pEnt);
		if (pTrig)
		{
			pTrig.SetActivationPresence(m_EActivationPresence);
			pTrig.SetOwnerFaction(m_sActivatedByThisFaction);
			pTrig.SetSpecificClass(m_sSpecificClassName);
			pTrig.SetSpecificPrefabName(m_sSpecificPrefabName);
			pTrig.SetOnce(m_bOnce);
			pTrig.SetNotificationEnabled(m_bNotificationEnabled);
			pTrig.SetEnableAudio(m_bEnableAudio);
			pTrig.SetMinimumPlayersNeeded(m_fMinimumPlayersNeededPercentage);
			pTrig.SetPlayerActivationNotificationTitle(m_sPlayerActivationNotificationTitle);
			pTrig.SetPlayerActivationNotificationSubtitle(m_sPlayerActivationNotificationSubtitle);
			pTrig.SetActivationCountdownTimer(m_iActivationCountdownTimer);
			pTrig.SetActivationCountdownTimerNotification(m_sActivationCountdownTimerNotification);
			pTrig.SetCountdownAudio(m_sCountdownAudio);
		}
	}
	
	override void OnWBKeyChanged(CP_LayerBase pObj) 
	{
		super.OnWBKeyChanged(pObj);
		pObj.SetDebugShapeSize(m_fAreaRadius);
		//src.Set("m_sAreaName", m_fAreaRadius);
	}
			
}
