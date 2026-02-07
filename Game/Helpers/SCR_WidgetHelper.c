class SCR_WidgetHelper
{
	static void GetAllChildren(notnull Widget widget, notnull out array<ref Widget> widgetArray)
	{
		widgetArray.Clear();
		Widget child = widget.GetChildren();
		while (child)
		{
			widgetArray.Insert(child);
			child = child.GetSibling();
		}
	}

	static void RemoveAllChildren(notnull Widget widget)
	{
		Widget oldChild;
		Widget child = widget.GetChildren();
		while (child)
		{
			oldChild = child;
			child = child.GetSibling();
			oldChild.RemoveFromHierarchy();
		}
	}

	static void ResizeToImage(notnull ImageWidget widget, int imageIndex = 0)
	{
		int x, y;
		widget.GetImageSize(imageIndex, x, y);
		widget.SetSize(x, y);
	}

	static Widget GetWidgetOrChild(notnull Widget widget, string widgetName)
	{
		if (widget.GetName() == widgetName)
		{
			return widget;
		}
		return widget.FindAnyWidget(widgetName);
	}
};

/*
class SCR_WidgetHelperT<Class T>
{
	static T GetScriptedComponent(notnull Widget widget)
	{
		return T.Cast(widget.FindHandler(T));
	}
};
// */
