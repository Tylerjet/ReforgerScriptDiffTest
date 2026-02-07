//#define WORKSHOP_DEBUG

//! Component for a tile in the content browser
//! You must call SetWorkshopItem() after tile creation to activate it
class SCR_ContentBrowserTileComponent : SCR_ScriptedWidgetComponent
{
	// Event Handlers
	protected ref ScriptInvokerScriptedWidgetComponent m_OnFocus = new ScriptInvokerScriptedWidgetComponent();

	protected ref SCR_WorkshopTileWidgets m_Widgets = new SCR_WorkshopTileWidgets();

	protected bool m_bFocused;
	protected bool m_bUpdatingContinuously;

	protected ref SCR_WorkshopItem m_Item;

	protected SCR_WorkshopItemBackendImageComponent m_BackendImageComponent;
	protected ref SCR_WorkshopDownloadSequence m_DownloadRequest;

	protected SCR_ERevisionAvailability m_eRevisionAvailability;

	protected SCR_EWorkshopTileErrorState m_eErrorState;
	protected SCR_EWorkshopTileActionState m_eMainActionState;

	protected const int CONTINUOUS_UPDATE_DELAY = 1000;
	protected const string MAIN_ACTION_MESSAGE_DECORATION = "[%1]";

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		if (!GetGame().InPlayMode())
			return;

		m_Widgets.Init(m_wRoot);

		m_Widgets.m_EnableAddonButtonComponent.m_OnToggled.Insert(OnEnableButtonToggled);
		m_Widgets.m_FavouriteButtonComponent0.m_OnToggled.Insert(OnFavouriteButtonToggled);

		Widget backendImage = m_wRoot.FindAnyWidget("m_BackendImage");
		if (backendImage)
			m_BackendImageComponent = SCR_WorkshopItemBackendImageComponent.Cast(backendImage.FindHandler(SCR_WorkshopItemBackendImageComponent));

		UpdateWarningOverlay();
		m_Widgets.m_WarningOverlayComponent.GetOnWarningIconButtonClicked().Insert(OnWarningButtonClicked);

		UpdateHeader();
		m_Widgets.m_MainActionButtonComponent0.m_OnClicked.Insert(OnMainActionButtonClicked);

		SCR_DownloadManager downloadManager = SCR_DownloadManager.GetInstance();
		if (downloadManager)
		{
			downloadManager.m_OnNewDownload.Insert(OnAnyNewDownload);
			downloadManager.m_OnDownloadFailed.Insert(OnAnyDownloadError);
			downloadManager.m_OnDownloadCanceled.Insert(OnAnyDownloadCompleted);
			downloadManager.m_OnDownloadComplete.Insert(OnAnyDownloadCompleted);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		SCR_ScriptedWidgetTooltip.GetOnTooltipShow().Remove(OnTooltipShow);

		SCR_DownloadManager downloadManager = SCR_DownloadManager.GetInstance();
		if (downloadManager)
		{
			downloadManager.m_OnNewDownload.Remove(OnAnyNewDownload);
			downloadManager.m_OnDownloadFailed.Remove(OnAnyDownloadError);
			downloadManager.m_OnDownloadCanceled.Remove(OnAnyDownloadCompleted);
			downloadManager.m_OnDownloadComplete.Remove(OnAnyDownloadCompleted);
		}

		StopContinuousUpdate();

		super.HandlerDeattached(w);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		m_OnFocus.Invoke(this);
		m_bFocused = true;

		UpdateFavouriteButton();
		UpdateNamesScrolling();
		UpdateHeader();

		// Tooltips
		SCR_ScriptedWidgetTooltip.GetOnTooltipShow().Insert(OnTooltipShow);

		return super.OnFocus(w, x, y);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		m_bFocused = false;

		UpdateFavouriteButton();
		UpdateNamesScrolling();
		UpdateHeader();

		// Tooltips
		SCR_ScriptedWidgetTooltip.GetOnTooltipShow().Remove(OnTooltipShow);

		return super.OnFocusLost(w, x, y);
	}

