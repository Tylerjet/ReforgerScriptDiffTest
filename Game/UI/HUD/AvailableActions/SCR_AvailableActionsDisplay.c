//------------------------------------------------------------------------------------------------
class SCR_AvailableActionsWidget
{
	protected Widget m_wRootWidget;
	protected RichTextWidget m_wRichTextWidget;
	protected TextWidget m_hintText;
	
	//------------------------------------------------------------------------------------------------
	void SetText(string text, string name)
	{
		string shadow = "<shadow color='0,0,0' size='10' offset='5,5' opacity='1'>";
		shadow += "<shadow color='0,0,0' size='1' offset='1,1' opacity='0.75' mode='image'>%1</shadow></shadow>";
		
		if (m_wRichTextWidget)
			m_wRichTextWidget.SetTextFormat(shadow, text);
		
		if (m_hintText)
			m_hintText.SetText(name);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetVisible(bool isVisible)
	{
		if (m_wRootWidget)
			m_wRootWidget.SetVisible(isVisible);
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsVisible()
	{
		if (m_wRootWidget)
			return m_wRootWidget.IsVisible();
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_AvailableActionsWidget CreateActionsWidget(string layout, Widget parent, string text = "", string name = "")
	{
		if (layout == string.Empty)
			return null;
		
		if (!parent)
			return null;
		
		SCR_AvailableActionsWidget instance = new SCR_AvailableActionsWidget();
		if (!instance)
			return null;
		
		Widget root = GetGame().GetWorkspace().CreateWidgets(layout, parent);
		instance.m_wRootWidget = root;
		if (root)
		{
			instance.m_wRichTextWidget = RichTextWidget.Cast(root.FindAnyWidget("SlotRichText"));
			instance.m_hintText = TextWidget.Cast(root.FindAnyWidget("Text"));
		}
		instance.SetText(text, name);
		
		return instance;
	}
	
	//------------------------------------------------------------------------------------------------
	Widget GetRootWidget() { return m_wRootWidget; }
	
	//------------------------------------------------------------------------------------------------
	private void SCR_AvailableActionsWidget() {}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_AvailableActionsWidget()
	{
		if (m_wRootWidget)
			m_wRootWidget.RemoveFromHierarchy();
	}
};

//------------------------------------------------------------------------------------------------
class SCR_AvailableActionContextTitle: BaseContainerCustomTitle
{
	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		// Make sure variable exists
		int index = source.GetVarIndex("m_sAction");
		if (index == -1)
			return false;
		
		// Tag string 
		string tag = "";
		source.Get("m_sTag", tag);
		
		source.Get("m_sAction", title);
		
		// Enabled string 		
		bool enabled;
		source.Get("m_bEnabled", enabled);
		
		string enabledStr = "x";
		if (enabled)
			enabledStr = "on";
		
		// Setup title string 
		title = "(" + title + ") " + tag + " - " + enabledStr;
		return true;
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_AvailableActionContextTitle()]
class SCR_AvailableActionContext
{
	[Attribute("true")]
	protected bool m_bEnabled; 
	
	[Attribute("0", UIWidgets.EditBox, "Amount of mili seconds to hide this hint, 0 and less means no hidding")]
	protected int m_iTimeForHide; 
	
	[Attribute("", UIWidgets.Object)]
	protected ref array<ref SCR_AvailableActionCondition> m_aConditions;	
	
	[Attribute("DefaultTag", UIWidgets.EditBox, "Tag for quick search and recongnition. Change default tag to allow easier search.")]
	protected string m_sTag;
	
	[Attribute("", UIWidgets.EditBox, "Name of the action in action manager")]
	protected string m_sAction;
	
	[Attribute("", UIWidgets.EditBox, "Name of the action in to be displayed in UI")]
	protected string m_sName;
	
	//protected const string MARKUP_FORMAT = "<action name=\"%1\" %2/>";
	protected const string MARKUP_FORMAT = "<action name=\"%1\"/>";
	
	protected bool m_bActivated = false;
	protected bool m_bHideTimeOver = true;
	
	//------------------------------------------------------------------------------------------------
	string ToString(bool forceText = true)
	{
		string text = "";
		if (forceText)
			text = string.Format(MARKUP_FORMAT, m_sAction);
		else
			text = string.Format(MARKUP_FORMAT, m_sAction);
		
		return string.Format("%1", text);
	}
	
	//------------------------------------------------------------------------------------------------
	string GetActionName()
	{
		return m_sAction;
	}
	
	//------------------------------------------------------------------------------------------------
	// TODO: This should possibly be setup somewhere in the actions manager,
	// so it can be reused instead of having to rely on setting it up manually in this object?
	string GetUIName()
	{
		if (m_sName == string.Empty)
			return m_sAction;
		
		return m_sName;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsAvailable(SCR_AvailableActionsConditionData data)
	{		
		if (!m_bEnabled)
			return false;
		
		bool isOk = true;
		foreach (auto cond : m_aConditions)
		{
			if (!cond.IsAvailable(data))
			{
				if (m_bActivated)
					m_bActivated = false;
				
				GetGame().GetCallqueue().Remove(CountHideTime);
				m_bHideTimeOver = true;
			
				isOk = false;
				break;
			}
			
			// Restart hide time counter
			if (m_iTimeForHide > 0 && m_bHideTimeOver && !m_bActivated)
			{
				m_bHideTimeOver = false;
				GetGame().GetCallqueue().Remove(CountHideTime);
				GetGame().GetCallqueue().CallLater(CountHideTime, m_iTimeForHide, false);
			}
			
			if (!m_bActivated)
				m_bActivated = true;
		}
		
		// Hide if time is over
		if (m_iTimeForHide > 0 && m_bHideTimeOver)
			 isOk = false;
		
		return isOk;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CountHideTime(SCR_AvailableActionContext context)
	{
		m_bHideTimeOver = true;
	}
};

//------------------------------------------------------------------------------------------------
class SCR_AvailableActionsDisplay : SCR_InfoDisplayExtended
{
	//! List of all actions to process at any given moment, these are filtered and available ones are displayed
	[Attribute("", UIWidgets.Object, "List of all actions to process at any given moment, these are filtered and available ones are displayed")]
	protected ref array<ref SCR_AvailableActionContext> m_aActions;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Layout used for individual action widgets", params: "layout")]
	protected ResourceName m_sChildLayout;
	
	[Attribute("1", UIWidgets.CheckBox, "Should the rich text widget show action as text only (no icons)?")]
	protected bool m_bForceText;

	[Attribute()]
	protected float m_fDefaultOffsetY;
	
	protected float m_fOffsetY;
	protected float m_fAdditionalOffsetY = 0;
	
	[Attribute("", UIWidgets.Object, "")]
	protected ref array<ref AvailableActionLayoutBehavior> m_aBehaviors;
	
	//! Count of maximum elements that will be pre-cached
	protected const int PRELOADED_WIDGETS_COUNT = 16;
	
	//! List of available action widget containers
	protected ref array<ref SCR_AvailableActionsWidget> m_aWidgets = new ref array<ref SCR_AvailableActionsWidget>();
	
	//! Layout widget in root or null if new
	protected Widget m_wLayoutWidget;
	
	//! Amount of previously shown widgets
	protected int m_iLastCount;
	
	//! 
	protected ref SCR_AvailableActionsConditionData m_data;
	
	//! Timer for fetching data limitation
	protected float m_fDataFetchTimer;
	
	const int HINT_SIZE_Y = 34;
	
	//protected ref array<SCR_AvailableActionContext> availableActions;
	//protected int actionsCount;	
	
	//------------------------------------------------------------------------------------------------
	//! Go through the list of passed in action names of inActions
	//! and populate the outActions list with actions that are currently active (available)
	//! Returns count of available actions or -1 in case of error
	protected int GetAvailableActions(SCR_AvailableActionsConditionData data, array<ref SCR_AvailableActionContext> inActions, out array<SCR_AvailableActionContext> outActions)
	{
		ArmaReforgerScripted game = GetGame();
		if (!game)
			return 0;
		
		InputManager inputManager = game.GetInputManager();
		if (!inputManager)
			return 0;
		
		outActions.Clear();
		int count = 0;
		foreach (auto action : inActions)
		{
			auto actionName = action.GetActionName();
			if (actionName == string.Empty)
				continue;
			
			if (inputManager.IsActionActive(actionName) && action.IsAvailable(data))
			{
				outActions.Insert(action);
				count++;
			}
		}
		
		return count;
	}
	
	//------------------------------------------------------------------------------------------------
	override event void DisplayUpdate(IEntity owner, float timeSlice)
	{	
		// See what we own, we expect this display to be inside the hud manager attached to player controller
		IEntity controlledEntity;
		PlayerController pc = PlayerController.Cast(owner);
		if (pc)
			controlledEntity = pc.GetControlledEntity();
		
		// Make sure we do not draw the UI in certain cases
		if (m_wRoot)
		{
			// TODO: Refactor into script invoker
			bool shouldShow;
			BaseContainer gameplaySettings = GetGame().GetGameUserSettings().GetModule("SCR_GameplaySettings");
			if (gameplaySettings)
				gameplaySettings.Get("m_bControlHints", shouldShow);
			
			// Make sure we control something?
			if (!controlledEntity)
				shouldShow = SCR_EditorManagerEntity.IsOpenedInstance();
			else
			{
				// Make sure our character is alive if anything
				ChimeraCharacter character = ChimeraCharacter.Cast(controlledEntity);
				if (character)
				{
					CharacterControllerComponent controller = CharacterControllerComponent.Cast(character.FindComponent(CharacterControllerComponent));
					if (!controller || controller.IsDead())
						shouldShow = false;
				}
			}
			
			// Turn visibility on or off
			if (!shouldShow)
			{
				m_wRoot.SetVisible(false);
				return;
			}
			else
				m_wRoot.SetVisible(true);
		}
		
		if(!m_data)
			m_data = new SCR_AvailableActionsConditionData();
		
		m_fDataFetchTimer += timeSlice;
			
		if (m_fDataFetchTimer >= 0.25)
		{
		    m_data.FetchData(controlledEntity, m_fDataFetchTimer);
			DisplayWidgetsUpdate();
			
		    m_fDataFetchTimer = 0;
		}
	}	
	
	//------------------------------------------------------------------------------------------------
	private void DisplayWidgetsUpdate()
	{
		array<SCR_AvailableActionContext> availableActions = new array<SCR_AvailableActionContext>();
		int actionsCount = GetAvailableActions(m_data, m_aActions, availableActions);
		
		// Enable additional ones
		if (actionsCount > m_iLastCount)
		{
			for (int i = m_iLastCount; i < actionsCount; i++)
			{
				// OOR
				if (i >= m_aWidgets.Count())
					break;
				
				if (m_aWidgets[i])
					DisplayHint(m_aWidgets[i].GetRootWidget(), WidgetAnimator.FADE_RATE_SUPER_FAST,  WidgetAnimator.FADE_RATE_SUPER_FAST);
			}
		}
		// Or hide previously shown
		else
		{	
			for (int i = actionsCount; i < m_iLastCount; i++)
			{
				// OOR
				if (i >= m_aWidgets.Count())
					break;
				
				/*if (m_aWidgets[i])
					m_aWidgets[i].SetVisible(false);*/
				if (m_aWidgets[i])
					HintFadeOut(m_aWidgets[i].GetRootWidget(), WidgetAnimator.FADE_RATE_DEFAULT,  WidgetAnimator.FADE_RATE_FAST);
			}				
		}

		for (int i = 0; i < actionsCount; i++)
		{
			// OOR
			if (i >= m_aWidgets.Count())
				break;
			
			m_bForceText = false;
			
			if (m_aWidgets[i] && availableActions[i])
				m_aWidgets[i].SetText(availableActions[i].ToString(m_bForceText), availableActions[i].GetUIName());
		}
		
		// Acknowledge new count
		m_iLastCount = actionsCount;
		
		ApplyLayoutBehavior();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DisplayHint(Widget widget, float delayFade, float delayShrink)
	{
		widget.SetOpacity(0);
		VerticalLayoutSlot.SetPadding(widget, 0, -HINT_SIZE_Y, 0, 0);
		widget.SetVisible(true);
		
		// Clear hiding 
		WidgetAnimator.StopAnimation(widget, WidgetAnimationType.Opacity);
		WidgetAnimator.StopAnimation(widget, WidgetAnimationType.PaddingLayout);
		GetGame().GetCallqueue().Remove(HintShrink);
		GetGame().GetCallqueue().Remove(HintHide);
		
		// Animations 
		WidgetAnimator.PlayAnimation(widget, WidgetAnimationType.Opacity, 1, delayFade);
		WidgetAnimator.PlayAnimation(widget, WidgetAnimationType.PaddingLayout, delayShrink, 0, 0, 0, 0);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Hide hint with fadeout 
	protected void HintFadeOut(Widget widget, float delayFade, float delayShrink)
	{
		/*GetGame().GetCallqueue().Remove(HintShrink);
		GetGame().GetCallqueue().Remove(HintHide);*/
		
		WidgetAnimator.PlayAnimation(widget, WidgetAnimationType.Opacity, 0, delayFade);
		GetGame().GetCallqueue().CallLater(HintShrink, 1000 / delayFade, false, widget, delayShrink);
		//HintShrink(widget, delayShrink);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Hide hint with fadeout 
	protected void HintShrink(Widget widget, float delayShrink)
	{
		WidgetAnimator.PlayAnimation(widget, WidgetAnimationType.PaddingLayout, delayShrink, 0, -HINT_SIZE_Y, 0, 0);
		GetGame().GetCallqueue().CallLater(HintHide, 1000 / delayShrink, false, widget);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Hide hint with fadeout 
	protected void HintHide(Widget widget)
	{
		widget.SetVisible(false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set available actions layout y position 
	protected void SetOffsetY(float offset = -1)
	{
		// Default position
		if (offset == -1)
			offset = m_fDefaultOffsetY;
		
		m_fOffsetY = offset;
		
		// Set layout position 
		ApplyOffsets();
	}
	
	//------------------------------------------------------------------------------------------------
	void SetAdditionalOffsetY(float offset)
	{
		m_fAdditionalOffsetY = offset;
		ApplyOffsets();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ApplyOffsets()
	{
		float sum = (-m_fOffsetY) + (-m_fAdditionalOffsetY);
		
		if (m_wLayoutWidget)
			FrameSlot.SetPosY(m_wLayoutWidget, sum);
	}
	
	//------------------------------------------------------------------------------------------------
	override event void DisplayStartDraw(IEntity owner)
	{
		if (m_wRoot)
			m_wLayoutWidget = m_wRoot.FindAnyWidget("VerticalLayout");
		
		// Layout setup 
		SetOffsetY();
		
		int count = m_aWidgets.Count();
		int toGenerate = PRELOADED_WIDGETS_COUNT - count;
		for (int i = 0; i < toGenerate; i++)
		{
			if (i < toGenerate)
			{
				SCR_AvailableActionsWidget widgetContainer = SCR_AvailableActionsWidget.CreateActionsWidget(m_sChildLayout, m_wLayoutWidget, "");
				if (widgetContainer)
					widgetContainer.SetVisible(false);
				
				m_aWidgets.Insert(widgetContainer);
			}
			
			if (m_aWidgets[i])
				m_aWidgets[i].SetVisible(false);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override event void DisplayStopDraw(IEntity owner)
	{
		m_aWidgets.Clear();
		m_wLayoutWidget = null;
		m_iLastCount = 0;
	}

	//------------------------------------------------------------------------------------------------
	override event void DisplayInit(IEntity owner)
	{
		PlayerController playerController = PlayerController.Cast(owner);
		if (!playerController)
		{
			Print("SCR_AvailableActionsDisplay is not an object in HUDManagerComponent attached onto a PlayerController entity! May result in undefined behaviour.", LogLevel.WARNING);
		}	
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ApplyLayoutBehavior()
	{
		// Go through each behavior  
		foreach (AvailableActionLayoutBehavior beh : m_aBehaviors)
		{
			if (ApplyBehavior(beh))
				return;
		}
		
		// Default behavior 
		SetOffsetY();
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool ApplyBehavior(AvailableActionLayoutBehavior behavior)
	{
		// Editor behavior
		AvailableActionEditorLayoutBehavior editorBehavior = AvailableActionEditorLayoutBehavior.Cast(behavior);
		
		if (editorBehavior)
		{
			SCR_EditorManagerEntity editorManagerEntity = SCR_EditorManagerEntity.GetInstance();
			if (!editorManagerEntity)
				return false;
				
			EEditorMode mode = editorManagerEntity.GetCurrentMode();
			
			if (SCR_EditorManagerEntity.IsOpenedInstance() && mode == editorBehavior.m_eEditorMode)
			{
				SetOffsetY(behavior.m_fOffsetY);
				return true;
			}

			return false;
		}
		
		// HUD behavior 
		if (!m_HUDManager)
			return false;
		
		Widget hud = m_HUDManager.FindLayoutByResourceName(behavior.m_sCheckHUD);
				
		if (!SCR_EditorManagerEntity.IsOpenedInstance() && hud && hud.IsEnabled())
		{
			SetOffsetY(behavior.m_fOffsetY); 
			return true;
		}
		
		return false;
	}
};

//------------------------------------------------------------------------------------------------
//! Variables that should be applied on available actions layout whenever given HUD is active
[BaseContainerProps()]
class AvailableActionLayoutBehavior
{
	[Attribute("", UIWidgets.ResourceNamePicker, "Layout", "layout")]
	ResourceName m_sCheckHUD;

	[Attribute("")]
	float m_fOffsetY;
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class AvailableActionEditorLayoutBehavior : AvailableActionLayoutBehavior
{
	[Attribute("1", UIWidgets.ComboBox, "In which mode should be this behavior applied", "", ParamEnumArray.FromEnum(EEditorMode))]
	EEditorMode m_eEditorMode;
};
