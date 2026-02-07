//------------------------------------------------------------------------------------------------
[EntityEditorProps(category: "GameScripted/Tasks", description: "Transport task support entity.", color: "0 0 255 255")]
class SCR_CP_TaskSupportEntityClass: SCR_BaseTaskSupportEntityClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CP_TaskSupportEntity : SCR_BaseTaskSupportEntity
{
	protected IEntity 	m_pEntity;
	
	//------------------------------------------------------------------------------------------------
	override void FinishTask(notnull SCR_BaseTask task)
	{
		PrintFormat( "CP: ->Task: Task %1 accomplished.", task.GetTitleText()  );
		super.FinishTask( task );
	}
	
	
	//------------------------------------------------------------------------------------------------
	IEntity GetTaskEntity() { return m_pEntity; }
	
	//------------------------------------------------------------------------------------------------
	SCR_BaseTask CreateTask( CP_LayerTask pLayer )
	{
		m_pEntity = pLayer.GetSpawnedEntity();
		return super.CreateTask();
	}

	//------------------------------------------------------------------------------------------------
	void SetTaskPrefab( ResourceName sTaskPrefab )
	{
		m_sTaskPrefab = sTaskPrefab;
	}
		
	//------------------------------------------------------------------------------------------------
	override SCR_BaseTask CreateTask()
	{
		return super.CreateTask();
	}
	
	
	//------------------------------------------------------------------------------------------------
	protected SCR_MilitaryFaction GetCharacterFaction(IEntity unit)
	{
		if (!unit)
			return null;
		
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(unit);
		if (!character)
			return null;
		
		Faction faction = character.GetFaction();
		if (!faction)
			return null;

		return SCR_MilitaryFaction.Cast(faction);
	}
}