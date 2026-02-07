class SCR_WorkshopAddonManagerDialogs
{
	const string DIALOGS_CONFIG = "{F96E0133AC81125A}UI/Dialogs.conf";
}

class SCR_WorkshopDialogCopyToClipboard : SCR_ConfigurableDialogUi
{
	string m_sText;
	
	void SCR_WorkshopDialogCopyToClipboard(string text)
	{
		m_sText = text;
		SCR_ConfigurableDialogUi.CreateFromPreset(SCR_WorkshopAddonManagerDialogs.DIALOGS_CONFIG, "copy_to_clipboard", this);
		TextWidget tw = TextWidget.Cast(GetRootWidget().FindAnyWidget("MessageInScroll"));
		tw.SetText(text);
	}
	
	override void OnConfirm()
	{
		System.ExportToClipboard(m_sText);
	}
}

// Basic error dialog with a message
class SCR_WorkshopErrorDialog : SCR_ConfigurableDialogUi
{
	void SCR_WorkshopErrorDialog(string message)
	{
		SCR_ConfigurableDialogUi.CreateFromPreset(SCR_WorkshopAddonManagerDialogs.DIALOGS_CONFIG, "error", this);
		SetMessage(message);
	}
}



// Dialog which also stores reference to addon preset
class SCR_WorkshopPresetConfirmDialog : SCR_ConfigurableDialogUi
{
	// "preset" term is used both in WEXT and Configurable Dialogs, it's a bit confusing
	
	// We store ptr to preset which we are going to delete/create,
	// when ok btn is presset we will use it
	ref SCR_WorkshopAddonPreset m_Preset;
	string m_sPresetName;
	
	void SCR_WorkshopPresetConfirmDialog(SCR_WorkshopAddonPreset preset, string presetName, string dlgPreset)
	{
		SCR_ConfigurableDialogUi.CreateFromPreset(SCR_WorkshopAddonManagerDialogs.DIALOGS_CONFIG, dlgPreset, this);
		m_Preset = preset;
		m_sPresetName = presetName;
	}
	
	static SCR_WorkshopPresetConfirmDialog CreateDeletePresetDialog(string presetName)
	{
		return new SCR_WorkshopPresetConfirmDialog(null, presetName, "delete_preset");
	}
	
	static SCR_WorkshopPresetConfirmDialog CreateOverridePresetDialog(SCR_WorkshopAddonPreset preset)
	{
		return new SCR_WorkshopPresetConfirmDialog(preset, preset.GetName(), "override_preset");
	}
}

// Dialog which shows error and lists addons which we failed to load in a preset
class SCR_WorkshopErrorPresetLoadDialog : SCR_ConfigurableDialogUi
{
	protected SCR_DownloadConfirmationDialog m_Dialog;
	
	void SCR_WorkshopErrorPresetLoadDialog(array<ref SCR_WorkshopAddonPresetAddonMeta> notFoundAddons)
	{
		SCR_ConfigurableDialogUi.CreateFromPreset(SCR_WorkshopAddonManagerDialogs.DIALOGS_CONFIG, "error_preset_load", this);
		
		string msg = "";
		
		foreach (SCR_WorkshopAddonPresetAddonMeta meta : notFoundAddons)
		{
			msg = msg + string.Format("%1 %2\n", meta.GetGuid(), meta.GetName());
		}
		
		SetMessage(msg);
		
		
		/*array<ref SCR_WorkshopItem>
		
		array<ref SCR_WorkshopItem> notFoundItems = SCR_AddonManager.SelectItemsOr(
			m_ModManager.GetRoomItemsScripted(), EWorkshopItemQuery.NOT_OFFLINE
		);
		*/
		
		/*
		array<ref Tuple2<SCR_WorkshopItem, string>> notFoundItems = new array<ref Tuple2<SCR_WorkshopItem, string>>;
		
		foreach (SCR_WorkshopAddonPresetAddonMeta meta : notFoundAddons)
		{
			string itemGuid = meta.GetGuid();
			Print("itemGuid: " + itemGuid);
			WorkshopItem item = GetGame().GetBackendApi().GetWorkshop().FindItem(itemGuid);
			
			//SCR_WorkshopItem item = SCR_AddonManager.GetInstance().GetItem(itemGuid);
			if (!item)
				continue;
			
			SCR_WorkshopItem scrItem = SCR_AddonManager.GetInstance().Register(item);
			if (!scrItem)
				continue;
			
			/*Dependency dependency = scrItem.GetDependency();
			if (!dependency)
				continue;
			
			Revision revision = scrItem.GetLatestRevision(); 
			string version = "1.0.0";
			
			if (revision)
				version = revision.GetVersion(); 
			
			ref Tuple2<SCR_WorkshopItem, string>> toDownload = new Tuple2<SCR_WorkshopItem, string>>(scrItem, version);
			notFoundItems.Insert(toDownload);
		}
		
		
		m_Dialog = SCR_DownloadConfirmationDialog.CreateForAddons(notFoundItems, false);
		if (!m_Dialog)
			return;
		
		m_Dialog.m_OnDownloadConfirmed.Insert(OnConfirmAddonsDownload);
		//SetMessage(msg);
		*/
	}
	
	protected int m_iDownloading;
	
	//---------------------------------------------------------------------------------------------------
	protected void OnConfirmAddonsDownload(SCR_DownloadConfirmationDialog dialog)
	{
		// Listen to downalod actinos 
		array<ref SCR_WorkshopItemAction> actions = dialog.GetActions();
		array<ref SCR_WorkshopItemActionDownload> downloads = new array<ref SCR_WorkshopItemActionDownload>;
		
		foreach (ref SCR_WorkshopItemAction act : actions)
		{
			//act.m_OnChanged.Insert(OnDownloadActionChange);
			act.m_OnCompleted.Insert(OnModDownloaded);
			
			// Insert download action OnReportedModsFix
			SCR_WorkshopItemActionDownload downloadAction = SCR_WorkshopItemActionDownload.Cast(act);
			if (downloadAction)
				downloads.Insert(downloadAction);
		}
		
		m_iDownloading = actions.Count();
		// Set downloads
		//m_ModManager.SetDownloadActions(downloads);
		
		dialog.m_OnDownloadConfirmed.Remove(OnConfirmAddonsDownload);
		
		GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.DownloadManagerDialog);
	}
	
	//---------------------------------------------------------------------------------------------------
	protected void OnModDownloaded(SCR_WorkshopItemAction action)
	{
		//array<ref SCR_WorkshopItem> items = m_ModManager.GetRoomItemsToUpdate();
		m_iDownloading--;
		
		// Invoke on all downloaded
		/*if (m_iDownloadedCount == items.Count())
			m_OnDownloadComplete.Invoke(m_JoinRoom);*/
		
		if (m_iDownloading  > 0)
			return;
		
		
		// Stop listening when completed 
		//action.m_OnChanged.Remove(OnDownloadActionChange);
		action.m_OnCompleted.Remove(OnModDownloaded);
	}
}