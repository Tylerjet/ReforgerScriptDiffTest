/*
Panel which shows state of a scenario.
It refreshes itself periodically.
*/

class SCR_ScenarioDetailsPanelComponent : SCR_ContentDetailsPanelBase
{		
	protected ref MissionWorkshopItem m_Scenario;
	protected SCR_MissionHeader m_Header;
	
	protected SCR_ScenarioBackendImageComponent m_BackendImageComponent;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{	
		super.HandlerAttached(w);
		
		m_BackendImageComponent = SCR_ScenarioBackendImageComponent.Cast(m_CommonWidgets.m_wBackendImage.FindHandler(SCR_ScenarioBackendImageComponent));

		UpdateAllWidgets();
	}
	
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
		if (!GetGame().InPlayMode())
			return;
		
		if (!m_Scenario && !m_Header)
		{
			m_CommonWidgets.m_wNameText.SetText(string.Empty);
			m_CommonWidgets.m_wAuthorNameText.SetText(string.Empty);
			SetDescriptionText(string.Empty);
			
			if (m_BackendImageComponent)
				m_BackendImageComponent.SetScenarioAndImage(null, null);
			
			return;
		}
		
		// Name
		string title;
		if (m_Header)
			title = m_Header.m_sName;
		else if (m_Scenario)
			title = m_Scenario.Name();

		m_CommonWidgets.m_wNameText.SetText(title);
		
		// Author name
		// We don't have author name in scenario header now.
		UpdateAuthorNameText();
		
		// Description
		if (m_Header)
			SetDescriptionText(m_Header.m_sDescription);
		else if(m_Scenario)
			SetDescriptionText(m_Scenario.Description());
		
		// Image
		if (m_BackendImageComponent)
		{
			if (m_Scenario)
				m_BackendImageComponent.SetScenarioAndImage(m_Scenario, m_Scenario.Thumbnail());
			else
				m_BackendImageComponent.SetScenarioAndImage(m_Scenario, null);
		}

		// Error message
		bool isInError = SCR_ScenarioEntryHelper.IsModInErrorState(m_Scenario);
		float saturation = UIConstants.ENABLED_WIDGET_SATURATION;
		
		m_CommonWidgets.m_WarningOverlayComponent.SetWarningVisible(isInError, false);
		m_CommonWidgets.m_WarningOverlayComponent.SetWarning(SCR_ScenarioEntryHelper.GetErrorMessageVerbose(m_Scenario), SCR_ScenarioEntryHelper.GetErrorTexture(m_Scenario));
	
		if (isInError)
			saturation = UIConstants.DISABLED_WIDGET_SATURATION;
		
		if (m_BackendImageComponent)
			m_BackendImageComponent.SetImageSaturation(saturation);
	}
	
	//-----------------------------------------------------------------------------------
	protected void UpdateAuthorNameText()
	{
		if (!m_Scenario)
			return;

		if (m_Scenario.GetOwner() && m_Scenario.GetOwner().Author())
			m_CommonWidgets.m_wAuthorNameText.SetText(m_Scenario.Author());
		else
			m_CommonWidgets.m_wAuthorNameText.SetText(UIConstants.BOHEMIA_INTERACTIVE);
	}
}