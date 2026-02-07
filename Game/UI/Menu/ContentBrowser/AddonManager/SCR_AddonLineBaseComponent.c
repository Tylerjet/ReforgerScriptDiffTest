/*!
Class for addon line button base. 
This component just vizualize state and invoke interaction. 
Items are not manipulated in this component.
*/

class SCR_AddonLineBaseComponent : SCR_ListMenuEntryComponent
{
	protected const string ICON_UPDATE = "update";
	protected const string TOOLTIP_UPDATE = "#AR-Workshop_ButtonUpdate";
	protected const string TOOLTIP_CANCEL_DOWNLOAD = "#AR-Workshop_ButtonCancelDownload";
	
	// Fields
	protected ref SCR_AddonLineBaseWidgets m_Widgets = new SCR_AddonLineBaseWidgets();
	
	protected ref SCR_WorkshopItem m_Item;
	
	protected bool m_bCanUpdate = true;
	
	protected ref ScriptInvokerScriptedWidgetComponent m_OnEnableButton;
	protected ref ScriptInvokerScriptedWidgetComponent m_OnDisableButton;
	protected ref ScriptInvokerScriptedWidgetComponent m_OnFixButton;
	
	// Image effects
	protected SCR_ButtonEffectColor m_UpdateButtonIconColor;
	
	// Error flags
	protected SCR_EAddonLineErrorFlags m_eAddonLineErrorFlags;
	
	//----------------------------------------------------------------------------------------------
	// Overrides
	//----------------------------------------------------------------------------------------------
	//----------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{	
		if (SCR_Global.IsEditMode())
			return;

		m_Widgets.Init(w);
		
		// Setup
		m_aMouseButtons.Insert(m_Widgets.m_MoveLeftButtonComponent0);
		m_aMouseButtons.Insert(m_Widgets.m_MoveRightButtonComponent0);
		m_aMouseButtons.Insert(m_Widgets.m_DeleteButtonComponent0);
		
		m_aMouseButtonsError.Insert(m_Widgets.m_MoveRightButtonComponent0);

		// Button callbacks
		m_Widgets.m_DeleteButtonComponent0.m_OnClicked.Insert(OnDeleteButton);
		m_Widgets.m_MoveRightButtonComponent0.m_OnClicked.Insert(OnEnableButton);
		m_Widgets.m_MoveLeftButtonComponent0.m_OnClicked.Insert(OnDisableButton);
		
		// Fix update 
		m_Widgets.m_UpdateButtonComponent0.m_OnClicked.Insert(OnUpdateButton);
		m_Widgets.m_FixButtonComponent0.m_OnClicked.Insert(OnFixButton);
		
		// Find button image effects 
		m_UpdateButtonIconColor = SCR_ButtonEffectColor.Cast(m_Widgets.m_UpdateButtonComponent0.FindEffect("IconColor"));
		
		// Setup tooltip callbacks 
		m_Widgets.m_UpdateButtonComponent0.m_OnMouseEnter.Insert(OnUpdateButtonMouseEnter);
		m_Widgets.m_UpdateButtonComponent0.m_OnMouseLeave.Insert(OnUpdateButtonMouseLeave);
		
		m_Widgets.m_IncompatibleButtonComponent0.m_OnMouseEnter.Insert(OnIncompatibleButtonMouseEnter);
		m_Widgets.m_IncompatibleButtonComponent0.m_OnMouseLeave.Insert(OnIncompatibleButtonMouseLeave);
		
		m_Widgets.m_MoveLeftButtonComponent0.m_OnMouseEnter.Insert(OnSideButtonMouseEnter);
		m_Widgets.m_MoveLeftButtonComponent0.m_OnMouseLeave.Insert(OnSideButtonMouseLeave);
		m_Widgets.m_MoveRightButtonComponent0.m_OnMouseEnter.Insert(OnSideButtonMouseEnter);
		m_Widgets.m_MoveRightButtonComponent0.m_OnMouseLeave.Insert(OnSideButtonMouseLeave);
		
