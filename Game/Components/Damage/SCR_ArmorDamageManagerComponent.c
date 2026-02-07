class SCR_ArmorDamageManagerComponentClass : SCR_DamageManagerComponentClass
{
}

class SCR_ArmorDamageManagerComponent : SCR_DamageManagerComponent
{
	[Attribute(defvalue: "0.4", uiwidget: UIWidgets.EditBox, desc: "Multiplies rawdamage to be passed to character from this armor piece")]
	protected float m_fPassedDamageScale;

	//------------------------------------------------------------------------------------------------
	override protected void OnDamage(notnull BaseDamageContext damageContext)
	{
		if (damageContext.damageType != EDamageType.KINETIC && damageContext.damageType != EDamageType.MELEE)
			return;

		IEntity parent = GetOwner();
		while (parent)
		{
			if (ChimeraCharacter.Cast(parent))
				break;

			parent = parent.GetParent();
		}
		
		ChimeraCharacter char = ChimeraCharacter.Cast(parent);
		if (!char)
			return;
		
		SCR_CharacterDamageManagerComponent charDamMan = SCR_CharacterDamageManagerComponent.Cast(char.GetDamageManager());
		if (!charDamMan)
			return;

		charDamMan.ArmorHitEventEffects(damageContext.damageValue);
		charDamMan.ArmorHitEventDamage(damageContext.damageType, damageContext.damageValue * m_fPassedDamageScale, damageContext.instigator.GetInstigatorEntity());
	}
}
