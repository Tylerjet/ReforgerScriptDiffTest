[BaseContainerProps()]
class SCR_EditableWaypointInteraction: SCR_EditableEntityInteraction
{
	override bool CanSetParent(SCR_EditableEntityComponent parentEntity, EEditableEntityInteractionFlag interactionFlags = int.MAX)
	{
		if (!parentEntity)
			return false;
		
		EEditableEntityType type = parentEntity.GetEntityType();
		EEditableEntityFlag flags = parentEntity.GetEntityFlags();
		
		return CanSetParentOfType(type, flags, interactionFlags);
	}
	
	override bool CanSetParentOfType(EEditableEntityType parentType, EEditableEntityFlag parentFlags, EEditableEntityInteractionFlag interactionFlags = int.MAX)
	{	
		return parentType == EEditableEntityType.CHARACTER
			|| parentType == EEditableEntityType.GROUP;
	}
};