class SCR_AICharacterStats : AITaskScripted
{
	protected SCR_AIInfoComponent m_AIInfo;
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		m_AIInfo = SCR_AIInfoComponent.Cast(owner.FindComponent(SCR_AIInfoComponent));
		if (!m_AIInfo)
			NodeError(this, owner, "Can't find AIInfo component.");		
	}
};
