//------------------------------------------------------------------------------------------------
[EntityEditorProps(category: "GameScripted/Tasks", description: "Transport task support entity.", color: "0 0 255 255")]
class SCR_CP_TaskClearAreaSupportEntityClass: SCR_CP_TaskSupportEntityClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CP_TaskClearAreaSupportEntity : SCR_CP_TaskSupportEntity
{
	//------------------------------------------------------------------------------------------------
	override SCR_BaseTask CreateTask( CP_LayerTask pLayer )
	{
		m_pEntity = pLayer.GetSpawnedEntity();
		return super.CreateTask();
	}
}