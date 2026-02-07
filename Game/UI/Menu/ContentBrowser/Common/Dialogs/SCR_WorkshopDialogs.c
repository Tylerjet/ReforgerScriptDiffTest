/*
Common dialogs in Workshop
// TODO: Cleanup and update looks
*/

class SCR_WorkshopDialogs
{
	static const ResourceName DIALOGS_CONFIG = "{26F075E5D30629E5}Configs/ContentBrowser/ContentBrowserDialogs.conf";
	static const string WIDGET_LIST = "AddonList";

	//------------------------------------------------------------------------------------------------
	static SCR_ConfigurableDialogUi CreateDialog(string presetName)
	{
		return SCR_ConfigurableDialogUi.CreateFromPreset(DIALOGS_CONFIG, presetName);
	}
}
