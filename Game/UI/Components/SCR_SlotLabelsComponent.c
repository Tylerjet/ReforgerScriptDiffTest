class SCR_SlotLabelsComponent : ScriptedWidgetComponent
{
    override void HandlerAttached(Widget w)
    {
        #ifdef WORKBENCH
			Game g = GetGame();
			GenericEntity world = GetGame().GetWorldEntity();
			
			if (world)
				return;
			
	        Widget child = w.GetChildren();
	        while (child)
	        {
	            TextWidget text = TextWidget.Cast(GetGame().GetWorkspace().CreateWidget(WidgetType.TextWidgetTypeID, WidgetFlags.VISIBLE, new Color(1,0,1,1), 0, child));
	            text.SetText(child.GetName());
	            child = child.GetSibling();
	        }
        #endif
    }
};