class SCR_AIDecoIsEnemyWithinRadius : DecoratorScripted
{
	static const string WAYPOINT_PORT = "WaypointIn";
	
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
		
		foreach (IEntity enemy : groupUtility.m_aListOfKnownEnemies)
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
			
			if (vector.DistanceSq(enemy.GetOrigin(),waypoint.GetOrigin()) < waypointRadiusSq)
				return true;
		}
		return false;	
	}	

	//---------------------------------------------------------------------------------------------------	
	protected override bool TestFunction(AIAgent owner)
	{
		AIWaypoint waypoint;
		SCR_AIGroupUtilityComponent guc;
		
		if ( !GetVariableIn(WAYPOINT_PORT,waypoint) )
			waypoint = owner.GetCurrentWaypoint();		
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