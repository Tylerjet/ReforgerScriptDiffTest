//------------------------------------------------------------------------------------------------
//! Type of consumable gadget
enum EConsumableType
{
	None,
	Bandage,
	Health,
	Tourniquet,
	Saline,
	Morphine
};

class SCR_ConsumableEffectAnimationParameters : Managed
{
	void SCR_ConsumableEffectAnimationParameters(
		int itemUseCommandId,
		int itemCmdIntArg,
		float itemCmdFloatArg,
		float animDuration,
		int intParam,
		float floatParam,
		bool boolParam
	)
	{
		m_itemUseCommandId = itemUseCommandId;
		m_itemCmdIntArg = itemCmdIntArg;
		m_itemCmdFloatArg = itemCmdFloatArg;
		m_fAnimDuration = animDuration;
		m_intParam = intParam;
		m_floatParam = floatParam;
		m_boolParam = boolParam;
	}
	
	TAnimGraphCommand m_itemUseCommandId;
	int m_itemCmdIntArg;
	float m_itemCmdFloatArg;
	float m_fAnimDuration;
	int m_intParam;
	float m_floatParam;
	bool m_boolParam;
}

//------------------------------------------------------------------------------------------------
//! Effect assigned to the consumable gadget
[BaseContainerProps()]
class SCR_ConsumableEffectBase : Managed
{
	protected TAnimGraphCommand m_iPlayerApplyToSelfCmdId = -1;
	protected TAnimGraphCommand m_iPlayerApplyToOtherCmdId = -1;
	protected TAnimGraphCommand m_iPlayerReviveCmdId = -1;
	
	[Attribute("true", UIWidgets.CheckBox, "Whether consumable should be deleted directly after completing use", category: "General")]
	protected bool m_bDeleteOnUse;
	
	[Attribute("1", UIWidgets.EditBox, "Duration of the animation for using consumable on self", category: "General")]
	protected float m_fApplyToSelfDuration;	
	
	[Attribute("1", UIWidgets.EditBox, "Duration of the animation for using consumable on other", category: "General")]
	protected float m_fApplyToOtherDuration;

	EConsumableType m_eConsumableType;
	
	bool ActivateEffect(IEntity target, IEntity user, IEntity item, SCR_ConsumableEffectAnimationParameters animParams = null)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(user);
		if (!character)
			return false;
		
		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller)
			return false;
		
		bool activatedAction;
		SCR_ConsumableEffectAnimationParameters localAnimParams;
		if (animParams)
			localAnimParams = animParams;

		if (localAnimParams)
			activatedAction = controller.TryUseItemOverrideParams(item, false, localAnimParams.m_itemUseCommandId, localAnimParams.m_itemCmdIntArg, localAnimParams.m_itemCmdFloatArg, localAnimParams.m_fAnimDuration, localAnimParams.m_intParam, localAnimParams.m_floatParam, localAnimParams.m_boolParam, null);
		else
			activatedAction = controller.TryUseItem(item);
		
		return activatedAction;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Apply consumable effect
	//! /param target is the character who is having the effect applied
	void ApplyEffect(notnull IEntity target, notnull IEntity user, IEntity item, SCR_ConsumableEffectAnimationParameters animParams)
	{}
	
	//------------------------------------------------------------------------------------------------
	//! Condition whther this effect can be applied
	//! /param target is the character who is having the effect applied
	bool CanApplyEffect(notnull IEntity target, notnull IEntity user)
	{}
	
	//------------------------------------------------------------------------------------------------
	//! Update the animCommands for all animations related to consumable
	bool UpdateAnimationCommands(IEntity user)
	{}
	
	//------------------------------------------------------------------------------------------------
	SCR_ConsumableEffectAnimationParameters GetAnimationParameters(IEntity target, ECharacterHitZoneGroup group = ECharacterHitZoneGroup.VIRTUAL)
	{
		return new SCR_ConsumableEffectAnimationParameters(GetApplyToSelfAnimCmnd(target), 1, 0, m_fApplyToSelfDuration, 0, 0, false);
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetDeleteOnUse()
	{
		return m_bDeleteOnUse;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetApplyToOtherDuraction()
	{
		return m_fApplyToOtherDuration;
	}	

	//------------------------------------------------------------------------------------------------
	TAnimGraphCommand GetApplyToSelfAnimCmnd(IEntity user)
	{
		UpdateAnimationCommands(user);
		return m_iPlayerApplyToSelfCmdId;
	}	
	
	//------------------------------------------------------------------------------------------------
	TAnimGraphCommand GetApplyToOtherAnimCmnd(IEntity user)
	{
		UpdateAnimationCommands(user);
		return m_iPlayerApplyToOtherCmdId;
	}
};