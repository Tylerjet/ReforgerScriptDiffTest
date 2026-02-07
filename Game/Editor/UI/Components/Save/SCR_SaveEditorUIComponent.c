class SCR_SaveEditorUIComponent : ScriptedWidgetComponent
{
	[Attribute()]
	protected bool m_bIsLoad;
	
	[Attribute(ESaveType.USER.ToString(), uiwidget: UIWidgets.Flags, enums: ParamEnumArray.FromEnum(ESaveType))]
	protected ESaveType m_eReadSaveTypes;
	
	[Attribute(ESaveType.USER.ToString(), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ESaveType))]
	protected ESaveType m_eWriteSaveType;
	
	[Attribute()]
	protected bool m_bCurrentMissionOnly;
	
	[Attribute("SaveList")]
	protected string m_sListWidgetName;
	
	[Attribute("SaveNameInput")]
	protected string m_sNameInputWidgetName;
	
	[Attribute("ExitButton")]
	protected string m_sCloseButtonWidgetName;
	
	[Attribute("OverrideButton")]
	protected string m_sOverrideButtonWidgetName;
	
	[Attribute("ConfirmButton")]
	protected string m_sConfirmButtonWidgetName;
	
	[Attribute("ConfirmPrompt")]
	protected string m_sConfirmPromptWidgetName;
	
	[Attribute("EntryName")]
	protected string m_sEntryNameWidgetName;
	
	[Attribute("PreviewImage")]
	protected string m_sPreviewImageWidgetName;
	
	[Attribute("PreviewName")]
	protected string m_sPreviewNameWidgetName;
	
	[Attribute("PreviewDate")]
	protected string m_sPreviewDateWidgetName;
	
	[Attribute("PreviewTime")]
	protected string m_sPreviewTimeWidgetName;
	
	[Attribute("PreviewWorld")]
	protected string m_sPreviewWorldWidgetName;
	
	[Attribute("#AR-Date_Format")]
	protected string m_sDateFormat;
	
	[Attribute(uiwidget: UIWidgets.ResourceNamePicker, params: "layout")]
	protected ResourceName m_sCreateLayout;
	
	[Attribute(uiwidget: UIWidgets.ResourceNamePicker, params: "layout")]
	protected ResourceName m_sEntryLayout;
	
	[Attribute(uiwidget: UIWidgets.ResourcePickerThumbnail, params: "edds")]
	protected ResourceName m_sFallbackImage;
	
	[Attribute("")]
	protected string m_sConfirmPrompt;
	
	protected Widget m_wRoot;
	protected Widget m_wList;
	protected Widget m_wConfirmPrompt;
	protected ImageWidget m_wPreviewImage;
	protected TextWidget m_wPreviewName;
	protected TextWidget m_wPreviewDate;
	protected TextWidget m_wPreviewTime;
	protected TextWidget m_wPreviewWorld;
	protected SCR_NavigationButtonComponent m_OverrideButton;
	protected SCR_NavigationButtonComponent m_ConfirmButton;
	protected SCR_EditBoxComponent m_SaveNameInput;
	
	protected ref map<Widget, string> m_mEntries = new map<Widget, string>();

	//////////////////////////////////////////////////////////////////////////////////////////
	// Control buttons
	//////////////////////////////////////////////////////////////////////////////////////////
	protected void OnClose(SCR_NavigationButtonComponent button, string actionName)
	{
		CloseMenu();
	}

	//----------------------------------------------------------------------------------------
	protected void OnConfirm(SCR_NavigationButtonComponent button, string actionName)
	{
		if (m_bIsLoad)
		{
			//--- Loading a file leads to restart, ask first
			ShowConfirmPrompt();
		}
		else
		{
			string customName = m_SaveNameInput.GetValue();
			if (GetGame().GetSaveManager().FileExists(m_eWriteSaveType, customName))
				//--- Overriding existing file - ask first
				ShowConfirmPrompt();
			else
				//--- Creating a new file - save directly
				OnConfirmPrompt();
		}
	}

	//----------------------------------------------------------------------------------------
	protected void OnConfirmPrompt()
	{
		if (m_bIsLoad)
		{
			string fileName;
			if (m_mEntries.Find(GetGame().GetWorkspace().GetFocusedWidget(), fileName))
				GetGame().GetSaveManager().RestartAndLoad(fileName);
		}
		else
		{
			string customName = m_SaveNameInput.GetValue();
			GetGame().GetSaveManager().Save(m_eWriteSaveType, customName);
		}
		
		HideConfirmPrompt();
		CloseMenu();
	}

	//----------------------------------------------------------------------------------------
	protected void ShowConfirmPrompt()
	{
		m_wConfirmPrompt.SetOpacity(1);
		m_wConfirmPrompt.SetVisible(true);
		m_wConfirmPrompt.SetEnabled(true);
		GetGame().GetWorkspace().AddModal(m_wConfirmPrompt, m_wConfirmPrompt);
	}

	//----------------------------------------------------------------------------------------
	protected void HideConfirmPrompt()
	{
		m_wConfirmPrompt.SetVisible(false);
		m_wConfirmPrompt.SetEnabled(false);
		GetGame().GetWorkspace().RemoveModal(m_wConfirmPrompt);
	}

	//----------------------------------------------------------------------------------------
	protected void CloseMenu()
	{
		//--- Confirmation prompt opened, ignore
		if (m_wConfirmPrompt.IsEnabled())
			return;
		
		MenuBase menu = MenuBase.Cast(m_wRoot.FindHandler(MenuBase));
		menu.Close();
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	// Interaction
	//////////////////////////////////////////////////////////////////////////////////////////
	protected void SelectEntry(Widget w, string fileName)
	{
		string customName = GetGame().GetSaveManager().GetCustomName(fileName);
		
		m_wPreviewName.SetText(customName);
		
		m_SaveNameInput.SetValue(customName);
		UpdateButtons();
		
		bool isDefined;
		SCR_MetaStruct meta = GetGame().GetSaveManager().GetMeta(fileName);
		if (meta)
		{
			Resource missionHeaderResource = Resource.Load(meta.GetHeaderResource());
			SCR_MissionHeader missionHeader;
			
			if (missionHeaderResource.IsValid())
				missionHeader = SCR_MissionHeader.Cast(BaseContainerTools.CreateInstanceFromContainer(missionHeaderResource.GetResource().ToBaseContainer()));
			
			if (missionHeader)
			{
				if (missionHeader.m_sIcon)
					m_wPreviewImage.LoadImageTexture(0, missionHeader.m_sIcon);
				
				int y, m, d, hh, mm;
				meta.GetDateAndTime(y, m, d, hh, mm);
				m_wPreviewDate.SetVisible(true);
				m_wPreviewDate.SetTextFormat(m_sDateFormat, d, SCR_DateTimeHelper.GetMonthString(m), y);
				
				m_wPreviewTime.SetVisible(true);
				m_wPreviewTime.SetText(SCR_Global.GetTimeFormattingHoursMinutes(hh, mm));
				
				//m_wPreviewWorld.SetVisible(true);
				m_wPreviewWorld.SetText(missionHeader.m_sName);
				
				isDefined = true;
			}
		}
		
		if (!isDefined)
		{
			m_wPreviewImage.LoadImageTexture(0, m_sFallbackImage);
			m_wPreviewDate.SetVisible(false);
			m_wPreviewTime.SetVisible(false);
			//m_wPreviewWorld.SetVisible(false);
			m_wPreviewWorld.SetText("Unknown"); //--- ToDo: Localize
		}
	}
	protected void UpdateButtons()
	{
		string customName = m_SaveNameInput.GetValue();
		bool isValid = !customName.IsEmpty();
		bool isOverride = customName && GetGame().GetSaveManager().FileExists(m_eWriteSaveType, customName);
		
		m_OverrideButton.SetVisible(isOverride, false);
		m_OverrideButton.SetEnabled(isOverride && isValid);
		
		m_ConfirmButton.SetVisible(!isOverride, false);
		m_ConfirmButton.SetEnabled(!isOverride && isValid);
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	// Overrides
	//////////////////////////////////////////////////////////////////////////////////////////
	override bool OnClick(Widget w, int x, int y, int button)
	{
		//--- Save entry
		string fileName;
		if (m_mEntries.Find(w, fileName))
			SelectEntry(w, fileName);
		
		return false;
	}
	
	//----------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		if (!GetGame().InPlayMode())
			return;
		
		//--- Find all widgets
		m_wRoot = w;
		m_wList = w.FindAnyWidget(m_sListWidgetName);
		m_wConfirmPrompt = w.FindAnyWidget(m_sConfirmPromptWidgetName);
		m_wPreviewImage = ImageWidget.Cast(w.FindAnyWidget(m_sPreviewImageWidgetName));
		m_wPreviewName = TextWidget.Cast(w.FindAnyWidget(m_sPreviewNameWidgetName));
		m_wPreviewDate = TextWidget.Cast(w.FindAnyWidget(m_sPreviewDateWidgetName));
		m_wPreviewTime = TextWidget.Cast(w.FindAnyWidget(m_sPreviewTimeWidgetName));
		m_wPreviewWorld = TextWidget.Cast(w.FindAnyWidget(m_sPreviewWorldWidgetName));
		
		//--- Name input field
		m_SaveNameInput = SCR_EditBoxComponent.GetEditBoxComponent(m_sNameInputWidgetName, w);
		m_SaveNameInput.m_OnChanged.Insert(UpdateButtons);
		
		//--- Confirm prompt
		SCR_ConfigurableDialogUi confirmPrompt = SCR_ConfigurableDialogUi.Cast(m_wConfirmPrompt.FindHandler(SCR_ConfigurableDialogUi));
		SCR_ConfigurableDialogUi.InitFromPreset(SCR_CommonDialogs.DIALOGS_CONFIG, m_sConfirmPrompt, m_wConfirmPrompt); //--- ToDo: Unique tag
		confirmPrompt.m_OnConfirm.Insert(OnConfirmPrompt);
		confirmPrompt.m_OnClose.Insert(HideConfirmPrompt);
		confirmPrompt.m_OnCancel.Insert(HideConfirmPrompt);
		
		//--- Assign control buttons
		Widget closeButtonWidget = w.FindAnyWidget(m_sCloseButtonWidgetName);
		Widget overrideButtonWidget = w.FindAnyWidget(m_sOverrideButtonWidgetName);
		Widget confirmButtonWidget = w.FindAnyWidget(m_sConfirmButtonWidgetName);
		
		SCR_NavigationButtonComponent closeButton = SCR_NavigationButtonComponent.Cast(closeButtonWidget.FindHandler(SCR_NavigationButtonComponent));
		m_OverrideButton = SCR_NavigationButtonComponent.Cast(overrideButtonWidget.FindHandler(SCR_NavigationButtonComponent));
		m_ConfirmButton = SCR_NavigationButtonComponent.Cast(confirmButtonWidget.FindHandler(SCR_NavigationButtonComponent));
		
		closeButton.m_OnActivated.Insert(OnClose);
		m_OverrideButton.m_OnActivated.Insert(OnConfirm);
		m_ConfirmButton.m_OnActivated.Insert(OnConfirm);
		
		//--- Clear the list first
		SCR_WidgetHelper.RemoveAllChildren(m_wList);
		
		//--- Create new entries
		array<string> fileNames = {};
		int fileCount = GetGame().GetSaveManager().GetLocalSaveFiles(fileNames, m_eReadSaveTypes, m_bCurrentMissionOnly);
		fileNames.Sort();
		
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		
		/*
		if (!m_bIsLoad)
		{
			entryWidget = workspace.CreateWidgets(m_sCreateLayout, m_wList);
			m_mEntries.Insert(entryWidget, "CreateNew");
			SelectEntry(entryWidget, "CreateNew");
		}
		*/
		
		Widget entryWidget;
		TextWidget entryNameWidget;
		for (int i = 0; i < fileCount; i++)
		{
			string fileName = fileNames[i];
			entryWidget = workspace.CreateWidgets(m_sEntryLayout, m_wList);
			m_mEntries.Insert(entryWidget, fileName);
			
			string entryName = GetGame().GetSaveManager().GetCustomName(fileName);
			entryNameWidget = TextWidget.Cast(entryWidget.FindAnyWidget(m_sEntryNameWidgetName));
			
			if (entryName)
				entryNameWidget.SetText(entryName);
			else
				entryNameWidget.SetText(fileName);
			
			if (m_bIsLoad && i == 0)
				SelectEntry(entryWidget, fileName);
		}
	}
};
