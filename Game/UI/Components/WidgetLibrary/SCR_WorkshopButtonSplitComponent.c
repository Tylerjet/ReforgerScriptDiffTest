// Script File
//------------------------------------------------------------------------------------------------
class SCR_WorkshopButtonSplitComponent : SCR_ButtonSplitComponent 
{
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseButtonDown(Widget w, int x, int y, int button)
	{
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	/*
	override bool OnFocus(Widget w, int x, int y)
	{
		m_OnFocus.Invoke(w);
		super.OnFocus(w, x, y);
		ColorizeWidgets(COLOR_BACKGROUND_HOVERED, COLOR_CONTENT_HOVERED);
		//GetGame().GetInputManager().AddActionListener("MenuSelect", EActionTrigger.DOWN, OnMenuSelect);
		return false;
	}*/
	
	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		//super.OnClick(w, x, y, button);
		if (button != 0)
			return false;
		
		m_OnClicked.Invoke(w);
		return false;
	}
};