class SCR_CampaignRadioRegisteringComponentClass : ScriptComponentClass
{
}

class SCR_CampaignRadioRegisteringComponent : ScriptComponent
{
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		if (!GetGame().InPlayMode())
			return;

		SetEventMask(owner, EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		IEntity parent = owner.GetParent();

		if (!parent)
			return;

		SCR_CampaignMilitaryBaseComponent base = SCR_CampaignMilitaryBaseComponent.Cast(parent.FindComponent(SCR_CampaignMilitaryBaseComponent));

		if (!base)
			return;

		base.RegisterHQRadio(owner);
	}
}
