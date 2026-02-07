//------------------------------------------------------------------------------
class SCR_CampaignMapUIService : SCR_CampaignMapUIElement
{
	protected SCR_CampaignMapUIBase m_Parent;

	protected ECampaignServicePointType m_eServiceType;
	protected SCR_CampaignDeliveryPoint m_ServiceEntity;

	protected bool m_bEnabled;
	protected string m_sServiceName;
	protected string m_sServiceIcon;
	protected string m_sServiceText;

	//------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		PlayHoverSound(m_sSoundService);

		ShowHint(true);
		if (m_Parent)
			m_Parent.m_bIsAnyElementHovered = true;

		return false;
	}

	//------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		ShowHint(false);
		if (m_Parent)
			m_Parent.m_bIsAnyElementHovered = false;

		return false;
	}

	//------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		SCR_MapEntity mapa = SCR_MapEntity.GetMapInstance();
		if (m_MapItem && m_MapItem.IsVisible()  && mapa)
		{
			vector pos = m_MapItem.GetPos();
			mapa.ZoomPanSmooth(0.5 * mapa.GetMaxZoom(), pos[0], pos[2]);
		}

		return false;
	}

	//------------------------------------------------------------------------------
	override void AnimExpand()
	{
	}

	override void AnimCollapse()
	{
	}

	//------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		m_eIconType = EIconType.SERVICE;
	}

	void ShowHint(bool show)
	{
		if (m_Parent)
		{
			SCR_CampaignBase base = m_Parent.GetBase();
			if (m_eServiceType == ECampaignServicePointType.SUPPLY_DEPOT && base)
			{
				Faction playerFaction = SCR_RespawnSystemComponent.GetLocalPlayerFaction();
				if (base.GetOwningFaction() == playerFaction)
				{
					int supplies = base.GetSupplies();
					int suppliesMax = base.GetSuppliesMax();

					m_sServiceText = "#AR-Campaign_BaseSuppliesHintAmount";
					m_Parent.ShowServiceHint(m_sServiceName, m_sServiceIcon, m_sServiceText, show, supplies, suppliesMax);
				}
				else
				{
					m_sServiceText = "#AR-Campaign_BaseSuppliesHintUnknown";
					m_Parent.ShowServiceHint(m_sServiceName, m_sServiceIcon, m_sServiceText, show);
				}
			}
			else
			{
				m_Parent.ShowServiceHint(m_sServiceName, m_sServiceIcon, m_sServiceText, show);
			}
		}
	}

	//------------------------------------------------------------------------------
	void SetServiceImage()
	{
		switch (m_eServiceType)
		{
			case ECampaignServicePointType.BARRACKS:
			{
				SetImage(m_sBarracks);
				m_sServiceName = "#AR-Campaign_Building_Barracks";
			} break;

			case ECampaignServicePointType.FIELD_HOSPITAL:
			{
				SetImage(m_sFieldHospital);
				m_sServiceName = "#AR-Campaign_Building_FieldHospital";
			} break;

			case ECampaignServicePointType.REPAIR_DEPOT:
			{
				SetImage(m_sRepairDepot);
				m_sServiceName = "#AR-Comm_Variable_Miscellaneous_Vehicledepot_US";
			} break;

			case ECampaignServicePointType.SUPPLY_DEPOT:
			{
				SetImage(m_sSupplyDepot);
				m_sServiceName = "#AR-Comm_Variable_Miscellaneous_Supplydepot_US";
			} break;

			case ECampaignServicePointType.ARMORY:
			{
				SetImage(m_sArmory);
				m_sServiceName = "#AR-Campaign_Building_Armory";
			} break;

			case ECampaignServicePointType.FUEL_DEPOT:
			{
				SetImage(m_sFuelDepot);
				m_sServiceName = "#AR-Comm_Variable_Miscellaneous_Fueldepot_US";
			} break;

			case ECampaignServicePointType.VEHICLE_DEPOT:
			{
				SetImage(m_sVehicleDepot);
				m_sServiceName = "#AR-Campaign_Building_MotorPool";
			} break;
		}
	}

	void SetParent(SCR_CampaignMapUIBase parent)
	{
		m_Parent = parent;
	}

	void SetService(SCR_CampaignDeliveryPoint service, bool enabled)
	{
		m_bEnabled = enabled;
		m_ServiceEntity = service;
		m_eServiceType = service.GetServiceType();
		SetServiceImage();
		SCR_CampaignServiceMapDescriptorComponent descr = SCR_CampaignServiceMapDescriptorComponent.Cast(service.FindComponent(SCR_CampaignServiceMapDescriptorComponent));
		if (descr)
			m_MapItem = descr.Item();
	}

	override void SetImage(string image)
	{
		m_sServiceIcon = image;
		string suffix = string.Empty;

		SCR_ButtonImageComponent img = SCR_ButtonImageComponent.Cast(m_wRoot.FindHandler(SCR_ButtonImageComponent));
		if (img)
		{
			img.SetImage(m_sImageSet, image + suffix);
			img.SetEnabled(m_bEnabled);
		}
	}
};