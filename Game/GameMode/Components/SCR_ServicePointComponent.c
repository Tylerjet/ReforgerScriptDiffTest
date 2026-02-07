class SCR_ServicePointComponentClass : SCR_MilitaryBaseLogicComponentClass
{
};

class SCR_ServicePointComponent : SCR_MilitaryBaseLogicComponent
{
	[Attribute(SCR_EServicePointType.SUPPLY_DEPOT.ToString(), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_EServicePointType))]
	protected SCR_EServicePointType m_eType;
	
	[Attribute(defvalue: "0", uiwidget: UIWidgets.SearchComboBox, desc: "Type", enums: ParamEnumArray.FromEnum(EEditableEntityLabel))]
	protected EEditableEntityLabel m_eBuildingLabel;
	
	// This var will later represent a service status - functional, broken or any other...
	protected SCR_EServicePointStatus m_eServiceStatus = SCR_EServicePointStatus.FUNCTIONAL;
	
	protected SCR_FactionAffiliationComponent m_FactionControl;
	
	//------------------------------------------------------------------------------------------------
	SCR_EServicePointType GetType()
	{
		return m_eType;
	}
	
	//------------------------------------------------------------------------------------------------
	EEditableEntityLabel GetLabel()
	{
		return m_eBuildingLabel;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_EServicePointStatus GetServiceStatus()
	{
		return m_eServiceStatus;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnFactionChanged(FactionAffiliationComponent owner, Faction previousFaction, Faction faction)
	{
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		m_FactionControl = SCR_FactionAffiliationComponent.Cast(owner.FindComponent(SCR_FactionAffiliationComponent));

		if (m_FactionControl)
		{
			Faction faction = m_FactionControl.GetAffiliatedFaction();
			if (!faction)
				faction = m_FactionControl.GetDefaultAffiliatedFaction();
			
			OnFactionChanged(null, null, faction);
			m_FactionControl.GetOnFactionChanged().Insert(OnFactionChanged);
		}
	}
};

enum SCR_EServicePointType
{
	SUPPLY_DEPOT,
	ARMORY,
	LIGHT_VEHICLE_DEPOT,
	HEAVY_VEHICLE_DEPOT,
	FIELD_HOSPITAL,
	BARRACKS,
	RADIO_ANTENNA
	//FUEL_DEPOT
}

enum SCR_EServicePointStatus
{
	FUNCTIONAL,
	BROKEN
};