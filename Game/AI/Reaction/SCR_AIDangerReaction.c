//------------------------------------------------------------------------------------------------
// DANGER REACTION BASE
//------------------------------------------------------------------------------------------------

[BaseContainerProps()]
class SCR_AIDangerReaction : SCR_AIReactionBase
{
	[Attribute("0", UIWidgets.ComboBox, "Type of event activating the reaction", "", ParamEnumArray.FromEnum(EAIDangerEventType) )]
	EAIDangerEventType m_eType;

	//eventualy move to some setting in component
	protected static const float BULLET_IMPACT_DISTANCE_MAX = 3;
	
	bool PerformReaction(notnull SCR_AIUtilityComponent utility, notnull SCR_AIThreatSystem threatSystem, AIDangerEvent dangerEvent) {}
};


//------------------------------------------------------------------------------------------------
// DANGER REACTIONS
//------------------------------------------------------------------------------------------------

[BaseContainerProps()]
class SCR_AIDangerReaction_DamageTaken : SCR_AIDangerReaction
{
	override bool PerformReaction(notnull SCR_AIUtilityComponent utility, notnull SCR_AIThreatSystem threatSystem, AIDangerEvent dangerEvent)
	{
		if (dangerEvent.GetVictim() != utility.m_OwnerEntity)
			return false;
		// amount of dmg to threatsystem
		
		IEntity shooter = dangerEvent.GetObject();
		vector shooterPos;
		if (shooter)
			shooterPos = shooter.GetOrigin();
		
		float dist = vector.Distance(shooterPos, utility.GetOrigin());
		

		if (utility.m_CombatComponent.GetCurrentTarget() == null && dist > LONG_RANGE_FIRE_DISTANCE && shooter)
		{
			utility.AddAction(new SCR_AIMoveFromUnknownFire(utility, false, null, shooterPos));
		}
		
		return true;
	}
};

[BaseContainerProps()]
class SCR_AIDangerReaction_WeaponFired : SCR_AIDangerReaction
{
	override bool PerformReaction(notnull SCR_AIUtilityComponent utility, notnull SCR_AIThreatSystem threatSystem, AIDangerEvent dangerEvent)
	{
		IEntity shooter = dangerEvent.GetObject();
		SCR_ChimeraAIAgent agent = SCR_ChimeraAIAgent.Cast(utility.GetOwner());
		if (!agent || !agent.IsEnemy(shooter))
			return false;
		
		vector min, max;
		shooter.GetBounds(min, max);
		vector lookPosition = shooter.GetOrigin() + (min + max) * 0.5;
		
		float distance = vector.Distance(lookPosition, utility.GetOrigin());
		
		threatSystem.ThreatShotFired(distance, dangerEvent.GetCount());

		utility.m_LookAction.LookAt(lookPosition, utility.m_LookAction.PRIO_DANGER_EVENT);
		
		// Ignore if we must defend a waypoint
		if (!utility.IsInvestigationAllowed(lookPosition))
			return false;
		
		EWeaponType currentWeaponType = utility.m_CombatComponent.GetCurrentWeaponType();
		
		if (currentWeaponType != EWeaponType.WT_NONE &&
			utility.m_CombatComponent.GetCurrentTarget() == null &&
			distance < AI_WEAPONFIRED_REACTION_DISTANCE &&
			shooter)
		{
			float radius = Math.Max(0.087 * distance, 10); // Roughly +-5 degrees precision
			
			// Randomize position
			vector movePos = s_AIRandomGenerator.GenerateRandomPointInRadius(0, radius, lookPosition, true);
			
			if (utility.m_CombatComponent.HasWeaponOfType(EWeaponType.WT_SNIPERRIFLE))
			{
				//Snap Y to terrain
				float y = GetGame().GetWorld().GetSurfaceY(movePos[0], movePos[2]);
				if (y > 0.0)
					movePos[1] = y;
				
				auto behavior = new SCR_AIFindFirePositionBehavior(utility, false, null, movePos,
					minDistance: SCR_AIFindFirePositionBehavior.SNIPER_MIN_DISTANCE, maxDistance: SCR_AIFindFirePositionBehavior.SNIPER_MAX_DISTANCE,
					targetUnitType: EAIUnitType.UnitType_Infantry, duration: SCR_AIFindFirePositionBehavior.SNIPER_DURATION_S);
				utility.AddAction(behavior);
			}
			else if (utility.IsInvestigationRelevant(movePos))
			{
				//Snap Y to terrain
				float y = GetGame().GetWorld().GetSurfaceY(movePos[0], movePos[2]);
				if (y > 0.0)
					movePos[1] = y;
				
				SCR_AIMoveAndInvestigateBehavior investigate = new SCR_AIMoveAndInvestigateBehavior(utility, false, null, pos: movePos, isDangerous: true, radius: radius);
				utility.AddAction(investigate);
			}
		}
		return true;
	}
};

