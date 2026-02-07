class SCR_ServicePointComponentClass : ScriptComponentClass
{
};

class SCR_ServicePointComponent : SCR_MilitaryBaseLogicComponent
{
	[Attribute(SCR_EServicePointType.SUPPLY_DEPOT.ToString(), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_EServicePointType))]
	protected SCR_EServicePointType m_eType;
	
	[Attribute(defvalue: "0", uiwidget: UIWidgets.ComboBox, desc: "Type", enums: ParamEnumArray.FromEnum(EEditableEntityLabel))]
	protected EEditableEntityLabel m_eBuildingLabel;
	
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
	protected void OnFactionChanged(Faction faction)
	{
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnBaseFactionChanged(Faction faction)
	{
		if (!m_FactionControl)
			return;
		
		m_FactionControl.SetAffiliatedFaction(faction);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnBaseRegistered(notnull SCR_MilitaryBaseComponent base)
	{
		super.OnBaseRegistered(base);
		
		OnBaseFactionChanged(base.GetFaction());
		base.GetOnFactionChanged().Insert(OnBaseFactionChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnBaseUnregistered(notnull SCR_MilitaryBaseComponent base)
	{
		super.OnBaseUnregistered(base);
		
		base.GetOnFactionChanged().Remove(OnBaseFactionChanged);
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
			
			OnFactionChanged(faction);
			m_FactionControl.GetOnFactionUpdate().Insert(OnFactionChanged);
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
};