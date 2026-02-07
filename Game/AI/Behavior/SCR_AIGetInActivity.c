class SCR_AIGetInActivity : SCR_AIActivityBase
{
	ref SCR_BTParam<IEntity> m_Vehicle = new SCR_BTParam<IEntity>(SCR_AIActionTask.ENTITY_PORT);
	ref SCR_BTParam<SCR_AIBoardingParameters> m_BoardingParameters = new SCR_BTParam<SCR_AIBoardingParameters>(SCR_AIActionTask.BOARDING_PARAMS_PORT);
	ref SCR_BTParam<ECompartmentType> m_eRoleInVehicle = new SCR_BTParam<ECompartmentType>(SCR_AIActionTask.ROLEINVEHICLE_PORT);
	protected SCR_AIGroup m_OwnerGroup;
	
	void InitParameters(IEntity vehicle, SCR_AIBoardingParameters parameters, ECompartmentType role, float priorityLevel)
	{
		m_Vehicle.Init(this, vehicle);
		m_BoardingParameters.Init(this, parameters);
		m_eRoleInVehicle.Init(this, role);
		m_fPriorityLevel.Init(this, priorityLevel);
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_AIGetInActivity(SCR_AIGroupUtilityComponent utility, AIWaypoint relatedWaypoint, IEntity vehicle, SCR_AIBoardingParameters boardingParameters = null, ECompartmentType role = ECompartmentType.CARGO, float priority = PRIORITY_ACTIVITY_GET_IN, float priorityLevel = PRIORITY_LEVEL_NORMAL)
	{
		InitParameters(vehicle, boardingParameters, role, priorityLevel);
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Group/ActivityGetIn.bt";
		SetPriority(priority);
		if (utility)
			m_OwnerGroup = utility.m_Owner;
	}
	
	//------------------------------------------------------------------------------------------------
	override string GetActionDebugInfo()
	{
		return this.ToString() + " moving to " + m_Vehicle.ValueToString() + " as " + m_eRoleInVehicle.ValueToString();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActionDeselected()
	{
		m_OwnerGroup.ReleaseCompartments();
		super.OnActionDeselected();	
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActionSelected()
	{
		if (m_Vehicle.m_Value)
			m_OwnerGroup.AddUsableVehicle(m_Vehicle.m_Value);
		super.OnActionDeselected();	
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActionCompleted()
	{
		if (m_RelatedWaypoint)
			m_OwnerGroup.CompleteWaypoint(m_RelatedWaypoint);
		super.OnActionCompleted();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActionFailed()
	{
		super.OnActionFailed();
		SendCancelMessagesToAllAgents();
		m_OwnerGroup.RemoveUsableVehicle(m_Vehicle.m_Value);
	}
	
	//------------------------------------------------------------------------------------------------
	void ClearActivityVehicle()
	{
		m_Vehicle.m_Value = null;
	}
};
