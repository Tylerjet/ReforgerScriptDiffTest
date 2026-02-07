class SCR_AIDecoIsEnemyWithinRadius : DecoratorScripted
{
	static const string WAYPOINT_PORT = "WaypointIn";
	static const string LOCATION_PORT = "LocationOut";
	
	//---------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		LOCATION_PORT
	};
	override TStringArray GetVariablesOut()
	{
		return s_aVarsOut;
	}
	
	//---------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		WAYPOINT_PORT
	};
	override TStringArray GetVariablesIn()
    {
        return s_aVarsIn;
    }
	
	//---------------------------------------------------------------------------------------------------
	bool TestLocationOfKnownEnemies(SCR_AIGroupUtilityComponent groupUtility, AIWaypoint waypoint)
	{ // at least one enemy known to group is within the completion radius?
		ChimeraCharacter character;
		DamageManagerComponent damageManager;
		float waypointRadiusSq = waypoint.GetCompletionRadius() * waypoint.GetCompletionRadius();
		vector positionInWaypoint;
		
		foreach (int index, IEntity enemy  : groupUtility.m_aTargetEntities)
		{
			if (!enemy)
				continue;
			
			character = ChimeraCharacter.Cast(enemy);
			if (character)
				damageManager = character.GetDamageManager();
			else
			 	damageManager = DamageManagerComponent.Cast(enemy.FindComponent(DamageManagerComponent));
			
			if (damageManager && damageManager.GetState() == EDamageState.DESTROYED)
				continue;
			
			positionInWaypoint = groupUtility.m_aTargetInfos[index].m_vLastSeenPosition;
			if (vector.DistanceSq(positionInWaypoint,waypoint.GetOrigin()) < waypointRadiusSq)
			{
				SetVariableOut(LOCATION_PORT, positionInWaypoint);
				return true;
			}
		}
		return false;
	}
	
	//---------------------------------------------------------------------------------------------------	
	protected override bool TestFunction(AIAgent owner)
	{
		AIWaypoint waypoint;
		SCR_AIGroupUtilityComponent guc;
		
		AIGroup group = AIGroup.Cast(owner);
		if (!group)
			return false;
		
		if ( !GetVariableIn(WAYPOINT_PORT,waypoint) )
			waypoint = group.GetCurrentWaypoint();
		if (! waypoint )
		{
			Print("Node IsEnemyWithinRadius executed without valid waypoint!");
			return false;
		};
		guc = SCR_AIGroupUtilityComponent.Cast(owner.FindComponent(SCR_AIGroupUtilityComponent));
		if ( !guc )	
		{
			Print("Node IsEnemyWithinRadius executed on owner without group utility component!");
			return false;
		}
		return TestLocationOfKnownEnemies(guc, waypoint);
	}	
	
	//---------------------------------------------------------------------------------------------------
	protected override bool VisibleInPalette()
	{
		return true;
	}
	
	protected override string GetOnHoverDescription()
	{
		return "Decorator that test that at least one detected enemy is within completion radius of waypoint, current waypoint is used if none is provided";
	}
};