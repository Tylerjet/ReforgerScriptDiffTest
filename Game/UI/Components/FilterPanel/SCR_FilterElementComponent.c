/*
Element of a filter listbox.
Compared to a normal list box element, it also has as checker or a radio button on the right.
*/
class SCR_FilterElementComponent : SCR_ListBoxElementComponent
{	
	// ------------ Public -------------------
	
	/*
	//------------------------------------------------------------------------------------------------
	void EnableRadioChecker(bool enable)
	{
		m_Widgets.m_RadioChecker.SetVisible(enable);
	}
	
	//------------------------------------------------------------------------------------------------
	void EnableToggleChecker(bool enable)
	{
		m_Widgets.m_ToggleChecker.SetVisible(enable);
	}
	*/
	
	// ------------- Private / Protected --------------
	
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
	
		m_bToggledOnlyThroughApi = true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetToggled(bool toggled, bool invokeOnToggled = true)
	{
	}
};