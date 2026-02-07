/*
Menu with a list view of scenarios.
*/
class SCR_ScenarioMenu : SCR_SuperMenuBase
{
	protected SCR_InputButtonComponent m_NavBack;

	//-----------------------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		
		// Find nav buttons
		m_NavBack = m_DynamicFooter.FindButton("Back");
		
		if (m_NavBack)
			m_NavBack.m_OnActivated.Insert(Close);
	}
};