[BaseContainerProps()]
class SCR_AIDangerReaction_ProjectileHit : SCR_AIDangerReaction
{
	override bool PerformReaction(notnull SCR_AIUtilityComponent utility, notnull SCR_AIThreatSystem threatSystem, AIDangerEvent dangerEvent)
	{
		float distance = vector.Distance(utility.GetOrigin(), dangerEvent.GetPosition());
		if (distance > BULLET_IMPACT_DISTANCE_MAX)
			return false;
		
		IEntity shooter = dangerEvent.GetObject();
		vector shooterPos;
		if (shooter)
			shooterPos = shooter.GetOrigin();
		float distanceToShooter = vector.Distance(utility.GetOrigin(), shooterPos);
		
		if (utility.m_CombatComponent.GetCurrentTarget() == null && distanceToShooter > LONG_RANGE_FIRE_DISTANCE && shooter)
		{
			utility.AddAction(new SCR_AIMoveFromUnknownFire(utility, false, null, shooterPos));
			//TODO: change combat type from SILENT
		}

		threatSystem.ThreatBulletImpact(dangerEvent.GetCount());
		
		return true;
	}	
};

[BaseContainerProps()]
class SCR_AIDangerReaction_Vehicle : SCR_AIDangerReaction
{
	override bool PerformReaction(notnull SCR_AIUtilityComponent utility, notnull SCR_AIThreatSystem threatSystem, AIDangerEvent dangerEvent)
	{
		IEntity vehicleObject = dangerEvent.GetVictim();
		if (!vehicleObject)
			return false;
		
		// Ignore if we already have an action to move from that vehicle
		if (SCR_AIMoveFromDangerBehavior.ExistsBehaviorForEntity(utility, vehicleObject))
			return false;
		
		SCR_BaseCompartmentManagerComponent compManager = SCR_BaseCompartmentManagerComponent.Cast(vehicleObject.FindComponent(SCR_BaseCompartmentManagerComponent));
		if (!compManager)
			return false;
		array<IEntity> occupants = {};
		SCR_ChimeraAIAgent agent = SCR_ChimeraAIAgent.Cast(utility.GetOwner());
		compManager.GetOccupantsOfType(occupants, ECompartmentType.Pilot);
		if (occupants.Count() > 0 && agent.IsEnemy(occupants[0]))	
		{
			SCR_AIMoveFromDangerBehavior behavior = new SCR_AIMoveFromIncomingVehicleBehavior(utility, false, null, dangerEntity: vehicleObject);
			utility.AddAction(behavior);
			return true;
		}
		return false;
	}	
};

[BaseContainerProps()]
class SCR_AIDangerReaction_GrenadeLanding : SCR_AIDangerReaction
{
	static const float GRENADE_AVOIDANCE_RADIUS_SQ = 625;
	
	override bool PerformReaction(notnull SCR_AIUtilityComponent utility, notnull SCR_AIThreatSystem threatSystem, AIDangerEvent dangerEvent)
	{
		IEntity grenadeObject = dangerEvent.GetObject();
		if (grenadeObject)
		{
			vector grenadePos = grenadeObject.GetOrigin();
			float distanceSqToGrenade = vector.DistanceSq(utility.GetOrigin(), grenadePos);
			if (distanceSqToGrenade < GRENADE_AVOIDANCE_RADIUS_SQ)
			{
				SCR_AIMoveFromDangerBehavior behavior = new SCR_AIMoveFromGrenadeBehavior(utility, false, null, dangerEntity: grenadeObject);
				utility.AddAction(behavior);
				return true;
			}
		}
		return false;
	}
};