		super.HandlerAttached(w);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		bool result = super.OnFocus(w, x, y);
		UpdateIssueState();
		UpdateAllWidgets();

		return result;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		bool result = super.OnFocusLost(w, x, y);
		UpdateIssueState();
		UpdateAllWidgets();
		
		return result;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnTooltipShow(SCR_ScriptedWidgetTooltip tooltipClass, Widget tooltipWidget, Widget hoverWidget, SCR_ScriptedWidgetTooltipPreset preset, string tag)
	{
		super.OnTooltipShow(tooltipClass, tooltipWidget, hoverWidget, preset, tag);
		
		if (tag == "MissingDependencies")
			tooltipClass.SetMessageColor(Color.FromInt(UIColors.WARNING.PackToInt()));
	}
	
	//----------------------------------------------------------------------------------------------
	// Protected 
	//----------------------------------------------------------------------------------------------
	//----------------------------------------------------------------------------------------------
	//! Update vizual state of line
	protected void UpdateAllWidgets()
	{	
		if (!m_bCanUpdate || !m_Item)
			return;
	
		// Update name
		m_Widgets.m_wNameText.SetText(m_Item.GetName());
		
		// Update state text
		string stateText;
		bool downloading = m_Item.GetDownloadAction() || m_Item.GetDependencyCompositeAction();
		bool problemCritical;
		string problemDescription;

		if (downloading)
		{
			float progress = SCR_DownloadManager.GetItemDownloadActionsProgress(m_Item);
			stateText = string.Format("%1%%", Math.Round(100.0*progress));
			m_Widgets.m_wHorizontalState.SetVisible(true);
			m_Widgets.m_wUpdateButton.SetVisible(false);
		}
		else
		{
			m_Widgets.m_wHorizontalState.SetVisible(false);
		}
		
		m_Widgets.m_wStateText.SetText(stateText);

		m_Widgets.m_wUpdateButton.SetVisible(m_Item.GetHighestPriorityProblem() == EWorkshopItemProblem.UPDATE_AVAILABLE);
		ShowCompatibilityState();
		
		// Delete Button
		SCR_ERevisionAvailability availability = SCR_AddonManager.ItemAvailability(m_Item.Internal_GetWorkshopItem());
		
		m_Widgets.m_wDeleteButton.SetVisible(
			!downloading && 
			m_bFocused &&
			availability != SCR_ERevisionAvailability.ERA_DOWNLOAD_NOT_FINISHED &&
			GetGame().GetInputManager().GetLastUsedInputDevice() == EInputDeviceType.MOUSE
		);
		
		// Dependencies Button
		m_Widgets.m_wFixButton.SetVisible(RequiresDownloadingDependencies());
	}
	
