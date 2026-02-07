class SCR_ListEntryHelper
{
	// Modular button effect tags
	static const string EFFECT_ICON_COLOR =			"IconColor";
	static const string EFFECT_BACKGROUND_COLOR =	"BackgroundColor";
	static const string EFFECT_NAME_COLOR =			"NameColor";
	static const string EFFECT_WRAPPER_COLOR =		"WrapperColor";
	
	//------------------------------------------------------------------------------------------------
	static void UpdateMouseButtons(array<SCR_ModularButtonComponent> mouseButtons, array<SCR_ModularButtonComponent> errorMouseButtons, bool inErrorState, bool focused)
	{
		Color color = Color.FromInt(UIColors.IDLE_DISABLED.PackToInt());
		SCR_ButtonEffectColor effect;
		
		// --- Mouse buttons color ---
		if (focused)
			color = Color.FromInt(UIColors.NEUTRAL_ACTIVE_STANDBY.PackToInt());
		
		foreach (SCR_ModularButtonComponent button : mouseButtons)
		{
			effect = SCR_ButtonEffectColor.Cast(button.FindEffect(EFFECT_ICON_COLOR));
			if (!effect)
				continue;

			effect.m_cDefault = color;
			effect.m_cToggledOff = color;
			button.InvokeAllEnabledEffects(false);
		}

		// --- Mouse buttons with error state color ---
		Color highlightedColor = Color.FromInt(UIColors.HIGHLIGHTED.PackToInt());
		if (inErrorState)
		{
			color = Color.FromInt(UIColors.WARNING_DISABLED.PackToInt());
			highlightedColor = Color.FromInt(UIColors.WARNING.PackToInt());
		}
		
		foreach (SCR_ModularButtonComponent button : errorMouseButtons)
		{
			effect = SCR_ButtonEffectColor.Cast(button.FindEffect(EFFECT_ICON_COLOR));
			if (effect)
			{
				effect.m_cDefault = color;
				effect.m_cToggledOff = color;
				effect.m_cHovered = highlightedColor;
				effect.m_cActivatedHovered = highlightedColor;
				effect.m_cFocusGained = highlightedColor;
			}
			
			effect = SCR_ButtonEffectColor.Cast(button.FindEffect(EFFECT_BACKGROUND_COLOR));
			if (effect)
			{
				effect.m_cHovered = highlightedColor;
				effect.m_cActivatedHovered = highlightedColor;
				effect.m_cFocusGained = highlightedColor;
			}
			
			button.InvokeAllEnabledEffects(false);
		}
	}
}

//------------------------------------------------------------------------------------------------
class SCR_ScenarioEntryHelper
{
	// Short messages
	static const string MESSAGE_CONNECTION_ISSUES =	"#AR-CoreMenus_Tooltips_NoConnection";
	
	// Verbose messages
	static const string MESSAGE_VERBOSE_CONNECTION_ISSUES =	"AR-Workshop_WarningNoConnection";

	// Button names
	static const string BUTTON_PLAY =			"Play";
	static const string BUTTON_CONTINUE =		"Continue";
	static const string BUTTON_RESTART =		"Restart";
	static const string BUTTON_HOST =			"Host";
	static const string BUTTON_FIND_SERVERS =	"FindServers";
	static const string BUTTON_FAVORITE =		"Favorite";
	
	// Actions
	static const string ACTION_DOUBLE_CLICK =	"MenuSelectDouble";
	static const string ACTION_RESTART =		"MenuRestart";
	static const string ACTION_FIND_SERVERS =	"MenuJoin";
	static const string ACTION_FAVORITE =		"MenuFavourite";
	static const string ACTION_HOST =			"MenuHost";
	
	// --- Modular buttons ---
	//------------------------------------------------------------------------------------------------
	static void UpdateMouseButtons(array<SCR_ModularButtonComponent> mouseButtons, array<SCR_ModularButtonComponent> errorMouseButtons, MissionWorkshopItem mission, bool focused)
	{
		SCR_ListEntryHelper.UpdateMouseButtons(mouseButtons, errorMouseButtons, IsInErrorState(mission), focused);
	}
	
	//------------------------------------------------------------------------------------------------
	static void UpdateErrorMouseButtonsTooltip(SCR_ScriptedWidgetTooltip tooltip, MissionWorkshopItem mission)
	{
		if (!tooltip || (tooltip.GetTag() != BUTTON_HOST && tooltip.GetTag() != BUTTON_FIND_SERVERS))
			return;
	
		if (!IsInErrorState(mission))
		{
			tooltip.ResetMessage();
			tooltip.SetMessageColor(Color.FromInt(UIColors.NEUTRAL_INFORMATION.PackToInt()));
			return;
		}

		tooltip.SetMessage(GetErrorMessage(mission));
		tooltip.SetMessageColor(Color.FromInt(UIColors.WARNING.PackToInt()));
	}
	
