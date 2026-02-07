[EntityEditorProps(category: "GameScripted/Tasks", description: "")]
class SCR_TriggerTaskClass: SCR_EditorTaskClass
{
};
class SCR_TriggerTask: SCR_EditorTask
{
	[Attribute(desc: "When enabled, task will fail upon activation instead of being completed.", category: "Trigger Task")]
	protected bool m_bToFail;
	
	[Attribute(desc: "When enabled, task will change state when its trigger is deactivated instead of activated.", category: "Trigger Task")]
	protected bool m_bOnTriggerDeactivate;
	
	protected SCR_BaseFactionTriggerEntity m_Trigger;
	
	protected void OnTriggerActivate()
	{
		if (m_iTaskCompletionType != EEditorTaskCompletionType.AUTOMATIC)
			return;
		
		if (!m_bOnTriggerDeactivate)
		{
			if (m_bToFail)
				GetTaskManager().FailTask(this);
			else
				GetTaskManager().FinishTask(this);
		}
	}
	protected void OnTriggerDeactivate()
	{
		if (m_iTaskCompletionType != EEditorTaskCompletionType.AUTOMATIC)
			return;
		
		if (m_bOnTriggerDeactivate)
		{
			if (m_bToFail)
				GetTaskManager().FailTask(this);
			else
				GetTaskManager().FinishTask(this);
		}
	}
	
	override void SetTargetFaction(Faction targetFaction)
	{
		super.SetTargetFaction(targetFaction);
		
		if (m_Trigger)
			m_Trigger.SetOwnerFaction(targetFaction);
	}
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		
		if (Replication.IsServer())
		{
			IEntity child = GetChildren();
			while (child)
			{
				m_Trigger = SCR_BaseFactionTriggerEntity.Cast(child);
				if (m_Trigger)
					break;
				else
					child = child.GetSibling();
			}
			
			if (m_Trigger)
			{
				m_Trigger.GetOnActivate().Insert(OnTriggerActivate);
				m_Trigger.GetOnDeactivate().Insert(OnTriggerDeactivate);
			}
			else if (!SCR_Global.IsEditMode(this))
			{
				Print("SCR_TriggerTask is missing a child of type SCR_BaseFactionTriggerEntity!", LogLevel.ERROR);
			}
		}
	}
};