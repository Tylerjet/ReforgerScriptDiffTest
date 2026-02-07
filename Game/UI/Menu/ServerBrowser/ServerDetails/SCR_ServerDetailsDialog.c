/*!
Details with mod list to display all server mods.
There should be displayed missing server.
Extra information about server can be added.
*/

class SCR_ServerDetailsDialog : SCR_AddonListDialog
{
	protected const string WIDGET_SCROLL = "ScrollSize";
	protected const string WIDGET_ADDON_LIST = "AddonList";
	protected const string WIDGET_LOADING = "Loading";
	protected const string WIDGET_FAVORITE = "FavoriteImage";
	protected const string WIDGET_TITLEFRAME = "TitleFrame";

	protected const string WIDGET_IPADDRESS_TEXT = "IPAddressText";
	protected const string WIDGET_DISCORD_TEXT = "DiscordText";
	protected const string WIDGET_MODS_NUMBER_TEXT = "ModsNumberText";
	protected const string WIDGET_MODS_SIZE_TEXT = "ModsSizeAmount";
	protected const string WIDGET_VERSION_ALERT_TEXT = "VersionAlertTextButton";
	protected const string WIDGET_MODS_SIZE_TO_DOWNLOAD_TEXT = "ModsSizeToDownload";

	protected const string WIDGET_MODS_SIZE_LAYOUT = "ModsSizeLayout";
	protected const string WIDGET_DETAIL_ICONS_LAYOUT = "DetailIconsLayout";
	protected const string WIDGET_MODS_SIZE_TO_DOWNLOAD_LAYOUT = "ModsSizeToDownloadLayout";

	protected const string IMG_WRONG_VERSION = "DetailIcon_WrongVersion";
	protected const string IMG_PASSWORD_PROTECTED = "Detailicon_PasswordProtected";
	protected const string IMG_CROSS_PLATFORM = "DetailIcon_CrossPlatform";
	protected const string IMG_MODDED = "DetailIcon_Modded";
	protected const string IMG_VERSION_ALERT = "VersionAlertIconButton";

	protected const string BTN_CONFIRM = "Confirm";
	protected const string BTN_FAVORITES = "favorites";
	protected const string BTN_FAVORITES_STAR = "FavoriteButton";
	protected const string BTN_COPY_IPADDRESS = "IPAddressButton";
	protected const string BTN_DISCORD = "DiscordButton";

	protected Widget m_wScroll;
	protected Widget m_wAddonList;
	protected Widget m_wLoading;
	protected Widget m_wFavoriteImage;
	protected Widget m_wModsSizeLayout;
	protected Widget m_wDetailIconsLayout;
	protected Widget m_wVersionAlertIcon;
	protected Widget m_wModsSizeToDownloadLayout;

	protected RichTextWidget m_wIPAddressText;
	protected RichTextWidget m_wDiscordText;
	protected RichTextWidget m_wModsNumberText;
	protected RichTextWidget m_wModsSizeText;
	protected RichTextWidget m_wVersionAlertText;
	protected RichTextWidget m_wModsSizeToDownloadText;

	protected SCR_NavigationButtonComponent m_NavConfirm;
	protected SCR_NavigationButtonComponent m_NavFavorites;

	protected SCR_ButtonComponent m_BtnFavorites;
	protected SCR_ButtonComponent m_BtnCopyIPAddress;
	protected SCR_ButtonComponent m_BtnDiscord;
	
	protected SCR_ScenarioBackendImageComponent m_BackendImageComp;
	protected Widget m_wBackgroundImageBackend;

	protected EInputDeviceType m_eLastInputDevice;
	protected EInputDeviceType m_eCurrentInputDevice;

	ref ScriptInvoker m_OnFavorites = new ScriptInvoker();

	static Room s_Room;

	protected float m_fVersionAlertIconPaddingLeft;

