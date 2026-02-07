class SCR_CampaignBuildingServicesEditorUIComponent : SCR_BaseEditorUIComponent
{
	[Attribute("{09DFB36A6C8D45C0}UI/layouts/Editor/GameInfo/GameInfo_CampaignBuilding_ServiceIcon.layout")]
	protected ResourceName m_sServiceIconsGridPrefab;

	[Attribute(DEFAULT_VALUE.ToString(), params: "1 inf 1", desc: "Maximum number of icons in the row")]
	protected int m_iMaxColumns;

	protected ref map<SCR_EServicePointType, SCR_ServicePointComponent> m_mServices = new map<EEditableEntityLabel, SCR_ServicePointComponent>();

	protected Widget m_wCampaignBuildingServicesRoot;

	protected const int PADDING_LEFT = 5;
	protected const int PADDING_RIGHT = 5;
	protected const int PADDING_TOP = 5;
	protected const int PADDING_BOTTOM = 5;
	protected static const int DEFAULT_VALUE = 7;

	protected ref array<ref SCR_EditableEntityUIInfo> m_aServicesUIinfo = {};
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wCampaignBuildingServicesRoot = w;

		SCR_CampaignBuildingEditorComponent buildingEditorComponent = SCR_CampaignBuildingEditorComponent.Cast(SCR_CampaignBuildingEditorComponent.GetInstance(SCR_CampaignBuildingEditorComponent));
		if (!buildingEditorComponent)
			return;

		// if provider is a base, set services icons
		SCR_CampaignMilitaryBaseComponent base = SCR_CampaignMilitaryBaseComponent.Cast(buildingEditorComponent.GetProviderEntity().FindComponent(SCR_CampaignMilitaryBaseComponent));
		if (!base)
		{
			w.SetOpacity(0);
			return;
		}

		GetAllServicesUIInfo();
		SetBaseServices(base);

		base.GetOnServiceRegistered().Insert(SetBaseServices);
	}

	//------------------------------------------------------------------------------------------------
	protected void GetAllServicesUIInfo()
	{
		SCR_PlacingEditorComponentClass placingPrefabData = SCR_PlacingEditorComponentClass.Cast(SCR_PlacingEditorComponentClass.GetInstance(SCR_PlacingEditorComponent, true));
		if (!placingPrefabData)
			return;

		array<ResourceName> AllPrefabs = {};
		placingPrefabData.GetPrefabs(AllPrefabs);

		foreach (ResourceName prefab : AllPrefabs)
		{
			Resource entityPrefab = Resource.Load(prefab);
			if (!entityPrefab.IsValid())
				continue;

			IEntitySource entitySource = SCR_BaseContainerTools.FindEntitySource(entityPrefab);
			if (!entitySource)
				continue;

			IEntityComponentSource editableEntitySource = SCR_EditableEntityComponentClass.GetEditableEntitySource(entitySource);
			if (!editableEntitySource)
				continue;

			SCR_EditableEntityUIInfo editableUIInfo = SCR_EditableEntityUIInfo.Cast(SCR_EditableEntityComponentClass.GetInfo(editableEntitySource));
			if (!editableUIInfo)
				continue;

			if (editableUIInfo.GetFaction() != SCR_EntityHelper.GetEntityFaction(SCR_EntityHelper.GetPlayer()))
				continue;

			array<EEditableEntityLabel> entityLabels = {};
			editableUIInfo.GetEntityLabels(entityLabels);
			if (!entityLabels.Contains(EEditableEntityLabel.TRAIT_SERVICE))
				continue;

			m_aServicesUIinfo.Insert(editableUIInfo);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void SetBaseServices(SCR_CampaignMilitaryBaseComponent base)
	{
		array<SCR_ServicePointComponent> built = {};
		array<int> allServices = {};

		BaseGameMode gameMode = GetGame().GetGameMode();

		SCR_CampaignBuildingManagerComponent buildingManagerComponent = SCR_CampaignBuildingManagerComponent.Cast(gameMode.FindComponent(SCR_CampaignBuildingManagerComponent));
		if (!buildingManagerComponent)
			return;

		SCR_Enum.GetEnumValues(SCR_EServicePointType, allServices);

		if (base)
			base.GetServices(built);

		foreach (SCR_EServicePointType type : allServices)
		{
			m_mServices.Set(buildingManagerComponent.GetEditorServiceEnum(type), null);
		}

		foreach (SCR_ServicePointComponent service : built)
		{
			m_mServices.Set(service.GetLabel(), service);
		}

		ClearServicesWidget();
		SetServices();
	}

	//------------------------------------------------------------------------------------------------
	void ClearServicesWidget()
	{
		Widget gridWidget = m_wCampaignBuildingServicesRoot.FindAnyWidget("Grid");
		if (!gridWidget)
			return;

		SCR_WidgetHelper.RemoveAllChildren(gridWidget);
	}

	//------------------------------------------------------------------------------------------------
	void SetServices()
	{
		Widget serviceBtn;
		Widget gridWidget = m_wCampaignBuildingServicesRoot.FindAnyWidget("Grid");

		if (m_iMaxColumns < 1)
			m_iMaxColumns = DEFAULT_VALUE;

		for (int serviceId = m_mServices.Count() -1; serviceId >= 0; --serviceId)
		{
			serviceBtn = GetGame().GetWorkspace().CreateWidgets(m_sServiceIconsGridPrefab, gridWidget);
			int row = Math.Floor(serviceId / m_iMaxColumns);
			int column = serviceId % m_iMaxColumns;
			GridSlot.SetPadding(serviceBtn, PADDING_LEFT, PADDING_TOP, PADDING_RIGHT, PADDING_BOTTOM);
			GridSlot.SetRow(serviceBtn, row);
			GridSlot.SetColumn(serviceBtn, column);

			EEditableEntityLabel serviceLabel = m_mServices.GetKey(serviceId);

			if (!m_mServices.Get(serviceLabel))
				serviceBtn.SetOpacity(0.3);
			else
			{
				serviceBtn.SetOpacity(1);
				SetServiceStatus(serviceBtn, serviceLabel);
			}

			ImageWidget imgWidget = ImageWidget.Cast(serviceBtn.FindAnyWidget("Image"));
			if (!imgWidget)
				continue;

			SetIcon(imgWidget, serviceLabel);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SetIcon(ImageWidget widget, EEditableEntityLabel serviceLabel)
	{
		array<EEditableEntityLabel> entityLabels = {};

		for (int i = m_aServicesUIinfo.Count() -1; i >= 0; --i)
		{
			m_aServicesUIinfo[i].GetEntityLabels(entityLabels);
			if (entityLabels.Contains(serviceLabel))
			{
				m_aServicesUIinfo[i].SetIconTo(widget);
				return;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	void SetServiceStatus(Widget serviceBtn, int serviceLabel)
	{
		SCR_ButtonImageComponent img = SCR_ButtonImageComponent.Cast(serviceBtn.FindHandler(SCR_ButtonImageComponent));
		if (!img)
			return;

		switch (m_mServices.Get(serviceLabel).GetServiceStatus())
		{
			case SCR_EServicePointStatus.FUNCTIONAL:
			{
				img.GetImageWidget().SetColor(Color.White);
				break;
			}

			case SCR_EServicePointStatus.BROKEN:
			{
				img.GetImageWidget().SetColor(Color.Red);
				break;
			}
		}
	}
};
