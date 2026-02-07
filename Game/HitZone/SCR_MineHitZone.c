class SCR_MineHitZone : SCR_HitZone
{
	//------------------------------------------------------------------------------------------------
	override float ComputeEffectiveDamage(notnull BaseDamageContext damageContext, bool isDOT)
	{
		if (damageContext.damageType != EDamageType.MELEE)
			return super.ComputeEffectiveDamage(damageContext, isDOT);
		
		// Inactive mines don't take melee damage, activated mines can be destroyed using melee and explode
		SCR_PressureTriggerComponent triggerComponent = SCR_PressureTriggerComponent.Cast(GetOwner().FindComponent(SCR_PressureTriggerComponent));
		if (!triggerComponent || triggerComponent.IsActivated())
			return super.ComputeEffectiveDamage(damageContext, isDOT);
		
		return 0;
	}
}