class SCR_CampaignServiceComponentClass : ScriptComponentClass
{
};

class SCR_CampaignServiceComponent : ScriptComponent
{
	[Attribute(defvalue: "0", uiwidget: UIWidgets.ComboBox, desc: "Type", enums: ParamEnumArray.FromEnum(ECampaignServicePointType))]
	protected ECampaignServicePointType m_eType;
	
	[Attribute(defvalue: "0", uiwidget: UIWidgets.ComboBox, desc: "Type", enums: ParamEnumArray.FromEnum(EEditableEntityLabel))]
	protected EEditableEntityLabel m_eBuildingLabel;
	
	protected SCR_CampaignBase m_Base;
	// This var will later represent a service status - functional, broken or any other...
	protected ECampaignServiceStatus m_eServiceStatus = ECampaignServiceStatus.FUNCTIONAL;
	
	[RplProp(onRplName: "OnParentBaseIDSet")]
	protected int m_iBaseRplID = -1;
	
	[RplProp(onRplName: "OnParentBaseIDSet")]
	protected bool m_bAdd;
	
	//------------------------------------------------------------------------------------------------
	ECampaignServicePointType GetType()
	{
		return m_eType;
	}
	
	//------------------------------------------------------------------------------------------------
	EEditableEntityLabel GetLabel()
	{
		return m_eBuildingLabel;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetParentBaseID(int id, bool add)
	{
		m_iBaseRplID = id;
		m_bAdd = add;
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
			m_Base.OnServiceBuilt(this, m_bAdd);
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
		
		if (base && base.GetIsEnabled())
		{
			base.RegisterAsParentBase(this, true);
		}
		else
		{
			MapDescriptorComponent comp = MapDescriptorComponent.Cast(GetOwner().FindComponent(MapDescriptorComponent));
			
			if (comp)
				comp.Item().SetVisible(false);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		// On deleting, notify campaign to remove the benefits service provides to base
		if (m_Base)
			m_Base.OnServiceBuilt(this, false);
	}
	
	//------------------------------------------------------------------------------------------------
	ECampaignServiceStatus GetServiceStatus()
	{
		return m_eServiceStatus;
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
	RADIO_ANTENNA
	//FUEL_DEPOT
};

enum ECampaignServiceStatus
{
	FUNCTIONAL,
	BROKEN
};