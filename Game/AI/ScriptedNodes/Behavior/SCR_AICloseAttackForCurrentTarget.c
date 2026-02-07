class SCR_AICloseAttackForCurrentTarget : AITaskScripted
{
	[Attribute("1", UIWidgets.ComboBox, "Message type", "", ParamEnumArray.FromEnum(ENodeResult) )]
	private ENodeResult m_eBehaviorResult;

	protected SCR_AIUtilityComponent m_UtilityComp;
	
	//------------------------------------------------------------------------------------------------
	override void OnEnter(AIAgent owner)
	{
		m_UtilityComp = SCR_AIUtilityComponent.Cast(owner.FindComponent(SCR_AIUtilityComponent));
		if (!m_UtilityComp)
		{
			NodeError(this, owner, "Can't find utility component.");
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_UtilityComp)
			return ENodeResult.FAIL;
		
		/*if (m_UtilityComp.CloseAttackForCurrentTarget(m_eBehaviorResult))
			return ENodeResult.SUCCESS;
		*/
		return ENodeResult.FAIL;		
	}

	//------------------------------------------------------------------------------------------------
	protected override bool VisibleInPalette()
	{
		return true;
	}	

	//------------------------------------------------------------------------------------------------
	override protected string GetNodeMiddleText()
	{
		string result;
		if (m_eBehaviorResult == ENodeResult.SUCCESS)
			result = "COMPLETE";		
		else
			result = "FAIL";
		return "Attack behavior will: " + result;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override string GetOnHoverDescription()
	{
		return "Completes or Fails attack of current enemy given by CombatComponent, node fails if attack couldnt be failed.";
	}		

};