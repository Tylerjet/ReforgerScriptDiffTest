[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_EnableArsenalAttribute : SCR_BaseEditorAttribute
{
	//------------------------------------------------------------------------------------------------
	//~ Get first arsenal component on child
	protected SCR_ArsenalComponent GetArsenalFromChildren(IEntity parent)
	{
		IEntity child = parent.GetChildren();
		SCR_ArsenalComponent arsenalComponent;
		
		while (child)
		{
			arsenalComponent = SCR_ArsenalComponent.Cast(child.FindComponent(SCR_ArsenalComponent));
			if (arsenalComponent)
				return arsenalComponent;
			
			child = child.GetSibling();
		}
		
		//~ Not found
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	override void UpdateInterlinkedVariables(SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, bool isInit = false)
	{		
		//Set sub labels
		if (isInit)
		{
			manager.SetAttributeAsSubAttribute(SCR_ArsenalAmmunitionModeAttribute);
			manager.SetAttributeAsSubAttribute(SCR_ArsenalEquipmentEditorAttribute);
			manager.SetAttributeAsSubAttribute(SCR_ArsenalOutfitEditorAttribute);
			manager.SetAttributeAsSubAttribute(SCR_ArsenalWeaponRackEquipmentEditorAttribute);
			manager.SetAttributeAsSubAttribute(SCR_ArsenalWeaponRackWeaponsEditorAttribute);
			manager.SetAttributeAsSubAttribute(SCR_ArsenalWeaponsEditorAttribute);
		}
		
		bool enabled = var && var.GetBool();
		
		manager.SetAttributeEnabled(SCR_ArsenalAmmunitionModeAttribute, enabled);
		manager.SetAttributeEnabled(SCR_ArsenalEquipmentEditorAttribute, enabled);
		manager.SetAttributeEnabled(SCR_ArsenalOutfitEditorAttribute, enabled);
		manager.SetAttributeEnabled(SCR_ArsenalWeaponRackEquipmentEditorAttribute, enabled);
		manager.SetAttributeEnabled(SCR_ArsenalWeaponRackWeaponsEditorAttribute, enabled);
		manager.SetAttributeEnabled(SCR_ArsenalWeaponsEditorAttribute, enabled);
	}
	
	//------------------------------------------------------------------------------------------------
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity)
			return null;
		
		SCR_ArsenalComponent arsenalComponent = SCR_ArsenalComponent.Cast(editableEntity.GetOwner().FindComponent(SCR_ArsenalComponent));
		if (!arsenalComponent)
		{
			//~ If vehicle check if arsenal is on children
			if (editableEntity.GetEntityType() == EEditableEntityType.VEHICLE)
				arsenalComponent = GetArsenalFromChildren(editableEntity.GetOwner());
			
			if (!arsenalComponent)
				return null;
		}

		return SCR_BaseEditorAttributeVar.CreateBool(arsenalComponent.IsArsenalEnabled());
	}
	
	//------------------------------------------------------------------------------------------------
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var)
			return;
		
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity)
			return;
		
		SCR_ArsenalComponent arsenalComponent = SCR_ArsenalComponent.Cast(editableEntity.GetOwner().FindComponent(SCR_ArsenalComponent));
		if (!arsenalComponent)
		{
			//~ If vehicle check if arsenal is on children
			if (editableEntity.GetEntityType() == EEditableEntityType.VEHICLE)
				arsenalComponent = GetArsenalFromChildren(editableEntity.GetOwner());
				
			if (!arsenalComponent)
				return;
		}
		
		arsenalComponent.SetArsenalEnabled(var.GetBool());
	}
}
