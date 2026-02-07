class SCR_AIVehicleBehavior : SCR_AIBehaviorBase
{
	override void OnActionSelected() 
	{
		super.OnActionSelected();		
		m_Utility.m_AIInfo.SetAIState(EUnitAIState.BUSY);
	}	
	
	override void OnActionCompleted() 
	{
		m_Utility.m_AIInfo.SetAIState(EUnitAIState.AVAILABLE);
		super.OnActionCompleted();
	}
	
	override void OnActionFailed() 
	{
		m_Utility.m_AIInfo.SetAIState(EUnitAIState.AVAILABLE);
		super.OnActionFailed();
	}
};

class SCR_AIGetInVehicle : SCR_AIVehicleBehavior
{
	ref SCR_BTParam<IEntity> m_Vehicle = new SCR_BTParam<IEntity>(SCR_AIActionTask.ENTITY_PORT);
	ref SCR_BTParam<ECompartmentType> m_eRoleInVehicle = new SCR_BTParam<ECompassType>(SCR_AIActionTask.ROLEINVEHICLE_PORT);
	
	void SCR_AIGetInVehicle(SCR_AIBaseUtilityComponent utility, bool prioritize, IEntity vehicleEntity, ECompartmentType role = ECompartmentType.Cargo, float priority = PRIORITY_BEHAVIOR_VEHICLE)
    {
		m_Vehicle.Init(m_aParams, vehicleEntity);
		m_eRoleInVehicle.Init(m_aParams, role);
		
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Soldier/GetInVehicle.bt";
		m_fPriority = priority;
    }
	
	override string GetActionDebugInfo()
	{
		return this.ToString() + " moving to " + m_Vehicle.ValueToString() + " as " + m_eRoleInVehicle.ValueToString();
	}
};

class SCR_AIGetOutVehicle : SCR_AIVehicleBehavior
{
	ref SCR_BTParam<IEntity> m_Vehicle = new SCR_BTParam<IEntity>(SCR_AIActionTask.ENTITY_PORT);
	
    void SCR_AIGetOutVehicle(SCR_AIBaseUtilityComponent utility, bool prioritize, IEntity vehicle, float priority = PRIORITY_BEHAVIOR_GET_OUT_VEHICLE)
    {
		m_Vehicle.Init(this, vehicle);
		
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Soldier/GetOutVehicle.bt";
       	m_fPriority = priority;
    }
	
	override string GetActionDebugInfo()
	{
		return this.ToString() + " leaving " + m_Vehicle.ValueToString();
	}
};

