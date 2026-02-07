/*class SCR_CampaignBuildingModeEditorUIComponent : SCR_BaseModeEditorUIComponent
{	
	protected SCR_CampaignBuildingEditorComponent m_BuildingEditorComponent;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttachedScripted(Widget w)
	{
		super.HandlerAttachedScripted(w);
		
		m_BuildingEditorComponent = SCR_CampaignBuildingEditorComponent.Cast(SCR_CampaignBuildingEditorComponent.GetInstance(SCR_CampaignBuildingEditorComponent, true, true));
		if (!m_BuildingEditorComponent)
			return;
		
		Print(string.Format("DEBUG LINE | " + FilePath.StripPath(__FILE__) + ":27 | %1", m_BuildingEditorComponent), LogLevel.DEBUG);
		
		IEntity targetEntity = m_BuildingEditorComponent.GetProviderEntity();
		if (!targetEntity)
			return;
		
	}
};*/
