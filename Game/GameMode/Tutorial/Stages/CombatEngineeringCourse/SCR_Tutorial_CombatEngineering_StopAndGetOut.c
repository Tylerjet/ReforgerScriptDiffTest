[EntityEditorProps(insertable: false)]
class SCR_Tutorial_CombatEngineering_StopAndGetOutClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_CombatEngineering_StopAndGetOut : SCR_BaseTutorialStage
{
	protected Vehicle m_Car;
	
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_Car = Vehicle.Cast(GetGame().GetWorld().FindEntityByName("BUILDING_VEHICLE"));
		RegisterWaypoint("WP_CE_PATH6","", "GETOUT");
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (!m_Car)
			return false;

		return m_Car.GetPilot() != m_Player;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnStageFinished()
	{
		if (m_TutorialComponent && m_Car)
			m_TutorialComponent.ChangeVehicleLockState(m_Car, true);
		
		super.OnStageFinished();
	}
};