[BaseContainerProps()]
class SCR_ConsumableEffectHealthItems : SCR_ConsumableEffectBase
{
	//------------------------------------------------------------------------------------------------	
	override bool ActivateEffect(IEntity target, IEntity user, IEntity item, ItemUseParameters animParams = null)
	{
		ItemUseParameters localAnimParams = animParams;
		if (!localAnimParams)
		{
			//user-held healthitem needs to get data from target to perform anim
			localAnimParams = GetAnimationParameters(item, target);
		}
		
		if (!localAnimParams)
			return false;
		
		if (!super.ActivateEffect(target, user, item, localAnimParams))
			return false;		

		ChimeraCharacter character = ChimeraCharacter.Cast(target);
		if (!character)
			return false;
		
		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller)
			return false;
			
		if (localAnimParams.GetIntParam() == EBandagingAnimationBodyParts.LeftLeg || localAnimParams.GetIntParam() == EBandagingAnimationBodyParts.RightLeg)
		{
			if (controller.GetStance() == ECharacterStance.STAND)
				controller.SetStanceChange(ECharacterStanceChange.STANCECHANGE_TOCROUCH);
		}
					
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void ApplyEffect(notnull IEntity target, notnull IEntity user, IEntity item, ItemUseParameters animParams)
	{
		InventoryItemComponent itemComp = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
 	 	if (itemComp)
 	 		itemComp.RequestUserLock(user, false);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool UpdateAnimationCommands(IEntity user)
	{
		ChimeraCharacter char = ChimeraCharacter.Cast(user);
		if (!char)
			return false;
		
		CharacterAnimationComponent animationComponent = char.GetAnimationComponent();
		if (!animationComponent)
			return false;		
		
		m_iPlayerApplyToSelfCmdId = animationComponent.BindCommand("CMD_HealSelf");
		m_iPlayerApplyToOtherCmdId = animationComponent.BindCommand("CMD_HealOther");
		m_iPlayerReviveCmdId = animationComponent.BindCommand("CMD_Revive");
		if (m_iPlayerApplyToSelfCmdId < 0 || m_iPlayerApplyToOtherCmdId < 0)
		{
			Print("One or both healing animationCommands have incorrect ID's!!", LogLevel.ERROR);
			return false;
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Condition whether this effect can be applied to the specific hit zone
	//! \param[in] target is the character who is having the effect applied
	//! \param[in] user
	//! \param[in] group the hitzonegroup which is having the effect applied
	//! \param[in] failReason
	bool CanApplyEffectToHZ(notnull IEntity target, notnull IEntity user, ECharacterHitZoneGroup group, out SCR_EConsumableFailReason failReason = SCR_EConsumableFailReason.NONE);
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] user
	//! \return
	TAnimGraphCommand GetReviveAnimCmnd(IEntity user)
	{
		UpdateAnimationCommands(user);
		return m_iPlayerReviveCmdId;
	}
}
