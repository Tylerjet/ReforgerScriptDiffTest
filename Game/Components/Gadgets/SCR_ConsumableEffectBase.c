//------------------------------------------------------------------------------------------------
//! Type of consumable gadget
enum EConsumableType
{
	None,
	Bandage,
	Health
};

enum EBandagingAnimationBodyParts
{
	Invalid = 0,
	UpperHead = 1,
	LowerHead = 2,
	UpperTorso = 3,
	LowerTorso = 4,
	LeftHand = 5,
	RightHand = 6,
	LeftLeg = 7,
	RightLeg = 8
};

class SCR_ConsumableEffectAnimationParameters : Managed
{
	void SCR_ConsumableEffectAnimationParameters(
		int itemUseCommandId,
		float animDuration,
		int intParam,
		float floatParam,
		bool boolParam
	)
	{
		m_itemUseCommandId = itemUseCommandId;
		m_animDuration = animDuration;
		m_intParam = intParam;
		m_floatParam = floatParam;
		m_boolParam = boolParam;
	}
	
	TAnimGraphCommand m_itemUseCommandId;
	float m_animDuration;
	int m_intParam;
	float m_floatParam;
	bool m_boolParam;
}

//------------------------------------------------------------------------------------------------
//! Effect assigned to the consumable gadget
[BaseContainerProps()]
class SCR_ConsumableEffectBase : Managed
{		
	EConsumableType m_eConsumableType;
	
	//------------------------------------------------------------------------------------------------
	//! Apply consumable effect
	//! /param target is the character who is having the effect applied
	//! /param animParams is a copy of the arguments with which the animation was started, EXCEPT for the anim duration variable.
	void ApplyEffect(IEntity target, SCR_ConsumableEffectAnimationParameters animParams)
	{}
	
	//------------------------------------------------------------------------------------------------
	//! Condition whther this effect can be applied
	//! /param target is the character who is having the effect applied
	bool CanApplyEffect(IEntity target)
	{}
	
	//------------------------------------------------------------------------------------------------
	SCR_ConsumableEffectAnimationParameters GetAnimationParameters(IEntity target)
	{
		return null;
	}
};

//------------------------------------------------------------------------------------------------
//! Bandage effect
[BaseContainerProps()]
class SCR_ConsumableBandage : SCR_ConsumableEffectBase
{

	//------------------------------------------------------------------------------------------------
	override void ApplyEffect(IEntity target, SCR_ConsumableEffectAnimationParameters animParams)
	{
		SCR_CharacterDamageManagerComponent damageMgr = SCR_CharacterDamageManagerComponent.Cast(target.FindComponent(SCR_CharacterDamageManagerComponent));
		if (damageMgr)
			damageMgr.RemoveBleeding();
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanApplyEffect(IEntity target)
	{
		SCR_CharacterDamageManagerComponent damageMgr = SCR_CharacterDamageManagerComponent.Cast(target.FindComponent(SCR_CharacterDamageManagerComponent));
		if (damageMgr)
			return damageMgr.IsDamagedOverTime(EDamageType.BLEEDING);
		
		return false;
	}
	//------------------------------------------------------------------------------------------------
	override SCR_ConsumableEffectAnimationParameters GetAnimationParameters(IEntity target)
	{
		SCR_CharacterDamageManagerComponent damageMgr = SCR_CharacterDamageManagerComponent.Cast(target.FindComponent(SCR_CharacterDamageManagerComponent));
		if (damageMgr)
		{
			array<HitZone> bleedingHitzones = new array<HitZone>();
			damageMgr.GetBleedingHitZones(bleedingHitzones);
			
			EBandagingAnimationBodyParts bodyPartToBandage = EBandagingAnimationBodyParts.Invalid;
			float maxFoundBleedingSpeed = -1;
			for (int i; i < bleedingHitzones.Count(); i++)
			{
				SCR_CharacterHitZone chHZ = SCR_CharacterHitZone.Cast(bleedingHitzones[i]);
				if (!chHZ)
					continue; // Not sure if needed.
				
				float bleedingSpeed = chHZ.GetMaxBleedingRate();
				if (maxFoundBleedingSpeed < bleedingSpeed && chHZ.m_aBleedingAreas.Count() > 0)
				{
					maxFoundBleedingSpeed = bleedingSpeed;
					bodyPartToBandage = chHZ.m_aBleedingAreas[0];
				}
			}
			
			return new SCR_ConsumableEffectAnimationParameters(damageMgr.GetPlayerBandageSelfCmdId(), 4.0, bodyPartToBandage, 0.0, false);	
		}
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_ConsumableBandage()
	{
		m_eConsumableType = EConsumableType.Bandage;
	}
};
