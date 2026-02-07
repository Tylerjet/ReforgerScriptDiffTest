class SCR_MANW_Dialogs
{
	const string CONFIG = "{A6D2D86E123D905F}Configs/Dialogs/MANW_Dialogs.conf";
	
	//------------------------------------------------------------------------------------------------
	static void CreateBannerDialog()
	{
		SCR_ConfigurableDialogUi.CreateFromPreset(CONFIG, "BannerDialog", new SCR_MANW_BannerDialog);
	}
}