/*!
Base dialog class for save and load dialogs handling
Fill data, display list, select entries, load, delete
*/
class SCR_SaveDialogUIComponent : SCR_ScriptedWidgetComponent
{
	[Attribute()]
	protected bool m_bIsLoad;

	[Attribute(ESaveType.USER.ToString(), uiwidget: UIWidgets.Flags, enums: ParamEnumArray.FromEnum(ESaveType))]
	protected ESaveType m_eReadSaveTypes;

	[Attribute(ESaveType.USER.ToString(), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ESaveType))]
	protected ESaveType m_eWriteSaveType;

	[Attribute()]
	protected bool m_bCurrentMissionOnly;

	[Attribute("SaveScroller")]
	protected string m_sScrollWidgetName;

	[Attribute("SaveList")]
	protected string m_sListWidgetName;

	[Attribute("SaveNameInput")]
	protected string m_sNameInputWidgetName;

	[Attribute("ExitButton")]
	protected string m_sCloseButtonWidgetName;

	[Attribute("DeleteButton")]
	protected string m_sDeleteButtonWidgetName;

	[Attribute("OverrideButton")]
	protected string m_sOverrideButtonWidgetName;

	[Attribute("ConfirmButton")]
	protected string m_sConfirmButtonWidgetName;

	[Attribute("ConfirmPrompt")]
	protected string m_sConfirmPromptWidgetName;

	[Attribute("DeletePrompt")]
	protected string m_sDeletePromptWidgetName;

	[Attribute("LoadBadVersionPrompt")]
	protected string m_sLoadBadVersionPromptWidgetName;

	[Attribute("LoadBadAddonsPrompt")]
	protected string m_sLoadBadAddonsPromptWidgetName;

	[Attribute("EntryName")]
	protected string m_sEntryNameWidgetName;

	[Attribute("SaveMetaLine")]
	protected string m_sEntryMeta;

	[Attribute("PreviewDate")]
	protected string m_sEntryDateWidgetName;

	[Attribute("PreviewTime")]
	protected string m_sEntryTimeWidgetName;

	[Attribute("PreviewWorld")]
	protected string m_sEntryMissionNameWidgetName;

	[Attribute("EntryVersion")]
	protected string m_sEntryVersionWidgetName;

	[Attribute("PreviewImage")]
	protected string m_sEntryImageWidgetName;

	[Attribute("SaveImage")]
	protected string m_sEntryIconWidgetName;

	[Attribute("false", UIWidgets.CheckBox)]
	protected bool m_bVerboseDate;

	[Attribute(uiwidget: UIWidgets.ResourceNamePicker, params: "layout")]
	protected ResourceName m_sCreateLayout;

	[Attribute(uiwidget: UIWidgets.ResourceNamePicker, params: "layout")]
	protected ResourceName m_sEntryLayout;

	[Attribute("session_save_override")]
	protected string m_sConfirmPrompt;

	[Attribute("session_delete")]
	protected string m_sDeletePrompt;

	[Attribute("session_load_bad_version")]
	protected string m_sLoadBadVersionPrompt;

	[Attribute("session_load_bad_addons")]
	protected string m_sLoadBadAddonsPrompt;
	
	[Attribute("save-published")]
	protected string m_sDownloadIconName;

	protected ScrollLayoutWidget m_wScroll;
	protected Widget m_wList;
	protected Widget m_wLastFocusedEntry;

	protected SCR_InputButtonComponent m_DeleteButton;
	protected SCR_InputButtonComponent m_OverrideButton;
	protected SCR_InputButtonComponent m_ConfirmButton;
	protected SCR_EditBoxComponent m_SaveNameInput;

	protected SCR_ConfigurableDialogUi m_ConfirmPrompt;
	protected SCR_ConfigurableDialogUi m_DeletePrompt;
	protected SCR_ConfigurableDialogUi m_LoadBadVersionPrompt;
	protected SCR_ConfigurableDialogUi m_LoadBadAddonsPrompt;
	protected SCR_LoadingOverlayDialog m_LoadingOverlay;

	protected float m_fSliderPosY = -1;
	protected Widget m_wSelectedWidget;
	protected string m_sSelectedFileName;

	protected ref map<string, ref SCR_SaveDialogEntry> m_mEntries = new map<string, ref SCR_SaveDialogEntry>();
	protected ref map<string, string> m_mEntryNames = new map<string, string>(); //--- file name, display name
	protected ref array<Widget> m_aEntriesHidden = {};
	protected ref array<Widget> m_aEntriesToShow = {};

	//////////////////////////////////////////////////////////////////////////////////////////
	// Control buttons
	//////////////////////////////////////////////////////////////////////////////////////////

	//------------------------------------------------------------------------------------------------
	protected void OnClose(SCR_InputButtonComponent button, string actionName)
	{
		CloseMenu();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnConfirm(SCR_InputButtonComponent button, string actionName);

	//------------------------------------------------------------------------------------------------
	protected void OnConfirmPrompt()
	{
		if (m_bIsLoad)
			LoadEntry();
		else
			SaveEntry();

		CloseMenu();
	}

	//------------------------------------------------------------------------------------------------
	//! Callback on clicking delete button or actoin
	protected void OnDelete(SCR_InputButtonComponent button, string actionName)
	{
		// Open delete dialog
		m_DeletePrompt = SCR_ConfigurableDialogUi.CreateFromPreset(SCR_CommonDialogs.DIALOGS_CONFIG, m_sDeletePrompt); //--- ToDo: Unique tag
		m_DeletePrompt.m_OnConfirm.Insert(OnDeletePrompt);

		// Setup string
		string displayName = m_mEntryNames[m_sSelectedFileName];
		m_DeletePrompt.SetTitle(displayName);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDeletePrompt()
	{
		SCR_SaveDialogEntry entryToRemove;
		foreach(SCR_SaveDialogEntry entry : m_mEntries)
		{
			if (entry.m_wEntry == m_wSelectedWidget)
			{
				entryToRemove = entry;
				break;
			}
		}
		
		//--- Update GUI
		m_mEntries.Remove(entryToRemove.m_sFileName);
		m_mEntryNames.Remove(m_sSelectedFileName);

		m_wSelectedWidget.RemoveFromHierarchy();
		SelectEntry(null, string.Empty);
	}

	//------------------------------------------------------------------------------------------------
	protected void CloseMenu()
	{
		//--- Confirmation prompt opened, ignore
		if (GetGame().GetWorkspace().GetModal() != m_wRoot)
			return;

		MenuBase menu = MenuBase.Cast(m_wRoot.FindHandler(MenuBase));
		menu.Close();
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	// Main operations
	//////////////////////////////////////////////////////////////////////////////////////////

	//------------------------------------------------------------------------------------------------
	protected void SaveEntry();

	//------------------------------------------------------------------------------------------------
	protected void LoadEntry();

	//------------------------------------------------------------------------------------------------
	protected void OnLoadEntryUploadResponse(SCR_BackendCallback callback)
	{
		callback.GetEventOnResponse().Remove(OnLoadEntryUploadResponse);

		if (callback.GetResponseType() != EBackendCallbackResponse.SUCCESS)
		{
			SCR_CommonDialogs.CreateRequestErrorDialog();

			if (m_LoadingOverlay)
				m_LoadingOverlay.Close();
		}

		CloseMenu();
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	// Interaction
	//////////////////////////////////////////////////////////////////////////////////////////

	//------------------------------------------------------------------------------------------------
	protected void SelectEntry(Widget w, string fileName)
	{
		m_wSelectedWidget = w;
		m_sSelectedFileName = fileName;

		UpdateButtons();
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateButtons();

	//------------------------------------------------------------------------------------------------
	protected void OnFrame()
	{
		//--- Ignore if all entries were shown
		if (m_aEntriesHidden.IsEmpty())
		{
			GetGame().GetCallqueue().Remove(OnFrame);
			return;
		}

		float sliderPosX, sliderPosY;
		m_wScroll.GetSliderPos(sliderPosX, sliderPosY);

		if (sliderPosY != m_fSliderPosY)
		{
			float scrollPosX, scrollPosY, scrollSizeW, scrollSizeH;
			m_wScroll.GetScreenPos(scrollPosX, scrollPosY);
			m_wScroll.GetScreenSize(scrollSizeW, scrollSizeH);

			//--- Widget not loaded yet, terminate
			if (scrollSizeH == 0)
				return;

			//--- Find widgets in view
			foreach (int i, Widget entryWidget : m_aEntriesHidden)
			{
				//--- Already shown
				if (entryWidget.GetOpacity() > 0)
					continue;

				float posX, posY, sizeW, sizeH;
				entryWidget.GetScreenPos(posX, posY);
				entryWidget.GetScreenSize(sizeW, sizeH);

				if ((posY + sizeH) > scrollPosY && posY < (scrollPosY + scrollSizeH))
				{
					//--- When the entry is in the scrolled view, mark it for showing
					if (!m_aEntriesToShow.Contains(entryWidget))
						m_aEntriesToShow.Insert(entryWidget);
				}
				else
				{
					//--- When the entry is not in the scrolled view, removing it from queue of entries to show again (to prioritize entries that are actually shown)
					m_aEntriesToShow.RemoveItemOrdered(entryWidget);
				}
			}
		}

		//--- Process the queue of entries to show, with one entry per frame
		if (!m_aEntriesToShow.IsEmpty())
			DisplaySaveEntries();

		m_fSliderPosY = sliderPosY;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DisplaySaveEntries()
	{
		Widget entryWidget = m_aEntriesToShow[0];
		if (entryWidget.GetOpacity() == 0)
		{
			TextWidget entryNameWidget = TextWidget.Cast(entryWidget.FindAnyWidget(m_sEntryNameWidgetName));
			ImageWidget entryImageWidget = ImageWidget.Cast(entryWidget.FindAnyWidget(m_sEntryImageWidgetName));
			ImageWidget entryIconWidget = ImageWidget.Cast(entryWidget.FindAnyWidget(m_sEntryIconWidgetName));

			SCR_SaveDialogEntry entry = GetEntryByWidget(entryWidget);
			if (!entry)
				return;
			
			string fileName = entry.m_sFileName;
			string entryName = EntryName(fileName);

			SCR_MetaStruct meta = EntryMeta(fileName);
			if (meta && meta.IsValid())
			{
				int y, m, d, hh, mm;
				meta.GetDateAndTime(y, m, d, hh, mm);

				SCR_UIInfo info = EntryUIInfo(fileName);
				string displayName = entryName;
				Color imageColor = Color.FromInt(Color.WHITE);
				if (info)
				{
					info.SetIconTo(entryIconWidget);
					
					if (GetGame().GetSaveManager().IsDownloaded(fileName))
					{
						entry.m_bIsDownloaded = true;
						entryIconWidget.LoadImageFromSet(0, UIConstants.ICONS_IMAGE_SET, m_sDownloadIconName);
					}
					
					displayName = string.Format("%1 %2", info.GetName(), displayName);

					SCR_ColorUIInfo colorInfo = SCR_ColorUIInfo.Cast(info);
					if (colorInfo)
						imageColor = colorInfo.GetColor();
				}
				m_mEntryNames.Insert(fileName, displayName);

				entryNameWidget.SetText(displayName);

				TextWidget entryDateWidget = TextWidget.Cast(entryWidget.FindAnyWidget(m_sEntryDateWidgetName));
				entryDateWidget.SetText(SCR_DateTimeHelper.GetDateString(d, m, y, m_bVerboseDate));

				TextWidget entryTimeWidget = TextWidget.Cast(entryWidget.FindAnyWidget(m_sEntryTimeWidgetName));
				entryTimeWidget.SetText(SCR_FormatHelper.GetTimeFormattingHoursMinutes(hh, mm));

				TextWidget entryVersionWidget = TextWidget.Cast(entryWidget.FindAnyWidget(m_sEntryVersionWidgetName));
				string versionText;
				bool isVersionCompatible = meta.IsVersionCompatible(versionText);
				entryVersionWidget.SetText(versionText);
				if (!isVersionCompatible)
					entryVersionWidget.SetColor(UIColors.WARNING);

				ResourceName headerResourceName = meta.GetHeaderResource();
				if (!headerResourceName.IsEmpty())
				{
					Resource missionHeaderResource = Resource.Load(headerResourceName);
					if (missionHeaderResource.IsValid())
					{
						SCR_MissionHeader missionHeader = SCR_MissionHeader.Cast(BaseContainerTools.CreateInstanceFromContainer(missionHeaderResource.GetResource().ToBaseContainer()));

						TextWidget entryMissionNameWidget = TextWidget.Cast(entryWidget.FindAnyWidget(m_sEntryMissionNameWidgetName));
						entryMissionNameWidget.SetText(missionHeader.m_sName);

						if (missionHeader.m_sIcon)
						{
							entryImageWidget.LoadImageTexture(0, missionHeader.m_sIcon);
							entryImageWidget.SetColor(imageColor);
						}
					}
				}

				if (!meta.AreAddonsCompatible())
				{
					entryIconWidget.SetColor(UIColors.WARNING);
				}

			}
			else
			{
				//--- Meta file missing or invalid
				entryNameWidget.SetText(entryName);
				entryImageWidget.SetVisible(false);

				Widget entryMetaWidget = entryWidget.FindAnyWidget(m_sEntryMeta);
				entryMetaWidget.SetVisible(false);

				entryIconWidget.SetColor(Color.FromInt(UIColors.WARNING.PackToInt()));
			}

			AnimateWidget.Opacity(entryWidget, 1, 3);
		}
		else if (entryWidget.GetOpacity() > 0)
		{
			//--- Started fading in, move on to the next entry
			m_aEntriesToShow.RemoveOrdered(0);
			m_aEntriesHidden.RemoveItemOrdered(entryWidget);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected string EntryName(string fileName);

	//------------------------------------------------------------------------------------------------
	protected SCR_MetaStruct EntryMeta(string fileName);

	//------------------------------------------------------------------------------------------------
	protected SCR_UIInfo EntryUIInfo(string fileName);
	
	//------------------------------------------------------------------------------------------------
	protected SCR_SaveDialogEntry GetEntryByWidget(Widget w)
	{
		SCR_SaveDialogEntry selectedEntry;
		foreach(SCR_SaveDialogEntry entry : m_mEntries)
		{
			if (entry.m_wEntry == w)
				return entry;
		}
		
		return null;
	}
	
	//////////////////////////////////////////////////////////////////////////////////////////
	// Overrides
	//////////////////////////////////////////////////////////////////////////////////////////

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		if (!GetGame().InPlayMode())
			return;

		//--- Find all widgets
		m_wRoot = w;
		m_wList = w.FindAnyWidget(m_sListWidgetName);
		m_wScroll = ScrollLayoutWidget.Cast(w.FindAnyWidget(m_sScrollWidgetName));

		//--- Name input field
		m_SaveNameInput = SCR_EditBoxComponent.GetEditBoxComponent(m_sNameInputWidgetName, w);
		m_SaveNameInput.m_OnChanged.Insert(UpdateButtons);

		//--- Assign control buttons
		Widget closeButtonWidget = w.FindAnyWidget(m_sCloseButtonWidgetName);
		Widget deleteButtonWidget = w.FindAnyWidget(m_sDeleteButtonWidgetName);
		Widget overrideButtonWidget = w.FindAnyWidget(m_sOverrideButtonWidgetName);
		Widget confirmButtonWidget = w.FindAnyWidget(m_sConfirmButtonWidgetName);

		SCR_InputButtonComponent closeButton = SCR_InputButtonComponent.Cast(closeButtonWidget.FindHandler(SCR_InputButtonComponent));
		m_DeleteButton = SCR_InputButtonComponent.Cast(deleteButtonWidget.FindHandler(SCR_InputButtonComponent));
		m_OverrideButton = SCR_InputButtonComponent.Cast(overrideButtonWidget.FindHandler(SCR_InputButtonComponent));
		m_ConfirmButton = SCR_InputButtonComponent.Cast(confirmButtonWidget.FindHandler(SCR_InputButtonComponent));

		closeButton.m_OnActivated.Insert(OnClose);
		m_DeleteButton.m_OnActivated.Insert(OnDelete);
		m_OverrideButton.m_OnActivated.Insert(OnConfirm);
		m_ConfirmButton.m_OnActivated.Insert(OnConfirm);

		UpdateButtons();

		//--- Clear the list first
		SCR_WidgetHelper.RemoveAllChildren(m_wList);

		//--- Create new entries
		array<string> fileNames = {};
		int fileCount = FileCount(fileNames);
		fileNames.Sort();

		WorkspaceWidget workspace = GetGame().GetWorkspace();

		Widget entryWidget;
		SCR_ModularButtonComponent entryButton;

		for (int i = 0, count = fileCount; i < count; i++)
		{;
			string fileName = fileNames[i % fileCount];
			entryWidget = workspace.CreateWidgets(m_sEntryLayout, m_wList);

			entryButton = SCR_ModularButtonComponent.Cast(entryWidget.FindHandler(SCR_ModularButtonComponent));
			if (entryButton)
			{
				entryButton.m_OnFocus.Insert(OnEntryFocus);
				entryButton.m_OnDoubleClicked.Insert(OnEntryDoubleClick);
			}
			
			SCR_SaveDialogEntry entry = new SCR_SaveDialogEntry();
			entry.m_wEntry = entryWidget;
			entry.m_sFileName = fileName;

			m_mEntries.Insert(fileName, entry);
			m_aEntriesHidden.Insert(entryWidget);

			//--- Hide by default
			entryWidget.SetOpacity(0);

			if (m_bIsLoad && i == 0)
				SelectEntry(entryWidget, fileName);
		}

		//--- Initiate periodic check which will load and show metadata only for entries that are actually shown. Doing it all at once here would be too expensive.
		GetGame().GetCallqueue().CallLater(OnFrame, 1, true);
	}

	//------------------------------------------------------------------------------------------------
	protected int FileCount(array<string> fileNames);

	//------------------------------------------------------------------------------------------------
	protected void OnEntryFocus(SCR_ModularButtonComponent button)
	{
		//--- Save entry
		Widget w = button.GetRootWidget();
		SCR_SaveDialogEntry entry = GetEntryByWidget(w);
		
		if (entry)
		{
			m_wLastFocusedEntry = w;
			SelectEntry(w, entry.m_sFileName);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnEntryDoubleClick(SCR_ModularButtonComponent button)
	{
		//--- Activate
		OnConfirm(null, string.Empty);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		GetGame().GetCallqueue().Remove(OnFrame);
	}
}

class SCR_SaveDialogEntry
{
	ref Widget m_wEntry;
	bool m_bIsDownloaded;
	string m_sFileName;
}