class SCR_CampaignBuildingServicesEditorUIComponent : SCR_BaseEditorUIComponent
{	
	[Attribute("{F7E8D4834A3AFF2F}UI/Imagesets/Conflict/conflict-icons-bw.imageset")]
	protected ResourceName m_sImageSet;	
	
	[Attribute("Armory")]
	protected string m_sArmory;
	[Attribute("FuelDepot")]
	protected string m_sFuelDepot;
	[Attribute("LightVehicleDepot")]
	protected string m_sLightVehicleDepot;
	[Attribute("HeavyVehicleDepot")]
	protected string m_sHeavyVehicleDepot;
	[Attribute("RadioAntenna")]
	protected string m_sRadioAntenna;
	[Attribute("Medical")]
	protected string m_sFieldHospital;
	[Attribute("Barracks")]
	protected string m_sBarracks;
	[Attribute("SupplyDepot")]
	protected string m_sSupplyDepot;
	[Attribute("VehicleDepot")]
	protected string m_sVehicleDepot;
	
	[Attribute("")]
	protected string m_sBarracksName;
	[Attribute("")]
	protected string m_sBaracksDesc;
	
	[Attribute("")]
	protected string m_sLightVehicleDepotName;
	[Attribute("")]
	protected string m_sLightVehicleDepotDesc;
	
	[Attribute("")]
	protected string m_sHeavyVehicleDepotName;
	[Attribute("")]
	protected string m_sHeavyVehicleDepotDesc;
	
	[Attribute("")]
	protected string m_sRadioAntennaName;
	[Attribute("")]
	protected string m_sRadioAntennaDesc;
	
	[Attribute("")]
	protected string m_sFieldHospitalName;
	[Attribute("")]
	protected string m_sFieldHospitalDesc;
	
	[Attribute("")]
	protected string m_sSupplyDepotName;
	[Attribute("")]
	protected string m_sSupplyDepotDesc;
	
	[Attribute("")]
	protected string m_sArmoryName;
	[Attribute("")]
	protected string m_sArmoryDesc;
	
	[Attribute("{09DFB36A6C8D45C0}UI/layouts/Editor/GameInfo/GameInfo_CampaignBuilding_ServiceIcon.layout")]
	protected ResourceName m_sServiceIconPrefab;
	
	protected ref map<ECampaignServicePointType, SCR_CampaignServiceComponent> m_mServices = new map<ECampaignServicePointType, SCR_CampaignServiceComponent>();
	protected Widget m_wCampaignBuildingServicesRoot;
	protected ref array<ref SCR_UIInfo> m_aInfo = {};
	
	ref SCR_UIInfo test;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wCampaignBuildingServicesRoot = w;
		
		SCR_CampaignBuildingEditorComponent buildingEditorComponent = SCR_CampaignBuildingEditorComponent.Cast(SCR_CampaignBuildingEditorComponent.GetInstance(SCR_CampaignBuildingEditorComponent));
		if (!buildingEditorComponent)
			return;
		
		// if provider is a base, set services icons
		SCR_CampaignBase base = SCR_CampaignBase.Cast(buildingEditorComponent.GetProviderEntity());
		if (!base)
		{
			w.SetOpacity(0);
			return;
		}
		
		SetBaseServices(base);
		
