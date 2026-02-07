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
		return !SCR_Enum.HasFlag(interactionFlags, EEditableEntityInteractionFlag.PLACING) //--- Don't allow any interaction while placing, it would add the waypoint to the target instead
			&&
			(
				parentType == EEditableEntityType.CHARACTER
				|| parentType == EEditableEntityType.GROUP
			);
	}
};