class SCR_RadialMenuVisualsEditorActions : SCR_RadialMenuVisuals
{
	// Widgets 
	
	protected IEntity m_eOwner;
	
	
	//------------------------------------------------------------------------------------------------
	override protected event void OnOpen(IEntity owner)
	{
		super.OnOpen(owner);
		m_eOwner = owner;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void SetElementData(SCR_RadialMenuPair<Widget, BaseSelectionMenuEntry> element, bool canBePerformed, SCR_SelectionEntryWidgetComponent widgetComp)
	{
		super.SetElementData(element, canBePerformed, widgetComp);
		
		if(!element)
			return;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void UpdateSelection(bool showSelector, bool performable, float selectorAngle, float selectedAngle, BaseSelectionMenuEntry selectedEntry)
	{
		super.UpdateSelection(showSelector, performable, selectorAngle, selectedAngle, selectedEntry);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void UpdateLastSelected(BaseSelectionMenuEntry selectedEntry, float angle)
	{
		super.UpdateLastSelected(selectedEntry, angle);
	}
};