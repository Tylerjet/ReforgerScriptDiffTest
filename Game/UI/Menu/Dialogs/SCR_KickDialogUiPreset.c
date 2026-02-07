//! Server kick message specific dialog 

//---------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_BaseContainerCustomTitleField("m_sTag")]
class SCR_KickDialogUiPreset : SCR_ConfigurableDialogUiPreset
{
	[Attribute("true", UIWidgets.CheckBox, "If true server browser will attempt to find last server to reconnect")]
	bool m_bCanBeReconnected;
	
	[Attribute("{ED17F4E49672E591}Configs/ServerBrowser/DialogPresets/ServerBrowserReconnectDialog.conf", UIWidgets.Auto, "What dialog should be displayed for reconnect", "conf")]
	ref SCR_ConfigurableDialogUiPreset m_ReconnectPreset;
};