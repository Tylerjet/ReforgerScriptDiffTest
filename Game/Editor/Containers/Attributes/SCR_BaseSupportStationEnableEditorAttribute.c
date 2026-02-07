[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_BaseSupportStationEnableEditorAttribute : SCR_BaseEditorAttribute
{
	//------------------------------------------------------------------------------------------------
	//~ Overwrite this
	protected bool IsValidSupportStation(ESupportStationType supportStationType)
	{
		Print("SCR_BaseSupportStationEnableEditorAttribute should never be used. Inherit instead!", LogLevel.ERROR);
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{					
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity)
			return null;

		SCR_BaseSupportStationComponent supportStation = SCR_BaseSupportStationComponent.Cast(editableEntity.GetOwner().FindComponent(SCR_BaseSupportStationComponent));
		if (supportStation && !IsValidSupportStation(supportStation.GetSupportStationType()))
			supportStation = null;
		
		if (!supportStation)
		{
			//~ If vehicle check if supportStation is on children
			if (editableEntity.GetEntityType() == EEditableEntityType.VEHICLE)
			{
				IEntity child = editableEntity.GetOwner().GetChildren();
				
				while (child)
				{
					supportStation = SCR_BaseSupportStationComponent.Cast(child.FindComponent(SCR_BaseSupportStationComponent));
					if (supportStation)
					{
						if (IsValidSupportStation(supportStation.GetSupportStationType()))
							break;
						else 
							supportStation = null;
					}

					child = child.GetSibling();
				}
			}
		}
		
		if (!supportStation)
			return null;
		
		return SCR_BaseEditorAttributeVar.CreateBool(supportStation.IsEnabled());
	}
	
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{						
		if (!var)
			return;
		
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity)
			return;
		
		SCR_BaseSupportStationComponent supportStation = SCR_BaseSupportStationComponent.Cast(editableEntity.GetOwner().FindComponent(SCR_BaseSupportStationComponent));
		if (supportStation && !IsValidSupportStation(supportStation.GetSupportStationType()))
			supportStation = null;
		
		if (!supportStation)
		{
			//~ If vehicle check if supportStation is on children
			if (editableEntity.GetEntityType() == EEditableEntityType.VEHICLE)
			{
				IEntity child = editableEntity.GetOwner().GetChildren();
				
				while (child)
				{
					supportStation = SCR_BaseSupportStationComponent.Cast(child.FindComponent(SCR_BaseSupportStationComponent));
					if (supportStation)
					{
						if (IsValidSupportStation(supportStation.GetSupportStationType()))
							break;
						else 
							supportStation = null;
					}
					
					child = child.GetSibling();
				}
			}
		}
		
		if (!supportStation)
			return;
		
		supportStation.SetEnabled(var.GetBool());
	}
};