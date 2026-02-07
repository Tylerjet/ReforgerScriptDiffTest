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
		

		if (utility.m_CombatComponent.GetCurrentEnemy() == null && dist > AI_PERCEPTION_DISTANCE && shooter)
		{
			utility.AddAction(new SCR_AIMoveFromDangerBehavior(utility, false, shooterPos));
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
			utility.m_CombatComponent.GetCurrentEnemy() == null &&
			distance < AI_WEAPONFIRED_REACTION_DISTANCE &&
			shooter)
		{
			float radius = Math.Max(0.087 * distance, 10); // Roughly +-5 degrees precision
			
			// Randomize position
			vector modePos = s_AIRandomGenerator.GenerateRandomPointInRadius(0, radius, lookPosition, true);
			
			if (!utility.IsInvestigationRelevant(modePos, radius * radius))
				return false;
			
			//Snap Y to terrain
			float y = GetGame().GetWorld().GetSurfaceY(modePos[0], modePos[2]);
			if (y > 0.0)
			{
				modePos[1] = y;
			}
			
			SCR_AIMoveAndInvestigateBehavior investigate = new SCR_AIMoveAndInvestigateBehavior(utility, false, pos: modePos, isDangerous: true, radius: radius);
			utility.AddAction(investigate);
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
		
		if (utility.m_CombatComponent.GetCurrentEnemy() == null && distanceToShooter > AI_PERCEPTION_DISTANCE && shooter)
		{
			utility.AddAction(new SCR_AIMoveFromDangerBehavior(utility, false, shooterPos));
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
		if (!vehicleObject || utility.GetEndangeringVehicle() == vehicleObject)
			return false;
		SCR_BaseCompartmentManagerComponent compManager = SCR_BaseCompartmentManagerComponent.Cast(vehicleObject.FindComponent(SCR_BaseCompartmentManagerComponent));
		if (!compManager)
			return false;
		array<IEntity> occupants = {};
		SCR_ChimeraAIAgent agent = SCR_ChimeraAIAgent.Cast(utility.GetOwner());
		compManager.GetOccupantsOfType(occupants, ECompartmentType.Pilot);
		if (occupants.Count() > 0 && agent.IsEnemy(occupants[0]))	
		{		
			utility.SetEndangeringVehicle(vehicleObject);
			SCR_AIMoveFromDangerBehavior behavior = new SCR_AIMoveFromDangerBehavior(utility, false, vehicleObject.GetOrigin(), entityToAvoid: vehicleObject);
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
				SCR_AIMoveFromDangerBehavior behavior = new SCR_AIMoveFromDangerBehavior(utility, false, grenadePos);
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
			SCR_AIHealBehavior behavior = new SCR_AIHealBehavior(utility, false, utility.m_OwnerEntity,true);
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