	// --- Input buttons ---
	//------------------------------------------------------------------------------------------------
	static void UpdateInputButtons(MissionWorkshopItem mission, array<SCR_InputButtonComponent> buttons, bool visible = true)
	{
		visible = visible && IsReady(mission);
		
		bool mp;
		bool canBeLoaded;
		
		if (visible)
		{
			mp = IsMultiplayer(mission);
			canBeLoaded = HasSave(mission);
		}
		
		foreach (SCR_InputButtonComponent button : buttons)
		{
			Widget w = button.GetRootWidget();
			if (!w)
				continue;

			switch (w.GetName())
			{
				case SCR_ConfigurableDialogUi.BUTTON_CONFIRM:
				{
					button.SetVisible(visible && !canBeLoaded, false);
					break;
				}
				
				case SCR_ScenarioEntryHelper.BUTTON_PLAY:
				{
					button.SetVisible(visible && !canBeLoaded, false);
					break;
				}
				
				case SCR_ScenarioEntryHelper.BUTTON_CONTINUE:
				{
					button.SetVisible(visible && canBeLoaded, false);
					break;
				}

				case SCR_ScenarioEntryHelper.BUTTON_RESTART:
				{
					button.SetVisible(visible && canBeLoaded, false);
					break;
				}

				case SCR_ScenarioEntryHelper.BUTTON_FIND_SERVERS:
				{
					button.SetVisible(visible && mp, false);
					if (button.IsVisible())
						UpdateInputButton(button, mission);
					
					break;
				}

				case SCR_ScenarioEntryHelper.BUTTON_HOST:
				{
					button.SetVisible(visible && mp && !GetGame().IsPlatformGameConsole(), false);
					if (button.IsVisible())
						UpdateInputButton(button, mission);
					
					break;		
				}
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	static void UpdateInputButton(SCR_InputButtonComponent button, MissionWorkshopItem mission, string service = SCR_ServicesStatusHelper.SERVICE_BI_BACKEND_MULTIPLAYER)
	{
		if (!button)
			return;
		
		switch (GetErrorState(mission))
		{
			case SCR_EScenarioEntryErrorState.NONE:
			{
				button.ResetTexture();
				button.SetEnabled(true);
				break;
			}
			
			case SCR_EScenarioEntryErrorState.CONNECTION_ISSUES:
			{
				SCR_InputButtonComponent.SetConnectionButtonEnabled(button, service);
				break;
			}

			case SCR_EScenarioEntryErrorState.MOD_ISSUES:
			{
				SetInputButtonEnabled(button, mission);
				break;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	static bool SetInputButtonEnabled(SCR_InputButtonComponent button, MissionWorkshopItem mission, bool animate = true)
	{
		if (!button)
			return false;

		bool enabled = !IsInErrorState(mission);
		button.SetEnabled(enabled, animate);
		
		if (enabled)
		{
			button.ResetTexture();
			return true;
		}
		
		string texture = GetErrorTexture(mission);
		button.SetTexture(UIConstants.ICONS_IMAGE_SET, texture, Color.FromInt(UIColors.WARNING_DISABLED.PackToInt()));
		
		return true;
	}
	
	// --- Getters ---
	//------------------------------------------------------------------------------------------------
	static SCR_EScenarioEntryErrorState GetErrorState(MissionWorkshopItem mission)
	{
		if (!mission)
			return SCR_EScenarioEntryErrorState.MOD_ISSUES;
	
		WorkshopItem item = mission.GetOwner();
		
		// No item means it's an Arma Reforger default
		if (!item)
		{
			if (!SCR_ServicesStatusHelper.AreMultiplayerServicesAvailable())
				return SCR_EScenarioEntryErrorState.CONNECTION_ISSUES;
			else
				return SCR_EScenarioEntryErrorState.NONE;
		}
		
		return GetErrorState(item);
	}
	
	//------------------------------------------------------------------------------------------------
	protected static SCR_EScenarioEntryErrorState GetErrorState(WorkshopItem item)
	{
		BackendApi backend = GetGame().GetBackendApi();
		WorkshopApi workshop;
		if (backend)
			workshop = backend.GetWorkshop();
		
		SCR_ERevisionAvailability availability;
		if (item)
			availability = SCR_AddonManager.ItemAvailability(item);
		
		bool modIssue = 
			!item || 
			!workshop || 
			workshop.NeedScan() || 
			(availability != SCR_ERevisionAvailability.ERA_AVAILABLE && availability != SCR_ERevisionAvailability.ERA_UNKNOWN_AVAILABILITY);
		
		if (modIssue)
			return SCR_EScenarioEntryErrorState.MOD_ISSUES;	

		if (!SCR_ServicesStatusHelper.AreMultiplayerServicesAvailable())
			return SCR_EScenarioEntryErrorState.CONNECTION_ISSUES;
		
		return SCR_EScenarioEntryErrorState.NONE;
	}
	
	//------------------------------------------------------------------------------------------------
	static bool IsInErrorState(MissionWorkshopItem mission)
	{
		return IsInErrorState(GetErrorState(mission));
	}
	
	//------------------------------------------------------------------------------------------------
	static bool IsInErrorState(SCR_EScenarioEntryErrorState state)
	{
		return state != SCR_EScenarioEntryErrorState.NONE;
	}
	
	//------------------------------------------------------------------------------------------------
	static bool IsModInErrorState(MissionWorkshopItem mission)
	{
		return GetErrorState(mission) == SCR_EScenarioEntryErrorState.MOD_ISSUES;
	}
	
	//------------------------------------------------------------------------------------------------
	static bool HasSave(MissionWorkshopItem mission)
	{
		if (!mission)
			return false;
		
		SCR_MissionHeader header = SCR_MissionHeader.Cast(MissionHeader.ReadMissionHeader(mission.Id()));
		return header && GetGame().GetSaveManager().HasLatestSave(header);
	}

	//------------------------------------------------------------------------------------------------
	static bool IsMultiplayer(MissionWorkshopItem mission)
	{
		return mission && mission.GetPlayerCount() > 1;
	}
	
	//------------------------------------------------------------------------------------------------
	// Default scenarios don't have a owner workshop item, so they're always ready. For the others we need to check
	static bool IsReady(MissionWorkshopItem mission)
	{
		if (!mission)
			return false;

		if (mission.GetOwner())
			return mission.GetOwner().IsReadyToRun();
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	static string GetErrorMessage(MissionWorkshopItem mission)
	{
		string output = string.Empty;
		
		switch (GetErrorState(mission))
		{
			case SCR_EScenarioEntryErrorState.CONNECTION_ISSUES:
			{	
				if (SCR_ServicesStatusHelper.GetLastReceivedCommStatus() == SCR_ECommStatus.FINISHED)
					output = UIConstants.MESSAGE_SERVICES_ISSUES;
				else
					output = UIConstants.MESSAGE_DISCONNECTION;
				
				break;
			}
			
			case SCR_EScenarioEntryErrorState.MOD_ISSUES:
			{
				if (!mission || GetGame().GetBackendApi().GetWorkshop().NeedScan())
					output = SCR_WorkshopUiCommon.MESSAGE_MOD_NOT_AVAILABLE;
				else
					output = SCR_WorkshopUiCommon.GetRevisionAvailabilityErrorMessage(mission.GetOwner());
				
				break;
			}
		}
		
		return output;
	}
	
	//------------------------------------------------------------------------------------------------
	static string GetErrorMessageVerbose(MissionWorkshopItem mission)
	{
		string output = string.Empty;
		
		switch (GetErrorState(mission))
		{
			case SCR_EScenarioEntryErrorState.CONNECTION_ISSUES:
			{	
				if (SCR_ServicesStatusHelper.GetLastReceivedCommStatus() == SCR_ECommStatus.FINISHED)
					output = UIConstants.MESSAGE_SERVICES_ISSUES;
				else
					output = UIConstants.MESSAGE_DISCONNECTION;
				
				break;
			}
			
			case SCR_EScenarioEntryErrorState.MOD_ISSUES:
			{
				if (!mission || GetGame().GetBackendApi().GetWorkshop().NeedScan())
					output = SCR_WorkshopUiCommon.MESSAGE_VERBOSE_MOD_NOT_AVAILABLE;
				else
					output = SCR_WorkshopUiCommon.GetRevisionAvailabilityErrorMessageVerbose(mission.GetOwner());
				
				break;
			}
		}
		
		return output;
	}
	
	//------------------------------------------------------------------------------------------------
	static string GetErrorTexture(MissionWorkshopItem mission)
	{
		string output = string.Empty;
		
		switch (GetErrorState(mission))
		{
			case SCR_EScenarioEntryErrorState.CONNECTION_ISSUES:
			{	
				if (SCR_ServicesStatusHelper.GetLastReceivedCommStatus() == SCR_ECommStatus.FINISHED)
					output = UIConstants.ICON_SERVICES_ISSUES;
				else
					output = UIConstants.ICON_DISCONNECTION;
				
				break;
			}
			
			case SCR_EScenarioEntryErrorState.MOD_ISSUES:
			{
				if (!mission || GetGame().GetBackendApi().GetWorkshop().NeedScan())
					output = SCR_WorkshopUiCommon.ICON_MOD_NOT_AVAILABLE;
				else
					output = SCR_WorkshopUiCommon.GetRevisionAvailabilityErrorTexture(mission.GetOwner());
				
				break;
			}
		}
		
		return output;
	}
}

//------------------------------------------------------------------------------------------------
enum SCR_EScenarioEntryErrorState
{
	NONE = 0, 			// No issues
	CONNECTION_ISSUES,	// Issues with internet or services
	MOD_ISSUES			// Issues with mod
}