 class SCR_AIDecoHasActionOfType : DecoratorScripted
{
	[Attribute("0", UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EAIActionType))]
	protected EAIActionType m_eActionType;
	
	protected SCR_AIBaseUtilityComponent m_Utility;
	
	//------------------------------------------------------------------------------------------------
	protected override bool TestFunction(AIAgent owner)
	{
		if (!m_Utility)
		{
			m_Utility = SCR_AIBaseUtilityComponent.Cast(owner.FindComponent(SCR_AIBaseUtilityComponent));
			if (!m_Utility)
				return false;
		}	
		
		return m_Utility.HasActionOfType(m_eActionType);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override bool VisibleInPalette()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override string GetOnHoverDescription()
	{
		return "Returns true when we have an action of specified type";
	}
	
	//------------------------------------------------------------------------------------------------
	protected override string GetNodeMiddleText()
	{
		return string.Format("Action type: %1", typename.EnumToString(EAIActionType, m_eActionType));
	}
};