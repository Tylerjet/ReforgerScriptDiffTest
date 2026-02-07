[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_ArsenalWeaponsEditorAttribute : SCR_ArsenalBaseEditorAttribute
{
	override void UpdateInterlinkedVariables(SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, bool isInit = false)
	{
		if (!var)
			return;
		
		//Set sub labels
		if (isInit)
			manager.SetAttributeAsSubAttribute(SCR_ArsenalAmmunitionModeAttribute);
		
		manager.SetAttributeEnabled(SCR_ArsenalAmmunitionModeAttribute, var.GetInt() > 0);
		
		super.UpdateInterlinkedVariables(var, manager, isInit);
	}
};