	protected const ResourceName ADDON_LINE_LAYOUT_SERVER_BROWSER = "{3BC78F295971FD3D}UI/layouts/Menus/ContentBrowser/DownloadManager/DownloadManager_AddonDownloadLineConfirmation_ServerBrowser.layout";
	protected const string STR_VERSION_MISMATCH = "#ar-serverbrowser_joinversionfail";
	
	//This should probably be a setting in SCR_HorizontalScrollAnimationComponent, as this is a bandaid solution to the title flickering
	protected const int MAX_TITLE_LENGTH = 55;

	//------------------------------------------------------------------------------------------------
	// Override
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	override protected void InitWidgets()
	{
		super.InitWidgets();

		m_wScroll = m_wRoot.FindAnyWidget(WIDGET_SCROLL);
		m_wAddonList = m_wRoot.FindAnyWidget(WIDGET_ADDON_LIST);
		m_wLoading = m_wRoot.FindAnyWidget(WIDGET_LOADING);
		m_wFavoriteImage = m_wRoot.FindAnyWidget(WIDGET_FAVORITE);
		m_wModsSizeLayout = m_wRoot.FindAnyWidget(WIDGET_MODS_SIZE_LAYOUT);
		m_wDetailIconsLayout = m_wRoot.FindAnyWidget(WIDGET_DETAIL_ICONS_LAYOUT);
		m_wModsSizeToDownloadLayout = m_wRoot.FindAnyWidget(WIDGET_MODS_SIZE_TO_DOWNLOAD_LAYOUT);

		m_wVersionAlertIcon = m_wRoot.FindAnyWidget(IMG_VERSION_ALERT);
		if (m_wVersionAlertIcon)
		{
			float top, right, bottom;
			AlignableSlot.GetPadding(m_wVersionAlertIcon, m_fVersionAlertIconPaddingLeft, top, right, bottom);
		}

		m_wIPAddressText = RichTextWidget.Cast(m_wRoot.FindAnyWidget(WIDGET_IPADDRESS_TEXT));
		m_wDiscordText = RichTextWidget.Cast(m_wRoot.FindAnyWidget(WIDGET_DISCORD_TEXT));
		m_wModsNumberText = RichTextWidget.Cast(m_wRoot.FindAnyWidget(WIDGET_MODS_NUMBER_TEXT));
		m_wModsSizeText = RichTextWidget.Cast(m_wRoot.FindAnyWidget(WIDGET_MODS_SIZE_TEXT));
		m_wVersionAlertText = RichTextWidget.Cast(m_wRoot.FindAnyWidget(WIDGET_VERSION_ALERT_TEXT));
		m_wModsSizeToDownloadText = RichTextWidget.Cast(m_wRoot.FindAnyWidget(WIDGET_MODS_SIZE_TO_DOWNLOAD_TEXT));
	}

