/*!
Classes for dialogs which can be used in any context.
*/

class SCR_CommonDialogs
{
	static const ResourceName DIALOGS_CONFIG = "{814FCA3CB7851F6B}Configs/Dialogs/CommonDialogs.conf";
	
	//---------------------------------------------------------------------------------------------
	static SCR_ConfigurableDialogUi CreateDialog(string presetName)
	{
		return SCR_ConfigurableDialogUi.CreateFromPreset(DIALOGS_CONFIG, presetName);
	}
	
	//---------------------------------------------------------------------------------------------
	static SCR_ConfigurableDialogUi CreateRequestErrorDialog()
	{
		return SCR_ConfigurableDialogUi.CreateFromPreset(DIALOGS_CONFIG, "request_error");
	}
	
	//---------------------------------------------------------------------------------------------
	static SCR_ConfigurableDialogUi CreateTimeoutOkDialog()
	{
		return SCR_ConfigurableDialogUi.CreateFromPreset(DIALOGS_CONFIG, "timeout_ok");
	}
	
	//---------------------------------------------------------------------------------------------
	static SCR_ConfigurableDialogUi CreateTimeoutTryAgainCancelDialog()
	{
		return SCR_ConfigurableDialogUi.CreateFromPreset(DIALOGS_CONFIG, "timeout_try_again_cancel");
	}
	
	//---------------------------------------------------------------------------------------------
	static SCR_ServicesStatusDialogUI CreateServicesStatusDialog()
	{
		SCR_ServicesStatusDialogUI dialog = new SCR_ServicesStatusDialogUI();
		return SCR_ServicesStatusDialogUI.Cast(SCR_ConfigurableDialogUi.CreateFromPreset(DIALOGS_CONFIG, "services_status", dialog));
	}
	
	//---------------------------------------------------------------------------------------------
	static SCR_ConfigurableDialogUi CreateTutorialDialog()
	{
		return SCR_ConfigurableDialogUi.CreateFromPreset(DIALOGS_CONFIG, "tutorial");
	}
	
	//---------------------------------------------------------------------------------------------
	static ServerHostingUI CreateServerHostingDialog()
	{
		ServerHostingUI dialog = new ServerHostingUI();
		return ServerHostingUI.Cast(SCR_ConfigurableDialogUi.CreateFromPreset(DIALOGS_CONFIG, "server_hosting", dialog));
	}
	
	//---------------------------------------------------------------------------------------------
	static AddonsToolsUI CreateModPresetsDialog()
	{
		AddonsToolsUI dialog = new AddonsToolsUI();
		return AddonsToolsUI.Cast(SCR_ConfigurableDialogUi.CreateFromPreset(DIALOGS_CONFIG, "mod_presets", dialog));
	}
	
	//---------------------------------------------------------------------------------------------
	static SCR_DownloadManager_Dialog CreateDownloadManagerDialog()
	{
		SCR_DownloadManager_Dialog dialog = new SCR_DownloadManager_Dialog();
		return SCR_DownloadManager_Dialog.Cast(SCR_ConfigurableDialogUi.CreateFromPreset(DIALOGS_CONFIG, "download_manager", dialog));
	}
	
	//---------------------------------------------------------------------------------------------
	static SCR_ConfigurableDialogUi CreateScenarioLoadDialog()
	{
		return SCR_ConfigurableDialogUi.CreateFromPreset(DIALOGS_CONFIG, "scenario_load");
	}
}

//---------------------------------------------------------------------------------------------
class SCR_ExitGameDialog : SCR_ConfigurableDialogUi
{
	//---------------------------------------------------------------------------------------------
	void SCR_ExitGameDialog()
	{
		SCR_ConfigurableDialogUi.CreateFromPreset(SCR_CommonDialogs.DIALOGS_CONFIG, "exit_game", this);
	}
	
	//---------------------------------------------------------------------------------------------
	override void OnConfirm()
	{		
		GetGame().RequestClose();
	}
}

//---------------------------------------------------------------------------------------------
class SCR_ExitGameWhileDownloadingDialog : SCR_ConfigurableDialogUi
{
	//---------------------------------------------------------------------------------------------
	void SCR_ExitGameWhileDownloadingDialog()
	{
		SCR_ConfigurableDialogUi.CreateFromPreset(SCR_CommonDialogs.DIALOGS_CONFIG, "exit_game_while_downloading", this);
	}
	
	//---------------------------------------------------------------------------------------------
	override void OnConfirm()
	{
		// Try to terminate all current downloads
		SCR_DownloadManager mgr = SCR_DownloadManager.GetInstance();
		if (mgr)
			mgr.EndAllDownloads();
		
		// Exit the game
		GetGame().RequestClose();
	}
}

