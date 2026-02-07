
class SCR_AIProcessFailedMovementResult : AITaskScripted
{
	static const string PORT_MOVE_RESULT = "MoveResult";
	static const string PORT_HANDLER_ID = "FailedHandlerId";
	static const string PORT_IS_WAYPOINT_RELEATED = "IsWaypointReleated";
	static const string PORT_MOVE_LOCATION = "MoveLocation";

	SCR_AIGroup m_Group;
	SCR_AIGroupUtilityComponent m_GroupUtilityComponent;
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		SCR_AIGroup group = SCR_AIGroup.Cast(owner);
		if (!group)
			return;
		
		m_Group = group;
		m_GroupUtilityComponent = SCR_AIGroupUtilityComponent.Cast(m_Group.FindComponent(SCR_AIGroupUtilityComponent));	
	}
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		int moveResult;
		if (!GetVariableIn(PORT_MOVE_RESULT, moveResult))
		{
			NodeError(this, owner, "Missing move result for SCR_AIProcessFailedMovementResult node");
			return ENodeResult.RUNNING; 
		}
		
#ifdef WORKBENCH
		PrintDebug(owner, string.Format("Move failed with result %1", SCR_Enum.GetEnumName(EMoveRequestResult, moveResult)));
#endif
				
		// Exit with error (and running status for debug) if we receive not-failed move result
		if (moveResult == EMoveRequestResult.Uninitialized || moveResult == EMoveRequestResult.Running || moveResult == EMoveRequestResult.Succeeded || moveResult == EMoveRequestResult.Aborted)
		{
			NodeError(this, owner, string.Format("Unexpected move result for SCR_AIProcessFailedMovementResult node: %1", SCR_Enum.GetEnumName(EMoveRequestResult, moveResult)));
			return ENodeResult.RUNNING; 
		}
				
		int failedHandlerId;
		GetVariableIn(PORT_HANDLER_ID, failedHandlerId);
		
		bool isWaypointReleated;
		GetVariableIn(PORT_IS_WAYPOINT_RELEATED, isWaypointReleated);
		
		vector moveLocation;
		GetVariableIn(PORT_MOVE_LOCATION, moveLocation);
		
		// we failed inside one of our vehicles -> leave the vehicle of the handlerId and try again
		IEntity vehicleUsed;
		if (failedHandlerId != AIGroupMovementComponent.DEFAULT_HANDLER_ID) 
		{
			SCR_AIGroupVehicle groupVehicle = m_GroupUtilityComponent.m_VehicleMgr.FindVehicleBySubgroupId(failedHandlerId);
			if (groupVehicle)
				vehicleUsed = groupVehicle.GetEntity();
		}
		
		// Navlink is being negotiated, try again
		if (moveResult == EMoveRequestResult.WaitingOnNavlink)
		{
			return ENodeResult.FAIL;
		}
		
		// Ivoke event about failed movement
		m_GroupUtilityComponent.OnMoveFailed(moveResult, vehicleUsed, isWaypointReleated, moveLocation);
		
		// Failed movement
		if (moveResult == EMoveRequestResult.Failed)
		{
			if (isWaypointReleated)
			{
				SCR_AIActivityBase activity = SCR_AIActivityBase.Cast(m_GroupUtilityComponent.GetCurrentAction());
				
				// Just fail action if failed move is a result or removed waypoint
				if (activity && !activity.m_RelatedWaypoint)
				{
#ifdef WORKBENCH
					PrintDebug(owner, string.Format("Failed move as result of deleted waypoint %1", moveLocation));
#endif
					FailAction();
					return ENodeResult.FAIL;
				}
				
				// If wp related and wp is the same we just return running
				// As a result group will be stuck to allow us to debug it
				NodeError(this, owner, string.Format("Failed move to waypoint %1", moveLocation));
				return ENodeResult.RUNNING;
			}
			
			FailAction();
#ifdef WORKBENCH
			PrintDebug(owner, string.Format("Failed move while following the entity %1", moveLocation));
#endif
			// If result is not waypoint related we just fail
			return ENodeResult.FAIL;
		}
		
		// If stopped or vehicle is not used, complete waypoint if related and fail action
		if (moveResult == EMoveRequestResult.Stopped || !vehicleUsed)
		{
#ifdef WORKBENCH
			PrintDebug(owner, string.Format("Move failed with result %1 to location %2, failing action.", SCR_Enum.GetEnumName(EMoveRequestResult, moveResult), moveLocation));
#endif
			if (isWaypointReleated)
				CompleteWaypoint();
			
			// Fail action
			FailAction();
			
			return ENodeResult.RUNNING;
		}
#ifdef WORKBENCH
		PrintDebug(owner, string.Format("Move failed with result %1 to location %2, retrying without vehicle.", SCR_Enum.GetEnumName(EMoveRequestResult, moveResult), moveLocation));
#endif	
		// Exit vehicle and try again
		if (vehicleUsed)
		{
			// If failed because the driver is dead, we dont need to leave vehicle
			if (moveResult != EMoveRequestResult.EntityCantMove) 
			{
				SCR_AIGetOutActivity getOut = new SCR_AIGetOutActivity(m_GroupUtilityComponent, null, vehicleUsed);
				m_GroupUtilityComponent.AddAction(getOut);
			}
			
			// If vehicle got stuck turn the hazard lights on
			if (moveResult == EMoveRequestResult.Stuck)
			{
				// Turn them only if there's no enemy around
				if (m_GroupUtilityComponent && m_GroupUtilityComponent.m_Perception && !m_GroupUtilityComponent.m_Perception.m_MostDangerousCluster)
					SCR_AIVehicleUsability.TurnOnVehicleHazardLights(vehicleUsed);
			}
		}
		
		// Fail a node to enable tree to try again
		return ENodeResult.FAIL;
	}
	
	//------------------------------------------------------------------------------------------------
	void PrintDebug(AIAgent owner, string message)
	{
#ifdef WORKBENCH		
		if (!DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AI_PRINT_DEBUG))
			return;
	
		Print(string.Format("%1: %2", owner.ToString(), message));
#endif
	}
	
	//------------------------------------------------------------------------------------------------
	void FailAction()
	{
		AIActionBase currentAction = m_GroupUtilityComponent.GetExecutedAction();
		if (currentAction)
			currentAction.Fail();
	}
	
	//------------------------------------------------------------------------------------------------
	void CompleteWaypoint()
	{
		AIWaypoint waypoint = m_Group.GetCurrentWaypoint();
		if (waypoint)
			m_Group.CompleteWaypoint(waypoint);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool CanReturnRunning() { return true; }
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {};
	override TStringArray GetVariablesOut() { return s_aVarsOut; }

	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {PORT_MOVE_RESULT, PORT_HANDLER_ID, PORT_IS_WAYPOINT_RELEATED, PORT_MOVE_LOCATION};
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
	
	//------------------------------------------------------------------------------------------------
	protected override string GetOnHoverDescription()
	{
		return "Processes failed movement result for proper BT reaction, calls group's failed move invoker";
	}	
}