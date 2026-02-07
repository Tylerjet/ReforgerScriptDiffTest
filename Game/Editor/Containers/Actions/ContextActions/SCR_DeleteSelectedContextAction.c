//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_DeleteSelectedContextAction : SCR_SelectedEntitiesContextAction
{
	override bool CanBeShown(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
	{
		return CanBePerformed(selectedEntity, cursorWorldPosition, flags);
	}
	override bool CanBePerformed(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
	{
		return selectedEntity
			&& !selectedEntity.HasEntityFlag(EEditableEntityFlag.NON_DELETABLE)
			&& !selectedEntity.HasEntityFlag(EEditableEntityFlag.NON_INTERACTIVE);
			//&& !selectedEntity.HasEntityState(EEditableEntityState.PLAYER);
	}
	override void Perform(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition)
	{
		if (selectedEntity)
			selectedEntity.Delete();
	}
};
