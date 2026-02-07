[BaseContainerProps(), SCR_ContainerMedicalActionTitle()]
class SCR_ScenarioFrameworkMedicalActionRemoveAllBleedings : SCR_ScenarioFrameworkMedicalAction
{
	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		m_DamageManager.RemoveAllBleedings();
	}
}