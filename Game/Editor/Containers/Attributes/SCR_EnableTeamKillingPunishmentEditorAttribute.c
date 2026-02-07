[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_EnableTeamKillingPunishmentEditorAttribute : SCR_BaseEditorAttribute
{
	//------------------------------------------------------------------------------------------------
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		return null;
		
		if (!IsGameMode(item))
			return null;
		
		SCR_AdditionalGameModeSettingsComponent additionalGameSettings = SCR_AdditionalGameModeSettingsComponent.GetInstance();
		if (!additionalGameSettings)
			return null;
		
		return SCR_BaseEditorAttributeVar.CreateBool(additionalGameSettings.IsTeamKillingPunished());
	}
	
	//------------------------------------------------------------------------------------------------
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var)
			return;
		
		SCR_AdditionalGameModeSettingsComponent additionalGameSettings = SCR_AdditionalGameModeSettingsComponent.GetInstance();
		if (!additionalGameSettings)
			return;
		
		additionalGameSettings.SetEnableTeamKillPunishment_S(var.GetBool(), playerID);
	}
}
