class SCR_AIGetOutActivity : SCR_AIActivityBase
{
	ref SCR_BTParam<IEntity> m_Vehicle = new SCR_BTParam<IEntity>(SCR_AIActionTask.ENTITY_PORT);
	ref SCR_BTParam<SCR_AIBoardingParameters> m_BoardingParameters = new SCR_BTParam<SCR_AIBoardingParameters>(SCR_AIActionTask.BOARDING_PARAMS_PORT);
	
	//-----------------------------------------------------------------------------------------------------------------------------------
	void InitParameters(IEntity vehicle, SCR_AIBoardingParameters boardingParameters, float priorityLevel)
	{
		m_Vehicle.Init(this, vehicle);
		m_fPriorityLevel.Init(this, priorityLevel);
		m_BoardingParameters.Init(this, boardingParameters);
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------
	void SCR_AIGetOutActivity(SCR_AIGroupUtilityComponent utility, AIWaypoint relatedWaypoint, IEntity vehicle, SCR_AIBoardingParameters boardingParameters = null, float priority = PRIORITY_ACTIVITY_GET_OUT, float priorityLevel = PRIORITY_LEVEL_NORMAL)
	{
		InitParameters(vehicle, boardingParameters, priorityLevel);
		
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Group/ActivityGetOut.bt";
		SetPriority(priority);
	}
	
	override void OnActionCompleted()
	{
		if (m_RelatedWaypoint)
			m_Utility.m_Owner.CompleteWaypoint(m_RelatedWaypoint);
		if (m_Vehicle.m_Value)
			m_Utility.m_Owner.RemoveUsableVehicle(m_Vehicle.m_Value);
		else
		{
			array<IEntity> groupVehicles = {};
			m_Utility.m_Owner.GetUsableVehicles(groupVehicles);
			foreach (IEntity vehicle : groupVehicles)
			{
				m_Utility.m_Owner.RemoveUsableVehicle(vehicle);
			}
		}
		super.OnActionCompleted();
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------
	override string GetActionDebugInfo()
	{
		string vehicleRef;
		if (!m_Vehicle.m_Value)
			vehicleRef = "all vehicles";
		else
			vehicleRef = m_Vehicle.ValueToString();	
		return this.ToString() + " leaving " + vehicleRef;
	}
};

