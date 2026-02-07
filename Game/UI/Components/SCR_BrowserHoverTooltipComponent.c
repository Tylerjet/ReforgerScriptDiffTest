class SCR_BrowserHoverTooltipComponent : SCR_ScriptedWidgetComponent
{
	[Attribute("{CD470B05867A77D5}UI/layouts/Menus/ServerBrowser/TooltipWidget.layout", params:"layout")]
	protected ResourceName m_sTooltipLayout;

	[Attribute("{87037226B1A2064B}UI/layouts/WidgetLibrary/Buttons/WLib_NavigationButtonSuperSmall.layout", params:"layout")]
	protected ResourceName m_sButtonsLayout;

	[Attribute(desc: "Actions to be shown in the Tooltip")]
	protected ref array<ref SCR_BrowserTooltipButtonPresetData> m_aButtons;

	[Attribute("1.000000 1.000000 1.000000 1.000000", UIWidgets.ColorPicker)] //0.508995 0.508995 0.502998 1.000000
	protected ref Color m_ButtonsColor;

	[Attribute("1")]
	protected float m_fButtonsOpacity;

	[Attribute("12", desc: "Spacing between each button")]
	protected int m_iButtonsPadding;

	[Attribute("", desc: "Message to display")]
	protected string m_sMessageText;

	[Attribute("0", UIWidgets.ComboBox, "Horizontal Alignment when using mouse", "", ParamEnumArray.FromEnum(SCR_ETooltipAlignmentHorizontal))]
	protected SCR_ETooltipAlignmentHorizontal m_eHorizontalAlignmentMouse;

	[Attribute("3", UIWidgets.ComboBox, "Horizontal Alignment when using gamepad", "", ParamEnumArray.FromEnum(SCR_ETooltipAlignmentHorizontal))]
	protected SCR_ETooltipAlignmentHorizontal m_eHorizontalAlignmentGamepad;

	[Attribute("0", UIWidgets.ComboBox, "Vertical Alignment when using mouse", "", ParamEnumArray.FromEnum(SCR_ETooltipAlignmentVertical))]
	protected SCR_ETooltipAlignmentVertical m_eVerticalAlignmentMouse;

	[Attribute("2", UIWidgets.ComboBox, "Vertical Alignment when using gamepad", "", ParamEnumArray.FromEnum(SCR_ETooltipAlignmentVertical))]
	protected SCR_ETooltipAlignmentVertical m_eVerticalAlignmentGamepad;

	[Attribute("15 15 0", desc: "Tooltip offset from cursor position")]
	protected vector m_vOffsetCursor;

	[Attribute("0 -3 0", desc: "Tooltip offset from hover widget border")]
	protected vector m_vOffsetBorder;

	[Attribute("1", UIWidgets.CheckBox)]
	protected bool m_bFollowMouse;

	protected Widget m_wTooltip;
	protected Widget m_wInputsLayout;
	protected Widget m_wMessageLayout;
	protected RichTextWidget m_wMessage;

	protected ref array<ref SCR_BrowserTooltipButtonPresetData> m_aScriptButtons = {};
	protected ref map<string, SCR_NavigationButtonComponent> m_aButtonComponents = new map<string, SCR_NavigationButtonComponent>();


	//------------------------------------------------------------------------------------------------
	Widget CreateTooltip()
	{
		SCR_ETooltipAlignmentHorizontal horizontalAlignment;
		SCR_ETooltipAlignmentVertical verticalAlignment;
		vector offset = vector.Zero;

		if (GetGame().GetInputManager().GetLastUsedInputDevice() == EInputDeviceType.MOUSE)
		{
			horizontalAlignment = m_eHorizontalAlignmentMouse;
			verticalAlignment = m_eVerticalAlignmentMouse;
		}
		else
		{
			horizontalAlignment = m_eHorizontalAlignmentGamepad;
			verticalAlignment = m_eVerticalAlignmentGamepad;
		}

		//! Adjust offset
		if (horizontalAlignment == SCR_ETooltipAlignmentHorizontal.CURSOR)
			offset[0] = m_vOffsetCursor[0];
		else
			offset[0] = m_vOffsetBorder[0];

		if (verticalAlignment == SCR_ETooltipAlignmentVertical.CURSOR)
			offset[1] = m_vOffsetCursor[1];
		else
			offset[1] = m_vOffsetBorder[1];

		//! --- TOOLTIP ---
		m_wTooltip = SCR_TooltipManagerEntity.CreateTooltip(m_sTooltipLayout, m_wRoot, m_bFollowMouse, offset, horizontalAlignment, verticalAlignment);

		if (!m_wTooltip)
			return null;

		//! --- MESSAGE ---
		m_wMessageLayout = m_wTooltip.FindAnyWidget("MessageLayout");
		if (m_wMessageLayout)
			m_wMessage = RichTextWidget.Cast(m_wMessageLayout.FindAnyWidget("Message"));

		SetMessage(m_sMessageText);

		//! --- BUTTONS ---
		m_wInputsLayout = m_wTooltip.FindAnyWidget("InputsLayout");

		//! Add buttons & handle padding
		m_aButtonComponents.Clear();

		int count;
		int padding;
		foreach (SCR_BrowserTooltipButtonPresetData buttonPreset : m_aButtons)
		{
			if (count != 0)
				padding = m_iButtonsPadding;

			CreateButton(buttonPreset, m_wInputsLayout, padding);
			count++;
		}

		return m_wTooltip;
	}

	//------------------------------------------------------------------------------------------------
	void ForceDeleteTooltip()
	{
		SCR_TooltipManagerEntity.DeleteTooltip();
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_NavigationButtonComponent CreateButton(SCR_BrowserTooltipButtonPresetData buttonPreset, Widget buttonContainer, int padding)
	{
		if (!m_wTooltip || !buttonContainer)
			return null;

		//! Create button
		Widget w = GetGame().GetWorkspace().CreateWidgets(m_sButtonsLayout, buttonContainer);
		if (!w)
			return null;

		//! Padding
		AlignableSlot.SetPadding(w, padding, 0, 0, 0);

		//! Setup
		SCR_NavigationButtonComponent comp = SCR_NavigationButtonComponent.Cast(w.FindHandler(SCR_NavigationButtonComponent));
		if (!comp)
		{
			GetGame().GetWorkspace().RemoveChild(w);
			return null;
		}

		comp.SetVisible(buttonPreset.m_bShowButton, false);
		comp.SetLabel(buttonPreset.m_sLabel);
		comp.SetAction(buttonPreset.UpdateDisplayedAction());
		comp.SetColorActionDisabled(m_ButtonsColor);
		comp.SetDisabledOpacity(m_fButtonsOpacity);
		comp.SetEnabled(false);

		//! Store
		m_aButtonComponents.Insert(buttonPreset.m_sTag, comp);

		return comp;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns a button with given tag
	SCR_NavigationButtonComponent FindButton(string tag)
	{
		return m_aButtonComponents.Get(tag);
	}


	//------------------------------------------------------------------------------------------------
	//! Returns a button preset with given tag from the ones that have been set in the parent layout
	SCR_BrowserTooltipButtonPresetData FindButtonPresetData(string tag)
	{
		foreach (SCR_BrowserTooltipButtonPresetData button : m_aButtons)
		{
			if (button.m_sTag == tag)
				return button;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Adds a button to the existing Tooltip
	SCR_NavigationButtonComponent AddButton(string tag, string label, string actionName, string actionNameMouse = string.Empty, bool showButton = true)
	{
		//! Check if there's a tooltip already
		if (!m_wTooltip)
			return null;

		SCR_BrowserTooltipButtonPresetData buttonPreset = new SCR_BrowserTooltipButtonPresetData();
		buttonPreset.SetData(tag, label, actionName, actionNameMouse, showButton);

		//! Leave padding at 0 if there are no other buttons
		int padding;
		if (!m_aButtonComponents.IsEmpty())
			padding = m_iButtonsPadding;

		SCR_NavigationButtonComponent newButton = CreateButton(buttonPreset, m_wInputsLayout, padding);
		return newButton;
	}

	//------------------------------------------------------------------------------------------------
	//! Removes an existing button
	bool RemoveButton(string tag)
	{
		if (!m_wTooltip)
			return false;
		
		SCR_NavigationButtonComponent button = FindButton(tag);

		if (!button)
			return false;

		m_aButtonComponents.Remove(tag);
		m_wTooltip.RemoveChild(button.GetRootWidget());
		return true;
	}


	//------------------------------------------------------------------------------------------------
	//! Sets up a button to be created when the Tooltip appears
	SCR_BrowserTooltipButtonPresetData AddSetupButton(string tag, string label, string actionName, string actionNameMouse = string.Empty, bool showButton = true)
	{
		SCR_BrowserTooltipButtonPresetData buttonPreset = new SCR_BrowserTooltipButtonPresetData;
		buttonPreset.SetData(tag, label, actionName, actionNameMouse, showButton);

		//! Prevent duplicates
		foreach (SCR_BrowserTooltipButtonPresetData button : m_aButtons)
		{
			if (button.m_sTag == tag)
				return null;
		}

		m_aScriptButtons.Insert(buttonPreset);
		m_aButtons.Insert(buttonPreset);
		return buttonPreset;
	}

	//------------------------------------------------------------------------------------------------
	//! Prevents the button to be created when the Tooltip appears
	bool RemoveSetupButton(string tag)
	{
		foreach (SCR_BrowserTooltipButtonPresetData buttonPreset : m_aScriptButtons)
		{
			if (tag == buttonPreset.m_sTag)
			{
				m_aScriptButtons.RemoveItem(buttonPreset);
				m_aButtons.RemoveItem(buttonPreset);
				return true;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Removes all buttons that might have been added to be created when the Tooltip appears
	void ClearSetupButtons()
	{
		foreach (SCR_BrowserTooltipButtonPresetData buttonPreset : m_aScriptButtons)
		{
			m_aButtons.RemoveItem(buttonPreset);
		}

		m_aScriptButtons.Clear();
	}

	//------------------------------------------------------------------------------------------------
	//! Changes button actions depending on last input device
	void UpdateAllButtonActions()
	{
		foreach (SCR_BrowserTooltipButtonPresetData buttonData : m_aButtons)
		{
			UpdateButtonAction(buttonData.m_sTag);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Changes a button's action depending on last input device
	void UpdateButtonAction(string tag)
	{
		//Button settings
		SCR_BrowserTooltipButtonPresetData buttonData = FindButtonPresetData(tag);

		if (!buttonData)
			return;

		string actionName = buttonData.UpdateDisplayedAction();
		SCR_NavigationButtonComponent button = FindButton(tag);
		if (button)
			button.SetAction(actionName);
	}


	//------------------------------------------------------------------------------------------------
	static SCR_BrowserHoverTooltipComponent FindComponent(notnull Widget w)
	{
		return SCR_BrowserHoverTooltipComponent.Cast(w.FindHandler(SCR_BrowserHoverTooltipComponent));
	}

	//------------------------------------------------------------------------------------------------
	bool SetMessage(string message)
	{
		m_sMessageText = message;

		if (!m_wMessage)
			return false;

		m_wMessage.SetText(message);
		SetMessageVisible(!message.IsEmpty());
		return true;
	}

	//------------------------------------------------------------------------------------------------
	bool SetMessageVisible(bool newVisible)
	{
		if (!m_wMessageLayout)
			return false;

		bool visible = newVisible && !GetMessage().IsEmpty();

		m_wMessageLayout.SetVisible(visible);

		//! Padding
		if (!m_aButtons.IsEmpty())
			AlignableSlot.SetPadding(m_wMessageLayout, m_iButtonsPadding, 0, 0, 0);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	string GetMessage()
	{
		return m_sMessageText;
	}

	//------------------------------------------------------------------------------------------------
	bool IsMessageVisible()
	{
		return m_wMessageLayout && m_wMessageLayout.IsVisible();
	}

	//------------------------------------------------------------------------------------------------
	Widget GetTooltip()
	{
		return SCR_TooltipManagerEntity.GetTooltip();
	}
};

//------------------------------------------------------------------------------------------------
//! Configuration for a button. These buttons are purely visual hints, their actions are disabled
[BaseContainerProps()]
class SCR_BrowserTooltipButtonPresetData
{
	[Attribute(desc: "Custom tag, used for finding this button at run time")]
	string m_sTag;

	[Attribute(desc: "Label of the button")]
	string m_sLabel;

	[Attribute(desc: "Action name the button will display")]
	string m_sActionName;

	[Attribute(desc: "Action name the button will display when using mouse")]
	string m_sActionNameMouse;

	[Attribute("1")]
	bool m_bShowButton;

	protected string m_sActionToDisplay;

	//------------------------------------------------------------------------------------------------
	void SetData(string tag, string label, string actionName, string actionNameMouse, bool showButton)
	{
		m_sTag = tag;
		m_sLabel = label;
		m_sActionName = actionName;
		m_sActionNameMouse = actionNameMouse;
		m_bShowButton = showButton;
	}

	//------------------------------------------------------------------------------------------------
	string GetDisplayedAction()
	{
		return m_sActionToDisplay;
	}

	//------------------------------------------------------------------------------------------------
	string UpdateDisplayedAction()
	{
		//! As far as my knowledge goes, actions bound to both mouse and keyboard will always display both icons, so the action needs to be swapped manually.
		//! The Enfusion team is already looking into the issue.
		
		EInputDeviceType inputDeviceType = GetGame().GetInputManager().GetLastUsedInputDevice();

		if (inputDeviceType == EInputDeviceType.MOUSE && !m_sActionNameMouse.IsEmpty())
			m_sActionToDisplay = m_sActionNameMouse;
		else
			m_sActionToDisplay = m_sActionName;

		return m_sActionToDisplay;
	}
};
