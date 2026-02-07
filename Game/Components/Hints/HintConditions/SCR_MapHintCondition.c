[BaseContainerProps(), SCR_BaseContainerHintCondition()]
class SCR_MapHintCondition: SCR_BaseEditorHintCondition
{
	override protected void OnInitCondition(Managed owner)
	{
		SCR_MapEntity mapEntity = SCR_MapEntity.GetMapInstance();
		if (mapEntity)
		{
			mapEntity.GetOnMapOpen().Insert(Activate);
			mapEntity.GetOnMapClose().Insert(Deactivate);
		}
	}
	override protected void OnExitCondition(Managed owner)
	{
		SCR_MapEntity mapEntity = SCR_MapEntity.GetMapInstance();
		if (mapEntity)
		{
			mapEntity.GetOnMapOpen().Remove(Activate);
			mapEntity.GetOnMapClose().Remove(Deactivate);
		}
	}
};