//! Explosive trigger attribute for manipulating the time that remains until the detonation
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_ExplosiveFuzeTimerAttribute : SCR_BaseValueListEditorAttribute
{
	//------------------------------------------------------------------------------------------------
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity)
			return null;

		IEntity owner = editableEntity.GetOwner();
		if (!owner)
			return null;

		SCR_ExplosiveChargeComponent explosiveComp = SCR_ExplosiveChargeComponent.Cast(owner.FindComponent(SCR_ExplosiveChargeComponent));
		if (!explosiveComp)
			return null;

		int remainingTime;
		if (explosiveComp.GetUsedFuzeType() == SCR_EFuzeType.TIMED)
			remainingTime = (explosiveComp.GetTimeOfDetonation() - GetGame().GetWorld().GetWorldTime()) * 0.001;

		return SCR_BaseEditorAttributeVar.CreateFloat(remainingTime);
	}

	//------------------------------------------------------------------------------------------------
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var)
			return;

		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity)
			return;

		IEntity owner = editableEntity.GetOwner();
		if (!owner)
			return;

		SCR_ExplosiveChargeComponent explosiveComp = SCR_ExplosiveChargeComponent.Cast(owner.FindComponent(SCR_ExplosiveChargeComponent));
		if (!explosiveComp)
			return;

		if (explosiveComp.GetUsedFuzeType() != SCR_EFuzeType.NONE)
			explosiveComp.DisarmChargeSilent();

		explosiveComp.SetFuzeTime(var.GetFloat(), true);
		explosiveComp.ArmWithTimedFuze(true);
	}
}

//! Explosive trigger attribute for manipulating the arming state
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_ExplosiveFuzeArmingAttribute : SCR_BaseEditorAttribute
{
	//------------------------------------------------------------------------------------------------
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity)
			return null;

		IEntity owner = editableEntity.GetOwner();
		if (!owner)
			return null;

		SCR_ExplosiveChargeComponent explosiveComp = SCR_ExplosiveChargeComponent.Cast(owner.FindComponent(SCR_ExplosiveChargeComponent));
		if (!explosiveComp)
			return null;

		return SCR_BaseEditorAttributeVar.CreateBool(explosiveComp.GetUsedFuzeType() != SCR_EFuzeType.NONE);
	}

	//------------------------------------------------------------------------------------------------
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var)
			return;

		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		IEntity owner = editableEntity.GetOwner();
		if (!owner)
			return;

		SCR_ExplosiveChargeComponent explosiveComp = SCR_ExplosiveChargeComponent.Cast(owner.FindComponent(SCR_ExplosiveChargeComponent));
		if (!explosiveComp)
			return;

		if (var.GetBool())
		{
			explosiveComp.SetFuzeTime(120, true);
			explosiveComp.ArmWithTimedFuze(true);
		}
		else
		{
			if (explosiveComp.GetUsedFuzeType() != SCR_EFuzeType.NONE)
				explosiveComp.DisarmChargeSilent();
		}
	}
}
