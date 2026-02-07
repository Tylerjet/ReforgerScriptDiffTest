[BaseContainerProps()]
class SCR_EditableGroupInteraction: SCR_EditableEntityInteraction
{
	override bool CanSetParent(SCR_EditableEntityComponent parentEntity, EEditableEntityInteractionFlag interactionFlags = int.MAX)
	{
		if (!parentEntity)
			return true;
		
		EEditableEntityType type = parentEntity.GetEntityType();
		EEditableEntityFlag flags = parentEntity.GetEntityFlags();
		
		return CanSetParentOfType(type, flags, interactionFlags);
	}
	
	override bool CanSetParentOfType(EEditableEntityType parentType, EEditableEntityFlag parentFlags, EEditableEntityInteractionFlag interactionFlags = int.MAX)
	{		
		return parentType == EEditableEntityType.CHARACTER
			|| parentType == EEditableEntityType.GROUP
			|| parentType == EEditableEntityType.VEHICLE;             
	}
};