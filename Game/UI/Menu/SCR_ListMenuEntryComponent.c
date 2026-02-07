/*
Base class for Scenario and Server list menu entries
*/
void ScriptInvokerListMenuEntryMethod(SCR_ListMenuEntryComponent entry, bool favorite);
typedef func ScriptInvokerListMenuEntryMethod;
typedef ScriptInvokerBase<ScriptInvokerListMenuEntryMethod> ScriptInvokerListMenuEntry;

//------------------------------------------------------------------------------------------------
class SCR_ListMenuEntryComponent : SCR_ScriptedWidgetComponent
{
	protected ref SCR_ModularButtonComponent m_FavComponent;

	protected bool m_bIsInErrorState; //Entry is enabled, but some of it's buttons are unavailable. Useful for disabling multiple mouse buttons based on one condition. TODO: make it a state of modular buttons 
	
	// Text scrolling anims
	protected ref array<ref SCR_HorizontalScrollAnimationComponent> m_aScrollAnimations = {};

	// Mouse buttons
	// These arrays need to be filled by child classes with the specific buttons
	protected bool m_bInnerButtonInteraction;
	protected ref array<SCR_ModularButtonComponent> m_aMouseButtons = {};
	protected ref array<SCR_ModularButtonComponent> m_aMouseButtonsError = {};

	// Main
	protected SCR_ModularButtonComponent m_MainModularButton;
	protected bool m_bFavorite;
	protected bool m_bDisabled;	//Entry is unavailable but the widget still needs to be interactable. TODO: clean up confusing naming
	protected bool m_bFocused;
	protected bool m_bMouseButtonsEnabled = true;

	// Tooltip
	protected SCR_ScriptedWidgetTooltip m_CurrentTooltip;
	
	// Invokers
	// Called when a mouse interaction button is pressed
	protected ref ScriptInvokerString m_OnMouseInteractionButtonClicked;

	protected ref ScriptInvokerScriptedWidgetComponent m_OnFocus;
	protected ref ScriptInvokerScriptedWidgetComponent m_OnFocusLost;
	protected ref ScriptInvokerListMenuEntry m_OnFavorite;

	// ---- OVERRIDES ----
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		// Setup scroll animations
		EnableTextAnimations(false);

		// Mouse Buttons
		SCR_ButtonEffectColor colorEffect;
		foreach (SCR_ModularButtonComponent button : m_aMouseButtons)
		{
			button.m_OnMouseEnter.Insert(OnInnerButtonHover);
			button.m_OnMouseLeave.Insert(OnInnerButtonLeave);
		}

		// Favorite button
		if (m_FavComponent)
			m_FavComponent.m_OnClicked.Insert(OnFavoriteClicked);

		super.HandlerAttached(w);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		if (m_OnFocus)
			m_OnFocus.Clear();

		if (m_OnFocusLost)
			m_OnFocusLost.Clear();

		SCR_ScriptedWidgetTooltip.GetOnTooltipShow().Remove(OnTooltipShow);

		super.HandlerDeattached(w);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		// LMB check
		if (button != 0 || m_bInnerButtonInteraction)
			return false;

		return super.OnClick(w, x, y, button);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnDoubleClick(Widget w, int x, int y, int button)
	{
		// LMB check
		if (button != 0 || m_bInnerButtonInteraction)
			return false;

		return super.OnDoubleClick(w, x, y, button);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		if (m_OnFocus)
			m_OnFocus.Invoke(this);

		m_bFocused = true;
		UpdateModularButtons();

		if (!m_aScrollAnimations.IsEmpty())
			EnableTextAnimations(true);

		// Tooltips
		SCR_ScriptedWidgetTooltip.GetOnTooltipShow().Insert(OnTooltipShow);

		return super.OnFocus(w, x, y);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		if (m_OnFocusLost)
			m_OnFocusLost.Invoke(this);

		m_bFocused = false;
		UpdateModularButtons();

		// Stop anim
		EnableTextAnimations(false);

		// Tooltips
		SCR_ScriptedWidgetTooltip.GetOnTooltipShow().Remove(OnTooltipShow);

		return super.OnFocusLost(w, x, y);
	}

