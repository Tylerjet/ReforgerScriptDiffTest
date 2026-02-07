class SCR_AIDrawDebugSphere: AITaskScripted
{
	static const string PORT_ORIGIN	= "OriginIn";	
		
	[Attribute("1", UIWidgets.EditBox, desc: "Radius of sphere" )]
	private float m_fRadius;
		
	ref Shape sphere;
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		PORT_ORIGIN
	};
	override TStringArray GetVariablesIn()
    {
        return s_aVarsIn;
    }
	
	//------------------------------------------------------------------------------------------------
	override bool VisibleInPalette()
    {
        return true;
    }

	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
    {
#ifdef WORKBENCH
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AI_SHOW_DEBUG_SHAPES))
		{
			vector position;
			
			GetVariableIn(PORT_ORIGIN, position);
						
			sphere = Shape.CreateSphere(COLOR_GREEN_A, ShapeFlags.TRANSP | ShapeFlags.NOOUTLINE, position, m_fRadius);
		}		
#endif
		return ENodeResult.SUCCESS;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override string GetOnHoverDescription()
	{
		return "Draws a sphere of given radius";
	}	
};