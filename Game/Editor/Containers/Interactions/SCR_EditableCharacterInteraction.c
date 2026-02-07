[BaseContainerProps()]
class SCR_EditableCharacterInteraction: SCR_EditableEntityInteraction
{
	override bool CanSetParent(SCR_EditableEntityComponent parentEntity, EEditableEntityInteractionFlag interactionFlags = int.MAX)
	{
		//--- Killed characters can be only in root
		if (!SCR_Enum.HasFlag(interactionFlags, EEditableEntityInteractionFlag.ALIVE))
			return !parentEntity;
		
		if (!parentEntity)
			return true;
		
		EEditableEntityType type = parentEntity.GetEntityType();
		EEditableEntityFlag flags = parentEntity.GetEntityFlags();
		
		return CanSetParentOfType(type, flags, interactionFlags);
	}
	
	override bool CanSetParentOfType(EEditableEntityType parentType, EEditableEntityFlag parentFlags, EEditableEntityInteractionFlag interactionFlags = int.MAX)
	{	
		//Cannot add to parent as can only add to root
		if (!SCR_Enum.HasFlag(interactionFlags, EEditableEntityInteractionFlag.ALIVE))
			return false;
		
		//--- Cannot move players to groups
		if (parentType == EEditableEntityType.GROUP && !SCR_Enum.HasFlag(interactionFlags, EEditableEntityInteractionFlag.NON_PLAYABLE))
			return false;
	
		//--- AI-compatible target
		return parentType == EEditableEntityType.CHARACTER
			|| parentType == EEditableEntityType.GROUP
			|| parentType == EEditableEntityType.VEHICLE;         
	}
};