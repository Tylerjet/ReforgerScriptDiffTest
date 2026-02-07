class SCR_BaseCampaignTutorialStageClass : GenericEntityClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_BaseCampaignTutorialStage : GenericEntity
{
	protected static const int DESIRED_FREQUENCY = 60000;
	
	protected float m_fTimer;
	protected float m_fConditionCheckPeriod = 0.25;
	protected float m_fDuration;
	protected float m_fDelay;
	protected float m_fWaypointCompletionRadius = 1;
	protected float m_fWaypointHeightOffset = 1;
	protected bool m_bShowWaypoint = true;
	protected bool m_bCheckWaypoint = true;
	
	protected ChimeraCharacter m_Player;
	protected IEntity m_WaypointEntity;
	protected SCR_CampaignTutorialComponent m_TutorialComponent;
	protected bool m_bFinished;
	
	//------------------------------------------------------------------------------------------------
	protected void Setup()
	{
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool GetIsFinished()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void Reset()
	{
	}
	
	//------------------------------------------------------------------------------------------------
	void OnInputDeviceChanged(bool switchedToKeyboard)
	{
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DelayedPopup(string text = "", string subtitle = "", float duration = SCR_PopUpNotification.DEFAULT_DURATION, string param1 = "", string param2 = "", string subtitleParam1 = "", string subtitleParam2 = "")
	{
		SCR_PopUpNotification.GetInstance().PopupMsg(text, duration, text2: subtitle, param1: param1, param2: param2, text2param1: subtitleParam1, text2param2: subtitleParam2, category: SCR_EPopupMsgFilter.TUTORIAL);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void RegisterWaypoint(string entityName)
	{
		m_WaypointEntity = GetGame().GetWorld().FindEntityByName(entityName);
	}
	
	//------------------------------------------------------------------------------------------------
	IEntity GetWaypoint()
	{
		return m_WaypointEntity;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetPlayer(ChimeraCharacter player)
	{
		m_Player = player;
	}
	
	//------------------------------------------------------------------------------------------------
	void OnStructureBuilt(SCR_CampaignBase base, IEntity structure)
	{
	}
	
	//------------------------------------------------------------------------------------------------
	string CreateString(string description, string keybind, string keybind2 = "")
	{
		string returnString = "<br/>";
		string startColor = "<br/><color rgba='226,168,79,200'><shadow mode='image' color='0,0,0' size='1' offset='1,1' opacity = '0.5'>";
		string endAction = "\"/>";
		string startAction = "<action name=\"";
		string endColor = "</shadow></color>"; 
		
		returnString = returnString + startColor + startAction + keybind + endAction;
		
		if (keybind2 != "")
			returnString = returnString + " | " + startAction + keybind2 + endAction;
		
		returnString = returnString + endColor + "<b> " + description + " </b>";
		
		return returnString;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsBuildingModeOpen()
	{
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		
		if (!core)
			return false;
		
		SCR_EditorManagerEntity editorManager = core.GetEditorManager();
		
		if (!editorManager)
		    return false;
		
		SCR_EditorModeEntity modeEntity = editorManager.FindModeEntity(EEditorMode.BUILDING);
		
		if (!modeEntity)
		    return false;
		
		return modeEntity.IsOpened();
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		// Update waypoint widget if enabled for this stage
		if (m_WaypointEntity && m_bShowWaypoint)
			m_TutorialComponent.UpdateWaypoint(m_WaypointEntity, m_fWaypointHeightOffset);
		
		m_fTimer += timeSlice;
		bool conditionComplete;
		bool waypointReached = true;
		
		// Check if the stage is already complete and we're just waiting for end delay timer
		if (m_bFinished)	
		{
			if (m_fTimer > m_fDelay)
			{
				m_TutorialComponent.FinishStage(this);
			}
			
			return;
		}
		
		if (!m_Player)
			return;
		
		// If duration is set up, simply check the timer
		// Otherwise periodically check the ending condition
		if (m_fDuration != 0)	
		{
			if (m_fTimer > m_fDuration)
				conditionComplete = true;
		}
		else if (m_fTimer > m_fConditionCheckPeriod)
		{
			m_fTimer = 0;
			conditionComplete = GetIsFinished();
			
			// If player is required to reach the waypoint, check its distance
			if (m_WaypointEntity && m_fWaypointCompletionRadius != 0 && m_bCheckWaypoint)
				waypointReached = vector.DistanceSq(m_Player.GetOrigin(), m_WaypointEntity.GetOrigin()) <= (Math.Pow(m_fWaypointCompletionRadius, 2));
		}
		
		// Stage was finished
		// If a delay is set up, just prepare the timer
		if (conditionComplete && waypointReached)
		{
			if (m_fDelay == 0)
			{
				m_TutorialComponent.FinishStage(this);
			}
			else
			{
				m_bFinished = true;
				m_bShowWaypoint = false;
				m_fTimer = 0;
			}
		}

	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_BaseCampaignTutorialStage(IEntitySource src, IEntity parent)
	{
		BaseGameMode gameMode = GetGame().GetGameMode();
		
		if (!gameMode)
			return;
		
		m_TutorialComponent = SCR_CampaignTutorialComponent.Cast(gameMode.FindComponent(SCR_CampaignTutorialComponent));
		
		if (!m_TutorialComponent)
			return;
		
		m_Player = m_TutorialComponent.GetPlayer();
		SetEventMask(EntityEvent.FRAME);
		Setup();
	}
};