[BaseContainerProps()]
class SCR_AIDangerReaction_StartedBleeding : SCR_AIDangerReaction
{
	override bool PerformReaction(notnull SCR_AIUtilityComponent utility, notnull SCR_AIThreatSystem threatSystem, AIDangerEvent dangerEvent)
	{
		if (dangerEvent.GetVictim() != utility.m_OwnerEntity)
			return false;
		
		if (utility.m_AIInfo.HasRole(EUnitRole.MEDIC))
		{
			// If we can heal ourselves, add Heal Behavior.
			SCR_AIHealBehavior behavior = new SCR_AIHealBehavior(utility, false, null, utility.m_OwnerEntity,true);
			utility.AddActionIfMissing(behavior);
		}
		else
		{
			// If we immediately know that we can't heal ourselves, report to group
			AIAgent myAgent = AIAgent.Cast(utility.GetOwner());
			AIGroup myGroup = myAgent.GetParentGroup();
			SCR_MailboxComponent myMailbox = SCR_MailboxComponent.Cast(myAgent.FindComponent(SCR_MailboxComponent));
			SCR_AIMessage_Wounded msg = SCR_AIMessage_Wounded.Create(utility.m_OwnerEntity);
			myMailbox.RequestBroadcast(msg, myGroup);
		}
		
		return true;
	}
};

[BaseContainerProps()]
class SCR_AIDangerReaction_DoorMovement : SCR_AIDangerReaction
{
	static const float AGENT_DEFAULT_RADIUS = 0.3;
	static const float AGENT_DEFAULT_HEIGHT = 1.8;
	static const float REACTION_DIST_SQ = 25.0;
	
	override bool PerformReaction(notnull SCR_AIUtilityComponent utility, notnull SCR_AIThreatSystem threatSystem, AIDangerEvent dangerEvent)
	{
		IEntity door = dangerEvent.GetObject();
		vector agentPos = utility.GetOrigin();
		vector doorPos = door.GetOrigin();
		
		//Initial filter for hearing distance (TODO: optimization spatial grid)
		float distSq = vector.DistanceSq(agentPos, doorPos);
		if (distSq >= REACTION_DIST_SQ)
			return false;
		
		vector min, max;
		float distToDoorY = doorPos[1] - agentPos[1];
		//If agent above door higher than the door height, skip 
		if (-distToDoorY > AGENT_DEFAULT_HEIGHT)
			return false;
			
		door.GetBounds(min, max);
		vector doorExtent = max - min;
		//If agent bellow door and dist is higher than agent height, skip
		if (distToDoorY > doorExtent[1])
			return false;
		
		float distToDoorXZ = vector.DistanceXZ(doorPos, agentPos) - AGENT_DEFAULT_RADIUS;
		doorExtent[1] = 0.0;
		float doorLength = doorExtent.Length();
		//If the agent isn't in XZ range, skip
		if (distToDoorXZ > doorLength)
			return false;
	
		//Ensure the necessary entities and components are valid
		SCR_ChimeraAIAgent agent = SCR_ChimeraAIAgent.Cast(utility.GetOwner());
		if (!agent)
			return false;
		
		//Turn around if the instigator of door is an enemy,
		IEntity actionStarter = dangerEvent.GetVictim();
		if (agent.IsEnemy(actionStarter))
		{
			utility.AddAction(new SCR_AIMoveIndividuallyBehavior(utility, true, null, agentPos, SCR_AIActionBase.PRIORITY_BEHAVIOR_MOVE_AND_INVESTIGATE, null, AGENT_DEFAULT_RADIUS));
			utility.m_LookAction.LookAt(actionStarter.GetOrigin(), utility.m_LookAction.PRIO_DANGER_EVENT);
			return true;
		}
			
		ChimeraCharacter character = ChimeraCharacter.Cast(agent.GetControlledEntity());
		if (character && character.IsInVehicle())
			return false;
		
		DoorComponent doorComponent = DoorComponent.Cast(door.FindComponent(DoorComponent));
		if (!doorComponent)
			return false;
	
		vector doorNormal = doorComponent.GetDoorNormal();
		float distanceToMove = Math.Clamp((doorLength - distToDoorXZ) * 2.0, 2.0 * AGENT_DEFAULT_RADIUS, doorLength);
		vector agentDirection = agentPos - doorPos;
		agentDirection.Normalize();
		vector movePos;
		//Adjust the doorNormal to lead the agent away from the door
		if (vector.DistanceSq(agentDirection, doorNormal) <  vector.DistanceSq(agentDirection, -doorNormal))
			movePos = agentPos + doorNormal * distanceToMove;
		else
			movePos = agentPos - doorNormal * distanceToMove;
		
		//Step away and free door space
		utility.AddAction(new SCR_AIMoveIndividuallyBehavior(utility, true, null, movePos, SCR_AIActionBase.PRIORITY_BEHAVIOR_MOVE_AND_INVESTIGATE, null, AGENT_DEFAULT_RADIUS));
		return true;
	}
};

