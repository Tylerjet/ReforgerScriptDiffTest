//------------------------------------------------------------------------------------------------
class SCR_CampaignDefendTaskClass: SCR_CampaignBaseTaskClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignDefendTask : SCR_CampaignBaseTask
{
	static const float DEFEND_TASK_DEFAULT_MIN_ALLY_DISTANCE = 300;
	static const float DEFEND_TASK_DEFAULT_MAX_ALLY_DISTANCE = 400;
	static const float DEFEND_TASK_DEFAULT_MIN_ENEMY_DISTANCE = 300;
	static const float DEFEND_TASK_DEFAULT_MAX_ENEMY_DISTANCE = 400;
	
	//**************************//
	//PROTECTED STATIC VARIABLES//
	//**************************//
	protected static SCR_CampaignBase s_ClosestBase; //This is the closest base to the local player's character (only updated when task list is open)
	
	//*********************//
	//PUBLIC STATIC METHODS//
	//*********************//
	
	//------------------------------------------------------------------------------------------------
	static float GetMinAllyDistance()
	{
		SCR_CampaignDefendTaskSupportClass supportClass = SCR_CampaignDefendTaskSupportClass.Cast(GetTaskManager().GetSupportedTaskBySupportClassType(SCR_CampaignDefendTaskSupportClass));
		if (supportClass)
			return supportClass.GetMinAllyDistance();
		
		return DEFEND_TASK_DEFAULT_MIN_ALLY_DISTANCE;
	}
	
	//------------------------------------------------------------------------------------------------
	static float GetMaxAllyDistance()
	{
		SCR_CampaignDefendTaskSupportClass supportClass = SCR_CampaignDefendTaskSupportClass.Cast(GetTaskManager().GetSupportedTaskBySupportClassType(SCR_CampaignDefendTaskSupportClass));
		if (supportClass)
			return supportClass.GetMaxAllyDistance();
		
		return DEFEND_TASK_DEFAULT_MAX_ALLY_DISTANCE;
	}
	
	//------------------------------------------------------------------------------------------------
	static float GetMinEnemyDistance()
	{
		SCR_CampaignDefendTaskSupportClass supportClass = SCR_CampaignDefendTaskSupportClass.Cast(GetTaskManager().GetSupportedTaskBySupportClassType(SCR_CampaignDefendTaskSupportClass));
		if (supportClass)
			return supportClass.GetMinEnemyDistance();
		
		return DEFEND_TASK_DEFAULT_MIN_ENEMY_DISTANCE;
	}
	
