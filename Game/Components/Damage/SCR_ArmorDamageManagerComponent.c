class SCR_ArmorDamageManagerComponentClass : SCR_DamageManagerComponentClass
{
}

class SCR_ArmorDamageManagerComponent : SCR_DamageManagerComponent
{
	[Attribute(defvalue: "0.4", uiwidget: UIWidgets.EditBox, desc: "Multiplies rawdamage to be passed to character from this armor piece")]
	protected float m_fPassedDamageScale;
	
	//------------------------------------------------------------------------------------------------
	//! Armor doesn't take collisiondamage
	override bool FilterContact(IEntity owner, IEntity other, Contact contact)
	{
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! We hijack damage on armor, and use it to damage the character. Always return true on the hijack to ensure the armor itself doesn't take damage
	override bool HijackDamageHandling(notnull BaseDamageContext damageContext)
	{
		if (damageContext.damageType != EDamageType.KINETIC && damageContext.damageType != EDamageType.MELEE)
			return true;

		ChimeraCharacter char;
		IEntity parent = GetOwner();
		while (parent)
		{
			char = ChimeraCharacter.Cast(parent);
			if (char)
				break;

			parent = parent.GetParent();
		}
		
		if (!char)
			return true;
		
		SCR_CharacterDamageManagerComponent charDamMan = SCR_CharacterDamageManagerComponent.Cast(char.GetDamageManager());
		if (!charDamMan)
			return true;

		charDamMan.ArmorHitEventEffects(damageContext.damageValue);
		charDamMan.ArmorHitEventDamage(damageContext.damageType, damageContext.damageValue * m_fPassedDamageScale, damageContext.instigator.GetInstigatorEntity());
		
		return true;
	}
}
