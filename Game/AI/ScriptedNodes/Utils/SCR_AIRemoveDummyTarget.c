class SCR_AIRemoveDummyTarget: AITaskScripted
{
	static const string PORT_TARGET_ENTITY 	= "TargetIn";
	SCR_AITargetStatisticsComponent m_Statistics;
	
	override void OnInit(AIAgent owner)
	{
		m_Statistics = SCR_AITargetStatisticsComponent.Cast(owner.GetControlledEntity().FindComponent(SCR_AITargetStatisticsComponent));
		if (!m_Statistics)
			Debug.Error("Did not find statistics component");		
	}
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		IEntity target;		
		if(!GetVariableIn(PORT_TARGET_ENTITY,target))	
			return ENodeResult.FAIL;
		SCR_Global.DeleteEntityAndChildren(target);
		
		m_Statistics.ClearShots();
		return ENodeResult.SUCCESS;
	} 
	
	protected static ref TStringArray s_aVarsIn = {
		PORT_TARGET_ENTITY
	};
	override TStringArray GetVariablesIn()
    {
        return s_aVarsIn;
    }
	
	override bool VisibleInPalette()
	{
		return true;
	}	
	
	override string GetOnHoverDescription() 
	{ 
		return "RemoveDummyTarget: removes entity in In parameter (used for spawning and despawning target in aim statistics";	
	};
};