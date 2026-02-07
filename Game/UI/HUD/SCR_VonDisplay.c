//------------------------------------------------------------------------------------------------
//! Data class for transmissions
class TransmissionData
{
	// Widgets
	ref SCR_VoNOverlay_ElementWidgets m_Widgets;
	Widget m_wBaseWidget;
	
	bool m_bForceUpdate;	// when transmission update is required
	bool m_bVisible;		// element visible, false if widget not visible
	bool m_bIsActive;		// active, false when widget starts fade out
	bool m_bIsAnimating;	// in process of fade in/out
	int m_iPlayerID;		// incoming tranmissions playerID
	float m_fAlpha;			// opacity
	float m_fActiveTimeout;	// delay before active element starts fading
	float m_fFrequency;		// current frequency
	IEntity m_Entity;		// transmitting character
	Faction m_Faction;
	BaseRadioComponent m_RadioComp;
	
	//------------------------------------------------------------------------------------------------
	// Hide transmission, skip animation 
	void HideTransmission()
	{
		m_fAlpha = 0;
		m_bIsActive = false;
		m_bIsAnimating = false;
		m_fActiveTimeout = 0;
		m_bVisible = false;
		m_wBaseWidget.SetVisible(false);
	}
	
	//------------------------------------------------------------------------------------------------
	void TransmissionData(Widget base, int iPlayerID)
	{
		m_wBaseWidget = base;
		m_wBaseWidget.SetOpacity(0);
		
		m_Widgets = new SCR_VoNOverlay_ElementWidgets();
		m_Widgets.Init(m_wBaseWidget);
		
		m_iPlayerID = iPlayerID;
	}
};

//------------------------------------------------------------------------------------------------
//! VON display of active outgoing and incoming transmissions
class SCR_VonDisplay : SCR_InfoDisplayExtended
{	
	const string IMAGESET = "{2EFEA2AF1F38E7F0}UI/Textures/Icons/icons_wrapperUI-64.imageset";
	const string IMAGESET_GLOW = "{ABC6B36856013403}UI/Textures/Icons/icons_wrapperUI-64-glow.imageset";
	
	const string ICON_DIRECT_SPEECH = "VON_directspeech";
	const string ICON_RADIO = "VON_frequency";

	
	const string VON_RECEIVING_TRANSMISSION_PATH = "{25221F619214A722}UI/layouts/HUD/VON/VoNOverlay_Element.layout";
	const string LABEL_FREQUENCY_UNITS = "#AR-VON_FrequencyUnits_MHz";
	const string LABEL_UNKNOWN_SOURCE = "#AR-VON_UnknownSource";
	const string LABEL_CHANNEL_PLATOON = "#AR-VON_ChannelPlatoon-UC";
	const string WIDGET_INCOMING = "VonIncoming";
	const string WIDGET_TRANSMIT = "VonTransmitting";

	const ref Color COLOR_WHITE = Color.FromSRGBA(255, 255, 255, 255);
	const ref Color COLOR_ORANGE = Color.FromSRGBA(226, 167, 79, 255);	
			
	const int TRANSMISSION_SLOTS = 4;			// max amount of receiving transmissions
	const float FADEOUT_TIMER_THRESHOLD = 1;
	const float FADEIN_SPEED = 10;
	const float FADEOUT_SPEED = 5;	
	
	protected bool m_bIsDirectToggled;			// direct speech toggle state
	protected bool m_bIsChannelToggled;			// channel toggle EFireState
	
	protected ref TransmissionData m_OutTransmission;												// outgoing transmission data class
	protected ref array<Widget> m_aWidgetsIncomingVON = {};											// available widgets for receiving transmission display
	protected ref array<ref TransmissionData> m_aTransmissions = {}; 								// array of existing incoming transmission data classes
	protected ref map<int, TransmissionData> m_aTransmissionMap = new map<int, TransmissionData>;	// map for lookup when transmission is received
		
	protected PlayerManager m_PlayerManager;
	protected SCR_VONController m_VONController;
	
