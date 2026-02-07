class SCR_CampaignServiceComponentClass : ScriptComponentClass
{
};

class SCR_CampaignServiceComponent : ScriptComponent
{
	[Attribute(defvalue: "0", uiwidget: UIWidgets.ComboBox, desc: "Type", enums: ParamEnumArray.FromEnum(ECampaignServicePointType))]
	protected ECampaignServicePointType m_eType;
	
	protected SCR_CampaignBase m_Base;
	
	[RplProp(onRplName: "OnParentBaseIDSet")]
	protected int m_iBaseRplID = -1;
	
	//------------------------------------------------------------------------------------------------
	ECampaignServicePointType GetType()
	{
		return m_eType;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetParentBaseID(int id)
	{
		m_iBaseRplID = id;
		OnParentBaseIDSet()
	}
	
	//------------------------------------------------------------------------------------------------
	void OnParentBaseIDSet()
	{
		RplComponent rpl = RplComponent.Cast(Replication.FindItem(m_iBaseRplID));
		
		if (!rpl)
			return;
		
		m_Base = SCR_CampaignBase.Cast(rpl.GetEntity());
		
		if (m_Base)
			m_Base.OnServiceBuilt(this);
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_CampaignBase GetParentBase()
	{
		return m_Base;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		if (!GetGame().InPlayMode())
			return;
		
		SetEventMask(owner, EntityEvent.INIT);
		owner.SetFlags(EntityFlags.ACTIVE, true);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		SCR_CampaignBase base = SCR_CampaignBase.Cast(SCR_EntityHelper.GetMainParent(GetOwner()));
		
		if (base)
			base.RegisterAsParentBase(this);
	}
};

enum ECampaignServicePointType
{
	SUPPLY_DEPOT,
	ARMORY,
	LIGHT_VEHICLE_DEPOT,
	HEAVY_VEHICLE_DEPOT,
	FIELD_HOSPITAL,
	BARRACKS,
	RADIO_ANTENNA,
	//FUEL_DEPOT
};