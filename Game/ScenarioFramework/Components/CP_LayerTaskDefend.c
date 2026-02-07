[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class CP_LayerTaskDefendClass : CP_LayerTaskClass
{
	// prefab properties here
}

class CP_LayerTaskDefend : CP_LayerTask
{
	[Attribute(defvalue: "", desc: "Will use trigger that is named for Defend params calculations", category: "Task")]
	protected string 											m_sTriggerName;
	
	[Attribute(defvalue: "", desc: "Text that will be displayed above the countdown number", category: "Defend params")]
	protected string 											m_sCountdownTitleText;
	
	[Attribute(defvalue: "-1", UIWidgets.Slider, desc: "For how long you have to Defend the objective of the task. Value -1 is for indefinitely.", params: "-1 86400 1", category: "Defend params")]
	protected float 											m_fDefendTime;
	
	[Attribute(defvalue: "1", UIWidgets.EditBox, desc: "When enabled, it will display the text and how much time remains for the Task Defend", category: "Defend params")];
	protected bool 												m_bDisplayCountdownHUD;
	
	[Attribute(defvalue: "{47864BB47AB0B1F4}UI/layouts/HUD/CampaignMP/CampaignMainHUD.layout", category:  "Defend params")]
	protected ResourceName 										m_sCountdownHUD;	
	
	[Attribute("", category: "Defend params")]
	protected ref array<ref CP_TaskDefendFactionSettings> 		m_aFactionSettings;
	
	[Attribute(defvalue: "0", UIWidgets.Slider, desc: "When compared to the number of attackers, minimum of how much of the characters present in the task area must be from defending side to successfully complete the objective on evaluation", params: "0 1 0.01", precision: 2, category: "Defend params")]
	protected float												m_fMinDefenderPercentageRatio;
	
	protected SCR_CharacterTriggerEntity m_characterTriggerEntity;
	
	protected float m_fTempCountdown = m_fDefendTime;
	
	protected string m_sFormattedCountdownTitle = string.Format(WidgetManager.Translate("<color rgba=\"226,168,80,255\">%1</color>", m_sCountdownTitleText));
	
	protected float m_fTempTimeSlice;
	
	protected bool m_bTaskEvaluated;
	protected bool 	m_bEvaluationSet;
	protected float m_fEvaluateTimeStart;
	protected float m_fEvaluateTimeEnd;	
	

	protected Widget m_wRoot;
	protected Widget m_wInfoOverlay;
	protected Widget m_wCountdownOverlay;
	protected Widget m_wCountdownOverlayMap;
	protected ImageWidget m_wLeftFlag;
	protected ImageWidget m_wRightFlag;
	protected ImageWidget m_wWinScoreSideLeft;
	protected ImageWidget m_wWinScoreSideRight;
	protected RichTextWidget m_wLeftScore;
	protected RichTextWidget m_wRightScore;
	protected RichTextWidget m_wWinScore;
	protected RichTextWidget m_wCountdown;
	protected RichTextWidget m_wFlavour;
	
	//------------------------------------------------------------------------------------------------
	SCR_CharacterTriggerEntity GetCharacterTriggerEntity()
	{
		return m_characterTriggerEntity;
	}
	
	//------------------------------------------------------------------------------------------------
	void FindCharacterTriggerEntity()
	{
		IEntity foundEntity = GetGame().GetWorld().FindEntityByName(m_sTriggerName);
		if (!foundEntity)
			return;
		
		CP_SlotTrigger slotTrigger = CP_SlotTrigger.Cast(foundEntity.FindComponent(CP_SlotTrigger));
		if (!slotTrigger)
			return;
		
		SCR_CharacterTriggerEntity trigger = SCR_CharacterTriggerEntity.Cast(slotTrigger.GetSpawnedEntity());
		if (!trigger)
			return;
		
		m_characterTriggerEntity = trigger;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool SetSupportEntity()
	{
		if (!GetTaskManager().FindSupportEntity(SCR_CP_TaskDefendSupportEntity))
		{
			Print("CP: Task Defend support entity not found in the world, task won't be created!", LogLevel.ERROR);
			return false;
		}
		m_pSupportEntity = SCR_CP_TaskDefendSupportEntity.Cast(GetTaskManager().FindSupportEntity(SCR_CP_TaskDefendSupportEntity));
		return m_pSupportEntity != null;	
	}
	
	//------------------------------------------------------------------------------------------------
	void CP_LayerTaskDefend(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_eTypeOfTask = ESFTaskType.DEFEND;
	}
	
	//------------------------------------------------------------------------------------------------
	void EvaluateStatus()
	{
		m_bTaskEvaluated = true;
		
		if (!m_pTask)
		{
			if (!m_bShowDebugShapesDuringRuntime)
				GetOwner().ClearFlags(EntityFlags.ACTIVE, true);
			
			UpdateHUD();
			return;
		}
		
		if (m_pTask.GetTaskState() == SCR_TaskState.CANCELLED || m_pTask.GetTaskState() ==  SCR_TaskState.FINISHED)
		{
			if (!m_bShowDebugShapesDuringRuntime)
				GetOwner().ClearFlags(EntityFlags.ACTIVE, true);
			
			UpdateHUD();
			return;
		}
		
		if (!m_characterTriggerEntity && !m_sTriggerName.IsEmpty())
			FindCharacterTriggerEntity();
		
		if (m_characterTriggerEntity)
		{
			int defenderCount = 0;
			int attackerCount = 0;
			
			int factionSettingsCount = m_aFactionSettings.Count();
			for (int i = 0; i < factionSettingsCount; i++)
			{
				CP_TaskDefendDefendingFaction defender = CP_TaskDefendDefendingFaction.Cast(m_aFactionSettings[i]);
				if (defender)
				{
					if (defender.GetCountOnlyPlayers())
						defenderCount += m_characterTriggerEntity.GetPlayersCountByFactionInsideTrigger(defender.GetFaction());
					else
						defenderCount += m_characterTriggerEntity.GetCharacterCountByFactionInsideTrigger(defender.GetFaction());
				}
				
				CP_TaskDefendAttackingFaction attacker = CP_TaskDefendAttackingFaction.Cast(m_aFactionSettings[i]);
				if (attacker)
				{
					if (attacker.GetCountOnlyPlayers())
						attackerCount += m_characterTriggerEntity.GetPlayersCountByFactionInsideTrigger(attacker.GetFaction());
					else
						attackerCount += m_characterTriggerEntity.GetCharacterCountByFactionInsideTrigger(attacker.GetFaction());
				}
			}
			
			if (m_fMinDefenderPercentageRatio == 0 || attackerCount == 0)
			{
				m_pSupportEntity.FinishTask(m_pTask);
				if (!m_bShowDebugShapesDuringRuntime)
					GetOwner().ClearFlags(EntityFlags.ACTIVE, true);
				
				UpdateHUD();
				return;
			}
			
			float defenderRatioEval = defenderCount / attackerCount;
			
			if (defenderRatioEval < m_fMinDefenderPercentageRatio)
			{
				m_pSupportEntity.FailTask(m_pTask);
				if (!m_bShowDebugShapesDuringRuntime)
					GetOwner().ClearFlags(EntityFlags.ACTIVE, true);
				
				UpdateHUD();
				return;
			}
		}
		
		m_pSupportEntity.FinishTask(m_pTask);
		if (!m_bShowDebugShapesDuringRuntime)
				GetOwner().ClearFlags(EntityFlags.ACTIVE, true);
		
		UpdateHUD();
	}
	
	//------------------------------------------------------------------------------------------------
	void SetupEvaluation()
	{
		if (m_fDefendTime > 0)
		{
			m_fEvaluateTimeStart = Replication.Time();
			m_fEvaluateTimeEnd = m_fEvaluateTimeStart + (m_fDefendTime * 1000);
			m_bEvaluationSet = true;
		}
		
		//If trigger is already set via the Trigger Name attribute, we don't need to search for it
		if (m_characterTriggerEntity)
			return;
		
		for (int i = 0, count = m_aChildren.Count(); i < count; i++)
		{
			CP_LayerBase layerBase = CP_LayerBase.Cast(m_aChildren[i]);
			if (!layerBase)
				continue;
			
			array<IEntity> spawnedEntities = layerBase.GetSpawnedEntities();
			for (int j = 0, countJ = spawnedEntities.Count(); j < countJ; j++)
			{
				SCR_CharacterTriggerEntity charTriggr = SCR_CharacterTriggerEntity.Cast(spawnedEntities[j]);
				if (charTriggr)
					m_characterTriggerEntity = charTriggr;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void Init(CP_Area pArea = null, CP_EActivationType EActivation = CP_EActivationType.SAME_AS_PARENT, bool bInit = true)
	{
 		super.Init(pArea, EActivation, bInit);
		
		if (!m_sTriggerName.IsEmpty())
			FindCharacterTriggerEntity();
		
		InitHUD();
		SetupEvaluation();
	}	
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		if (m_fTempCountdown >= 0)
		{
			SetEventMask(owner, EntityEvent.INIT | EntityEvent.FRAME);	
			GetOwner().SetFlags(EntityFlags.ACTIVE, true);
		}
	}
	
	void InitHUD()
	{
		m_wRoot = GetGame().GetHUDManager().CreateLayout(m_sCountdownHUD, EHudLayers.MEDIUM, 0);
		m_wInfoOverlay = m_wRoot.FindAnyWidget("Info");
		m_wCountdownOverlay = m_wRoot.FindAnyWidget("Countdown");
		m_wLeftFlag = ImageWidget.Cast(m_wRoot.FindAnyWidget("FlagSideBlue"));
		m_wRightFlag = ImageWidget.Cast(m_wRoot.FindAnyWidget("FlagSideRed"));
		m_wLeftScore = RichTextWidget.Cast(m_wRoot.FindAnyWidget("ScoreBlue"));
		m_wRightScore = RichTextWidget.Cast(m_wRoot.FindAnyWidget("ScoreRed"));
		m_wWinScore = RichTextWidget.Cast(m_wRoot.FindAnyWidget("TargetScore"));
		m_wCountdown = RichTextWidget.Cast(m_wRoot.FindAnyWidget("CountdownWin"));
		m_wFlavour = RichTextWidget.Cast(m_wRoot.FindAnyWidget("FlavourText"));
		m_wWinScoreSideLeft = ImageWidget.Cast(m_wRoot.FindAnyWidget("ObjectiveLeft"));
		m_wWinScoreSideRight = ImageWidget.Cast(m_wRoot.FindAnyWidget("ObjectiveRight"));
			
		m_wInfoOverlay.SetVisible(false);
		m_wLeftFlag.SetVisible(false);
		m_wRightFlag.SetVisible(false);
		m_wLeftScore.SetVisible(false);
		m_wRightScore.SetVisible(false);
		m_wWinScore.SetVisible(false);
		m_wWinScoreSideLeft.SetVisible(false);
		m_wWinScoreSideRight.SetVisible(false);
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateHUD()
	{
		m_fTempTimeSlice = 0;
		m_fTempCountdown--;
		
		if (m_fTempCountdown < 0 || (!m_pTask) || !m_bDisplayCountdownHUD)
		{
			m_wRoot.SetVisible(false);
			return;
		}
		
		if (m_pTask.GetTaskState() == SCR_TaskState.CANCELLED || m_pTask.GetTaskState() ==  SCR_TaskState.FINISHED)
		{
			m_wRoot.SetVisible(false);
			return;
		}
		
		string shownTime = SCR_Global.GetTimeFormatting(m_fTempCountdown, ETimeFormatParam.DAYS | ETimeFormatParam.HOURS, ETimeFormatParam.DAYS | ETimeFormatParam.HOURS | ETimeFormatParam.MINUTES);
		m_wCountdown.SetTextFormat("%1", shownTime);
		
		m_wFlavour.SetTextFormat("%1", m_sFormattedCountdownTitle);
		
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		super.EOnFrame(owner, timeSlice);
		
		m_fTempTimeSlice += timeSlice;
		if (m_fTempTimeSlice >= 1 && m_fTempCountdown >= 0)
			UpdateHUD();
		
		if (!m_bTaskEvaluated && m_bEvaluationSet && (Replication.Time() >= m_fEvaluateTimeEnd))
			 EvaluateStatus();
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class CP_TaskDefendFactionSettings
{
	[Attribute(defvalue: "", UIWidgets.EditBox, desc: "Faction Name", category: "")];
	protected FactionKey				m_sFactionKey;
	
	[Attribute(defvalue: "0", UIWidgets.EditBox, desc: "When disabled, all units from this faction will be counted with for other Task Defend conditions", category: "")];
	protected bool				m_bCountOnlyPlayers;
	
	//------------------------------------------------------------------------------------------------
	Faction GetFaction()
	{
		FactionManager factionManager = GetGame().GetFactionManager();
		if (factionManager)
			return factionManager.GetFactionByKey(m_sFactionKey);
	
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetFactionKey(FactionKey factionKey)
	{
		m_sFactionKey = factionKey;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetCountOnlyPlayers()
	{
		return m_bCountOnlyPlayers;
	}
}
	
//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class CP_TaskDefendDefendingFaction : CP_TaskDefendFactionSettings
{
}	

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class CP_TaskDefendAttackingFaction : CP_TaskDefendFactionSettings
{
}	