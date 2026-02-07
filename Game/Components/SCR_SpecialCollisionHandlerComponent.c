class SCR_SpecialCollisionHandlerComponentClass : ScriptComponentClass
{
}

class SCR_SpecialCollisionHandlerComponent : ScriptComponent
{
	[Attribute()]
	protected ref array<ref SCR_SpecialCollisionDamageEffect> m_aSpecialCollisions;

	[Attribute(desc: "Value used to set the SpecialContact signal which is used by audio. If left at 0 contact with this entity will not alter the signal.", params: "0 inf")]
	protected int m_iContactType;

	[Attribute("0", desc: "Value used to set the SpecialContactEntityHeight signal.\nWhen set to 0 then height is calculated from entity bounding box", params: "0 inf")]
	protected float m_fContactHeightOverride;

	//------------------------------------------------------------------------------------------------
	// Add all damage effects on this component to given array
	void GetSpecialCollisionDamageEffects(notnull inout array<SCR_SpecialCollisionDamageEffect> damageEffects)
	{
		foreach (SCR_SpecialCollisionDamageEffect damageEffect : m_aSpecialCollisions)
		{
			damageEffects.Insert(damageEffect)
		}
	}

	//------------------------------------------------------------------------------------------------
	int GetContactType()
	{
		return m_iContactType;
	}

	//------------------------------------------------------------------------------------------------
	float GetContactHeight()
	{
		if (m_fContactHeightOverride > 0)
			return m_fContactHeightOverride;

		vector mins, maxs;
		GetOwner().GetBounds(mins, maxs);
		m_fContactHeightOverride = maxs[1] - mins[1];
		return m_fContactHeightOverride;
	}
}