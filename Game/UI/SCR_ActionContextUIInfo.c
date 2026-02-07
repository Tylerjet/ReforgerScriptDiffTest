class SCR_ActionContextUIInfo : UIInfo
{
	[Attribute("{BF5FA7B21D658280}UI/layouts/HUD/InteractionSystem/ContextBasicInteractionBlip.layout", UIWidgets.ResourceNamePicker, "Layout", "layout")]
	protected ResourceName m_sLayoutName;

	protected const string DEFAULT_ICON = "use";

	protected Widget m_wAssignedWidget;

	// Invokes when a Widget is assigned to the context this UIInfo is part of
	// Passes the assigned Widget so it can be used in other scripts
	protected ref ScriptInvokerWidget m_OnWidgetAssigned;

	//------------------------------------------------------------------------------------------------
	//! Set icon to given image widgets.
	//! \param[in] ImageWidget Target image widget
	//! \param[in] ImageWidget Target glow widget. Can be null
	//! \return True when the image was set
	bool SetIconTo(ImageWidget imageWidget, ImageWidget glowWidget = null)
	{
		if (!imageWidget)
			return false;

		ResourceName icon = GetIconPath();

		// If no Icon is defined, use the default one.
		if (!icon)
			icon = UIConstants.ICONS_IMAGE_SET;

		string ext;
		FilePath.StripExtension(icon, ext);
		if (ext == "imageset")
			imageWidget.LoadImageFromSet(0, icon, DEFAULT_ICON);
		else
			imageWidget.LoadImageTexture(0, icon);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Get the layout which will be shwon for the context
	//! \return layout
	ResourceName GetLayout()
	{
		return m_sLayoutName;
	}

	//------------------------------------------------------------------------------------------------
	//! Gets called everytime a new Widget gets assigned to the context this UIInfo is part of.
	//! \param[in] Widget Widget that is assigned to this context.
	void OnWidgetAssigned(Widget widget)
	{
		m_wAssignedWidget = widget;

		if (m_OnWidgetAssigned)
			m_OnWidgetAssigned.Invoke(widget);
	}

	//------------------------------------------------------------------------------------------------
	//! Get the currently assigned Widget
	//! Use this to check if a Widget was already assigned to the context before you listened to the ScriptInvoker
	Widget GetAssignedWidget()
	{
		return m_wAssignedWidget;
	}

	//------------------------------------------------------------------------------------------------
	//! Listen to this ScriptInvoker to detect if and what Widget is assigned to it, to modify it as needed.
	ScriptInvokerWidget GetOnWidgetAssigned()
	{
		if (!m_OnWidgetAssigned)
			m_OnWidgetAssigned = new ScriptInvokerWidget();

		return m_OnWidgetAssigned;
	}
}
