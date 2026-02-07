class SCR_HealingUserAction : ScriptedUserAction
{
	[Attribute("0", UIWidgets.ComboBox, "Which hitzone group will be checked for conditions", "Healing useraction", ParamEnumArray.FromEnum(ECharacterHitZoneGroup) )]
	protected ECharacterHitZoneGroup m_eHitZoneGroup;

	//------------------------------------------------------------------------------------------------
	//! Method called from scripted interaction handler when an action is started (progress bar appeared)
	//! \param pUserEntity The entity that started performing this action
	override void OnActionStart(IEntity pUserEntity)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(pUserEntity);
		if (!character)
			return;
		
		SCR_ConsumableItemComponent consumableComponent = GetConsumableComponent(character);
		if (consumableComponent)
			consumableComponent.SetAlternativeModel(true);
	}
	
	//------------------------------------------------------------------------------------------------
	protected SCR_ConsumableItemComponent GetConsumableComponent(notnull ChimeraCharacter userChar)
	{
		CharacterControllerComponent controller = userChar.GetCharacterController();
		if (!controller)
			return null;

		IEntity item = controller.GetAttachedGadgetAtLeftHandSlot();
		if (!item)
			return null;
		
		return SCR_ConsumableItemComponent.Cast(item.FindComponent(SCR_ConsumableItemComponent));;
	}
		
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		// It is not allowed to perform healing useraction on self
		if (!user || user == GetOwner())
			return false;

		// Target character
		ChimeraCharacter targetCharacter = ChimeraCharacter.Cast(GetOwner());
		if (!targetCharacter)
			return false;
		
		// Medic character
		ChimeraCharacter userCharacter = ChimeraCharacter.Cast(user);
		if (!userCharacter)
			return false;

		SCR_ConsumableItemComponent consumableComponent = GetConsumableComponent(userCharacter);
		if (!consumableComponent || !consumableComponent.GetConsumableEffect())
			return false;
		
		if (!consumableComponent.GetConsumableEffect().CanApplyEffect(GetOwner(), user))
			return false;
		
		// Medics' item use ability check
		CharacterControllerComponent userController = userCharacter.GetCharacterController();
		if (!userController || userController.IsUsingItem())
			return false;

		
		SCR_CharacterDamageManagerComponent damageMan = SCR_CharacterDamageManagerComponent.Cast(targetCharacter.GetDamageManager());
		if (!damageMan || damageMan.GetState() == EDamageState.DESTROYED)
			return false;
			
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		ChimeraCharacter userCharacter = ChimeraCharacter.Cast(pUserEntity);
		if (!userCharacter)
			return;		
		
		ChimeraCharacter targetCharacter = ChimeraCharacter.Cast(pOwnerEntity);
		if (!targetCharacter)
			return;
		
		SCR_CharacterControllerComponent userController = SCR_CharacterControllerComponent.Cast(userCharacter.GetCharacterController());
		if (!userController)
			return;
		
		IEntity item = userController.GetAttachedGadgetAtLeftHandSlot();
		if (!item)
			return;
		
		SCR_ConsumableItemComponent consumableComponent = GetConsumableComponent(userCharacter);
		if (!consumableComponent)
			return;
		
		SCR_ConsumableEffectHealthItems consumableEffect = SCR_ConsumableEffectHealthItems.Cast(consumableComponent.GetConsumableEffect());
		if (!consumableEffect)
			return;
			
		TAnimGraphCommand desiredCmd = consumableEffect.GetApplyToOtherAnimCmnd(pOwnerEntity);

		SCR_CharacterControllerComponent targetController = SCR_CharacterControllerComponent.Cast(targetCharacter.GetCharacterController());
		if (targetController && targetController.IsUnconscious())
			desiredCmd = consumableEffect.GetReviveAnimCmnd(pOwnerEntity);
	
		SCR_CharacterDamageManagerComponent targetDamageMan = SCR_CharacterDamageManagerComponent.Cast(targetCharacter.GetDamageManager());
		if (!targetDamageMan)
			return;

		consumableComponent.SetTargetCharacter(pOwnerEntity);
		consumableComponent.GetConsumableEffect().ActivateEffect(pOwnerEntity, pUserEntity, item, new SCR_ConsumableEffectAnimationParameters(desiredCmd, 1, 0.0, consumableEffect.GetApplyToOtherDuraction(), targetDamageMan.FindAssociatedBandagingBodyPart(m_eHitZoneGroup), 0.0, false));
	}	
	
	//------------------------------------------------------------------------------------------------
	ECharacterHitZoneGroup GetUserActionGroup()
	{
		return m_eHitZoneGroup;
	}
	
	//------------------------------------------------------------------------------------------------	
	override bool CanBeShownScript(IEntity user)
	{
		return CanBePerformedScript(user);
	}
};