//------------------------------------------------------------------------------------------------
//! Dialog which is shown when user wants to start a scenario while there are still downloads in progress.
//! When confirmed, downloads are paused/canceled, and the scenario is started.
class SCR_StartScenarioWhileDownloadingDialog : SCR_ConfigurableDialogUi
{
	protected ref MissionWorkshopItem m_Scenario;
	
	//---------------------------------------------------------------------------------------------
	static void CreateDialog(MissionWorkshopItem scenario)
	{
		SCR_StartScenarioWhileDownloadingDialog dialog = new SCR_StartScenarioWhileDownloadingDialog();
		dialog.SetScenario(scenario);
		
		SCR_ConfigurableDialogUi.CreateFromPreset(SCR_CommonDialogs.DIALOGS_CONFIG, "start_scenario_while_downloading", dialog);
	}
	
	//---------------------------------------------------------------------------------------------
	override void OnConfirm()
	{
		SCR_DownloadManager mgr = SCR_DownloadManager.GetInstance();
		if (mgr)
			mgr.EndAllDownloads();
		
		if (m_Scenario)
			m_Scenario.Play();
		
		super.OnConfirm();
	}
	
	//---------------------------------------------------------------------------------------------
	void SetScenario(MissionWorkshopItem scenario)
	{
		m_Scenario = scenario;
	}
}

//------------------------------------------------------------------------------------------------
class SCR_HostScenarioWhileDownloadingDialog : SCR_ConfigurableDialogUi
{
	protected ref MissionWorkshopItem m_Scenario;
	protected ref SCR_DSConfig m_DSConfig;
	
	//---------------------------------------------------------------------------------------------
	static void CreateDialog(MissionWorkshopItem scenario, SCR_DSConfig config)
	{
		SCR_HostScenarioWhileDownloadingDialog dialog = new SCR_HostScenarioWhileDownloadingDialog();
		dialog.SetScenario(scenario);
		dialog.SetConfig(config);
		
		SCR_ConfigurableDialogUi.CreateFromPreset(SCR_CommonDialogs.DIALOGS_CONFIG, "start_scenario_while_downloading", dialog);
	}
	
	//---------------------------------------------------------------------------------------------
	override void OnConfirm()
	{
		SCR_DownloadManager mgr = SCR_DownloadManager.GetInstance();
		if (mgr)
			mgr.EndAllDownloads();
		
		if (m_Scenario && m_DSConfig)
			m_Scenario.Host(m_DSConfig);
		
		super.OnConfirm();
	}
	
	//---------------------------------------------------------------------------------------------
	void SetScenario(MissionWorkshopItem scenario)
	{
		m_Scenario = scenario;
	}
	
	//---------------------------------------------------------------------------------------------
	void SetConfig(SCR_DSConfig config)
	{
		m_DSConfig = config;
	}
}

//------------------------------------------------------------------------------------------------
//! There is not enough storage on your hard drive. The space required is at least %1.
class SCR_NotEnoughStorageDialog : SCR_ConfigurableDialogUi
{
	protected float m_fSizeBytes;
	protected string m_sPresetMessage;
	
	//------------------------------------------------------------------------------------------------
	static SCR_NotEnoughStorageDialog Create(float sizeBytes)
	{
		SCR_NotEnoughStorageDialog dlg = new SCR_NotEnoughStorageDialog(sizeBytes);
		
		SCR_ConfigurableDialogUi.CreateFromPreset(
			SCR_CommonDialogs.DIALOGS_CONFIG,
			"error_not_enough_storage",
			dlg);
		
		return dlg;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Increase size displayed in dialog
	void AddToSize(float sizeBytes)
	{
		m_fSizeBytes += sizeBytes;
		
		string sizeStr = SCR_ByteFormat.GetReadableSize(m_fSizeBytes);
		string messageStr = WidgetManager.Translate(m_sPresetMessage, sizeStr);
		
		SetMessage(messageStr);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_ConfigurableDialogUiPreset preset)
	{
		// Set the message with formatted size
		string sizeStr = SCR_ByteFormat.GetReadableSize(m_fSizeBytes);
		
		m_sPresetMessage = preset.m_sMessage;
		string messageStr = WidgetManager.Translate(m_sPresetMessage, sizeStr);
		SetMessage(messageStr);
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_NotEnoughStorageDialog(float sizeBytes)
	{
		m_fSizeBytes = sizeBytes;
	}
}