	// ---- Protected ----
	//------------------------------------------------------------------------------------------------
	protected void UpdateStateFlags()
	{
		if (!m_Item)
			return;

		SCR_AddonManager addonManager = SCR_AddonManager.GetInstance();

		WorkshopItem item = m_Item.GetWorkshopItem();
		m_eRevisionAvailability = SCR_ERevisionAvailability.ERA_UNKNOWN_AVAILABILITY;
		if (item)
			m_eRevisionAvailability = addonManager.ItemAvailability(item);

		EWorkshopItemProblem itemProblem = m_Item.GetHighestPriorityProblem();

		// --- Download States ---
		// Downloading is true if we are downloading anything for this addon or if we have started a download for any of its dependencies through this item
		if (m_Item.GetDownloadAction() || m_Item.GetDependencyCompositeAction())
			m_eMainActionState = SCR_EWorkshopTileActionState.DOWNLOADING;
		else if (itemProblem != EWorkshopItemProblem.NO_PROBLEM)
		{
			switch (itemProblem)
			{
				case EWorkshopItemProblem.UPDATE_AVAILABLE:		m_eMainActionState = SCR_EWorkshopTileActionState.UPDATE;					break;
				case EWorkshopItemProblem.DEPENDENCY_MISSING:	m_eMainActionState = SCR_EWorkshopTileActionState.DEPENDENCIES_DOWNLOAD;	break;
				case EWorkshopItemProblem.DEPENDENCY_OUTDATED:	m_eMainActionState = SCR_EWorkshopTileActionState.DEPENDENCIES_UPDATE;		break;
				case EWorkshopItemProblem.DEPENDENCY_DISABLED:	m_eMainActionState = SCR_EWorkshopTileActionState.DEPENDENCIES_ENABLE;		break;
			}
		}
		else if (!m_Item.GetOffline())
			m_eMainActionState = SCR_EWorkshopTileActionState.DOWNLOAD;
		else
			m_eMainActionState = SCR_EWorkshopTileActionState.DOWNLOADED;

		// --- Error States ---
		if (m_Item.GetRestricted())
			m_eErrorState = SCR_EWorkshopTileErrorState.RESTRICTED;
		else if (m_eRevisionAvailability != SCR_ERevisionAvailability.ERA_AVAILABLE && m_eRevisionAvailability != SCR_ERevisionAvailability.ERA_UNKNOWN_AVAILABILITY)
			m_eErrorState = SCR_EWorkshopTileErrorState.REVISION_AVAILABILITY_ISSUE;
		else if (itemProblem == EWorkshopItemProblem.DEPENDENCY_MISSING || itemProblem == EWorkshopItemProblem.DEPENDENCY_DISABLED)
			m_eErrorState = SCR_EWorkshopTileErrorState.DEPENDENCIES_ISSUE;
		else
			m_eErrorState = SCR_EWorkshopTileErrorState.NONE;
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateNames()
	{
		m_Widgets.m_wAddonName.SetText(m_Item.GetName());
		m_Widgets.m_wAddonAuthor.SetText(WidgetManager.Translate("#AR-Workshop_AddonAuthorPrefix", m_Item.GetAuthorName()));
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateNamesScrolling()
	{
		if (m_bFocused)
		{
			if (!m_Widgets.m_FrameNameComponent.GetContentFit())
				m_Widgets.m_FrameNameComponent.AnimationStart();

			if (!m_Widgets.m_FrameAuthorComponent.GetContentFit())
				m_Widgets.m_FrameAuthorComponent.AnimationStart();
		}
		else
		{
			m_Widgets.m_FrameNameComponent.AnimationStop();
			m_Widgets.m_FrameNameComponent.ResetPosition();

			m_Widgets.m_FrameAuthorComponent.AnimationStop();
			m_Widgets.m_FrameAuthorComponent.ResetPosition();
		}

		// TODO: additional authors
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateRating()
	{
		m_Widgets.m_wRating.SetVisible(m_eErrorState != SCR_EWorkshopTileErrorState.RESTRICTED);
		if (!m_Widgets.m_wRating.IsVisible())
			return;

		int rating = Math.Ceil(100 * m_Item.GetAverageRating());
		m_Widgets.m_RatingComponent1.SetTitle(WidgetManager.Translate("#AR-ValueUnit_Percentage", rating));
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateImage()
	{
		if (!m_BackendImageComponent)
			return;

		m_BackendImageComponent.SetWorkshopItemAndImage(m_Item, m_Item.GetThumbnail());
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateDependencyCountWidgets()
	{
		if (!m_Item)
			return;

		bool offline = m_Item.GetOffline();

		int nDependencies = SCR_AddonManager.CountItemsBasic(m_Item.GetLatestDependencies(), EWorkshopItemQuery.OFFLINE);
		int nDependent = SCR_AddonManager.CountItemsBasic(m_Item.GetDependentAddons(), EWorkshopItemQuery.OFFLINE);

		// Displays
		m_Widgets.m_wDependent.SetVisible(nDependent > 0);
		m_Widgets.m_wDependencies.SetVisible(nDependencies > 0);
		
		m_Widgets.m_DependentComponent1.SetTitle(Math.ClampInt(nDependent, 0, 999).ToString());
		m_Widgets.m_DependenciesComponent1.SetTitle(Math.ClampInt(nDependencies, 0, 999).ToString());
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateDownloadProgressBar()
	{
		float progress;

		if (m_Item && m_eMainActionState == SCR_EWorkshopTileActionState.DOWNLOADING)
			progress = GetDownloadProgress();

		m_Widgets.m_wDownloadProgressBar.SetCurrent(progress);
		m_Widgets.m_wDownloadProgressBarOverlay.SetVisible(progress > 0);
	}

	//------------------------------------------------------------------------------------------------
	//! Returns download progress of current action or of current composite action
	protected float GetDownloadProgress()
	{
		return SCR_DownloadManager.GetItemDownloadActionsProgress(m_Item);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateEnableButton()
	{
		m_Widgets.m_EnableAddonButtonComponent.SetVisible(m_Item.GetOffline());
		m_Widgets.m_EnableAddonButtonComponent.SetToggled(m_Item.GetEnabled(), false);

		if (!m_Item.GetOffline())
			return;

		m_Widgets.m_EnableAddonButtonComponent.SetEnabled(m_eMainActionState != SCR_EWorkshopTileActionState.DOWNLOADING);

		string enableButtonMode = "no_problems";
		if (m_eMainActionState == SCR_EWorkshopTileActionState.DEPENDENCIES_DOWNLOAD || m_eMainActionState == SCR_EWorkshopTileActionState.DEPENDENCIES_ENABLE)
			enableButtonMode = "problems";

		m_Widgets.m_EnableAddonButtonComponent.SetEffectsWithAnyTagEnabled({"all", enableButtonMode});
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateFavouriteButton()
	{
		if (!m_Item)
			return;
		
		m_Widgets.m_FavouriteButtonComponent0.SetToggled(m_Item.GetFavourite(), false);
		m_Widgets.m_FavouriteButtonComponent0.SetVisible((m_bFocused || m_Item.GetFavourite()) && m_eErrorState != SCR_EWorkshopTileErrorState.RESTRICTED);
	}

	//------------------------------------------------------------------------------------------------
	protected void ShowBackendEnv()
	{
		string gameEnv = GetGame().GetBackendApi().GetBackendEnv();
		string modEnv = m_Item.GetWorkshopItem().GetBackendEnv();

		bool display = (modEnv != "local") && (modEnv != "ask") && (modEnv != "invalid");
		bool envMatch = gameEnv == modEnv;

		m_Widgets.m_wBackendSource.SetVisible(display && !envMatch);
		if (!display)
			return;

		m_Widgets.m_BackendSourceComponent1.SetTitle(modEnv);

		if (envMatch)
			m_Widgets.m_BackendSourceComponent0.SetIconColor(UIColors.CONFIRM);
		else
			m_Widgets.m_BackendSourceComponent0.SetIconColor(UIColors.WARNING);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateWarningOverlay()
	{
		// No error, hide
		if (!m_Item || m_eErrorState == SCR_EWorkshopTileErrorState.NONE)
		{
			m_Widgets.m_WarningOverlayComponent.SetWarningVisible(false);
			
			if (m_BackendImageComponent)
				m_BackendImageComponent.SetImageSaturation(UIConstants.ENABLED_WIDGET_SATURATION);
			
			return;
		}

		// Errors, show and desaturate the image
		m_Widgets.m_WarningOverlayComponent.SetWarningVisible(true);
		
		if (m_BackendImageComponent)
			m_BackendImageComponent.SetImageSaturation(UIConstants.DISABLED_WIDGET_SATURATION);

		// Update Message
		switch (m_eErrorState)
		{
			case SCR_EWorkshopTileErrorState.RESTRICTED:
			{
				m_Widgets.m_WarningOverlayComponent.SetWarning("#AR-Workshop_State_Restricted", "reportedByMe");
				break;
			}

			case SCR_EWorkshopTileErrorState.DEPENDENCIES_ISSUE:
			{
				m_Widgets.m_WarningOverlayComponent.SetWarning("#AR-Workshop_State_MissingDependencies", "dependencies");
				break;
			}

			case SCR_EWorkshopTileErrorState.REVISION_AVAILABILITY_ISSUE:
			{
				m_Widgets.m_WarningOverlayComponent.SetWarning(
					SCR_WorkshopUiCommon.GetRevisionAvailabilityErrorMessage(m_eRevisionAvailability),
					SCR_WorkshopUiCommon.GetRevisionAvailabilityErrorTexture(m_eRevisionAvailability)
				);
				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateHeader()
	{
		if (!m_Item)
			return;

		m_Widgets.m_wMainActionButton.SetVisible(
			(m_bFocused || (m_eMainActionState != SCR_EWorkshopTileActionState.DOWNLOAD)) &&
			m_eMainActionState != SCR_EWorkshopTileActionState.DOWNLOADED &&
			m_eErrorState != SCR_EWorkshopTileErrorState.RESTRICTED
		);

		if (!m_Widgets.m_wMainActionButton.IsVisible())
			return;

		SCR_ButtonEffectColor iconColor = SCR_ButtonEffectColor.Cast(m_Widgets.m_MainActionButtonComponent0.FindEffect("IconColor"));
		if (!iconColor)
			return;
		
		SCR_ButtonEffectColor messageColor = SCR_ButtonEffectColor.Cast(m_Widgets.m_MainActionButtonComponent0.FindEffect("MessageColor"));
		if (!iconColor)
			return;

		string icon;
		string message;

		switch (m_eMainActionState)
		{
			case SCR_EWorkshopTileActionState.DOWNLOAD:
			{
				iconColor.m_cDefault = UIColors.IDLE_ACTIVE;
				icon = "download";

				message = SCR_ByteFormat.GetReadableSize(m_Item.GetSizeBytes());
				message = string.Format(MAIN_ACTION_MESSAGE_DECORATION, message);

				messageColor.m_cDefault = UIColors.IDLE_ACTIVE;
				break;
			}

			case SCR_EWorkshopTileActionState.DOWNLOADING:
			{
				iconColor.m_cDefault = UIColors.CONTRAST_COLOR;
				icon = "downloading";

				string percentage = WidgetManager.Translate("#AR-ValueUnit_Percentage", Math.Round(100.0 * GetDownloadProgress()));
				string label = "#AR-DownloadManager_State_Downloading";
				message = WidgetManager.Translate("%1 %2", label, percentage);

				messageColor.m_cDefault = UIColors.NEUTRAL_INFORMATION;
				break;
			}

			case SCR_EWorkshopTileActionState.UPDATE:
			{
				if (m_eRevisionAvailability != SCR_ERevisionAvailability.ERA_COMPATIBLE_UPDATE_AVAILABLE)
				{
					iconColor.m_cDefault = UIColors.SLIGHT_WARNING;
					messageColor.m_cDefault = UIColors.IDLE_ACTIVE;
				}
				else
				{
					iconColor.m_cDefault = UIColors.WARNING;
					messageColor.m_cDefault = UIColors.WARNING;
				}

				icon = "update";
				//TODO: update size
				message = string.Format(MAIN_ACTION_MESSAGE_DECORATION, SCR_WorkshopUiCommon.GetPrimaryActionName(m_Item));
				break;
			}

			case SCR_EWorkshopTileActionState.DEPENDENCIES_UPDATE:
			{
				iconColor.m_cDefault = UIColors.SLIGHT_WARNING;
				icon = "update";
				message = string.Format(MAIN_ACTION_MESSAGE_DECORATION, SCR_WorkshopUiCommon.GetPrimaryActionName(m_Item));
				messageColor.m_cDefault = UIColors.IDLE_ACTIVE;
				break;
			}

			case SCR_EWorkshopTileActionState.DEPENDENCIES_DOWNLOAD:
			{
				iconColor.m_cDefault = UIColors.SLIGHT_WARNING;
				icon = "download";
				message = string.Format(MAIN_ACTION_MESSAGE_DECORATION, SCR_WorkshopUiCommon.GetPrimaryActionName(m_Item));
				messageColor.m_cDefault = UIColors.IDLE_ACTIVE;
				break;
			}

			case SCR_EWorkshopTileActionState.DEPENDENCIES_ENABLE:
			{
				iconColor.m_cDefault = UIColors.SLIGHT_WARNING;
				icon = "repairCircle";
				message = string.Format(MAIN_ACTION_MESSAGE_DECORATION, SCR_WorkshopUiCommon.GetPrimaryActionName(m_Item));
				messageColor.m_cDefault = UIColors.IDLE_ACTIVE;
				break;
			}
		}

		m_Widgets.m_MainActionButtonComponent0.InvokeAllEnabledEffects(false);
		m_Widgets.m_MainActionButtonComponent1.SetImage(icon);
		m_Widgets.m_wMainActionText.SetText(message);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateAllWidgets()
	{
		// Tile visibility
		if (!m_Item)
		{
			m_wRoot.SetVisible(false);
			return;
		}

		m_wRoot.SetVisible(true);

		// Check states and update dynamic elements
		HandleDownloadChanges();

		// Update static elements
		UpdateNames();
		UpdateNamesScrolling();
		UpdateRating();
		UpdateImage();
		ShowBackendEnv();
		UpdateEnableButton();
		UpdateFavouriteButton();
		UpdateDependencyCountWidgets();
	}

	//------------------------------------------------------------------------------------------------
	protected void HandleDownloadChanges()
	{
		UpdateStateFlags();

		UpdateDownloadProgressBar();
		UpdateWarningOverlay();
		UpdateHeader();

		if (m_Item && m_eMainActionState == SCR_EWorkshopTileActionState.DOWNLOADING)
			StartContinuousUpdate();
		else
			StopContinuousUpdate();
	}

	//------------------------------------------------------------------------------------------------
	protected void StartContinuousUpdate()
	{
		if (m_bUpdatingContinuously || !m_Item)
			return;

		GetGame().GetCallqueue().CallLater(ContinuousUpdate, CONTINUOUS_UPDATE_DELAY, true);

		m_bUpdatingContinuously = true;
	}

	//------------------------------------------------------------------------------------------------
	protected void StopContinuousUpdate()
	{
		if (m_bUpdatingContinuously)
			GetGame().GetCallqueue().Remove(ContinuousUpdate);

		m_bUpdatingContinuously = false;
	}

	//------------------------------------------------------------------------------------------------
	protected void ContinuousUpdate()
	{
		if (!m_Item || m_eMainActionState != SCR_EWorkshopTileActionState.DOWNLOADING)
			StopContinuousUpdate();

		UpdateDownloadProgressBar();
		UpdateHeader();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnEnableButtonToggled(SCR_ModularButtonComponent comp)
	{
		SCR_WorkshopUiCommon.OnEnableAddonToggleButton(m_Item, comp);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnFavouriteButtonToggled(SCR_ModularButtonComponent comp, bool toggled)
	{
		if (!m_Item)
			return;

		m_Item.SetFavourite(toggled);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnWarningButtonClicked(ScriptInvokerScriptedWidgetComponent comp)
	{
		SCR_WorkshopUiCommon.ExecutePrimaryAction(m_Item, m_DownloadRequest);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMainActionButtonClicked(SCR_ModularButtonComponent comp)
	{
		SCR_WorkshopUiCommon.ExecutePrimaryAction(m_Item, m_DownloadRequest);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnWorkshopItemChange(SCR_WorkshopItem item)
	{
		UpdateAllWidgets();
	}

	// --- Any downloads ---
	// We need to update the progress bar and main action display if we start downloading this item or any of it's dependencies
	//------------------------------------------------------------------------------------------------
	protected void OnAnyNewDownload(SCR_WorkshopItem item, SCR_WorkshopItemActionDownload action)
	{
		HandleDownloadChanges();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnAnyDownloadError(SCR_WorkshopItemActionDownload action, int reason)
	{
		HandleDownloadChanges();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnAnyDownloadCompleted(SCR_WorkshopItemActionDownload action)
	{
		HandleDownloadChanges();
	}

	// ---- Dependency icons Tooltips ----
	//------------------------------------------------------------------------------------------------
	protected void OnTooltipShow(SCR_ScriptedWidgetTooltip tooltipClass, Widget tooltipWidget, Widget hoverWidget, SCR_ScriptedWidgetTooltipPreset preset, string tag)
	{
		if (!m_Item || !tooltipWidget)
			return;

		array<ref SCR_WorkshopItem> addons = {};

		switch (tag)
		{
			case "Dependent":
				addons = SCR_AddonManager.SelectItemsBasic(m_Item.GetDependentAddons(), EWorkshopItemQuery.OFFLINE);
				break;

			case "Dependencies":
				addons = SCR_AddonManager.SelectItemsBasic(m_Item.GetLatestDependencies(), EWorkshopItemQuery.OFFLINE);
				break;
		}

		SCR_ContentBrowser_AddonsTooltipComponent comp = SCR_ContentBrowser_AddonsTooltipComponent.FindComponent(tooltipWidget);
		if (comp)
			comp.Init(addons);
	}

	// ---- Public ----
	//------------------------------------------------------------------------------------------------
	//! If a null pointer is passed, tile becomes hidden
	void SetWorkshopItem(SCR_WorkshopItem workshopItem)
	{
		// Unregister from previous item
		if (m_Item)
		{
			m_Item.m_OnChanged.Remove(OnWorkshopItemChange);
			m_Item = null; // Release the old item
		}

		if (workshopItem)
		{
			m_Item = workshopItem;
			m_Item.m_OnChanged.Insert(OnWorkshopItemChange);
		}

		UpdateAllWidgets();

		// Since we reuse these tiles, when we assign a new workshop item we should not animate the effects, but we must force them into the correct value
		m_Widgets.m_EnableAddonButtonComponent.InvokeAllEnabledEffects(true);
	}

	//------------------------------------------------------------------------------------------------
	SCR_WorkshopItem GetWorkshopItem()
	{
		return m_Item;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerScriptedWidgetComponent GetOnFocus()
	{
		return m_OnFocus;
	}

	//------------------------------------------------------------------------------------------------
	static SCR_ContentBrowserTileComponent FindComponent(notnull Widget w)
	{
		return SCR_ContentBrowserTileComponent.Cast(w.FindHandler(SCR_ContentBrowserTileComponent));
	}
}

// Displayed in the warning overlay
enum SCR_EWorkshopTileErrorState
{
	NONE,
	RESTRICTED,
	REVISION_AVAILABILITY_ISSUE,
	DEPENDENCIES_ISSUE
}

// Displayed in the header
enum SCR_EWorkshopTileActionState
{
	DOWNLOAD,
	UPDATE,
	DEPENDENCIES_UPDATE,
	DEPENDENCIES_DOWNLOAD,	// Missing
	DEPENDENCIES_ENABLE,	// Disabled
	DOWNLOADING,
	DOWNLOADED
}
