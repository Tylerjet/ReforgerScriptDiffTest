//! Returns true if VoN UI is active
[BaseContainerProps(), BaseContainerCustomStringTitleField("Radial menu Active")]
class SCR_RadialMenuActiveActionCondition: SCR_AvailableActionCondition
{		
	//bool commandOpenTemp;
	protected SCR_RadialMenuManagerEditorComponent m_RadialMenu;
	
	//~Todo: Check if command menu is active
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;
		
		if (!m_RadialMenu)
		{
			m_RadialMenu = SCR_RadialMenuManagerEditorComponent.Cast(SCR_RadialMenuManagerEditorComponent.GetInstance(SCR_RadialMenuManagerEditorComponent));
			
			if (!m_RadialMenu)
				return GetReturnResult(false));
		}
		
		return GetReturnResult(m_RadialMenu.IsRadialMenuOpen()));
	}
};