class SCR_LightManagerInfo: SCR_BaseVehicleInfo
{
	[Attribute(SCR_Enum.GetDefault(ELightType.NoLight), UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ELightType))]
	protected ELightType m_eLightType;
	
	[Attribute(SCR_Enum.GetDefault(ELightType.NoLight), UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ELightType))]
	protected ELightType m_eLightTypeSuppressing;
	
	[Attribute("-1", uiwidget: UIWidgets.ComboBox, enums: { ParamEnum("Either", "-1"), ParamEnum("Left", "0"), ParamEnum("Right", "1")})]
	protected int m_iLightSide;
	
	protected BaseLightManagerComponent m_pLightManager;
	protected BaseLightSlot m_pLightSlot;
	protected ref ScriptInvoker m_pOnLightStateChanged;
	
	protected bool m_bIsLit = true;		// Used for directional/hazard lights that blink
	
	//------------------------------------------------------------------------------------------------
	//! Can be overridden to get state of actual system or linked to an event
	override bool GetState()
	{
		// Light manager does not have events yet
		bool isSuppressed;
		if (m_eLightTypeSuppressing != ELightType.NoLight)
			isSuppressed = m_pLightManager && m_pLightManager.GetLightsState(m_eLightTypeSuppressing, m_iLightSide);
		
		return m_bIsLit && !isSuppressed && m_pLightManager && m_pLightManager.GetLightsState(m_eLightType, m_iLightSide);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool DisplayStartDrawInit(IEntity owner)
	{
		// Terminate if there is no light manager
		if (!m_pLightManager)
			return false;

		array<BaseLightSlot> outLights = new array<BaseLightSlot>;
		int iLights = m_pLightManager.GetLights(outLights);
		
		BaseLightSlot lightSlot;
		ELightType lightType;
		int lightSide;

		for (int i = 0; i < iLights; i++)
		{
			lightSlot = outLights.Get(i);
			
			lightType = lightSlot.GetLightType();
			lightSide = lightSlot.GetLightSide();
		
			if ((m_eLightType == lightType) && (m_iLightSide == -1 || m_iLightSide == lightSide))
			{
					#ifdef DEBUG_VEHICLE_UI
					PrintFormat("[%1] BaseLightSlot detected %2 | type: %3 | side: %4", i, lightSlot, lightType, lightSide);
					#endif					
				
					m_pLightSlot = lightSlot;
				
					if (SCR_LightSlot.Cast(m_pLightSlot))
						m_pOnLightStateChanged = SCR_LightSlot.Cast(m_pLightSlot).GetOnLightStateChanged();
				
					break;
			};
		}
		
		// Terminate if there is no light slot associated with the setup
		if (!m_pLightSlot)
			return false;

		// Add monitoring of light states changes
		if (m_pOnLightStateChanged)
			m_pOnLightStateChanged.Insert(OnLightStateChanged);
		
		return super.DisplayStartDrawInit(owner);
	}
	
	//------------------------------------------------------------------------------------------------
	override void DisplayStopDraw(IEntity owner)
	{
		// Remove monitoring of light states changes
		if (m_pOnLightStateChanged)
			m_pOnLightStateChanged.Remove(OnLightStateChanged);		
		
		super.DisplayStopDraw(owner);
	}	
	
	//------------------------------------------------------------------------------------------------
	void OnLightStateChanged(bool state)
	{
		m_bIsLit = state;
		
		#ifdef DEBUG_VEHICLE_UI
		PrintFormat("%1 OnLightStateChanged: %2", this, m_bIsLit);
		#endif		
	}
	
	//------------------------------------------------------------------------------------------------
	//! Init the UI, runs 1x at the start of the game
	override void DisplayInit(IEntity owner)
	{
		super.DisplayInit(owner);
		
		m_pLightManager = BaseLightManagerComponent.Cast(owner.FindComponent(BaseLightManagerComponent));
	}
};