	//------------------------------------------------------------------------------------------------
	//! VONComponent event
	event void OnCapture(BaseRadioComponent radio)
	{
		if (!m_wRoot)
			return;
		
		int frequency;
		
		if (radio)
			frequency = radio.GetFrequency();
		
		// update only when first activation / device changed / frequency changed
		if ( m_OutTransmission.m_bForceUpdate == true || m_OutTransmission.m_bIsActive == false || m_OutTransmission.m_RadioComp != radio || (radio && m_OutTransmission.m_fFrequency != radio.GetFrequency()) )
			UpdateTransmission(m_OutTransmission, radio, frequency, false);
		
		m_OutTransmission.m_bIsActive = true;
		m_OutTransmission.m_fActiveTimeout = 0;
	}

	//------------------------------------------------------------------------------------------------
	//! VONComponent event
	event void OnReceive(int playerId, BaseRadioComponent radio, int frequency, float quality, int transceiverIdx)
	{
		if (!m_wRoot)
			return;
		
		// Check if there is an active transmission from given player 		
		TransmissionData pTransmission = m_aTransmissionMap.Get(playerId);
		
		// Active transmission from the player not found, create it
		if (!pTransmission)
		{
			Widget baseWidget;
			int iWidgetIdx = 0;
			bool bWidgetAvailable = false;
			
			while (iWidgetIdx < TRANSMISSION_SLOTS)
			{
				baseWidget = m_aWidgetsIncomingVON.Get(iWidgetIdx);
				if (!baseWidget.IsVisible())
				{
					bWidgetAvailable = true;
					break;
				}
				
				iWidgetIdx++;
			}
			
			// No free widget
			if (!bWidgetAvailable)
				return;	
			
			pTransmission = new TransmissionData(baseWidget, playerId);
			m_aTransmissions.Insert(pTransmission);
			m_aTransmissionMap.Insert(playerId, pTransmission);
		}
		
		if ( pTransmission.m_bIsActive == false || pTransmission.m_RadioComp != radio || (radio && m_OutTransmission.m_fFrequency != frequency) )	// update only when first activation / device changed / frequency changed
		{	
			bool filtered = UpdateTransmission(pTransmission, radio, frequency, true);	
			if (!filtered)
			{
				pTransmission.HideTransmission();
				return;
			}
		}
		
		pTransmission.m_bIsActive = true;
		pTransmission.m_fActiveTimeout = 0;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Update transmission data
	//! \param TransmissionData is the subject
	//! \param BaseRadioComponent is the transmission entity radio component
	//! \param IsReceiving is true when receiving transmission, false when transmitting
	//! \return false if the transimission is filtered out to not be visible
	protected bool UpdateTransmission(TransmissionData data, BaseRadioComponent radioComp, int frequency, bool IsReceiving)
	{	
		data.m_RadioComp = radioComp;
		data.m_bForceUpdate = false;
		
		PlayerManager playerMgr = GetGame().GetPlayerManager();
		if (playerMgr)
		{
			data.m_Entity = playerMgr.GetPlayerControlledEntity(data.m_iPlayerID);
			if (data.m_Entity)
			{
				FactionAffiliationComponent factionComp = FactionAffiliationComponent.Cast( data.m_Entity.FindComponent( FactionAffiliationComponent ) );
				if (factionComp)	
					data.m_Faction = factionComp.GetAffiliatedFaction();
			}
		}
		
		// own faction
		Faction playerFaction;
		IEntity controlledEnt = GetGame().GetPlayerController().GetControlledEntity();
		if (controlledEnt)
		{
			FactionAffiliationComponent factionComp = FactionAffiliationComponent.Cast( controlledEnt.FindComponent( FactionAffiliationComponent ) );
			if (factionComp)	
				playerFaction = factionComp.GetAffiliatedFaction();
		}
		
		bool enemyTransmission;
		if (IsReceiving && playerFaction && playerFaction.IsFactionEnemy(data.m_Faction))	// enemy transmission case
		{
			if (!radioComp)
				return false;		// if direct ignore
			
			enemyTransmission = true;
		}

		string sDeviceIcon;
		data.m_bIsAnimating = true;		// start anim
		data.m_wBaseWidget.SetVisible(true);
		data.m_Widgets.m_wIcon.SetVisible(true);
		data.m_Widgets.m_wMicFrame.SetVisible(false);
		data.m_Widgets.m_wChannelFrame.SetVisible(false);
		data.m_Widgets.m_wFrequency.SetVisible(false);
		data.m_Widgets.m_wSeparator.SetVisible(false);
		
		// radio
		if (radioComp)
		{			
			sDeviceIcon = ICON_RADIO;
			
			float adjustedFreq;
			data.m_fFrequency = frequency;
			adjustedFreq = Math.Round(data.m_fFrequency / 10) / 100;
			data.m_Widgets.m_wFrequency.SetText(adjustedFreq.ToString() + " " + LABEL_FREQUENCY_UNITS);
			data.m_Widgets.m_wFrequency.SetVisible(true);
		}
		// direct speech
		else
		{
		 	sDeviceIcon = ICON_DIRECT_SPEECH;
		}
				
		if (sDeviceIcon != string.Empty)
		{
			data.m_Widgets.m_wIcon.LoadImageFromSet(0, IMAGESET, sDeviceIcon);
			data.m_Widgets.m_wIconBackground.LoadImageFromSet(0, IMAGESET_GLOW, sDeviceIcon);
		}			
		 
		// incoming
		if (IsReceiving)
		{
			data.m_Widgets.m_wIcon.SetColor(COLOR_WHITE);
			
			if (!enemyTransmission)
				data.m_Widgets.m_wName.SetText(m_PlayerManager.GetPlayerName(data.m_iPlayerID));
			else 
				data.m_Widgets.m_wName.SetText(LABEL_UNKNOWN_SOURCE);
			
			data.m_Widgets.m_wName.SetVisible(true);
			
			if (radioComp)
				data.m_Widgets.m_wSeparator.SetVisible(false);
		}
		else	// outgoing 
		{
			data.m_Widgets.m_wIcon.SetColor(COLOR_ORANGE);
			data.m_Widgets.m_wName.SetText(string.Empty);
			data.m_Widgets.m_wName.SetVisible(false);
						
			if (radioComp)	// direct vs radio
			{
				data.m_Widgets.m_wMicFrame.SetVisible(m_bIsChannelToggled);
				
				if (SCR_MilitaryFaction.Cast(playerFaction))	// show platoon
				{
					int factionHQFrequency = SCR_MilitaryFaction.Cast(playerFaction).GetFactionRadioFrequency();
					if (factionHQFrequency == frequency)
					{
						data.m_Widgets.m_wChannelText.SetText(LABEL_CHANNEL_PLATOON);
						data.m_Widgets.m_wChannelFrame.SetVisible(true);
					}
				}

			}
			else 
			{
				data.m_Widgets.m_wMicFrame.SetVisible(m_bIsDirectToggled);
			}
		}
		
		return true;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Fade in/out elements
	//! \param TransmissionData is the subject
	//! \param timeSlice is the OnFrame slice
	protected void OpacityFade(TransmissionData data, float timeSlice)
	{
		if (data.m_bIsActive)
		{
			data.m_wBaseWidget.SetVisible(true);
			data.m_bVisible = true;
			
			data.m_fAlpha = Math.Lerp(data.m_fAlpha, 1, FADEIN_SPEED * timeSlice);
			if (data.m_fAlpha > 0.99)
			{
				data.m_fAlpha = 1;
				data.m_bIsAnimating = false;
			}
		}
		else
		{
			data.m_fAlpha = Math.Lerp(data.m_fAlpha, 0, FADEOUT_SPEED * timeSlice);
			if (data.m_fAlpha < 0.01) 
			{
				data.HideTransmission();
			}
		}
		
		data.m_wBaseWidget.SetOpacity(data.m_fAlpha);
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_VONController event -> Toggle microphone indication
	protected void OnVONActiveToggled(bool directState, bool channelState)
	{
		m_bIsDirectToggled = directState;
		m_bIsChannelToggled = channelState;
		
		if (m_OutTransmission)
		{
			m_OutTransmission.m_bForceUpdate = true;
			OnCapture(m_OutTransmission.m_RadioComp);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_PlayerController Event
	protected void OnControlledEntityChanged(IEntity from, IEntity to)
	{
		if (m_OutTransmission)
			m_OutTransmission.HideTransmission();
		
		int count = m_aTransmissions.Count();
		for (int i = 0; i < count; i++)
		{
			m_aTransmissions[i].HideTransmission();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_PlayerController Event
	protected void OnDestroyed(IEntity killer)
	{
		if (m_OutTransmission)
			m_OutTransmission.HideTransmission();
		
		int count = m_aTransmissions.Count();
		for (int i = 0; i < count; i++)
		{
			m_aTransmissions[i].HideTransmission();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsCapturingTransmisionActive()
	{
		return m_OutTransmission != null && m_OutTransmission.m_bIsActive;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetActiveTransmissionsCount()
	{
		return m_aTransmissions.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Initialize
	protected void InitDisplay()
	{			
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		playerController.m_OnControlledEntityChanged.Insert(OnControlledEntityChanged);
		playerController.m_OnDestroyed.Insert(OnDestroyed);
		
		m_VONController = SCR_VONController.Cast(playerController.FindComponent(SCR_VONController));
		m_VONController.m_OnVONActiveToggled.Insert(OnVONActiveToggled);
		m_VONController.SetDisplay(this);	// we set this from here instead of the other way around to avoid load order issues
							
		m_OutTransmission = new TransmissionData(m_wRoot.FindAnyWidget(WIDGET_TRANSMIT), 0);
				
		Widget verticalLayout = m_wRoot.FindAnyWidget(WIDGET_INCOMING);
		
		for (int i = 0; i < TRANSMISSION_SLOTS; i++)
		{
			Widget w = GetGame().GetWorkspace().CreateWidgets(VON_RECEIVING_TRANSMISSION_PATH, verticalLayout);
			if (w)
			{
				w.SetVisible(false);
				m_aWidgetsIncomingVON.Insert(w);
			}
		}
	}
		
	//------------------------------------------------------------------------------------------------
	// Overrides
	//------------------------------------------------------------------------------------------------
	override event void DisplayUpdate(IEntity owner, float timeSlice)
	{
		// update visibility timer
		if (m_OutTransmission.m_bIsActive)
		{
			m_OutTransmission.m_fActiveTimeout += timeSlice;
			
			if (m_OutTransmission.m_fActiveTimeout > FADEOUT_TIMER_THRESHOLD)
			{
				m_OutTransmission.m_bIsActive = false;
				m_OutTransmission.m_bIsAnimating = true;
			}
		}
		
		// update fade
		if (m_OutTransmission.m_bIsAnimating)	
			OpacityFade(m_OutTransmission, timeSlice);

		// update incoming transmissions
		if (m_aTransmissions.IsEmpty())
			return;
		
		TransmissionData pTransmission;
		int count = m_aTransmissions.Count() - 1;
		
		for (int i = count; i >= 0; i--)
		{			
			pTransmission = m_aTransmissions[i];

			// update visibility timer
			if (pTransmission.m_bIsActive)
			{
				pTransmission.m_fActiveTimeout += timeSlice;
				
				if (pTransmission.m_fActiveTimeout > FADEOUT_TIMER_THRESHOLD)
				{
					pTransmission.m_bIsActive = false;
					pTransmission.m_bIsAnimating = true;
				}
			}
			
			// update fade
			if (pTransmission.m_bIsAnimating)
				OpacityFade(pTransmission, timeSlice);					
			
			// remove faded transmissions
			if (!pTransmission.m_bVisible)
			{				
				if (pTransmission.m_iPlayerID)
					m_aTransmissionMap.Remove(pTransmission.m_iPlayerID);
				
				m_aTransmissions.Remove(i);
				count - 1;
			}
		}		
	}

	//------------------------------------------------------------------------------------------------
	override bool DisplayStartDrawInit(IEntity owner)
	{
		m_PlayerManager = GetGame().GetPlayerManager();

		return m_PlayerManager != null;
	}		
	
	//------------------------------------------------------------------------------------------------
	override event void DisplayStartDraw(IEntity owner)
	{
		if (!m_wRoot)
			return;
		
		InitDisplay();
	}
	
	//------------------------------------------------------------------------------------------------
	override event void DisplayStopDraw(IEntity owner)
	{
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (playerController)
		{
			playerController.m_OnControlledEntityChanged.Remove(OnControlledEntityChanged);
			playerController.m_OnDestroyed.Remove(OnDestroyed);
		}
		
		if (m_VONController)
			m_VONController.m_OnVONActiveToggled.Remove(OnVONActiveToggled);
	}
};
