// Script File//------------------------------------------------------------------------------------------------
class SCR_AIIsTargetVisible : DecoratorScripted
{
	static const string BASE_TARGET_PORT = "BaseTargetIn";
	static const string LAST_POSITION_PORT = "LastPositionOut";
	
	[Attribute("1", UIWidgets.EditBox, "How long in past target becomes invisibile", "")]
	float m_lastSeenMax;
	
	//-----------------------------------------------------------------------------------------------------
	protected override bool TestFunction(AIAgent owner)
	{
		BaseTarget target;
		GetVariableIn(BASE_TARGET_PORT,target);
		ClearVariable(LAST_POSITION_PORT);
		
		if (!target)
			return false;
		SetVariableOut(LAST_POSITION_PORT,target.GetLastSeenPosition());
		return target.GetTimeSinceSeen() < m_lastSeenMax;
		
	}
	
	//-----------------------------------------------------------------------------------------------------
	protected override bool VisibleInPalette()
	{
		return true;
	}	
	
	//-----------------------------------------------------------------------------------------------------
	protected override string GetOnHoverDescription()
	{
		return "IsTargetVisible: checks timeout of visibility of provided base target. Returns true if it is below lastSeenMax parameter.";
	}
	
	//-----------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		BASE_TARGET_PORT
	};
	protected override TStringArray GetVariablesIn()
	{
		return s_aVarsIn;
	}
	
	//-----------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		LAST_POSITION_PORT
	};
	protected override TStringArray GetVariablesOut()
	{
		return s_aVarsOut;
	}
};
