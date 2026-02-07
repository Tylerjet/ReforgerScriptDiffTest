//------------------------------------------------------------------------------------------------
class SCR_CampaignRadioRegisteringComponentClass : ScriptComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignRadioRegisteringComponent : ScriptComponent
{
	//------------------------------------------------------------------------------------------------
	protected bool IsParentBase(IEntity ent)
	{
		SCR_CampaignBase base = SCR_CampaignBase.Cast(ent);

		// Base was found, stop query
		if (base)
		{
			base.RegisterHQRadio(GetOwner());
			return false;
		}
		
		// Keep looking
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		BaseWorld world = GetGame().GetWorld();
		
		if (!world)
			return;
		
		world.QueryEntitiesBySphere(owner.GetOrigin(), SCR_CampaignReconfigureHQRadioUserAction.MAX_BASE_DISTANCE, IsParentBase, null, EQueryEntitiesFlags.ALL);
	}
};