	// ---- PROTECTED ----
	//------------------------------------------------------------------------------------------------
	protected void OnTooltipShow(SCR_ScriptedWidgetTooltip tooltipClass, Widget tooltipWidget, Widget hoverWidget, SCR_ScriptedWidgetTooltipPreset preset, string tag)
	{
		m_CurrentTooltip = tooltipClass;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnInnerButtonHover()
	{
		m_bInnerButtonInteraction = true;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnInnerButtonLeave()
	{
		m_bInnerButtonInteraction = false
	}

	//------------------------------------------------------------------------------------------------
	protected void OnFavoriteClicked(SCR_ModularButtonComponent comp)
	{
		SetFavorite(!m_bFavorite);
	}

	// ----
	//------------------------------------------------------------------------------------------------
	//! Apply behavior on all potentially longer entry text
	protected void EnableTextAnimations(bool enable)
	{
		foreach (SCR_HorizontalScrollAnimationComponent anim : m_aScrollAnimations)
		{
			HandleTextAnimation(anim, enable);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Handle animation enabled-disabling
	//! enable = false restarts positions
	protected void HandleTextAnimation(SCR_HorizontalScrollAnimationComponent anim, bool enable)
	{
		if (!anim)
			return;

		if (enable)
		{
			if (!anim.GetContentFit())
			{
				anim.AnimationStart();
			}
			else
			{
				anim.AnimationStop();
				anim.ResetPosition();
			}

			return;
		}

		// Disabled
		anim.AnimationStop();
		anim.ResetPosition();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateModularButtons()
	{
		SCR_ListEntryHelper.UpdateMouseButtons(m_aMouseButtons, m_aMouseButtonsError, m_bIsInErrorState, m_bFocused);
		
		// --- Main elements color ---
		// Toggling the line's main modular button component is used as a quck way to change the line's elements while mantaining it active (they inherit color)
		if (!m_MainModularButton)
			m_MainModularButton = SCR_ModularButtonComponent.FindComponent(m_wRoot);
		if (!m_MainModularButton)
			return;

		m_MainModularButton.SetToggled(m_bDisabled, false);
		
		// Name
		Color color = Color.FromInt(UIColors.NEUTRAL_INFORMATION.PackToInt());
		if (m_bDisabled)
			color = Color.FromInt(UIColors.IDLE_DISABLED.PackToInt());
		
		SCR_ButtonEffectColor nameEffect = SCR_ButtonEffectColor.Cast(m_MainModularButton.FindEffect(SCR_ListEntryHelper.EFFECT_NAME_COLOR));
		if (nameEffect)
			nameEffect.m_cFocusLost = color;

		// Elements
		if (!m_bDisabled)
			color = Color.FromInt(UIColors.NEUTRAL_ACTIVE_STANDBY.PackToInt());
		
		array<SCR_ButtonEffectBase> effects = m_MainModularButton.FindAllEffects(SCR_ListEntryHelper.EFFECT_WRAPPER_COLOR);
		if (effects.IsEmpty())
			return;
		
		foreach (SCR_ButtonEffectBase effect : effects)
		{
			SCR_ButtonEffectColor colorEffect = SCR_ButtonEffectColor.Cast(effect);
			if (colorEffect)
				colorEffect.m_cFocusLost = color;
		}
		
		m_MainModularButton.InvokeAllEnabledEffects(false);
	}

	//------------------------------------------------------------------------------------------------
	protected void OpacityAnimation(int time, float opacityEnd)
	{
		AnimateWidget.Opacity(GetRootWidget(), opacityEnd, time);
	}

	// ---- PUBLIC ----
	//------------------------------------------------------------------------------------------------
	//! Animate whole widget opacity
	void AnimateOpacity(int delay, float animationTime, float opacityEnd, float opacityStart = -1)
	{
		if (opacityStart != -1)
			GetRootWidget().SetOpacity(opacityStart);

		GetGame().GetCallqueue().Remove(OpacityAnimation);
		
		// Start animating with a delay to prevent flickering
		GetGame().GetCallqueue().CallLater(OpacityAnimation, delay, false, animationTime, opacityEnd);
	}

	//------------------------------------------------------------------------------------------------
	void ShowMouseInteractionButtons(bool show)
	{
		m_bMouseButtonsEnabled = show;
		UpdateModularButtons();
	}

	//------------------------------------------------------------------------------------------------
	bool SetFavorite(bool favorite)
	{
		if (m_FavComponent)
			m_FavComponent.SetToggled(favorite);

		if (m_bFavorite == favorite)
			return false;

		m_bFavorite = favorite;

		if (m_OnFavorite)
			m_OnFavorite.Invoke(this, favorite);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	bool IsInnerButtonInteraction()
	{
		return m_bInnerButtonInteraction;
	}

	//------------------------------------------------------------------------------------------------
	bool IsFavorite()
	{
		return m_bFavorite;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsInErrorState()
	{
		return m_bIsInErrorState;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerString GetOnMouseInteractionButtonClicked()
	{
		if (!m_OnMouseInteractionButtonClicked)
			m_OnMouseInteractionButtonClicked = new ScriptInvokerString();

		return m_OnMouseInteractionButtonClicked;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerScriptedWidgetComponent GetOnFocus()
	{
		if (!m_OnFocus)
			m_OnFocus = new ScriptInvokerScriptedWidgetComponent();

		return m_OnFocus;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerScriptedWidgetComponent GetOnFocusLost()
	{
		if (!m_OnFocusLost)
			m_OnFocusLost = new ScriptInvokerScriptedWidgetComponent();

		return m_OnFocusLost;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerListMenuEntry GetOnFavorite()
	{
		if (!m_OnFavorite)
			m_OnFavorite = new ScriptInvokerListMenuEntry();

		return m_OnFavorite;
	}
}
