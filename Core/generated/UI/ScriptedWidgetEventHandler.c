/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup UI
\{
*/

class ScriptedWidgetEventHandler: Managed
{
	event bool OnClick(Widget w, int x, int y, int button);
	event bool OnModalResult(Widget w, int x, int y, int code, int result);
	event bool OnDoubleClick(Widget w, int x, int y, int button);
	event bool OnSelect(Widget w, int x, int y);
	event bool OnItemSelected(Widget w, int x, int y, int row, int column, int oldRow, int oldColumn);
	event bool OnFocus(Widget w, int x, int y);
	event bool OnFocusLost(Widget w, int x, int y);
	event bool OnMouseEnter(Widget w, int x, int y);
	event bool OnMouseLeave(Widget w, Widget enterW, int x, int y);
	event bool OnMouseWheel(Widget w, int x, int y, int wheel);
	event bool OnMouseButtonDown(Widget w, int x, int y, int button);
	event bool OnMouseButtonUp(Widget w, int x, int y, int button);
	//! control is one of ControlID
	event bool OnController(Widget w, int control, int value);
	event bool OnKeyDown(Widget w, int x, int y, int key);
	event bool OnKeyUp(Widget w, int x, int y, int key);
	event bool OnKeyPress(Widget w, int x, int y, int key);
	event bool OnChange(Widget w, int x, int y, bool finished);
	event bool OnNeedScroll(Widget w, int x, int y, float prevScrollPosX, float prevScrollPosY, float newScrollPosX, float newScrollPosY);
	event bool OnResize(Widget w, int x, int y);
	event bool OnChildAdd(Widget w, Widget child);
	event bool OnChildRemove(Widget w, Widget child);
	event bool OnUpdate(Widget w);
	event bool OnEvent(EventType eventType, Widget target, int parameter0, int parameter1);
	event bool OnModalClickOut(Widget modalRoot, int x, int y, int button);
	event void HandlerAttached(Widget w);
	event void HandlerDeattached(Widget w);

}

/*!
\}
*/
