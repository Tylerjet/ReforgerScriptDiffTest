class SCR_EditorSaveLoadUIComponent : SCR_SaveDialogUIComponent
{
	//------------------------------------------------------------------------------------------------
	override protected void OnConfirm(SCR_InputButtonComponent button, string actionName)
	{
		// Save dialogs
		if (!m_bIsLoad)
		{
			string customName = m_SaveNameInput.GetValue();
			if (!GetGame().GetSaveManager().FileExists(m_eWriteSaveType, customName))
			{
				//--- Creating a new file - save directly
				OnConfirmPrompt();
				return;
			}

			//--- Confirm prompt
			m_ConfirmPrompt = SCR_ConfigurableDialogUi.CreateFromPreset(SCR_CommonDialogs.DIALOGS_CONFIG, m_sConfirmPrompt);
			m_ConfirmPrompt.m_OnConfirm.Insert(OnConfirmPrompt);

			m_ConfirmPrompt.SetTitle(m_SaveNameInput.GetValue());
			return;
		}

		//--- Loading a file leads to restart, ask first
		string fileName;
		Widget focusedEntry = GetGame().GetWorkspace().GetFocusedWidget();
		
		foreach (SCR_SaveDialogEntry entry : m_mEntries)
		{
			if (entry.m_wEntry == focusedEntry)
			{
				fileName = entry.m_sFileName;
				break;
			}
		}

		if (fileName.IsEmpty())
			return;

		string displayName = m_mEntryNames[m_sSelectedFileName];
		SCR_MetaStruct meta = GetGame().GetSaveManager().GetMeta(fileName);
		if (meta)
		{
			if (!meta.IsVersionCompatible())
			{
				// Warning - incompatible version
				m_LoadBadVersionPrompt = SCR_ConfigurableDialogUi.CreateFromPreset(SCR_CommonDialogs.DIALOGS_CONFIG, m_sLoadBadVersionPrompt);
				m_LoadBadVersionPrompt.m_OnConfirm.Insert(LoadEntry);
				m_LoadBadVersionPrompt.SetTitle(displayName);
				return;
			}

			if (!meta.AreAddonsCompatible())
			{
				// Warning - incompatible addons
				m_LoadBadAddonsPrompt = SCR_ConfigurableDialogUi.CreateFromPreset(SCR_CommonDialogs.DIALOGS_CONFIG, m_sLoadBadAddonsPrompt);
				m_LoadBadAddonsPrompt.m_OnConfirm.Insert(LoadEntry);
				m_LoadBadAddonsPrompt.SetTitle(displayName);
				return;
			}
		}

		//--- Confirm prompt
		m_ConfirmPrompt = SCR_ConfigurableDialogUi.CreateFromPreset(SCR_CommonDialogs.DIALOGS_CONFIG, m_sConfirmPrompt);
		m_ConfirmPrompt.m_OnConfirm.Insert(OnConfirmPrompt);
		m_ConfirmPrompt.SetTitle(displayName);
	}

	//------------------------------------------------------------------------------------------------
	override protected void SaveEntry()
	{
		string customName = m_SaveNameInput.GetValue();
		GetGame().GetSaveManager().Save(m_eWriteSaveType, customName);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnDeletePrompt()
	{
		SCR_SaveDialogEntry entry = m_mEntries[m_sSelectedFileName];
		
		//--- Delete the file
		GetGame().GetSaveManager().Delete(m_sSelectedFileName, entry.m_bIsDownloaded);

		//--- Update GUI
		super.OnDeletePrompt();
	}

	//------------------------------------------------------------------------------------------------
	override protected void LoadEntry()
	{
		SCR_SaveDialogEntry selectedEntry = GetEntryByWidget(m_wLastFocusedEntry);
		if (!selectedEntry)
			return;
		
		string fileName = m_mEntries.GetKeyByValue(selectedEntry);
		
		if (!fileName.IsEmpty())
		{
			SCR_SaveManagerCore saveManager = GetGame().GetSaveManager();

			saveManager.RestartAndLoad(fileName);

			SCR_ServerSaveRequestCallback uploadCallback = saveManager.GetUploadCallback();
			if (uploadCallback)
			{
				uploadCallback.GetEventOnResponse().Insert(OnLoadEntryUploadResponse);
				m_LoadingOverlay = SCR_LoadingOverlayDialog.Create();
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void SelectEntry(Widget w, string fileName)
	{
		string customName = GetGame().GetSaveManager().GetCustomName(fileName);
		m_SaveNameInput.SetValue(customName);

		super.SelectEntry(w, fileName);
	}

	//------------------------------------------------------------------------------------------------
	override protected void UpdateButtons()
	{
		string customName = m_SaveNameInput.GetValue();
		bool isValid = !customName.IsEmpty();
		bool isOverride = !m_bIsLoad && customName && GetGame().GetSaveManager().FileExists(m_eWriteSaveType, customName);

		if (m_bIsLoad)
			m_DeleteButton.SetEnabled(m_sSelectedFileName && GetGame().GetSaveManager().FileExists(m_sSelectedFileName));
		else
			m_DeleteButton.SetEnabled(isOverride);

		m_OverrideButton.SetVisible(isOverride, false);
		m_OverrideButton.SetEnabled(isOverride && isValid);

		m_ConfirmButton.SetVisible(!isOverride, false);
		m_ConfirmButton.SetEnabled(!isOverride && isValid);
	}

	//------------------------------------------------------------------------------------------------
	override protected int FileCount(array<string> fileNames)
	{
		return GetGame().GetSaveManager().GetLocalSaveFiles(fileNames, m_eReadSaveTypes, m_bCurrentMissionOnly);
	}

	//------------------------------------------------------------------------------------------------
	override protected string EntryName(string fileName)
	{
		return GetGame().GetSaveManager().GetCustomName(fileName);
	}

	//------------------------------------------------------------------------------------------------
	override protected SCR_MetaStruct EntryMeta(string fileName)
	{
		return GetGame().GetSaveManager().GetMeta(fileName);
	}

	//------------------------------------------------------------------------------------------------
	override protected SCR_UIInfo EntryUIInfo(string fileName)
	{
		return GetGame().GetSaveManager().GetSaveTypeInfo(fileName);
	}
}
