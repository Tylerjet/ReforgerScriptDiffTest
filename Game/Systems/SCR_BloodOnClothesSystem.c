/*
System to update the bleeding mask on clothing items covering bleeding hitZones
*/

class SCR_BloodOnClothesSystem : GameSystem
{
	protected ref array<SCR_BleedingDamageEffect> m_aEffects = {};
	
	protected float m_fUpdateClothesTimer;
	protected const float UPDATE_CLOTHES_DELAY = 0.2;

	//------------------------------------------------------------------------------------------------
	protected override void OnInit()
	{
		Enable(false);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnUpdate(ESystemPoint point)
	{
		bool nullValuePresent;
		m_fUpdateClothesTimer += GetWorld().GetTimeSlice();
		if (m_fUpdateClothesTimer < UPDATE_CLOTHES_DELAY)
			return;
			
		SCR_CharacterDamageManagerComponent charDamageMan;
		SCR_CharacterHitZone hitZone;

		foreach (SCR_BleedingDamageEffect effect : m_aEffects)
		{
			if (!effect)
			{
				nullValuePresent = true;
				continue;
			}
			
			hitZone = SCR_CharacterHitZone.Cast(effect.GetAffectedHitZone());
			charDamageMan = SCR_CharacterDamageManagerComponent.Cast(hitZone.GetHitZoneContainer());
			if (charDamageMan)
				charDamageMan.AddBloodToClothes(hitZone, effect.GetDPS() * m_fUpdateClothesTimer);
		}
		
		if (nullValuePresent)
		{
			for (int i = m_aEffects.Count() - 1; i >= 0; i--)
			{
				if (!m_aEffects[i])
					m_aEffects.Remove(i);
			}
		}
		
		m_fUpdateClothesTimer = 0;
	}

	//------------------------------------------------------------------------------------------------
	void Register(SCR_BleedingDamageEffect bleedingEffect)
	{
		if (!bleedingEffect)
			return;
		
		if (!m_aEffects.Contains(bleedingEffect))
			m_aEffects.Insert(bleedingEffect);
		
		if (!IsEnabled())
			Enable(true);
	}

	//------------------------------------------------------------------------------------------------
	void Unregister(SCR_BleedingDamageEffect bleedingEffect)
	{
		m_aEffects.RemoveItem(bleedingEffect);
		if (m_aEffects.IsEmpty())
			Enable(false);
	}
}
