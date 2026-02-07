[EntityEditorProps(category: "GameScripted/Campaign", description: "Campaign version of FlagComponent, handling its material according to assigned base faction.", color: "0 0 255 255")]
class SCR_CampaignFlagComponentClass: SCR_FlagComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignFlagComponent: SCR_FlagComponent
{
	[Attribute("10", UIWidgets.Range, "Search for base in this range")]
	protected int m_iBaseSearchRange;
	
	[Attribute("", desc: "Search for specific base by name. If no base is found, range search will be used as fallback")]
	protected string m_sBaseSearchName;
	
	protected SCR_CampaignBase m_Base;
	
	//------------------------------------------------------------------------------------------------
	//! Assign base to component. String name will be prioritized before range search
	protected void SetBase()
	{
		if (!m_sBaseSearchName.IsEmpty())
		{
			m_Base = SCR_CampaignBase.Cast(GetGame().FindEntity(m_sBaseSearchName));
			if (m_Base)
			{
				SetupFlag();
				return;
			}
			else
				Print("Base not found, switching to sphere search", LogLevel.VERBOSE);
		}
		
		GetGame().GetWorld().QueryEntitiesBySphere(GetOwner().GetOrigin(), m_iBaseSearchRange, QueryNearBase, null, EQueryEntitiesFlags.ALL);
	}
	//------------------------------------------------------------------------------------------------
	//! Changes flag material according to base owner
	protected void ChangeFactionFlag()
	{
		if (!m_Base)
			return;
		
		if (!m_Base.GetOwningFaction())
			ChangeMaterial(GetDefaultMaterial());
		else
			ChangeMaterial(m_Base.GetOwningFaction().GetFactionFlagResource());
	}
	
	//------------------------------------------------------------------------------------------------
	//! Callback used in QueryEntitiesBySphere
	protected bool QueryNearBase(IEntity entity)
	{
		SCR_CampaignBase base = SCR_CampaignBase.Cast(entity);
		if (!base)
			return true;
		else
		{
			m_Base = base;
			SetupFlag()
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetupFlag()
	{
		ChangeFactionFlag();
		m_Base.m_OnFactionChanged.Insert(ChangeFactionFlag);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		if (SCR_GameModeCampaignMP.NotPlaying())
			return;
		
		super.EOnInit(owner);
		SetBase();
	}
	
};