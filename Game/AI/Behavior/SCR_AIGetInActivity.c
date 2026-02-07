typedef array<BaseCompartmentSlot> SCR_AITCompartmentsArray;

class SCR_AIGetInActivity : SCR_AIActivityBase
{
	ref SCR_BTParam<IEntity> m_Vehicle = new SCR_BTParam<IEntity>(SCR_AIActionTask.ENTITY_PORT);		
	ref SCR_BTParam<ECompartmentType> m_eRoleInVehicle = new SCR_BTParam<ECompartmentType>(SCR_AIActionTask.ROLEINVEHICLE_PORT);
	
	 ref SCR_AITCompartmentsArray m_AllocatedCompartments;
	
	//------------------------------------------------------------------------------------------------
	void SCR_AIGetInActivity(SCR_AIBaseUtilityComponent utility, bool prioritize, bool isWaypointRelated, IEntity vehicle = null, ECompartmentType role = ECompartmentType.Cargo, float priority = PRIORITY_ACTIVITY_GET_IN)
	{
		m_Vehicle.Init(this, vehicle);	
		m_eRoleInVehicle.Init(this, role);
		
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Group/ActivityGetIn.bt";
		m_eType = EAIActionType.GET_IN_VEHICLE;	
		m_fPriority = priority;
		m_AllocatedCompartments = new SCR_AITCompartmentsArray();
	}
	
	//------------------------------------------------------------------------------------------------
	override string GetActionDebugInfo()
	{
		return this.ToString() + " moving to " + m_Vehicle.ValueToString() + " as " + m_eRoleInVehicle.ValueToString();
	}
	
	//------------------------------------------------------------------------------------------------
	void AllocateCompartment(BaseCompartmentSlot compartment)
	{
		m_AllocatedCompartments.Insert(compartment);
		compartment.SetCompartmentAccessible(false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ReleaseCompartments()
	{
		foreach (BaseCompartmentSlot comp : m_AllocatedCompartments)
		{
			comp.SetCompartmentAccessible(true);
		}
		m_AllocatedCompartments.Clear();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActionDeselected()
	{
		super.OnActionDeselected();
		ReleaseCompartments();
	}	
	
	//------------------------------------------------------------------------------------------------
	override void OnActionCompleted()
	{
		super.OnActionCompleted();
		ReleaseCompartments();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActionFailed()
	{
		super.OnActionFailed();
		ReleaseCompartments();
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_AIGetInActivity()
	{
		ReleaseCompartments();
	}
};
