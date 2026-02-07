[ComponentEditorProps(category: "GameScripted/Editor (Editables)", description: "", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_EditableTaskComponentClass: SCR_EditableEntityComponentClass
{
};

/** @ingroup Editable_Entities
*/

/*!
Editable SCR_BaseTask.
*/
class SCR_EditableTaskComponent: SCR_EditableDescriptorComponent
{
	protected SCR_EditorTask m_Task;
	protected Faction m_TargetFaction;
	protected int m_iTextIndex;
	
	/*!
	Initialize task on all machines.
	Tasks are not replicated by default; instead SCR_BaseTaskManager takes care of that.
	However, editor task entities are replicated, so we need to handle it here.
	*/
	void InitTask()
	{
		if (!m_Task)
			return;
		
		SCR_BaseTaskManager taskManager = GetTaskManager();
		if (!taskManager)
			return;
		
		int taskID = m_Task.GetTaskID();
		if (taskID == -1)
			return;
		
		SCR_BaseTaskData taskData = taskManager.CreateTaskData(m_Task);
		taskData.LoadDataFromTask(m_Task);
		
		InitTaskBroadcast(taskID, taskData);
		Rpc(InitTaskBroadcast, taskID, taskData);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void InitTaskBroadcast(int taskID, SCR_BaseTaskData taskData) //--- Rpc function receives SCR_BaseTaskData, not matter what the original was. ToDo: Improve
	{
		m_Task.SetTaskID(taskID);
		taskData.SetupTask(m_Task);
	}
	
	/*!
	Reveal the task to all players.
	*/
	void ActivateTask()
	{
		GetTaskManager().SetTaskFaction(m_Task, m_TargetFaction);
	}
	/*!
	Check if the task is actived, i.e., shown to players.
	\return True when actived
	*/
	bool IsTaskActivated()
	{
		return m_Task.GetTargetFaction() == m_TargetFaction;
	}
	
	/*!
	Get type of custom texts this task should use.
	\return Type of texts
	*/
	ETaskTextType GetTextType()
	{
		return m_Task.GetTextType();
	}
	/*!
	Get index of custom text from SCR_TextsTaskManagerComponent.
	\return Index from the array of texts
	*/
	int GetTextIndex()
	{
		return m_iTextIndex;
	}
	/*!
	Set index of custom text from SCR_TextsTaskManagerComponent.
	\param index Index from the array of texts
	*/
	void SetTextIndex(int index)
	{
		SetTextIndexBroadcast(index);
		Rpc(SetTextIndexBroadcast, index);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void SetTextIndexBroadcast(int index)
	{
		m_iTextIndex = index;
		m_Task.SetTextIndex(index);
		UpdateText();
	}
	
	/*!
	Get the task completion type
	\return MANUAL and ALWAYS_MENUAL means only the GM can complete the task. AUTOMATIC means that the task can auto complete and/or fail depending on the task
	*/
	EEditorTaskCompletionType GetTaskCompletionType()
	{
		return m_Task.GetTaskCompletionType();
	}
	
	/*!
	Set the task completion type
	\param completionType MANUAL and ALWAYS_MENUAL means only the GM can complete the task. AUTOMATIC means that the task can auto complete and/or fail depending on the task
	*/
	void SetTaskCompletionType(EEditorTaskCompletionType completionType)
	{
		if (m_Task.GetTaskCompletionType() == completionType)
			return;
		
		SetTaskCompletionTypeBroadcast(completionType);
		Rpc(SetTaskCompletionTypeBroadcast, completionType);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void SetTaskCompletionTypeBroadcast(EEditorTaskCompletionType completionType)
	{
		m_Task.SetTaskCompletionType(completionType);
	}
	
	protected void UpdateText()
	{
		UpdateInfo(m_Task.GetInfo());
		
		m_Task.SetTextIndex(m_iTextIndex);
		m_Task.SetLocationName(m_UIInfoDescriptor.GetLocationName());
	}
	override protected void GetOnLocationChange(SCR_EditableCommentComponent nearestLocation)
	{
		UpdateText();
	}
	
	override Faction GetFaction()
	{
		return m_TargetFaction;
	}
	
	override void SetTransform(vector transform[4])
	{	
		super.SetTransform(transform);
		UpdateNearestLocation();
	}
	
	override ScriptInvoker GetOnUIRefresh()
	{
		return Event_OnUIRefresh;
	}
	override bool RplSave(ScriptBitWriter writer)
	{
		if (!super.RplSave(writer))
			return false;
		
		int factionIndex;
		if (GetGame().GetFactionManager())
			factionIndex = GetGame().GetFactionManager().GetFactionIndex(m_TargetFaction);
		
		writer.WriteInt(factionIndex);
		writer.WriteInt(m_iTextIndex);
			
		if (m_Task)
			m_Task.Serialize(writer);
		
		return true;
	}
	override bool RplLoad(ScriptBitReader reader)
	{
		if (!super.RplLoad(reader))
			return false;
		
		int factionIndex;
		reader.ReadInt(factionIndex);
		if (GetGame().GetFactionManager())
			m_TargetFaction = GetGame().GetFactionManager().GetFactionByIndex(factionIndex);
		
		reader.ReadInt(m_iTextIndex);
		m_Task.SetTextIndex(m_iTextIndex);
		
		SCR_BaseTaskManager taskManager = GetTaskManager();
		if (taskManager && m_Task)
		{
			SCR_BaseTaskData taskData = taskManager.CreateTaskData(m_Task);
			taskData.Deserialize(reader);
			taskData.SetupTask(m_Task);
		}
		UpdateText();
		
		return true;
	}
	override SCR_EditableEntityComponent EOnEditorPlace(out SCR_EditableEntityComponent parent, SCR_EditableEntityComponent recipient, EEditorPlacingFlags flags, bool isQueue)
	{
		if (recipient)
		{
			m_TargetFaction = recipient.GetFaction();
			if (m_TargetFaction)
			{
				//--- When the task is placed as inactive, don't assign faction yet, do it only upon manual activation
				if (!SCR_Enum.HasFlag(flags, EEditorPlacingFlags.TASK_INACTIVE))
					m_Task.SetTargetFaction(m_TargetFaction);
				
				UpdateNearestLocation();
				
				InitTask();
			}
		}
		return this;
	}
	void SCR_EditableTaskComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_Task = SCR_EditorTask.Cast(ent);
	}
	
	
	override bool Delete(bool updateNavmesh = true)
	{
		if (m_Task)
			m_Task.ShowTaskNotification(ENotification.EDITOR_TASK_DELETED, true);
		
		return super.Delete(updateNavmesh);
	}
};
