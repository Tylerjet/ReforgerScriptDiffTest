[EntityEditorProps(insertable: false)]
class SCR_Tutorial_CombatEngineering_ExitBuilding_1Class : SCR_BaseTutorialStageClass
{
}

class SCR_Tutorial_CombatEngineering_ExitBuilding_1 : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return !IsBuildingModeOpen();
	}
}