//
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_ArsenalBaseEditorAttribute : SCR_LoadoutBaseEditorAttribute
{	
	//~ Checks if the arsenal has the given arsenal group flag enabled. Else will not show this Arsenal Attribute
	[Attribute("2", UIWidgets.ComboBox, "Editable Arsenal Group", "", ParamEnumArray.FromEnum(SCR_EArsenalAttributeGroup) )] 
	protected SCR_EArsenalAttributeGroup m_eEditableAttributeGroups;
	
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity)
			return null;
		
		SCR_ArsenalComponent arsenalComponent = SCR_ArsenalComponent.Cast(editableEntity.GetOwner().FindComponent(SCR_ArsenalComponent));
		if (!arsenalComponent)
			return null;
		
		SCR_EArsenalAttributeGroup arsenalGroups = arsenalComponent.GetEditableAttributeGroups();
		if (!(arsenalGroups & m_eEditableAttributeGroups))
			return null;
		
		return SCR_BaseEditorAttributeVar.CreateInt(arsenalComponent.GetSupportedArsenalItemTypes());
	}
	
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var)
			return;
		
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity)
			return;
		
		SCR_ArsenalComponent arsenalComponent = SCR_ArsenalComponent.Cast(editableEntity.GetOwner().FindComponent(SCR_ArsenalComponent));
		if (!arsenalComponent)
			return;
		
		//~ Make sure Values are created if it didn't yet exist
		if (m_aValues.IsEmpty())
			CreatePresets();
		
		//Get the current and add/remove flags as all arsenal attributes edit the same enum and would otherwise override itself
		SCR_EArsenalItemType arsenalFlags = arsenalComponent.GetSupportedArsenalItemTypes();
		int newArsenalflags = var.GetInt();

		foreach(SCR_EditorAttributeFloatStringValueHolder value: m_aValues)
		{
			//Check if flag is true or not
			if (newArsenalflags & (int)value.GetFloatValue())
				arsenalFlags |= (int)value.GetFloatValue();
			else 
				arsenalFlags &= ~(int)value.GetFloatValue();
		}

		//Update the arsenal
		arsenalComponent.SetSupportedArsenalItemTypes(arsenalFlags);
	}
};