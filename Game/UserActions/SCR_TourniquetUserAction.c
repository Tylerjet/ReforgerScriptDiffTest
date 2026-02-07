class SCR_TourniquetUserAction : SCR_HealingUserAction
{
	
	//------------------------------------------------------------------------------------------------
	//! Method called when the action is interrupted/canceled.
	//! \param pUserEntity The entity that was performing this action prior to interruption
	override void OnActionCanceled(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(pUserEntity);
		if (!character)
			return;
		
		SCR_CharacterDamageManagerComponent damageMan = SCR_CharacterDamageManagerComponent.Cast(character.GetDamageManager());
		if (damageMan) 
		{
			if (damageMan.GetIsUnconscious() || damageMan.GetHealth() <= 0)
				return;
		}
		
		SCR_ConsumableItemComponent consumableComponent = GetConsumableComponent(character);
		if (consumableComponent)
			consumableComponent.SetAlternativeModel(false);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		if (!super.CanBePerformedScript(user))
			return false;
		
		ChimeraCharacter userCharacter = ChimeraCharacter.Cast(user);
		if (!userCharacter)
			return false;
			
		SCR_ConsumableItemComponent consumableComponent = GetConsumableComponent(userCharacter);
		if (!consumableComponent || consumableComponent.GetConsumableType() != EConsumableType.Tourniquet)
			return false;
		
		ChimeraCharacter ownerCharacter = ChimeraCharacter.Cast(GetOwner());
		if (!ownerCharacter)
			return false;
		
		//Only bleeding HZs can be TQ'd
		SCR_CharacterDamageManagerComponent charDamMan = SCR_CharacterDamageManagerComponent.Cast(ownerCharacter.GetDamageManager());
		if (!charDamMan || charDamMan.GetGroupDamageOverTime(m_eHitZoneGroup, EDamageType.BLEEDING) == 0)
			return false;
		
		//Only extremities can be TQ'd
		array<ECharacterHitZoneGroup> tqGroups = {};
		SCR_CharacterDamageManagerComponent.GetAllExtremities(tqGroups);
		
		if (!tqGroups.Contains(m_eHitZoneGroup))
			return false;
		
		//No TQ'ing groups that are already TQ'd
		if (charDamMan.GetGroupTourniquetted(m_eHitZoneGroup))
			return false;
		
		return true;
	}
};
