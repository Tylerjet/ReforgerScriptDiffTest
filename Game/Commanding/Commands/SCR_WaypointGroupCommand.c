//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_WaypointGroupCommand : SCR_BaseGroupCommand
{
	[Attribute(desc: "Prefab of the AI waypoint used in this command", params: "et class=AIWaypoint")]
	protected ResourceName m_sWaypointPrefab;
	
	[Attribute(defvalue: "1", desc: "Will the command override all previous commands?")]
	protected bool m_bOverridePreviousCommands;
	
	[Attribute(defvalue: "0", desc: "Will the command be automatically set to players position?")]
	protected bool m_bTargetSelf;
	
	[Attribute(defvalue: "0", desc: "Will the command override autonomous group behavior?")]
	protected bool m_bForceCommand;
	
	//------------------------------------------------------------------------------------------------
	override bool Execute(IEntity cursorTarget, IEntity target, vector targetPosition, int playerID, bool isClient)
	{
		if (isClient)
		{
			//place to place a logic that would be executed for other players
			return true;
		}
		
		if (!m_sWaypointPrefab || !target || !targetPosition)
			return false;
		
		//Hotfix until we get api to know when the speaker is done saying the command voiceline
		GetGame().GetCallqueue().CallLater(PlayAIResponse, 2000, false, target);
		return SetWaypointForAIGroup(target, targetPosition, playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	bool SetWaypointForAIGroup(IEntity target, vector targetPosition, int playerID)
	{
		SCR_AIGroup slaveGroup = SCR_AIGroup.Cast(target);
		if (!slaveGroup)
			return false;
		
		array<AIWaypoint> currentWaypoints = {};
		slaveGroup.GetWaypoints(currentWaypoints);
		foreach(AIWaypoint currentwp : currentWaypoints)
		{
			slaveGroup.RemoveWaypoint(currentwp);
		}
		
		Resource waypointPrefab = Resource.Load(GetWaypointPrefab());
		if (!waypointPrefab.IsValid())
		{
			//add custom error
			return false;
		}
		
		vector position;
		position = GetTargetPosition(playerID, targetPosition);
		
		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = position; 
	
		CheckPreviousWaypoints(slaveGroup);
		
		SCR_AIWaypoint wp = SCR_AIWaypoint.Cast(GetGame().SpawnEntityPrefabLocal(waypointPrefab, null, params));
		if (!wp)
			return false;
		
		if (m_bForceCommand)
			wp.SetPriority(true);
		slaveGroup.AddWaypoint(wp);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetWaypointPrefab()
	{
		return m_sWaypointPrefab;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetWaypointPrefab(ResourceName wpPrefab)
	{
		m_sWaypointPrefab = wpPrefab;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetOverridePreviousCommands()
	{
		return m_bOverridePreviousCommands;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetOverridePreviousCommands(bool overridePrevious)
	{
		m_bOverridePreviousCommands = overridePrevious;
	}
	
	//----------------------------------------------------------------------------------------------
	vector GetTargetPosition(int playerID, vector targetPosition)
	{
		if (m_bTargetSelf)
		{
			SCR_PlayerController orderingPlayerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerID));
			IEntity playerEntity = orderingPlayerController.GetControlledEntity();
			if (!playerEntity)
				return vector.Zero;
			return playerEntity.GetOrigin();
		}
		else
			return targetPosition;
	}
	
	//-------------------------------------------------------------------------------------------------
	void CheckPreviousWaypoints(SCR_AIGroup slaveGroup)
	{
		if (!m_bOverridePreviousCommands)
			return;
		
		array<AIWaypoint> currentWaypoints = {};
		slaveGroup.GetWaypoints(currentWaypoints);
		foreach(AIWaypoint currentwp : currentWaypoints)
		{
			slaveGroup.RemoveWaypoint(currentwp);
		}
		slaveGroup.SetFormationDisplacement(0);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShown()
	{
		SCR_PlayerControllerGroupComponent groupController = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		return groupController && CanRoleShow();
	}
}