	//------------------------------------------------------------------------------------------------
	static float GetMaxEnemyDistance()
	{
		SCR_CampaignDefendTaskSupportClass supportClass = SCR_CampaignDefendTaskSupportClass.Cast(GetTaskManager().GetSupportedTaskBySupportClassType(SCR_CampaignDefendTaskSupportClass));
		if (supportClass)
			return supportClass.GetMaxEnemyDistance();
		
		return DEFEND_TASK_DEFAULT_MAX_ENEMY_DISTANCE;
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_CampaignDefendTask FindDefendTask(SCR_CampaignBase targetBase, SCR_CampaignFaction targetFaction)
	{
		array<SCR_BaseTask> tasks = new array<SCR_BaseTask>();
		GetTaskManager().GetTasks(tasks);
		
		if (!tasks)
			return null;
		
		for (int i = tasks.Count() - 1; i >= 0; i--)
		{
			SCR_CampaignDefendTask defendTask = SCR_CampaignDefendTask.Cast(tasks[i]);
			if (!defendTask)
				continue;
			
			if (defendTask.GetTargetBase() == targetBase && defendTask.GetTargetFaction() == targetFaction)
				return defendTask;
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_CampaignBase FindClosestFriendlyBase(notnull IEntity controlledEntity, notnull Faction faction)
	{
		array<SCR_CampaignBase> bases = SCR_CampaignBaseManager.GetInstance().GetBases();
		if (!bases)
			return null;
		
		int closestBaseIndex = -1;
		float closestBaseDistance = float.MAX;
		float minAllyDistanceSq = Math.Pow(GetMinAllyDistance(), 2);
		vector controlledEntityOrigin = controlledEntity.GetOrigin();
		for (int i = bases.Count() - 1; i >= 0; i--)
		{
			if (bases[i].GetOwningFaction() != faction)
				continue;
			
			float distance = vector.DistanceSq(bases[i].GetOrigin(), controlledEntityOrigin);
			if (distance < minAllyDistanceSq && distance < closestBaseDistance)
			{
				closestBaseDistance = distance;
				closestBaseIndex = i;
			}
		}
		
		if (closestBaseIndex != -1)
			return bases[closestBaseIndex];
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	static bool CheckDefendRequestConditions(notnull SCR_PlayerController playerController, out SCR_CampaignBase closestBase)
	{
		IEntity locallyControlledEntity = playerController.GetMainEntity();
		
		if (!locallyControlledEntity)
			return false;
		
		FactionAffiliationComponent factionAffiliationComponent = FactionAffiliationComponent.Cast(locallyControlledEntity.FindComponent(FactionAffiliationComponent));
		if (!factionAffiliationComponent)
			return false;
		
		SCR_CampaignFaction faction = SCR_CampaignFaction.Cast(factionAffiliationComponent.GetAffiliatedFaction());
		if (!faction)
			return false;
		
		closestBase = FindClosestFriendlyBase(locallyControlledEntity, faction);
		
		if (!closestBase)
			return false;
		else
		{
			if (FindDefendTask(closestBase, faction))
				return false;
			
			if (!FindEnemyPresence(closestBase, faction))
				return false;
			
			return true;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set initialCheck to false if the task is already created for this base
	static bool FindEnemyPresence(notnull SCR_CampaignBase base, notnull SCR_CampaignFaction alliedFaction, bool initialCheck = true)
	{
		SCR_CampaignFaction enemyFaction = SCR_CampaignFactionManager.GetInstance().GetEnemyFaction(alliedFaction);
		if (!enemyFaction)
			return false;
		
		SCR_CampaignDefendTaskSupportClass supportClass = SCR_CampaignDefendTaskSupportClass.Cast(GetTaskManager().GetSupportedTaskBySupportClassType(SCR_CampaignDefendTaskSupportClass));
		if (!supportClass)
			return false;
		
		float distanceSq;
		if (initialCheck)
			distanceSq = supportClass.GetMinEnemyDistance();
		else
			distanceSq = supportClass.GetMaxEnemyDistance();
		
		distanceSq = Math.Pow(distanceSq, 2);
		
		array<SCR_ChimeraCharacter> enemyCharacters = supportClass.GetCharactersByFaction(enemyFaction);
		if (!enemyCharacters)
			return false;
		
		vector baseOrigin = base.GetOrigin();
		for (int i = enemyCharacters.Count() - 1; i >= 0; i--)
		{
			if (vector.DistanceSq(baseOrigin, enemyCharacters[i].GetOrigin()) < distanceSq)
				return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	static bool IsCharacterInAnyDefendTaskRange(SCR_ChimeraCharacter character)
	{
		array<SCR_BaseTask> tasks = new array<SCR_BaseTask>();
		GetTaskManager().GetTasks(tasks);
		
		if (!tasks)
			return false;
		
		SCR_CampaignDefendTaskSupportClass supportClass = SCR_CampaignDefendTaskSupportClass.Cast(GetTaskManager().GetSupportedTaskBySupportClassType(SCR_CampaignDefendTaskSupportClass));
		if (!supportClass)
			return false;
		
		for (int i = tasks.Count() - 1; i >= 0; i--)
		{
			SCR_CampaignDefendTask defendTask = SCR_CampaignDefendTask.Cast(tasks[i]);
			if (!defendTask)
				continue;
			
			if (defendTask.IsCharacterInDefendTaskRange(character, supportClass))
				return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//Call this only on server!
	static SCR_CampaignDefendTask CreateTask(SCR_CampaignBase targetBase, SCR_CampaignFaction targetFaction)
	{
		SCR_BaseTaskSupportClass supportClass = GetTaskManager().GetSupportedTaskByTaskType(SCR_CampaignDefendTask);
		if (!supportClass)
			return null;
		
		SCR_CampaignDefendTask task = SCR_CampaignDefendTask.Cast(GetGame().SpawnEntityPrefab(supportClass.GetTaskPrefab(), GetGame().GetWorld()));
		if (!task)
			return null;
		
		task.SetTargetFaction(targetFaction);
		task.SetTargetBase(targetBase);
		
		SCR_CampaignDefendTaskData taskData = new SCR_CampaignDefendTaskData();
		taskData.LoadDataFromTask(task);
		
		SCR_CampaignTaskManager taskManager = SCR_CampaignTaskManager.GetCampaignTaskManagerInstance();
		if (!taskManager)
			return task;
		
		taskManager.CreateCampaignDefendTask(taskData);
		
		return task;
	}
	
	//*********************************//
	//PROTECTED OVERRIDE MEMBER METHODS//
	//*********************************//
	
	//------------------------------------------------------------------------------------------------
	protected override void UpdateMapInfo()
	{
		//Insert task icon into base's map UI
		//m_TargetBase cannot be null here
	}
	
	//------------------------------------------------------------------------------------------------
	//! Shows a message about this task being available again
	protected override void ShowAvailableTask(bool afterAssigneeRemoved = false)
	{
		SCR_ECannotAssignReasons reason;
		if (CanBeAssigned(reason))
		{
			SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
			if (campaign)
			{
				string text = TASK_AVAILABLE_TEXT + " " + GetTitle();
				string baseName;
				
				if (m_TargetBase)
					baseName = m_TargetBase.GetBaseNameUpperCase();

				SCR_PopUpNotification.GetInstance().PopupMsg(text, prio: SCR_ECampaignPopupPriority.TASK_AVAILABLE, param1: baseName, text2: TASK_HINT_TEXT, text2param1: SCR_PopUpNotification.TASKS_KEY_IMAGE_FORMAT);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void ShowTaskProgress(bool showMsg = true)
	{
		SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
		if (campaign && showMsg)
		{
			string baseName;
			
			if (m_TargetBase)
				baseName = m_TargetBase.GetBaseNameUpperCase();
			
			SCR_PopUpNotification.GetInstance().PopupMsg(TASK_PROGRESS_TEXT + " " + GetTitle() + " " + TASK_AMOUNT_COMPLETED_TEXT, prio: SCR_ECampaignPopupPriority.TASK_PROGRESS, param1: baseName);
		}
	}
	
	//******************************//
	//PUBLIC OVERRIDE MEMBER METHODS//
	//******************************//
	
	//------------------------------------------------------------------------------------------------
	override void SetDescriptionWidgetText(notnull TextWidget textWidget, string taskText)
	{
		string baseName;
		
		if (m_TargetBase)
			baseName = m_TargetBase.GetBaseName();
		
		textWidget.SetTextFormat(taskText, baseName);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Finish(bool showMsg = true)
	{
		showMsg = SCR_RespawnSystemComponent.GetLocalPlayerFaction() == m_TargetFaction;
		super.Finish(showMsg);
		
		if (!m_TargetBase)
			return;
		
		SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
		if (!campaign)
			return;
		
		if (showMsg)
		{
			SCR_PopUpNotification.GetInstance().PopupMsg(TASK_COMPLETED_TEXT + " " + GetTitle(), prio: SCR_ECampaignPopupPriority.TASK_DONE, param1: m_TargetBase.GetBaseNameUpperCase(), sound: UISounds.TASK_SUCCEED);
		}
		
		// Reward XP for reconfiguring a relay
		if (!GetTaskManager().IsProxy())
		{
			SCR_CampaignDefendTaskSupportClass supportClass = SCR_CampaignDefendTaskSupportClass.Cast(GetTaskManager().GetSupportedTaskByTaskType(Type()));
			if (!supportClass)
				return;
			
			array<SCR_ChimeraCharacter> characters = supportClass.GetCharactersByFaction(SCR_CampaignFaction.Cast(GetTargetFaction()));
			if (!characters)
				return;
			
			for (int i = characters.Count() - 1; i >= 0; i--)
			{
				if (IsCharacterInDefendTaskRange(characters[i]))
					campaign.AwardXP(characters[i], CampaignXPRewards.TASK_DEFEND, 1.0);
			}
		};
	}
	
	//------------------------------------------------------------------------------------------------
	//! Fails the task.
	override void Fail(bool showMsg = true)
	{
		showMsg = SCR_RespawnSystemComponent.GetLocalPlayerFaction() == m_TargetFaction;
		super.Fail(showMsg);
		
		if (!m_TargetBase)
			return;
		
		SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
		if (campaign && showMsg)
		{
			SCR_PopUpNotification.GetInstance().PopupMsg(TASK_FAILED_TEXT + " " + GetTitle(), prio: SCR_ECampaignPopupPriority.TASK_DONE, param1: m_TargetBase.GetBaseNameUpperCase(), sound: UISounds.TASK_FAILED);
		}
	}
	
	//*********************//
	//PUBLIC MEMBER METHODS//
	//*********************//
	
	//------------------------------------------------------------------------------------------------
	bool IsCharacterInDefendTaskRange(notnull SCR_ChimeraCharacter character, SCR_CampaignDefendTaskSupportClass supportClass = null)
	{
		if (!supportClass)
		{
			supportClass = SCR_CampaignDefendTaskSupportClass.Cast(GetTaskManager().GetSupportedTaskByTaskType(Type()));
			if (!supportClass)
				return false;
		}
		
		float maxDistanceSq;
		// Is ally, use ally distance
		if (character.GetFaction() == GetTargetFaction())
			maxDistanceSq = supportClass.GetMaxAllyDistance();
		else
			maxDistanceSq = supportClass.GetMaxEnemyDistance();
		
		maxDistanceSq *= maxDistanceSq;
		
		float distanceSq = vector.DistanceSq(character.GetOrigin(), GetTargetBase().GetOrigin());
		
		return (distanceSq < maxDistanceSq);
	}
	
	//***************************//
	//PUBLIC MEMBER EVENT METHODS//
	//***************************//
	
	//------------------------------------------------------------------------------------------------
	void PeriodicalCheck()
	{
		SCR_CampaignFaction campaignFaction = SCR_CampaignFaction.Cast(m_TargetFaction);
		if (!m_TargetBase || !campaignFaction)
			return;
		
		// Enemy presence not found == success
		if (!FindEnemyPresence(m_TargetBase, campaignFaction, false))
			GetTaskManager().FinishTask(this);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnCharacterDeath(SCR_ChimeraCharacter character)
	{
		// Enemy died in range of this tasks target base, let's check if the town is safe
		if (character.GetFaction() != GetTargetFaction() && IsCharacterInDefendTaskRange(character))
			PeriodicalCheck();
	}
	
	//------------------------------------------------------------------------------------------------
	//! An event called when a base has been captured.
	void OnBaseCaptured(SCR_CampaignBase capturedBase)
	{
		if (!capturedBase || capturedBase != m_TargetBase)
			return;
		
		if (capturedBase.GetOwningFaction() == m_TargetFaction)
			return;
		
		GetTaskManager().FailTask(this);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Serialize(ScriptBitWriter writer)
	{
		super.Serialize(writer);
		
		int baseID = -1;
		SCR_CampaignBase base = GetTargetBase();
		if (base)
			baseID = base.GetBaseID();
		
		writer.WriteInt(baseID);
	}
	
	//------------------------------------------------------------------------------------------------
	override string GetMapDescriptorText()
	{
		return GetTaskListTaskText();
	}
	
	//------------------------------------------------------------------------------------------------
 	void SCR_CampaignDefendTask(IEntitySource src, IEntity parent)
	{
		if (SCR_Global.IsEditMode(this))
			return;
		
		if (GetTaskManager().IsProxy())
			return;
		
		if (SCR_BaseTaskManager.s_OnPeriodicalCheck60Second)
			SCR_BaseTaskManager.s_OnPeriodicalCheck60Second.Insert(PeriodicalCheck);
		
		if (SCR_GameModeCampaignMP.s_OnBaseCaptured)
			SCR_GameModeCampaignMP.s_OnBaseCaptured.Insert(OnBaseCaptured);
		
		SCR_CampaignDefendTaskSupportClass supportClass = SCR_CampaignDefendTaskSupportClass.Cast(GetTaskManager().GetSupportedTaskByTaskType(Type()));
		if (supportClass)
			supportClass.m_OnCharacterDeath.Insert(OnCharacterDeath);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_CampaignDefendTask(string description)
	{
		if (SCR_Global.IsEditMode(this) || !GetGame().GetGameMode())
			return;
		
		if (GetTaskManager().IsProxy())
			return;
		
		if (SCR_BaseTaskManager.s_OnPeriodicalCheck60Second)
			SCR_BaseTaskManager.s_OnPeriodicalCheck60Second.Remove(PeriodicalCheck);
		
		if (SCR_GameModeCampaignMP.s_OnBaseCaptured)
			SCR_GameModeCampaignMP.s_OnBaseCaptured.Remove(OnBaseCaptured);
		
		SCR_CampaignDefendTaskSupportClass supportClass = SCR_CampaignDefendTaskSupportClass.Cast(GetTaskManager().GetSupportedTaskByTaskType(Type()));
		if (supportClass)
			supportClass.m_OnCharacterDeath.Remove(OnCharacterDeath);
	}
};
