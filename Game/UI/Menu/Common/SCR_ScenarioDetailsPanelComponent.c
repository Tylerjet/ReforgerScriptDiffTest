/*
Panel which shows state of a scenario.
It refreshes itself periodically.
*/

class SCR_ScenarioDetailsPanelComponent : SCR_ContentDetailsPanelBase
{	
	protected ref MissionWorkshopItem m_Scenario;
	protected SCR_MissionHeader m_Header;
	
	protected ref SCR_ScenarioDetailsPanelWidgets m_Widgets = new SCR_ScenarioDetailsPanelWidgets();
	
	// -------- Public API -----------
	
	
	
	//-----------------------------------------------------------------------------------
	void SetScenario(MissionWorkshopItem scenario)
	{
		if (m_Scenario == scenario)
			return;
		
		m_Scenario = scenario;	
		if (scenario)
		{
			SCR_MissionHeader header = SCR_MissionHeader.Cast(scenario.GetHeader());
			m_Header = header;
		}
		else
			m_Header = null;
		
		UpdateAllWidgets();
	}	
	
	
	
	// -------- Protected / Private -----------
	
	
	
	
	//-----------------------------------------------------------------------------------
	protected void UpdateAllWidgets()
	{
		if (!m_Scenario && !m_Header)
		{
			m_Widgets.m_NameText.SetText(string.Empty);
			m_Widgets.m_TypeOverlay.SetVisible(false);
			m_Widgets.m_AuthorNameText.SetText(string.Empty);
			m_Widgets.m_DescriptionText.SetText(string.Empty);
			m_Widgets.m_BackendImageComponent.SetScenarioAndImage(null, null);
			return;
		}
		
		// Name
		if (m_Header)
			m_Widgets.m_NameText.SetText(m_Header.m_sName);
		else if (m_Scenario)
			m_Widgets.m_NameText.SetText(m_Scenario.Name());
		
		// Author name
		// We don't have author name in scenario header now.
		if (m_Scenario)
		{
			if (m_Scenario.GetOwner())
			{
				string author = m_Scenario.GetOwner().Author().Name();
				m_Widgets.m_AuthorNameText.SetText(author);
			}
			else
				m_Widgets.m_AuthorNameText.SetText("Bohemia Interactive");
		}
		
		// Type
		if (m_Header)
		{
			string gameModeText = m_Header.m_sGameMode;
			m_Widgets.m_TypeText.SetText(m_Header.m_sGameMode);
			m_Widgets.m_TypeOverlay.SetVisible(!gameModeText.IsEmpty());
		}
		else
		{
			m_Widgets.m_TypeOverlay.SetVisible(false);
			m_Widgets.m_TypeText.SetText(string.Empty);
		}
		
		// Description
		if (m_Header)
			m_Widgets.m_DescriptionText.SetText(m_Header.m_sDescription);
		else if(m_Scenario)
			m_Widgets.m_DescriptionText.SetText(m_Scenario.Description());
		
		// Image
		if (m_Scenario)
			m_Widgets.m_BackendImageComponent.SetScenarioAndImage(m_Scenario, m_Scenario.Thumbnail());
		else
			m_Widgets.m_BackendImageComponent.SetScenarioAndImage(m_Scenario, null);
		
		// World name
		// Not supported yet
	}
	
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{	
		super.HandlerAttached(w);
		m_Widgets.Init(w);		
		UpdateAllWidgets();
	}
};