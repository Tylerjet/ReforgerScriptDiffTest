[EntityEditorProps(category: "GameScripted/Gadgets", description: "Wristwatch gadget")]
class SCR_WristwatchComponentClass : SCR_GadgetComponentClass
{
	[Attribute("0", UIWidgets.ComboBox, "Set wristwatch type", "", ParamEnumArray.FromEnum(EWristwatchType), category: "Wristwatch")]
	int m_iWristwatchType;
	
	bool m_bSignalInit = false;
	int m_iSignalHour = -1;
	int m_iSignalMinute = -1;
	int m_iSignalSecond = -1;
	int m_iSignalDay = -1;
	
	//------------------------------------------------------------------------------------------------
	//! Cache procedural animation signals
	void InitSignals(IEntity owner)
	{	
		SignalsManagerComponent signalMgr = SignalsManagerComponent.Cast( owner.FindComponent( SignalsManagerComponent ) );
		if (!signalMgr)
			return;

		// cache signals
		m_iSignalHour = signalMgr.FindSignal("Hour");
		m_iSignalMinute = signalMgr.FindSignal("Minute");
		m_iSignalSecond = signalMgr.FindSignal("Second");
		
		if (m_iWristwatchType == EWristwatchType.VOSTOK)
			m_iSignalDay = signalMgr.FindSignal("Day");
		
		if (m_iSignalHour != -1 && m_iSignalMinute != -1 && m_iSignalSecond != -1)
			m_bSignalInit = true;
	}
};

//------------------------------------------------------------------------------------------------
//! Wristwatch type 
enum EWristwatchType
{
	SandY184A,	// US
	VOSTOK		// Soviet
};

//------------------------------------------------------------------------------------------------
class SCR_WristwatchComponent : SCR_GadgetComponent
{
	protected int m_iSeconds;
	protected int m_iMinutes;
	protected int m_iHours;
	protected int m_iDay;
	
	protected SCR_WristwatchComponentClass m_PrefabData;
	protected SignalsManagerComponent m_SignalManager;
	protected TimeAndWeatherManagerEntity m_TimeMgr;
		
	//------------------------------------------------------------------------------------------------
	void UpdateTime()
	{		
		if (!m_TimeMgr)
		{
			m_TimeMgr = GetGame().GetTimeAndWeatherManager();
			return;
		}
		
		m_TimeMgr.GetHoursMinutesSeconds(m_iHours, m_iMinutes, m_iSeconds);
		if (m_iHours >= 12)
			m_iHours -= 12;
		
		m_SignalManager.SetSignalValue(m_PrefabData.m_iSignalHour, m_iHours);
		m_SignalManager.SetSignalValue(m_PrefabData.m_iSignalMinute, m_iMinutes);
		m_SignalManager.SetSignalValue(m_PrefabData.m_iSignalSecond, m_iSeconds);
		
		m_iDay = m_TimeMgr.GetDay();
		
		m_SignalManager.SetSignalValue(m_PrefabData.m_iSignalDay, m_iDay);
	}
		
	//------------------------------------------------------------------------------------------------
	//! Get wristwatch prefab name 
	//! \return returns wristwatch prefab ResourceName
	ResourceName GetWatchPrefab()
	{		
		EntityPrefabData prefabData = GetOwner().GetPrefabData();		
		if (!prefabData)
			return string.Empty;
			
		return prefabData.GetPrefabName();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Update state of wristwatch -> active/inactive
	protected void UpdateWristwatchState()
	{
		if (m_bActivated)
			ActivateGadget();
		else 
			DeactivateGadget();
		
		if (!m_PrefabData.m_bSignalInit)
			m_PrefabData.InitSignals(GetOwner());
	}
	
	//------------------------------------------------------------------------------------------------
	//! Activate in a map UI mode 
	void SetMapMode()
	{
		m_iMode = EGadgetMode.IN_HAND;
		m_bActivated = true;
		
		m_TimeMgr = GetGame().GetTimeAndWeatherManager();
		m_PrefabData.InitSignals(GetOwner());
	}
	
	//------------------------------------------------------------------------------------------------
	override void ModeSwitch(EGadgetMode mode, IEntity charOwner)
	{
		super.ModeSwitch(mode, charOwner);
		
		if (mode == EGadgetMode.IN_HAND)
		{
			m_bActivated = true;
			UpdateWristwatchState();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void ModeClear(EGadgetMode mode)
	{				
		if (mode == EGadgetMode.IN_HAND)
		{
			m_bActivated = false;
			UpdateWristwatchState();
		}
			
		super.ModeClear(mode);
	}
	
	//------------------------------------------------------------------------------------------------
	override void ActivateAction()
	{
		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast( m_CharacterOwner.FindComponent(SCR_CharacterControllerComponent) );	
		if (controller)		
			controller.SetGadgetRaisedModeWanted(!m_bFocused); 
	}
	
	//------------------------------------------------------------------------------------------------
	override EGadgetType GetType()
	{
		return EGadgetType.WRISTWATCH;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeRaised()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		m_PrefabData = SCR_WristwatchComponentClass.Cast( GetComponentData(owner) );
		m_SignalManager = SignalsManagerComponent.Cast( owner.FindComponent( SignalsManagerComponent ) );
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{					
		UpdateTime();
	}

};