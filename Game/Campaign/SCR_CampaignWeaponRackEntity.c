[EntityEditorProps(category: "GameScripted/Campaign", description: "A rack filled with weapons used in campaign gamemode.", color: "0 0 255 255")]
class SCR_CampaignWeaponRackEntityClass: SCR_WeaponRackEntityClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignWeaponRackEntity : SCR_WeaponRackEntity
{
	protected float m_fTime;
	
	//------------------------------------------------------------------------------------------------
	void ResetTimer()
	{
		m_fTime = m_fPeriodTime;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetTimerTime()
	{
		return m_fTime;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetTimerTime(float time)
	{
		m_fTime = time;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_CampaignBase FindCampaignBaseParent()
	{
		IEntity parent = GetParent();
		SCR_CampaignBase base;
		while (parent)
		{
			base = SCR_CampaignBase.Cast(parent);
			if (base)
				break;
			
			parent = parent.GetParent();
		}
		
		return base;
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		
		if (SCR_Global.IsEditMode())
			return;
		
		SCR_CampaignArmoryComponent armoryComponent = SCR_CampaignArmoryComponent.GetNearestArmory(GetOrigin());
		if (!armoryComponent)
			return;
		
		armoryComponent.RegisterWeaponRack(this);
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_CampaignWeaponRackEntity(IEntitySource src, IEntity parent)
	{
		
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_CampaignWeaponRackEntity()
	{
	}

};
