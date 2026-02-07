[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_TeleportPlayerHereContextAction : SCR_BaseContextAction
{
	override bool CanBeShown(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		if (cursorWorldPosition == vector.Zero || SCR_Enum.HasFlag(flags, EEditorContextActionFlags.LIMITED))
			return false;
		
		IEntity controlledEntity = SCR_PlayerController.GetLocalControlledEntity();
		if (!controlledEntity)
			return false;
		
		DamageManagerComponent damageManager = DamageManagerComponent.Cast(controlledEntity.FindComponent(DamageManagerComponent));
		if (!damageManager)
			return true;
		
		return damageManager.GetState() != EDamageState.DESTROYED;
	}
	override bool CanBePerformed(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		return CanBeShown(hoveredEntity, selectedEntities, cursorWorldPosition, flags);
	}
	
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition,int flags, int param = -1)
	{
		SCR_PlayersManagerEditorComponent playersManager = SCR_PlayersManagerEditorComponent.Cast(SCR_PlayersManagerEditorComponent.GetInstance(SCR_PlayersManagerEditorComponent));
		playersManager.TeleportPlayerToPosition(cursorWorldPosition);
	}
};