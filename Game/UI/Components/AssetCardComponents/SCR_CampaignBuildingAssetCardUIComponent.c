class SCR_CampaignBuildingAssetCardUIComponent : ScriptedWidgetComponent
{
	protected SCR_CampaignBase m_Base;
	protected Widget m_wRootWidget;
	protected SCR_ContentBrowserEditorComponent m_ContentBrowserManager;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
    {
		m_wRootWidget = w;		
		m_ContentBrowserManager = SCR_ContentBrowserEditorComponent.Cast(SCR_ContentBrowserEditorComponent.GetInstance(SCR_ContentBrowserEditorComponent));
		
		SCR_CampaignBuildingEditorComponent buildingEditorComponent = SCR_CampaignBuildingEditorComponent.Cast(SCR_CampaignBuildingEditorComponent.GetInstance(SCR_CampaignBuildingEditorComponent));
		if (!buildingEditorComponent)
			return;
		
		IEntity targetEntity = buildingEditorComponent.GetProviderEntity();
		if (!targetEntity)
			return;	
		
		m_Base = SCR_CampaignBase.Cast(targetEntity);
		if (!m_Base)
			return;
		
		SCR_AssetCardFrontUIComponent assetCardUI = SCR_AssetCardFrontUIComponent.Cast(m_wRootWidget.FindHandler(SCR_AssetCardFrontUIComponent));
		if (!assetCardUI)
			return;
		
		assetCardUI.GetOnCardInit().Insert(CheckServiceAccessibility);
		m_Base.s_OnServiceRegistered.Insert(UpdateServiceAccessibility);
	}
		
	//------------------------------------------------------------------------------------------------
	// Check if this service can be build - if the same service exist at base, it can't. This doesn't prevent service to be placed, it's for UI only. Runs every time when the UI is refreshed.
	void CheckServiceAccessibility(int prefabID)	
	{
		if (!m_ContentBrowserManager)
			return;
		
		ResourceName prefab = m_ContentBrowserManager.GetResourceNamePrefabID(prefabID);
		Resource entityPrefab = Resource.Load(prefab);
		if (!entityPrefab.IsValid())
			return;
		
		// Get entity source
		IEntitySource entitySource = SCR_BaseContainerTools.FindEntitySource(entityPrefab);
		if (!entitySource)
			return;
		
		IEntityComponentSource editableEntitySource = SCR_EditableEntityComponentClass.GetEditableEntitySource(entitySource);
		if (!editableEntitySource)
			return;
		
		SCR_EditableEntityUIInfo editableUIInfo = SCR_EditableEntityUIInfo.Cast(SCR_EditableEntityComponentClass.GetInfo(editableEntitySource));
		if (!editableUIInfo)
			return;
		
		array<EEditableEntityLabel> entityLabels = {};
		array<SCR_CampaignServiceComponent> baseServices = {};
		editableUIInfo.GetEntityLabels(entityLabels);
		int count = m_Base.GetAllBaseServices(baseServices);
		
		for (int i = 0; i < count; i++)
		{
			if (entityLabels.Contains(baseServices[i].GetLabel()))
			{
				SCR_AssetCardFrontUIComponent assetCardUI = SCR_AssetCardFrontUIComponent.Cast(m_wRootWidget.FindHandler(SCR_AssetCardFrontUIComponent));
				if (!assetCardUI)
					return;
				
				SetUI(assetCardUI, editableUIInfo);
				return;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// Set the icon and saturation of the asset card.
	void SetUI(notnull SCR_AssetCardFrontUIComponent assetCardUI, notnull SCR_EditableEntityUIInfo editableUIInfo)
	{
		// dissable regular reresh of budget UI
		assetCardUI.SetEvaluateBlockingBudget(false);
		Widget budgetW = Widget.Cast(m_wRootWidget.FindAnyWidget("NoBudget"));
		if (!budgetW)
			return;
		
		ImageWidget widgetImage = ImageWidget.Cast(m_wRootWidget.FindAnyWidget("Image"));
		ImageWidget budgetIcon = ImageWidget.Cast(budgetW.FindAnyWidget("BudgetIcon"));
		if (!widgetImage || !budgetIcon)
			return;
		
		budgetW.SetVisible(true);
		widgetImage.SetSaturation(false);
		editableUIInfo.SetIconTo(budgetIcon);
	}
	
	//------------------------------------------------------------------------------------------------
	// This method is called from event when the service is build at base. The service is registered after the regular UI refresh.
	void UpdateServiceAccessibility()
	{
		SCR_AssetCardFrontUIComponent assetCardUIComponent = SCR_AssetCardFrontUIComponent.Cast(m_wRootWidget.FindHandler(SCR_AssetCardFrontUIComponent));
		if (!assetCardUIComponent)
			return;
		
		CheckServiceAccessibility(assetCardUIComponent.GetPrefabIndex());	
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		if (!m_wRootWidget)
			return;
		
		SCR_AssetCardFrontUIComponent assetCardUI = SCR_AssetCardFrontUIComponent.Cast(m_wRootWidget.FindHandler(SCR_AssetCardFrontUIComponent));
		if (assetCardUI)
			assetCardUI.GetOnCardInit().Remove(CheckServiceAccessibility);
		
		m_Base.s_OnServiceRegistered.Remove(UpdateServiceAccessibility);
	}
};