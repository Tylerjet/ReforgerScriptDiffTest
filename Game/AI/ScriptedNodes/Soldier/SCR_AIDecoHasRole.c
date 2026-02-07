class SCR_AIDecoHasRole : DecoratorScripted
{
	[Attribute("0", UIWidgets.ComboBox, "Role to find:", "", ParamEnumArray.FromEnum(EUnitRole) )]
	private EUnitRole m_eUnitRole;
		
	protected SCR_AIInfoComponent m_AIInfoComponent;	

	//-------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		m_AIInfoComponent = SCR_AIInfoComponent.Cast(owner.FindComponent(SCR_AIInfoComponent));
		if( !m_AIInfoComponent )
		{
			NodeError(this, owner, "Agent does not have a valid m_AIInfoComponent");
			return;
		}
	}
	
	//-------------------------------------------------------------------
	protected override bool TestFunction(AIAgent owner)
	{
		return m_AIInfoComponent.HasRole(m_eUnitRole);
	}	
	
	//-------------------------------------------------------------------	
	protected override bool VisibleInPalette()
	{
		return true;
	}
	
	//-------------------------------------------------------------------
	protected override string GetOnHoverDescription()
	{
		return "Decorator that tests if character has requested role";
	}	
	
	//-------------------------------------------------------------------
	override string GetNodeMiddleText() 
	{ 
		return "Test " + typename.EnumToString(EUnitRole,m_eUnitRole);	
	};
};