class SCR_AIPrintAimStatistics: AITaskScripted
{
	static const string PORT_TARGET_ENTITY = "TargetEntity";
	
	[Attribute( defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Visualize screens to debug accuracy" )]
	bool m_bVisualizeStatistics;
	
	[Attribute( defvalue: "1 1 1", uiwidget: UIWidgets.Coords, desc: "Visualize target axis scale" )]
	vector m_vScaleTarget;
	
	private IEntity m_ownerEntity, m_targetEntity;
	private SCR_AITargetStatisticsComponent m_targetStatisticsComp;	
	
	override void OnInit(AIAgent owner)
	{
		m_ownerEntity = owner.GetControlledEntity();	
		if (m_ownerEntity)
			m_targetStatisticsComp = SCR_AITargetStatisticsComponent.Cast(m_ownerEntity.FindComponent(SCR_AITargetStatisticsComponent));
	}
	
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_ownerEntity || !m_targetStatisticsComp)
			return ENodeResult.FAIL;
		
		if (!GetVariableIn(PORT_TARGET_ENTITY, m_targetEntity))
			return ENodeResult.FAIL;			
		
		if (m_bVisualizeStatistics)
		{
			m_targetStatisticsComp.VisualizeStatistics(owner,m_targetEntity.GetOrigin(),m_vScaleTarget);
		}
		
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
		return "PrintAimStatistics: shows collected statistics from SCR_AITargetStatisticsComponent on screen";	
	};
};