		base.s_OnServiceRegistered.Insert(SetBaseServices);
	}
	
	//------------------------------------------------------------------------------
	protected void SetBaseServices(SCR_CampaignBase base)
	{
		array<SCR_CampaignServiceComponent> built = {};
		array<int> allServices = {};
		
		SCR_Enum.GetEnumValues(ECampaignServicePointType, allServices);

		if (base)
			base.GetAllBaseServices(built);

		foreach (ECampaignServicePointType type : allServices)
		{
			m_mServices.Set(type, null);
		}

		foreach (SCR_CampaignServiceComponent service : built)
		{
			m_mServices.Set(service.GetType(), service);
		}
		
		ClearServicesWidget();
		SetServices();
	}
		
	//------------------------------------------------------------------------------
	void ClearServicesWidget()
	{
		Widget gridWidget = m_wCampaignBuildingServicesRoot.FindAnyWidget("Grid");
		if (!gridWidget)
			return;
		
		SCR_WidgetHelper.RemoveAllChildren(gridWidget);
	}
	
	//------------------------------------------------------------------------------
	void SetServices()
	{
		Widget serviceBtn;
		Widget gridWidget = m_wCampaignBuildingServicesRoot.FindAnyWidget("Grid");
			
		for (int serviceId = m_mServices.Count() -1 ; serviceId >= 0; --serviceId)
		{
			serviceBtn = GetGame().GetWorkspace().CreateWidgets(m_sServiceIconPrefab, gridWidget);
			UniformGridSlot.SetRow(serviceBtn, 0);
			UniformGridSlot.SetColumn(serviceBtn, serviceId);

			if (m_mServices.Get(serviceId) == null)
				serviceBtn.SetOpacity(0.3);
			else
			{
				serviceBtn.SetOpacity(1);
				SetServiceStatus(serviceBtn, serviceId);
			}

			SetServiceImage(m_mServices.GetKey(serviceId), serviceBtn);
		}
	}
	
	//------------------------------------------------------------------------------
	// Set a tooltip to service icons
	void SetServiceTooltip(string name, string description, Widget serviceBtn, ECampaignServicePointType serviceType)
	{
		SCR_LinkTooltipTargetEditorUIComponent tooltipComp = SCR_LinkTooltipTargetEditorUIComponent.Cast(serviceBtn.FindHandler(SCR_LinkTooltipTargetEditorUIComponent));
		if (!tooltipComp)
			return;
		
		
		LocalizedString str1 = name;
		LocalizedString str2 = description;
		
		SCR_UIInfo info = SCR_UIInfo.CreateInfo(str1, str2);
		test = info;
		//m_aInfo.Insert(info);
		tooltipComp.SetInfo(serviceBtn, info);
	}
	
	//------------------------------------------------------------------------------
	void SetServiceStatus(Widget serviceBtn, int serviceId)
	{
		SCR_ButtonImageComponent img = SCR_ButtonImageComponent.Cast(serviceBtn.FindHandler(SCR_ButtonImageComponent));
		if (!img)
			return;
		
		switch (m_mServices.Get(serviceId).GetServiceStatus())
		{
			case ECampaignServiceStatus.FUNCTIONAL:
			{
				img.GetImageWidget().SetColor(Color.White);
				break;
			}
			
			case ECampaignServiceStatus.BROKEN:
			{
				img.GetImageWidget().SetColor(Color.Red);
				break;
			}
		}
	}
		
	//------------------------------------------------------------------------------
	void SetImage(string image, Widget serviceBtn)
	{
		SCR_ButtonImageComponent img = SCR_ButtonImageComponent.Cast(serviceBtn.FindHandler(SCR_ButtonImageComponent));
		if (img)
			img.SetImage(m_sImageSet, image);
	}
	
	//------------------------------------------------------------------------------
	//ToDo: Load icons directly from the service component
	void SetServiceImage(ECampaignServicePointType type, Widget serviceBtn)
	{
		switch (type)
		{
			case ECampaignServicePointType.BARRACKS:
			{
				SetImage(m_sBarracks, serviceBtn);
				SetServiceTooltip(m_sBarracksName, m_sBaracksDesc, serviceBtn, type);
				break;
			}

			case ECampaignServicePointType.FIELD_HOSPITAL:
			{
				SetImage(m_sFieldHospital, serviceBtn);
				SetServiceTooltip(m_sFieldHospitalName, m_sFieldHospitalDesc, serviceBtn, type);
				break;
			}

			case ECampaignServicePointType.LIGHT_VEHICLE_DEPOT:
			{
				SetImage(m_sLightVehicleDepot, serviceBtn);
				SetServiceTooltip(m_sLightVehicleDepotName, m_sLightVehicleDepotDesc, serviceBtn, type);
				break;
			}
			
			case ECampaignServicePointType.HEAVY_VEHICLE_DEPOT:
			{
				SetImage(m_sHeavyVehicleDepot, serviceBtn);
				SetServiceTooltip(m_sHeavyVehicleDepotName, m_sHeavyVehicleDepotDesc, serviceBtn, type);
				break;
			}
			
			case ECampaignServicePointType.RADIO_ANTENNA:
			{
				SetImage(m_sRadioAntenna, serviceBtn);
				SetServiceTooltip(m_sRadioAntennaName, m_sRadioAntennaDesc, serviceBtn, type);
				break;
			}

			case ECampaignServicePointType.SUPPLY_DEPOT:
			{
				SetImage(m_sSupplyDepot, serviceBtn);
				SetServiceTooltip(m_sSupplyDepotName, m_sSupplyDepotDesc, serviceBtn, type);
				break;
			}

			case ECampaignServicePointType.ARMORY:
			{
				SetImage(m_sArmory, serviceBtn);
				SetServiceTooltip(m_sArmoryName, m_sArmoryDesc, serviceBtn, type);
				break;
			} 

			/*case ECampaignServicePointType.FUEL_DEPOT:
			{
				SetImage(m_sFuelDepot, serviceBtn);
				m_sServiceName = "#AR-Comm_Variable_Miscellaneous_Fueldepot_US";
			} break;*/

			/*case ECampaignServicePointType.VEHICLE_DEPOT:
			{
				SetImage(m_sVehicleDepot, serviceBtn);
				m_sServiceName = "#AR-Campaign_Building_MotorPool";
			} break;*/
		}
	}
};