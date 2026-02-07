class WorldSelectionMenuUI: WorldSelectionMenuBase
{
	const float HEIGHT_ROW_1 = 300;
	const float HEIGHT_ROW_2 = 250;
	const float HEIGHT_ROW_3 = 200;
	const float HEIGHT_ROW_4 = 170;
	
	// Anchor limiting width of whole grid (max = 0.5 => size 0)
	const float ANCHOR_1COL = 0.275;
	const float ANCHOR_2COL = 0.2;
	const float ANCHOR_3COL = 0.125;
	const float ANCHOR_4COL = 0.05;
	
	const int GRID_PADDING = 4;
	
	// TODO: Re-enable
	//[MenuBindAttribute()]
	//ButtonWidget Host;

	//[MenuBindAttribute()]
	//ButtonWidget Configure;
	
	//------------------------------------------------------------------------------------------------
	[MenuBindAttribute()]
	override void Play()
	{
		if (m_FocusedMissionHeader)
		{
			GetGame().SetGameFlags(m_eFocusedGameflags, false);
			SCR_UISoundEntity.SoundEvent(UISounds.CLICK);
			
			if (!GameStateTransitions.RequestMissionChangeTransition(m_FocusedMissionHeader))
			{
				MenuManager menuManager = GetGame().GetMenuManager();
				if (menuManager)
				{
					//menuManager.OpenMenu(ChimeraMenuPreset.WorkshopSelectionMenu);
					ShowInvalidMissionDialog();
				}
			}
			else
			{
				GetGame().GetMenuManager().CloseAllMenus();
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	[MenuBindAttribute()]
	override void Host()
	{
		if (m_FocusedMissionHeader)
		{
			GetGame().SetGameFlags(m_eFocusedGameflags, false);

			bool res = GameStateTransitions.RequestPublicServerTransition(m_FocusedMissionHeader, new ArmaReforgerServerParams(m_FocusedMissionHeader));
			
			if (res)
			{
				GetGame().GetMenuManager().CloseAllMenus();
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	[MenuBindAttribute()]
	void Configure()
	{
		/*
		m_LastFocused = GetGame().GetWorkspace().GetFocusedWidget();
		MenuBase dialog = GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.MissionSettingsDialog);		
		dialog.BindItem("Confirm", ConfigureConfirm);
		dialog.BindItem("Cancel", ConfigureCancel);
		
		// Read game flags to configure the mission
		ArmaReforgerScripted game = GetGame();
		if (!game)
			return;
		
		EGameFlags flags = m_eFocusedGameflags;
		
		Widget root = dialog.GetRootWidget();
		if (root)
		{
			CheckBoxWidget metabolism = CheckBoxWidget.Cast(root.FindAnyWidget("Metabolism"));
			CheckBoxWidget vehicles = CheckBoxWidget.Cast(root.FindAnyWidget("Vehicles"));
			if (metabolism && vehicles)
			{
				bool metabolismState = flags & EGameFlags.Metabolism;
				bool vehicleState = flags & EGameFlags.SpawnVehicles;
				
				metabolism.SetChecked(metabolismState);
				vehicles.SetChecked(vehicleState);
			}
		}
		*/
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		InputManager inputManager = GetGame().GetInputManager();
		if (!inputManager)
			return;
		
		inputManager.AddActionListener("MenuConfigure",EActionTrigger.PRESSED, Action_OnMenuConfigure);
		inputManager.AddActionListener("MenuChange",EActionTrigger.PRESSED, Action_OnMenuChange);
		
		CreateGridWidgets( m_aGridHeaders, m_aAddons );
	}

	//------------------------------------------------------------------------------------------------
	void Action_OnMenuConfigure()
	{
		if (!Configure.IsEnabled())
			return;
		
		SCR_UISoundEntity.SoundEvent(UISounds.CLICK);
		Configure();
	}
	
	//------------------------------------------------------------------------------------------------
	void Action_OnMenuChange()
	{
		SCR_UISoundEntity.SoundEvent(UISounds.FOCUS);
		Host();
	}
	
	//------------------------------------------------------------------------------------------------
	void ConfigureConfirm(MenuBase dialog)
	{
		// Find all checkboxes and their states
		Widget root = dialog.GetRootWidget();
		if (root)
		{
			CheckBoxWidget metabolism = CheckBoxWidget.Cast(root.FindAnyWidget("Metabolism"));
			CheckBoxWidget vehicles = CheckBoxWidget.Cast(root.FindAnyWidget("Vehicles"));
			ArmaReforgerScripted game = GetGame();
						
			if (game && metabolism && vehicles)
			{
				EGameFlags metabolismState = EGameFlags.Metabolism * metabolism.IsChecked();
				EGameFlags vehicleState = EGameFlags.SpawnVehicles * vehicles.IsChecked();
				
				EGameFlags flags = metabolismState | vehicleState;
				m_eFocusedGameflags = flags;
			}
		}
		
		GetGame().GetMenuManager().CloseMenu(dialog);
		SetLastFocus();
	}

	//------------------------------------------------------------------------------------------------
	void ConfigureCancel(MenuBase dialog)
	{
		GetGame().GetMenuManager().CloseMenu(dialog);
		SetLastFocus();
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void CreateGridWidgets(array<ref SCR_MissionHeader> headers, array<string> addons)
	{
		if (!headers)
		{
			Print("Specified mission headers in CreateWorldWidgets was null or invalid!", LogLevel.ERROR);
			return;
		}
		
		DestroyGridWidgets();
		
		int countHeaders = headers.Count();
		int countAddons = addons.Count();
		int countTotal = countHeaders;
		
		float anchorX;
		float anchorY;
		float height;
		int columns;
		Widget sizeLayout = GetRootWidget().FindAnyWidget("CenterGrid");
		Widget root = GetRootWidget().FindAnyWidget("CenterGrid");
		Widget hideGrid = GetRootWidget().FindAnyWidget("Scroll");
		
		if (countTotal <= 1)
		{
			columns = 2;
			height = HEIGHT_ROW_1;
			anchorX = ANCHOR_1COL;
			anchorY = ANCHOR_1COL * 1.25;
		}
		else if (countTotal <= 2)
		{
			columns = 2;
			height = HEIGHT_ROW_1;
			anchorX = ANCHOR_2COL;
			anchorY = ANCHOR_2COL * 1.25;
		}
		else if (countTotal <= 4)
		{
			columns = 2;
			height = HEIGHT_ROW_2;
			anchorX = ANCHOR_2COL;
			anchorY = ANCHOR_2COL * 1.25;
		}
		else if (countTotal <= 9)
		{
			columns = 3;
			height = HEIGHT_ROW_3;
			anchorX = ANCHOR_3COL;
			anchorY = ANCHOR_3COL * 1.25;
		}
		else
		{
			columns = 4;
			height = HEIGHT_ROW_4;
			
			anchorX = ANCHOR_4COL;
			anchorY = ANCHOR_4COL;
			sizeLayout = GetRootWidget().FindAnyWidget("Scroll");
			root = GetRootWidget().FindAnyWidget("Grid");
			hideGrid = GetRootWidget().FindAnyWidget("CenterGrid");
			
			/*
			if (sizeLayout)
			{
				float scrollSize = FrameSlot.GetSizeY(sizeLayout);
				height = (scrollSize + GRID_PADDING * 2) / 3;
			}*/
			
		}
		
		int row = 0;
		int column = 0;
		
		if (!root || !sizeLayout)
			return;
		
		root.SetVisible(true);
		if (hideGrid)
			hideGrid.SetVisible(false);
		
		// Set size of the sizeLayout widget according to element count
		if (columns < 4)
		{
			FrameSlot.SetAnchorMin(sizeLayout, anchorY,anchorX);
			FrameSlot.SetAnchorMax(sizeLayout, 1 - anchorY, 1 - anchorX);
			FrameSlot.SetOffsets(sizeLayout, 0, 60, 0, 60);
		}
		
		for (int i = 0; i < countHeaders; i++)
		{	
			Widget widget = m_Workspace.CreateWidgets(m_GridElementLayout, root);
			m_aGridButtons.Insert(widget);
			
			if (widget)
			{
				// Iterate through rows and colums in uniform grid slot
				UniformGridSlot.SetRow(widget, row);
				UniformGridSlot.SetColumn(widget, column);
				
				column++;
				if (column >= columns)
				{
					column = 0;
					row++;
				}
				
				// Get widgets
				TextWidget title = TextWidget.Cast(widget.FindAnyWidget(m_sElementTitle));
				TextWidget description = TextWidget.Cast(widget.FindAnyWidget(m_sElementDescription));
				ImageWidget icon = ImageWidget.Cast(widget.FindAnyWidget(m_sElementIcon));
				
				// Set data from mission header to corresponding widgets
				SCR_MissionHeader currentHeader = headers[i];
				if (currentHeader)
				{					
					if (title)
					{
						string name = currentHeader.m_sName;
						name.ToUpper();
						title.SetText(name);
					}
					
					if (description)
					{
						description.SetText(currentHeader.m_sDescription);
					}
					
					if (icon)
					{
						icon.LoadImageTexture(0, currentHeader.m_sIcon);
						icon.SetImage(0);
						
						// Resize the image according to the shorter side
						int ix, iy;
						float x, y;
						float ratioX, ratioY, ratio;
						icon.GetImageSize(0, ix,iy);
						x = (float)ix;
						y = (float)iy;
						
						ratioX = height * 1.6 / x;
						ratioY = height / y;
						
						ratio = Math.Max(ratioX,ratioY);
						if (ratio != 1)
						{
							x *= ratio;
							y *= ratio;
							icon.SetSize(x,y);
						}
					}
				}
			}
		}
		
		// addons
		if (m_bListAddons)
		{
			foreach (int i, string addon: m_aAddons)
			{
				int index = i + countHeaders;
				Widget element = GetGame().GetWorkspace().CreateWidgets(m_GridElementLayout, root);
				UniformGridSlot.SetRow(element, index / columns);
				UniformGridSlot.SetColumn(element, index % columns);
				
				TextWidget title = TextWidget.Cast(element.FindAnyWidget(m_sElementTitle));
				title.SetText("ADDON");
				
				TextWidget desc = TextWidget.Cast(element.FindAnyWidget(m_sElementDescription));
				desc.SetText(addon);
				
				m_aAddonButtons.Insert(element);
			}
		}
	}

	
};

//------------------------------------------------------------------------------------------------
class EditorSelectionMenu: WorldSelectionMenuUI
{
	override void OnMenuOpen()
	{
		m_sSectionName = "#AR-Editor";
		m_aMissionFolderPaths = {};
		m_aMissionPaths = {"Missions/20_GM_Arland.conf","Missions/21_GM_Eden.conf"};
		super.OnMenuOpen();
	}
};

//------------------------------------------------------------------------------------------------
class OldWorkshopSelectionMenu : WorldSelectionMenuUI
{
	override void OnMenuOpen()
	{
		m_sSectionName = "WORKSHOP";
		m_aMissionPaths = {};
		m_aMissionFolderPaths = {"Missions"};
		super.OnMenuOpen();
	}
};