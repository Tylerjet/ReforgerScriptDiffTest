/**
Entity Health Attribute for getting and setting varriables in Editor Attribute window
*/
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_BloodEditorAttribute : SCR_BaseValueListEditorAttribute
{
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity) 
			return null;
		
		IEntity owner = editableEntity.GetOwner();
		if (!owner) 
			return null;
		
		SCR_CharacterDamageManagerComponent characterDamageManager = SCR_CharacterDamageManagerComponent.Cast(owner.FindComponent(SCR_CharacterDamageManagerComponent));
		if (!characterDamageManager) 
			return null;
		
		if (characterDamageManager.GetState() == EDamageState.DESTROYED)  
			return null;
		
		HitZone bloodHitzone = characterDamageManager.GetBloodHitZone();
		if (!bloodHitzone)
			return null;
		
		//~Todo: Once an unconscious character can be revived, just use raw GetHealthScaled
		float bloodLevelLoseConsciousness = bloodHitzone.GetDamageStateThreshold(ECharacterBloodState.UNCONSCIOUS);
		float bloodLevel = Math.InverseLerp(bloodLevelLoseConsciousness, 1, bloodHitzone.GetHealthScaled());
		bloodLevel = Math.Clamp(bloodLevel, 0, 1);
		
		return SCR_BaseEditorAttributeVar.CreateFloat(Math.Round(bloodLevel * 100)));
	}
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var) 
			return;
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		
		IEntity owner =  editableEntity.GetOwner();
		if (!owner) 
			return;
		
		SCR_CharacterDamageManagerComponent characterDamageManager = SCR_CharacterDamageManagerComponent.Cast(owner.FindComponent(SCR_CharacterDamageManagerComponent));
		if (!characterDamageManager) 
			return;
		
		if (characterDamageManager.GetState() == EDamageState.DESTROYED)  
			return;
		
		HitZone bloodHitzone = characterDamageManager.GetBloodHitZone();
		if (!bloodHitzone)
			return;
		
		if (!characterDamageManager.IsDamageHandlingEnabled())
			return;
		
		//~Todo: Once an unconscious character can be revived, just use raw SetHealthScaled
		float bloodLevelLoseConsciousness = bloodHitzone.GetDamageStateThreshold(ECharacterBloodState.UNCONSCIOUS);
		float bloodLevel = Math.Lerp(bloodLevelLoseConsciousness, 1, var.GetFloat() * 0.01);
		bloodLevel = Math.Clamp(bloodLevel, 0, 1);
		bloodHitzone.SetHealthScaled(bloodLevel);
	}
};