	//------------------------------------------------------------------------------------------------
	override protected void Init(Widget root, SCR_ConfigurableDialogUiPreset preset, MenuBase proxyMenu)
	{
		super.Init(root, preset, proxyMenu);

		m_NavConfirm = SCR_NavigationButtonComponent.Cast(root.FindAnyWidget(BTN_CONFIRM).FindHandler(SCR_NavigationButtonComponent));
		if (m_NavConfirm)
			BindButtonConfirm(m_NavConfirm);

		m_NavFavorites = FindButton(BTN_FAVORITES);

		m_BtnFavorites = SCR_ButtonComponent.Cast(root.FindAnyWidget(BTN_FAVORITES_STAR).FindHandler(SCR_ButtonComponent));

		if (m_NavFavorites)
			m_NavFavorites.m_OnActivated.Insert(OnFavorites);

		if (m_BtnFavorites)
			m_BtnFavorites.m_OnClicked.Insert(OnFavorites);

		m_BtnCopyIPAddress = SCR_ButtonComponent.Cast(root.FindAnyWidget(BTN_COPY_IPADDRESS).FindHandler(SCR_ButtonComponent));

		if (m_BtnCopyIPAddress)
			m_BtnCopyIPAddress.m_OnClicked.Insert(OnCopyIPAddress);

		m_BtnDiscord = SCR_ButtonComponent.Cast(root.FindAnyWidget(BTN_DISCORD).FindHandler(SCR_ButtonComponent));

		if (m_BtnDiscord)
			m_BtnDiscord.m_OnClicked.Insert(OnDiscord);

		m_eLastInputDevice = GetGame().GetInputManager().GetLastUsedInputDevice();
		m_eCurrentInputDevice = m_eLastInputDevice;
		
		m_wBackgroundImageBackend = GetRootWidget().FindAnyWidget("BackgroundImageBackend");
		if (m_wBackgroundImageBackend)
			m_BackendImageComp = SCR_ScenarioBackendImageComponent.Cast(m_wBackgroundImageBackend.FindHandler(SCR_ScenarioBackendImageComponent));
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_ConfigurableDialogUiPreset preset)
	{
		// Set visibility
		m_wScroll.SetVisible(false);
		m_wLoading.SetVisible(true);

		m_wModsSizeLayout.SetVisible(false);
		m_wModsSizeToDownloadLayout.SetVisible(false);
		
		//! Background Image
		if(!s_Room)
			return;
		
		SetScenarioImage(s_Room.HostScenario());
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuUpdate(float tDelta)
	{
		if (!m_wVersionAlertIcon || !m_wVersionAlertIcon.IsVisible())
			return;

		m_eCurrentInputDevice = GetGame().GetInputManager().GetLastUsedInputDevice();

		if (m_eCurrentInputDevice == m_eLastInputDevice)
		return;

		m_eLastInputDevice = m_eCurrentInputDevice;
		UpdateVersionAlertIconPadding();
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetTitle(string text)
	{
		super.SetTitle(text);
		
		Widget titleFrame = m_wRoot.FindAnyWidget(WIDGET_TITLEFRAME);
		if (!titleFrame)
			return;
		
		SCR_HorizontalScrollAnimationComponent scrollComp = SCR_HorizontalScrollAnimationComponent.Cast(titleFrame.FindHandler(SCR_HorizontalScrollAnimationComponent));
		if (!scrollComp)
			return;
		
		if (text.Length() < MAX_TITLE_LENGTH)
			scrollComp.AnimationStop();
		else
			scrollComp.AnimationStart();
	}

	//------------------------------------------------------------------------------------------------
	// Public
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	static SCR_ServerDetailsDialog CreateServerDetailsDialog(Room room, array<ref SCR_WorkshopItem> items, string preset, ResourceName dialogsConfig = "", ScriptInvoker onFavoritesResponse = null)
	{
		if (dialogsConfig == "")
			dialogsConfig = SCR_WorkshopUiCommon.DIALOGS_CONFIG;

		s_Room = room;

		SCR_ServerDetailsDialog dialog = new SCR_ServerDetailsDialog(items, "");
		SCR_ConfigurableDialogUi.CreateFromPreset(dialogsConfig, preset, dialog);

		dialog.SetTitle(room.Name());
		dialog.SetIPAddressText("#AR-ServerBrowser_IP: " + room.HostAddress());
		//dialog.SetDiscordText();

		//! Favorites button
		if (onFavoritesResponse)
		{
			onFavoritesResponse.Insert(dialog.OnRoomSetFavoriteResponseDialog);
		}

		dialog.DisplayFavoriteAction(s_Room.IsFavorite());
		dialog.UpdateDetailIcons();
		dialog.UpdateVersionAlertMessage();

		return dialog;
	}

	//------------------------------------------------------------------------------------------------
	void FillModList(array<ref SCR_WorkshopItem> items, SCR_RoomModsManager modsManager)
	{
		m_aItems = items;

		// Set visibility
		m_wLoading.SetVisible(false);

		if (m_aItems.IsEmpty())
			return;

		//------------------------------------------------------------------------------------------------
		// The items must be sorted by size and placed in the following order: Missing, needs download -> Downloaded, needs update -> Downloaded
		array<SCR_WorkshopItem> itemsSorted = {};

		array<SCR_WorkshopItem> itemsToUpdate = {};
		array<SCR_WorkshopItem> itemsToDownload = {};
		array<SCR_WorkshopItem> itemsDownloaded = {};
		array<SCR_WorkshopItem> itemsOrdered = {};
		
		foreach (SCR_WorkshopItem item : m_aItems)
		{
			itemsSorted.Insert(item);
		}
		
		SCR_Sorting<SCR_WorkshopItem, SCR_CompareWorkshopItemTargetSize>.HeapSort(itemsSorted, true);
		
		// Split the items based on their state
		Revision versionFrom;
		Revision versionTo;
		foreach (SCR_WorkshopItem item : itemsSorted)
		{
			versionFrom = item.GetCurrentLocalRevision();
			versionTo = item.GetItemTargetRevision();

			if (!item.GetOffline()) //Missing, needs download
				itemsToDownload.Insert(item);

			else if(versionFrom && !Revision.AreEqual(versionFrom, versionTo)) //Downloaded, needs update
				itemsToUpdate.Insert(item);

			else //Downloaded
				itemsDownloaded.Insert(item);
		}

		// Create the ordered array
		itemsOrdered.InsertAll(itemsToDownload);
		itemsOrdered.InsertAll(itemsToUpdate);
		itemsOrdered.InsertAll(itemsDownloaded);
		//------------------------------------------------------------------------------------------------

		// Setup downloaded
		foreach (SCR_WorkshopItem item : itemsOrdered)
		{
			Widget w = GetGame().GetWorkspace().CreateWidgets(ADDON_LINE_LAYOUT_SERVER_BROWSER, m_wAddonList);
			
			if(item == itemsOrdered[itemsOrdered.Count() - 1])
				AlignableSlot.SetPadding(w, 0, 0, 0, 0);

			SCR_DownloadManager_AddonDownloadLine comp = SCR_DownloadManager_AddonDownloadLine.Cast(w.FindHandler(SCR_DownloadManager_AddonDownloadLine));
			comp.InitForServerBrowser(item, item.GetItemTargetRevision(), true);

			m_aDownloadLines.Insert(comp);
		}

		m_wScroll.SetVisible(true);
		UpdateModsAmountMessages(modsManager);
	}

	//------------------------------------------------------------------------------------------------
	void SetScenarioImage(MissionWorkshopItem scenario)
	{
		if (!m_BackendImageComp)
			return;
		
		if (scenario)
			m_BackendImageComp.SetScenarioAndImage(scenario, scenario.Thumbnail());
		else
			m_BackendImageComp.SetScenarioAndImage(null, null);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetCanJoin(bool canJoin)
	{
		if (!m_NavConfirm)
			return;

		m_NavConfirm.SetEnabled(canJoin, false);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnFavorites()
	{
		m_OnFavorites.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	protected void DisplayFavoriteAction(bool isFavorite)
	{
		if (m_NavFavorites)
		{
			if (isFavorite)
				m_NavFavorites.SetLabel("#AR-Workshop_ButtonRemoveFavourites");
			else
				m_NavFavorites.SetLabel("#AR-Workshop_ButtonAddToFavourites");
		}

		if (m_wFavoriteImage)
		{
			// Play star fade animation
			if (isFavorite)
				AnimateWidget.Color(m_wFavoriteImage, UIColors.CONTRAST_COLOR, UIConstants.FADE_RATE_FAST);
			else
				AnimateWidget.Color(m_wFavoriteImage, UIColors.LIGHT_GREY, UIConstants.FADE_RATE_FAST);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnRoomSetFavoriteResponseDialog()
	{
		DisplayFavoriteAction(s_Room.IsFavorite());
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCopyIPAddress()
	{
		System.ExportToClipboard(s_Room.HostAddress());
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDiscord()
	{
		//TODO once we introduce the ability for the users to give a discord link on server creation
	}

	//------------------------------------------------------------------------------------------------
	protected void SetIPAddressText(string text)
	{
		if (m_wIPAddressText)
			m_wIPAddressText.SetText(text);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetDiscordText(string text)
	{
		if (m_wDiscordText)
			m_wDiscordText.SetText(text);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateModsAmountMessages(SCR_RoomModsManager modsManager = null)
	{
		if (m_aItems.IsEmpty())
			return;

		m_wModsSizeLayout.SetVisible(true);

		float totalSize;
		foreach (ref SCR_WorkshopItem item : m_aItems)
		{
			totalSize += item.GetSizeBytes();
		}

		if (m_wModsNumberText)
			m_wModsNumberText.SetTextFormat(m_aItems.Count().ToString());

		if (m_wModsSizeText)
			m_wModsSizeText.SetText(SCR_ByteFormat.GetReadableSize(totalSize));

		// Check mods to update size
		if (!modsManager)
			return;

		array<ref SCR_WorkshopItem> toUpdateMods = modsManager.GetRoomItemsToUpdate();
		
		// Display size to update
		if (m_wModsSizeToDownloadLayout)
			m_wModsSizeToDownloadLayout.SetVisible(!toUpdateMods.IsEmpty());

		if (m_wModsSizeToDownloadText)
		{
			string toUpdateSize = modsManager.GetModListPatchSizeString(toUpdateMods);
			m_wModsSizeToDownloadText.SetText(toUpdateSize);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateDetailIcons()
	{
		if (!s_Room || !m_wDetailIconsLayout)
			return;

		bool isModded = s_Room.IsModded();

		// Versions mismatch
		bool wrongVersion = s_Room.GameVersion() != GetGame().GetBuildVersion();
		bool restrictedUGC = isModded && !SCR_AddonManager.GetInstance().GetUgcPrivilege();

		//m_wDetailIconsLayout.FindWidget(IMG_WRONG_VERSION).SetVisible(wrongVersion || restrictedUGC);
		m_wDetailIconsLayout.FindWidget(IMG_WRONG_VERSION).SetVisible(false);

		// Locked with password
		m_wDetailIconsLayout.FindWidget(IMG_PASSWORD_PROTECTED).SetVisible(s_Room.PasswordProtected());

		// Crossplay
		m_wDetailIconsLayout.FindWidget(IMG_CROSS_PLATFORM).SetVisible(s_Room.IsCrossPlatform());

		// Moded
		//m_wDetailIconsLayout.FindWidget(IMG_MODDED).SetVisible(isModded);
		m_wDetailIconsLayout.FindWidget(IMG_MODDED).SetVisible(false);
	}


	//------------------------------------------------------------------------------------------------
	protected void UpdateVersionAlertMessage()
	{
		if (!s_Room || !m_wVersionAlertText || !m_wVersionAlertIcon)
			return;

		bool showMessage = s_Room.GameVersion() != GetGame().GetBuildVersion();

		m_wVersionAlertText.SetVisible(showMessage);
		m_wVersionAlertIcon.SetVisible(showMessage);

		if (!showMessage)
			return;

		m_wVersionAlertText.SetText("v" + s_Room.GameVersion() + " - " + STR_VERSION_MISMATCH + "!");
		UpdateVersionAlertIconPadding();
	}


	//------------------------------------------------------------------------------------------------
	protected void UpdateVersionAlertIconPadding()
	{
		float left, top, right, bottom;
		AlignableSlot.GetPadding(m_wVersionAlertIcon, left, top, right, bottom);

		if (m_eCurrentInputDevice == EInputDeviceType.GAMEPAD)
			left = 0 - (m_fVersionAlertIconPaddingLeft * 0.2);
		else
			left = m_fVersionAlertIconPaddingLeft;

		LayoutSlot.SetPadding(m_wVersionAlertIcon, left, top, right, bottom);
	}
};
