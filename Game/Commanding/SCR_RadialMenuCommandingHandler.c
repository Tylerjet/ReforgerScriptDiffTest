//------------------------------------------------------------------------------------------------
//! Expanded radial menu class for commanding radial menu
class SCR_RadialMenuCommandingHandler : SCR_RadialMenuHandler
{
	protected float m_fTimeFromLastUpdate = 0;
	
	protected const float DISTANCE_UPDATE_FREQUENCY = 0.1;
	
	//------------------------------------------------------------------------------------------------
	protected override event void OnUpdate(IEntity owner, float timeSlice)
	{
		super.OnUpdate(owner, timeSlice);
		
		if (!IsOpen())
			return;
		
		m_fTimeFromLastUpdate += timeSlice;
		
		if (m_fTimeFromLastUpdate < DISTANCE_UPDATE_FREQUENCY)
			return;
		
		m_fTimeFromLastUpdate = 0;
		
		SCR_RadialMenuVisualsCommands commandingVisuals = SCR_RadialMenuVisualsCommands.Cast(m_RadialMenuVisuals);
		if (!commandingVisuals)
			return;	
		
		commandingVisuals.UpdateTargetDistance();
	}
}