//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_NeutralizeEntityContextAction : SCR_SelectedEntitiesContextAction
{
	override bool CanBeShown(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
	{
		/*EEditableEntityType entityType = selectedEntity.GetEntityType();
		if (entityType == EEditableEntityType.GROUP)
			return true;
		
		if (entityType != EEditableEntityType.VEHICLE && entityType != EEditableEntityType.CHARACTER)
			return false;*/
		
		DamageManagerComponent damageManager = DamageManagerComponent.Cast(selectedEntity.GetOwner().FindComponent(DamageManagerComponent));
		return damageManager && damageManager.GetDefaultHitZone();
	}
	
	override bool CanBePerformed(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
	{
		if (selectedEntity.GetEntityType() == EEditableEntityType.GROUP)
			return true;
		
		DamageManagerComponent damageManager = DamageManagerComponent.Cast(selectedEntity.GetOwner().FindComponent(DamageManagerComponent));
		return damageManager && damageManager.GetState() != EDamageState.DESTROYED && damageManager.IsDamageHandlingEnabled();
	}
	
	override void Perform(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition)
	{		
		//TODO @Zguba: Do we want to do health scaled here, or call kill or something similar?
		DamageManagerComponent damageManager = DamageManagerComponent.Cast(selectedEntity.GetOwner().FindComponent(DamageManagerComponent));
		if (damageManager && damageManager.IsDamageHandlingEnabled())
			damageManager.SetHealthScaled(0);
	}
};