	//----------------------------------------------------------------------------------------------
	protected void HandleEnableButtons(bool addonEnabled, bool forceHidden = false)
	{
		// Left Panel
		m_Widgets.m_wLeftSeparator.SetVisible(!addonEnabled || forceHidden);
		m_Widgets.m_wSizeMoveRight.SetVisible(!addonEnabled && !forceHidden);
		m_Widgets.m_wMoveRightButton.SetVisible(!addonEnabled && m_bFocused && !forceHidden);
		
		// Right Panel
		m_Widgets.m_wRightSeparator.SetVisible(addonEnabled || forceHidden);
		m_Widgets.m_wSizeMoveLeft.SetVisible(addonEnabled || forceHidden);
		m_Widgets.m_wMoveLeftButton.SetVisible(addonEnabled && m_bFocused && !forceHidden);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Display incopatible icon 
	//! Set icon to incompatible and update base on current state
	protected void ShowCompatibilityState()
	{
		SCR_ERevisionAvailability availability = SCR_AddonManager.ItemAvailability(m_Item.Internal_GetWorkshopItem());
		
		// Incompatible
		bool showIncompatibility = 	availability == SCR_ERevisionAvailability.ERA_OBSOLETE || 
									availability == SCR_ERevisionAvailability.ERA_DELETED;
		
		m_Widgets.m_wIncompatibleButton.SetVisible(showIncompatibility);
		
		if (!showIncompatibility)
		{
			if (availability == SCR_ERevisionAvailability.ERA_DELETED)
			{
				m_Widgets.m_IncompatibleButtonComponent1.SetImage(SCR_WorkshopUiCommon.ICON_MOD_NOT_AVAILABLE_REMOVED);
			}
			else
			{
				string texture = SCR_WorkshopUiCommon.GetRevisionAvailabilityErrorTexture(m_Item.Internal_GetWorkshopItem());
				if (!texture.IsEmpty())
					m_Widgets.m_IncompatibleButtonComponent1.SetImage(texture);
			}
		}
		
		// Update
		// Displaying button to update and fixing issues with download 
		bool showUpdate = 	availability == SCR_ERevisionAvailability.ERA_COMPATIBLE_UPDATE_AVAILABLE ||
							availability == SCR_ERevisionAvailability.ERA_DOWNLOAD_NOT_FINISHED ||
							availability == SCR_ERevisionAvailability.ERA_AVAILABLE && m_Item.GetHighestPriorityProblem() == EWorkshopItemProblem.UPDATE_AVAILABLE;
		
		m_Widgets.m_wUpdateButton.SetVisible(showUpdate);
		
		if (showUpdate)
 		{
			string updateImage;
			if (m_Item && m_Item.IsDownloadRunning())
				updateImage = SCR_WorkshopUiCommon.GetRevisionAvailabilityErrorTexture(m_Item.Internal_GetWorkshopItem());
			
			// Compatible update available for incompatible local revision
			if (availability != SCR_ERevisionAvailability.ERA_AVAILABLE && m_Item && m_Item.IsDownloadRunning())
				m_UpdateButtonIconColor.m_cDefault = Color.FromInt(UIColors.WARNING_DISABLED.PackToInt());
			
			m_Widgets.m_UpdateButtonComponent0.InvokeAllEnabledEffects(true);
			
			if (updateImage.IsEmpty())
				updateImage = ICON_UPDATE;
			
			m_Widgets.m_UpdateButtonComponent1.SetImage(updateImage);
 		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateIssueState()
	{
		SCR_ERevisionAvailability availability = SCR_AddonManager.ItemAvailability(m_Item.Internal_GetWorkshopItem());
		bool downloadIssue = availability == SCR_ERevisionAvailability.ERA_DOWNLOAD_NOT_FINISHED;
		bool issues = HasItemAnyIssue();
		
		m_bIsInErrorState = issues || downloadIssue;
		m_bDisabled = m_bIsInErrorState;
		
		// Update flags
		m_eAddonLineErrorFlags = 0;
		
		if (issues)
			m_eAddonLineErrorFlags |= SCR_EAddonLineErrorFlags.ITEM_ISSUES;
		
		if (availability != SCR_ERevisionAvailability.ERA_AVAILABLE)
			m_eAddonLineErrorFlags |= SCR_EAddonLineErrorFlags.REVISION_AVAILABILITY_ISSUE;
		
		if (RequiresDownloadingDependencies())
			m_eAddonLineErrorFlags |= SCR_EAddonLineErrorFlags.MISSING_DEPENDENCIES;
		
		if (downloadIssue)
			m_eAddonLineErrorFlags |= SCR_EAddonLineErrorFlags.DOWNLOAD_ISSUES;
	}
	
	//----------------------------------------------------------------------------------------------
	// Callbacks 
	//----------------------------------------------------------------------------------------------
	//----------------------------------------------------------------------------------------------
	protected void OnDeleteButton();
	protected void OnActionButton();
	protected void OnUpdateButton();
	
	//----------------------------------------------------------------------------------------------
	protected void OnFixButton()
	{
		if (m_OnFixButton)
			m_OnFixButton.Invoke(this);
	}
	
	//----------------------------------------------------------------------------------------------
	void OnOpenDetailsButton()
	{
		if (!m_Item)
			return;
		
		ContentBrowserDetailsMenu.OpenForWorkshopItem(m_Item);
	}
	
	//----------------------------------------------------------------------------------------------
	void OnEnableButton()
	{
		if (!m_Item)
			return;

		SCR_ERevisionAvailability availability = SCR_AddonManager.ItemAvailability(m_Item.Internal_GetWorkshopItem());
		if (availability == SCR_ERevisionAvailability.ERA_DOWNLOAD_NOT_FINISHED || m_eAddonLineErrorFlags & SCR_EAddonLineErrorFlags.MISSING_DEPENDENCIES)
			return;
		
		// Enable
		if (m_OnEnableButton)
			m_OnEnableButton.Invoke(this);
		
		HandleEnableButtons(true);
		UpdateModularButtons();
	}
	
	//----------------------------------------------------------------------------------------------
	void OnDisableButton()
	{
		if (!m_Item)
			return;
		
		if (m_OnDisableButton)
			m_OnDisableButton.Invoke(this);
		
		HandleEnableButtons(false);
		UpdateModularButtons();
	}
	
	// --- Tooltips ---
	// Update Button Tooltip
	//------------------------------------------------------------------------------------------------
	protected void OnUpdateButtonMouseEnter()
	{
		SCR_ScriptedWidgetTooltip.GetOnTooltipShow().Insert(OnUpdateTooltipooltipShow);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnUpdateButtonMouseLeave()
	{
		SCR_ScriptedWidgetTooltip.GetOnTooltipShow().Remove(OnUpdateTooltipooltipShow);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnUpdateTooltipooltipShow(SCR_ScriptedWidgetTooltip tooltipClass, Widget tooltipWidget, Widget hoverWidget, SCR_ScriptedWidgetTooltipPreset preset, string tag)
	{
		SCR_ERevisionAvailability availability = SCR_AddonManager.ItemAvailability(m_Item.Internal_GetWorkshopItem());
		bool downloading = m_Item.GetDownloadAction() || m_Item.GetDependencyCompositeAction();
		string message = TOOLTIP_UPDATE;
		Color color = Color.FromInt(UIColors.NEUTRAL_INFORMATION.PackToInt());
		
		bool error = 	availability != SCR_ERevisionAvailability.ERA_UNKNOWN_AVAILABILITY && 
						availability != SCR_ERevisionAvailability.ERA_AVAILABLE && 
						availability != SCR_ERevisionAvailability.ERA_DOWNLOAD_NOT_FINISHED;
		
		if (downloading)
			message = TOOLTIP_CANCEL_DOWNLOAD;
		
		else if (error)
		{
			message = SCR_WorkshopUiCommon.GetRevisionAvailabilityErrorMessage(availability);
			color = Color.FromInt(UIColors.WARNING.PackToInt());
		}
		
		tooltipClass.SetMessage(message);
		tooltipClass.SetMessageColor(color);
	}
	
	// Incompatible Button Tooltip
	//------------------------------------------------------------------------------------------------
	protected void OnIncompatibleButtonMouseEnter()
	{
		SCR_ScriptedWidgetTooltip.GetOnTooltipShow().Insert(OnIncompatibleTooltipooltipShow);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnIncompatibleButtonMouseLeave()
	{
		SCR_ScriptedWidgetTooltip.GetOnTooltipShow().Remove(OnIncompatibleTooltipooltipShow);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnIncompatibleTooltipooltipShow(SCR_ScriptedWidgetTooltip tooltipClass, Widget tooltipWidget, Widget hoverWidget, SCR_ScriptedWidgetTooltipPreset preset, string tag)
	{
		tooltipClass.SetMessageColor(Color.FromInt(UIColors.WARNING.PackToInt()));
		
		SCR_ERevisionAvailability availability = SCR_AddonManager.ItemAvailability(m_Item.Internal_GetWorkshopItem());
		
		tooltipClass.SetMessage(SCR_WorkshopUiCommon.GetRevisionAvailabilityErrorMessage(availability));
		tooltipClass.SetMessageColor(Color.FromInt(UIColors.WARNING.PackToInt()));
	}
	
	// Side Buttons Tooltip
	//------------------------------------------------------------------------------------------------
	protected void OnSideButtonMouseEnter()
	{
		SCR_ScriptedWidgetTooltip.GetOnTooltipShow().Insert(OnSideTooltipooltipShow);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnSideButtonMouseLeave()
	{
		SCR_ScriptedWidgetTooltip.GetOnTooltipShow().Remove(OnSideTooltipooltipShow);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnSideTooltipooltipShow(SCR_ScriptedWidgetTooltip tooltipClass, Widget tooltipWidget, Widget hoverWidget, SCR_ScriptedWidgetTooltipPreset preset, string tag)
	{
		if (m_bIsInErrorState && m_Item && !m_Item.GetEnabled())
			tooltipClass.SetMessageColor(Color.FromInt(UIColors.WARNING.PackToInt()));
	}
	
	//----------------------------------------------------------------------------------------------
	// API
	//----------------------------------------------------------------------------------------------
	//----------------------------------------------------------------------------------------------
	//! Setup line
	void Init(SCR_WorkshopItem item)
	{
		m_Item = item;
		
		UpdateIssueState();
		
		UpdateAllWidgets();
		UpdateModularButtons();
	}
	
	//----------------------------------------------------------------------------------------------
	SCR_WorkshopItem GetWorkshopItem()
	{
		return m_Item;
	}
	
	//----------------------------------------------------------------------------------------------
	void EnableUpdate(bool enable)
	{
		m_bCanUpdate = enable;
	}
	
	//------------------------------------------------------------------------------------------------
	bool HasItemAnyIssue()
	{
		EWorkshopItemProblem problem = m_Item.GetHighestPriorityProblem();
		
		bool hasIssue = (
			problem == EWorkshopItemProblem.BROKEN ||
			problem == EWorkshopItemProblem.DEPENDENCY_MISSING ||
			problem == EWorkshopItemProblem.DEPENDENCY_OUTDATED
		);
		
		return hasIssue;
	}
	
	//------------------------------------------------------------------------------------------------
	bool RequiresDownloadingDependencies()
	{
		if (!m_Item)
			return false;
		
		array<ref SCR_WorkshopItem> dependencies = m_Item.GetLatestDependencies();
		
		if (!dependencies.IsEmpty())
			dependencies = SCR_AddonManager.SelectItemsOr(dependencies, EWorkshopItemQuery.NOT_OFFLINE | EWorkshopItemQuery.UPDATE_AVAILABLE);
		
		return !dependencies.IsEmpty();
	}
	
	//------------------------------------------------------------------------------------------------
 	SCR_EAddonLineErrorFlags GetErrorFlags()
	{
		return m_eAddonLineErrorFlags;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerScriptedWidgetComponent GetOnEnableButton()
	{
		if (!m_OnEnableButton)
			m_OnEnableButton = new ScriptInvokerScriptedWidgetComponent();
		
		return m_OnEnableButton;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerScriptedWidgetComponent GetOnDisableButton()
	{
		if (!m_OnDisableButton)
			m_OnDisableButton = new ScriptInvokerScriptedWidgetComponent();
		
		return m_OnDisableButton;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerScriptedWidgetComponent GetOnFixButton()
	{
		if (!m_OnFixButton)
			m_OnFixButton = new ScriptInvokerScriptedWidgetComponent();
		
		return m_OnFixButton;
	}
}

//TODO: unify error states, there's no reason this should have it's custom handling compared to workshop tiles or othe reperesentations of mods
enum SCR_EAddonLineErrorFlags
{
	MISSING_DEPENDENCIES		= 1 << 0,
	ITEM_ISSUES					= 1 << 1,
	REVISION_AVAILABILITY_ISSUE	= 1 << 2,
	DOWNLOAD_ISSUES				= 1 << 3
}