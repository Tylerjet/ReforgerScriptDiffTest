//------------------------------------------------------------------------------------------------
[EntityEditorProps(category: "GameScripted/Tasks", description: "Transport task support entity.", color: "0 0 255 255")]
class SCR_CP_TaskDeliverSupportEntityClass: SCR_CP_TaskSupportEntityClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CP_TaskDeliverSupportEntity : SCR_CP_TaskSupportEntity
{
	//------------------------------------------------------------------------------------------------
	override void FinishTask(notnull SCR_BaseTask task)
	{
		PrintFormat("CP: ->Task: Item was delivered to trigger %1.", SCR_TaskDeliver.Cast(task).GetTriggerNameToDeliver());
		super.FinishTask(task);
	}	
}