[BaseContainerProps()]
class SCR_AIDangerReaction_MeleeDamageTaken : SCR_AIDangerReaction
{
	static const float REACTION_DIST_SQ = 3.5;
	static const float MIN_DIST_RUN = 10.0;
	static const float MAX_DIST_RUN = 15.0;
	
	override bool PerformReaction(notnull SCR_AIUtilityComponent utility, notnull SCR_AIThreatSystem threatSystem, AIDangerEvent dangerEvent)
	{
		if (dangerEvent.GetVictim() != utility.m_OwnerEntity)
			return false;
		// amount of dmg to threatsystem
				
		vector shooterPos = dangerEvent.GetPosition();
			
		//position is 
		vector myPos = dangerEvent.GetVictim().GetOrigin();
		
		//distance
		float distSq = vector.DistanceSq(myPos, shooterPos);
		if (distSq >= REACTION_DIST_SQ)
			return false;
		
		//Rotate towards it
		utility.m_LookAction.LookAt(shooterPos, utility.m_LookAction.PRIO_DANGER_EVENT);

		
		//get the direction from enemy to me = V
		vector V = myPos - shooterPos;
		V.Normalize();
		
		//take a random distance in range [d,D] = d'
		float d = Math.RandomFloat(MIN_DIST_RUN,MAX_DIST_RUN);
		
		//Calculate the center of the pointer, at d' distance from my position in V direction = C
		//Find correct operator
		myPos += V * d;
		 	
		//Call to the BT (in it, it will get a random point, with center at C, and radius R. Then move to it WHILE looking at shooterPos)
		
		//Use the behaviour tree
		SCR_AIRetreatWhileLookAtBehavior behavior = new SCR_AIRetreatWhileLookAtBehavior(utility,true, null);
		behavior.m_Target.m_Value = myPos;
		behavior.m_LookAt.m_Value = shooterPos;
		
		utility.AddAction(behavior);
		
		return true;
	}
};


[BaseContainerProps()]
class SCR_AIDangerReaction_VehicleHorn : SCR_AIDangerReaction
{
	static const float REACTION_ENEMY_DIST_SQ = 50*50;
	static const float REACTION_FRIENDLY_DIST_SQ = 12*12;
	
	override bool PerformReaction(notnull SCR_AIUtilityComponent utility, notnull SCR_AIThreatSystem threatSystem, AIDangerEvent dangerEvent)
	{		
		IEntity vehicleObject = dangerEvent.GetVictim();
		if (!vehicleObject)
			return false;
		
		// Ignore if we already have an action to move from that vehicle
		if (SCR_AIMoveFromDangerBehavior.ExistsBehaviorForEntity(utility, vehicleObject))
			return false;
		
		SCR_ChimeraAIAgent agent = SCR_ChimeraAIAgent.Cast(utility.GetOwner());
		
		//Check distance
		vector vehiclePos 	= vehicleObject.GetOrigin();
		vector agentPos 	= utility.GetOrigin();
		float distSq = vector.DistanceSq(vehiclePos, agentPos); 
		
		if (distSq >= REACTION_ENEMY_DIST_SQ)
			return false;
		
		//Enemy car
		if(agent.IsEnemy(vehicleObject))
		{
			//Orientate towards the sound origin
			utility.m_LookAction.LookAt(dangerEvent.GetPosition(), utility.m_LookAction.PRIO_DANGER_EVENT);
			return true;
		}
		//Ally car
		else
		{
			if (distSq <= REACTION_FRIENDLY_DIST_SQ)
			{
				//Move away from danger
				SCR_AIMoveFromDangerBehavior behavior = new SCR_AIMoveFromVehicleHornBehavior(utility, false, null, dangerEntity: vehicleObject);
				utility.AddAction(behavior);
				return true;
			}
		}
		return false;
	}	
};