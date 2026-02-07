[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_ArsenalUseSuppliesEditorAttribute : SCR_BaseEditorAttribute
{
	//------------------------------------------------------------------------------------------------
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{					
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity || !editableEntity.GetOwner())
			return null;
		
		//~ Get Arsenal
		SCR_ArsenalComponent arsenalComponent = SCR_ArsenalComponent.Cast(editableEntity.GetOwner().FindComponent(SCR_ArsenalComponent));
		
		//~ If vehicle check if arsenal is on children
		if (!arsenalComponent && editableEntity.GetEntityType() == EEditableEntityType.VEHICLE)
			arsenalComponent = SCR_ArsenalComponent.GetArsenalComponentFromChildren(editableEntity.GetOwner());
		
		if (!arsenalComponent)
			return null;
		
		return SCR_BaseEditorAttributeVar.CreateBool(arsenalComponent.IsArsenalUsingSupplies());
	}
	
	//------------------------------------------------------------------------------------------------
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{						
		if (!var)
			return;
		
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity || !editableEntity.GetOwner())
			return;
		
		//~ Get arsenal component
		SCR_ArsenalComponent arsenalComponent = SCR_ArsenalComponent.Cast(editableEntity.GetOwner().FindComponent(SCR_ArsenalComponent));
		
		//~ If vehicle check if arsenal is on children
		if (!arsenalComponent && editableEntity.GetEntityType() == EEditableEntityType.VEHICLE)
			arsenalComponent = SCR_ArsenalComponent.GetArsenalComponentFromChildren(editableEntity.GetOwner());
		
		if (!arsenalComponent)
			return;
		
		arsenalComponent.SetArsenalUseSupplies(var.GetBool());
	}
}
