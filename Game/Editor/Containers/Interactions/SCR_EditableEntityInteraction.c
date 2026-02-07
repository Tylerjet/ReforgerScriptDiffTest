[BaseContainerProps()]
class SCR_EditableEntityInteraction
{
	/*!
	Check if the entity can be moved to intended parent.
	\param parentEntity New parent. Null when evaluating root.
	\params interactionFlags Flags defining details about the interaction (all are enabled if undefined)
	\return True if interaction is possible
	*/
	bool CanSetParent(SCR_EditableEntityComponent parentEntity, EEditableEntityInteractionFlag interactionFlags = int.MAX)
	{
		if (!SCR_Enum.HasFlag(interactionFlags, EEditableEntityInteractionFlag.DELEGATE) || !SCR_Enum.HasFlag(interactionFlags, EEditableEntityInteractionFlag.LAYER_EDITING))
			return false;
		
		if (!parentEntity)
			return true;
		
		EEditableEntityType type = parentEntity.GetEntityType();
		EEditableEntityFlag flags = parentEntity.GetEntityFlags();
	
		return CanSetParentOfType(type, flags, interactionFlags);
	}
	
	/*!
	Check if the entity can be moved to intended parent of given type.
	\param EEditableEntityType entity type of parent
	\param EEditableEntityFlag entity flags of parent
	\params interactionFlags Flags defining details about the interaction (all are enabled if undefined)
	\return True if interaction is possible
	*/
	bool CanSetParentOfType(EEditableEntityType parentType, EEditableEntityFlag parentFlags, EEditableEntityInteractionFlag interactionFlags = int.MAX)
	{
		if (!SCR_Enum.HasFlag(interactionFlags, EEditableEntityInteractionFlag.DELEGATE))
			return false;
		
		if ((parentFlags & EEditableEntityFlag.LAYER) != EEditableEntityFlag.LAYER)
			return false;
		
		return parentType == EEditableEntityType.GENERIC;
		//	|| parentType == EEditableEntityType.VEHICLE; //--- ToDo: Attach to vehicles
	}
	
	/*!
	Check if new layer can be created for entity
	\param EEditableEntityType new layer type
	\param newLayerFlags new layer m_Flags
	\param parentEntity parent entity
	\param CheckParentEntity if should check if new layer can be added to parent
	\return bool if the given layer can be created for the entity
	*/
	bool CanCreateParentFor(EEditableEntityType newLayerType, EEditableEntityFlag newLayerFlags, SCR_EditableEntityComponent parentEntity, bool CheckParentEntity = true)
	{
		if (CheckParentEntity && parentEntity)
		{
			SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
			
			if (core)
			{
				if (!core.CanSetParent(EEditableEntityType.GENERIC, parentEntity))
					return false;
			}
		}
		
		return CanSetParentOfType(newLayerType, newLayerFlags);
	}
};