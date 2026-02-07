[BaseContainerProps(), SCR_ContainerMedicalActionTitle()]
class SCR_ScenarioFrameworkMedicalActionAddParticularBleeding : SCR_ScenarioFrameworkMedicalAction
{
	[Attribute(uiwidget: UIWidgets.EditBox, desc: "Which hit zone will start bleeding")];
	string m_sHitZoneName;

	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		m_DamageManager.AddParticularBleeding(m_sHitZoneName);
	}
}