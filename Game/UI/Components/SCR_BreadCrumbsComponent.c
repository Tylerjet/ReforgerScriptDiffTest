class SCR_BreadCrumbsComponent : SCR_ScriptedWidgetComponent
{
	[Attribute()]
	protected ref array<string> m_aContent;

	[Attribute(defvalue: "{86576034F67F9F64}UI/layouts/Menus/Breadcrumbs/BreadCrumbsElement.layout")]
	protected ResourceName m_ElementLayout;

	[Attribute(defvalue: "{287A657BF9A33943}UI/layouts/Menus/Breadcrumbs/BreadCrumbsRichElement.layout")]
	protected ResourceName m_RichElementLayout;

	[Attribute(defvalue: "{240DC2208952C2C2}UI/layouts/Menus/Breadcrumbs/BreadCrumbsSeparator.layout")]
	protected ResourceName m_SeparatorLayout;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		Refresh();
	}

	//------------------------------------------------------------------------------------------------
	void Clear()
	{
		if (!m_aContent)
			return;

		m_aContent.Clear();
		Refresh();
	}

	//------------------------------------------------------------------------------------------------
	void Set(array<string> values)
	{
		if (!values) // can be empty
			return;

		if (!m_aContent)
			m_aContent = {};
		else
			m_aContent.Clear();

		foreach (string value : values)
		{
			value = value.Trim();
			if (value)
				m_aContent.Insert(value);
		}

		Refresh();
	}

	//------------------------------------------------------------------------------------------------
	void Set(string value1, string value2 = "", string value3 = "")
	{
		array<string> values = { value1, value2, value3 };
		Set(values);
	}

	//------------------------------------------------------------------------------------------------
	void SetFormat(string value, string arg1, string arg2 = "")
	{
		SCR_WidgetHelper.RemoveAllChildren(m_wRoot);

		Widget widget = GetGame().GetWorkspace().CreateWidgets(m_ElementLayout, m_wRoot);
		if (!widget)
			return;

		TextWidget textWidget = TextWidget.Cast(widget);
		if (!textWidget)
			return;

		textWidget.SetTextFormat(value, arg1, arg2);
		m_aContent = { value };
	}

	//------------------------------------------------------------------------------------------------
	void SetRichFormat(string value, string arg1, string arg2 = "")
	{
		SCR_WidgetHelper.RemoveAllChildren(m_wRoot);

		Widget widget = GetGame().GetWorkspace().CreateWidgets(m_RichElementLayout, m_wRoot);
		if (!widget)
			return;

		RichTextWidget richTextWidget = RichTextWidget.Cast(widget);
		if (!richTextWidget)
			return;

		richTextWidget.SetTextFormat(value, arg1, arg2);
		m_aContent = { value };
	}

	//------------------------------------------------------------------------------------------------
	protected void Refresh()
	{
		SCR_WidgetHelper.RemoveAllChildren(m_wRoot);

		if (!m_aContent || !m_aContent.Count() || !m_ElementLayout)
			return;

		Widget widget;
		TextWidget textWidget;
		array<TextWidget> createdWidgets = {};
		for (int i, count = m_aContent.Count(); i < count; i++)
		{
			widget = GetGame().GetWorkspace().CreateWidgets(m_ElementLayout, m_wRoot);
			if (!widget)
				continue;

			textWidget = TextWidget.Cast(widget);
			if (!textWidget)
				continue;

			textWidget.SetText(m_aContent[i]);
			createdWidgets.Insert(textWidget);

			if (m_SeparatorLayout && i < count -1)
				GetGame().GetWorkspace().CreateWidgets(m_SeparatorLayout, m_wRoot);
		}

		if (!createdWidgets.Count())
			return;

		float left, top, right, bottom;

		widget = createdWidgets[0];
		LayoutSlot.GetPadding(widget, left, top, right, bottom);
		LayoutSlot.SetPadding(widget, 0, top, right, bottom);

		widget = createdWidgets[createdWidgets.Count() - 1];
		LayoutSlot.GetPadding(widget, left, top, right, bottom);
		LayoutSlot.SetPadding(widget, left, top, 0, bottom);
	}
};
