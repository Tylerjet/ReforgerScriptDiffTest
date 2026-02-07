class SCR_BandageUserAction : ScriptedUserAction
{
	SCR_CharacterDamageManagerComponent			m_pDamageManager;
	SCR_AIUtilityComponent						m_Utility;
	
	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{		
		m_pDamageManager = SCR_CharacterDamageManagerComponent.Cast(pOwnerEntity.FindComponent(SCR_CharacterDamageManagerComponent));		
	}

	//------------------------------------------------------------------------------------------------
	
	//! Method called when the action is interrupted/canceled.
	//! \param pUserEntity The entity that was performing this action prior to interruption
	override void OnActionCanceled(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		DamageManagerComponent damageManager = DamageManagerComponent.Cast(pUserEntity.FindComponent(DamageManagerComponent));
		if (damageManager && damageManager.GetState() == EDamageState.DESTROYED)
			return;
		
		SCR_ConsumableItemComponent consumableComponent = GetConsumableComponent(pUserEntity);
		if (consumableComponent)
			consumableComponent.SetAlternativeModel(false);
	}
	
	//! Method called from scripted interaction handler when an action is started (progress bar appeared)
	//! \param pUserEntity The entity that started performing this action
	override void OnActionStart(IEntity pUserEntity)
	{
		SCR_ConsumableItemComponent consumableComponent = GetConsumableComponent(pUserEntity);
		if (consumableComponent)
			consumableComponent.SetAlternativeModel(true);
	}	
	//------------------------------------------------------------------------------------------------
	SCR_ConsumableItemComponent GetConsumableComponent(notnull IEntity pUserEntity)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(pUserEntity);
		if (!character)
			return null;
		
		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller)
			return null;

		IEntity item = controller.GetAttachedGadgetAtLeftHandSlot();
		if (!item)
			return null;
		
		return SCR_ConsumableItemComponent.Cast(item.FindComponent(SCR_ConsumableItemComponent));;
	}
	

	
	//------------------------------------------------------------------------------------------------	
	override bool CanBeShownScript(IEntity user)
	{
		return CanBePerformedScript(user);
	}
		
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		if (user == GetOwner())
			return false;
		
		if (!m_pDamageManager)
			return false;
		
		if (m_pDamageManager.GetState() == EDamageState.DESTROYED)
			return false;
		
		if (!m_pDamageManager.IsDamagedOverTime(EDamageType.BLEEDING))
			return false;
		
		ChimeraCharacter character = ChimeraCharacter.Cast(user);
		if (!character)
			return false;
		
		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller || controller.IsUsingItem())
			return false;
		
		SCR_ConsumableItemComponent consumableComponent = GetConsumableComponent(user);
		if (!consumableComponent)
			return false;
		
		return consumableComponent.GetConsumableType() == EConsumableType.Bandage;
	}
	
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		SCR_ConsumableItemComponent consumableComponent = GetConsumableComponent(pUserEntity);
		if (consumableComponent)
			consumableComponent.ApplyItemEffect(pOwnerEntity, new SCR_ConsumableEffectAnimationParameters(EConsumableType.Bandage, 0.0, 0, 0.0, false));
	}
};
