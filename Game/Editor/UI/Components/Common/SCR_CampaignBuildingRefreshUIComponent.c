class SCR_CampaignBuildingRefreshUIComponent : ScriptedWidgetComponent
{
	SCR_ExternalPaginationUIComponent m_PagUIComp;
	override void HandlerAttached(Widget w)
	{
		m_PagUIComp = SCR_ExternalPaginationUIComponent.Cast(w.FindHandler(SCR_ExternalPaginationUIComponent));
		if (!m_PagUIComp)
			return;

		SCR_CampaignBuildingEditorComponent buildingEditorComponent = SCR_CampaignBuildingEditorComponent.Cast(SCR_CampaignBuildingEditorComponent.GetInstance(SCR_CampaignBuildingEditorComponent));
		if (!buildingEditorComponent)
			return;

		IEntity provider = buildingEditorComponent.GetProviderEntity();
		if (!provider)
			return;

		SCR_CampaignSuppliesComponent supplyComponent = SCR_CampaignSuppliesComponent.Cast(provider.FindComponent(SCR_CampaignSuppliesComponent));
		if (supplyComponent)
			supplyComponent.m_OnSuppliesChanged.Insert(m_PagUIComp.RefreshPage);

		SCR_CampaignBuildingProviderComponent providerComponent = SCR_CampaignBuildingProviderComponent.Cast(provider.FindComponent(SCR_CampaignBuildingProviderComponent));
		if (!providerComponent)
			return;

		SCR_MilitaryBaseComponent base = providerComponent.GetMilitaryBaseComponent();
		if (!base)
			return;

		base.GetOnServiceRegistered().Insert(m_PagUIComp.RefreshPage);
	}
}
