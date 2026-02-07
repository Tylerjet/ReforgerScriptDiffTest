//------------------------------------------------------------------------------------------------
//! Attach this component to a widget in a map layout to make the widget draggable
class SCR_MapElementMoveComponent : ScriptedWidgetComponent
{
	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		SCR_MapDragComponent.SetDraggedWidget(w);

		return true;
	}
};
