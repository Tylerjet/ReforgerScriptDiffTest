/*Component for menu footers. 
Stores a reference to the buttons and deactivates them if the player is interacting with edit boxes or dropdown menus
The idea is to have it also handle button padding in the future*/

//------------------------------------------------------------------------------------------------
class SCR_DynamicFooterComponent : SCR_ScriptedWidgetComponent
{
	protected ref array<ref SCR_NavigationButtonComponent> m_wButtons = new array<ref SCR_NavigationButtonComponent>;

	protected Widget m_wLayoutRoot;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		
		super.HandlerAttached(w);
		
		array<ref Widget> children = {};
		SCR_WidgetHelper.GetAllChildren(w, children, true);
		
		if(children.IsEmpty())
			return;
		
		foreach(Widget child : children)
		{
			SCR_NavigationButtonComponent button = SCR_NavigationButtonComponent.GetNavigationButtonComponent(child.GetName(), m_wRoot);
			if(button)
				m_wButtons.Insert(button);
		}
		
		if(m_wButtons.IsEmpty())
			return;
		
		//TODO: handle padding in here
	}
};