class SCR_AIDecoIsGroupInitializing : DecoratorScripted
{
	//------------------------------------------------------------
	override bool TestFunction(AIAgent owner)
	{
		SCR_AIGroup group = SCR_AIGroup.Cast(owner);
		
		return group && group.IsInitializing();
	}
	
	//------------------------------------------------------------
	override string GetOnHoverDescription() { return "Tests SCR_AIGroup.IsInitializing"; }
}