//------------------------------------------------------------------------------------------------
// INFO REACTION BASE
//------------------------------------------------------------------------------------------------

[BaseContainerProps()]
class SCR_AIInfoReaction : SCR_AIReactionBase
{
	[Attribute("0", UIWidgets.ComboBox, "Type of event activating the reaction", "", ParamEnumArray.FromEnum(EMessageType_Info) )]
	EMessageType_Info m_eType;
	
	void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message) {}
	void PerformReaction(notnull SCR_AIGroupUtilityComponent utility, SCR_AIMessageBase message) {}
};


//------------------------------------------------------------------------------------------------
// INFO REACTIONS - Reactions on different info inputs from children (groups or agents)
//------------------------------------------------------------------------------------------------

[BaseContainerProps()]
class SCR_AIInfoReaction_Contact : SCR_AIInfoReaction
{
	override void PerformReaction(notnull SCR_AIGroupUtilityComponent utility, SCR_AIMessageBase message)
	{
		auto msg = SCR_AIMessage_Contact.Cast(message);
		if (!msg)
			return;
		
		if (!SCR_AIDamageHandling.IsAlive(msg.m_TargetInfo.m_TargetEntity))
			return;
		
		if (utility.AddOrUpdateTarget(msg.m_TargetInfo))
		{
			auto activity = new SCR_AIAttackActivity(utility, false, msg.m_TargetInfo, msg.GetSender());
			utility.AddAction(activity);
		}	
	}
};

[BaseContainerProps()]
class SCR_AIInfoReaction_TargetLost : SCR_AIInfoReaction_Contact
{
	// TODO: planning move of unoccupied unit to the last known location
	override void PerformReaction(notnull SCR_AIGroupUtilityComponent utility, SCR_AIMessageBase message)
	{
		auto msg = SCR_AIMessage_TargetLost.Cast(message);
		if(!msg)
			return;
		
		utility.RemoveTarget(msg.m_TargetInfo.m_TargetEntity);
		if (!utility.IsSomeEnemyKnown())
		{
			utility.SetStateAllActionsOfType(SCR_AIAttackActivity,EAIActionState.FAILED, true);
			//SetStateAllActionsOfType(EAIActionType.MOVE_COMBAT,EAIActionState.COMPLETED);
		}		
	}
	
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		auto msg = SCR_AIMessage_TargetLost.Cast(message);
		if (!msg)
			return;
		
		vector targetPos = msg.m_TargetInfo.m_vLastSeenPosition;
		
		// We either investigate or find a fire position at this target, depending on our weapons
		
		bool investigate = false;
		
		if (utility.m_CombatComponent.HasWeaponOfType(EWeaponType.WT_SNIPERRIFLE))
		{
			auto behavior = new SCR_AIFindFirePositionBehavior(utility, null, targetPos,
				minDistance: SCR_AIFindFirePositionBehavior.SNIPER_MIN_DISTANCE, maxDistance: SCR_AIFindFirePositionBehavior.SNIPER_MAX_DISTANCE,
				targetUnitType: EAIUnitType.UnitType_Infantry, duration: SCR_AIFindFirePositionBehavior.SNIPER_DURATION_S);
			utility.AddAction(behavior);
		}
		else if (utility.IsInvestigationAllowed(targetPos))
		{
			if (utility.IsInvestigationRelevant(targetPos))
			{
				auto behavior = new SCR_AIMoveAndInvestigateBehavior(utility, null, targetPos,
					isDangerous: true, targetUnitType: EAIUnitType.UnitType_Infantry);
			
				utility.AddAction(behavior);
			}
		}
	}
};

[BaseContainerProps()]
class SCR_AIInfoReaction_TargetEliminated : SCR_AIInfoReaction_Contact
{
	override void PerformReaction(notnull SCR_AIGroupUtilityComponent utility, SCR_AIMessageBase message)
	{
		auto msg = SCR_AIMessage_TargetEliminated.Cast(message);
		if(!msg)
			return;
		utility.RemoveTarget(msg.m_TargetInfo.m_TargetEntity);
		if (!utility.IsSomeEnemyKnown())
		{
			utility.SetStateAllActionsOfType(SCR_AIAttackActivity, EAIActionState.COMPLETED, true);
			//SetStateAllActionsOfType(EAIActionType.MOVE_COMBAT,EAIActionState.COMPLETED);
		}
	}
};

[BaseContainerProps()]
class SCR_AIInfoReaction_NoAmmo : SCR_AIInfoReaction
{
	override void PerformReaction(notnull SCR_AIGroupUtilityComponent utility, SCR_AIMessageBase message)
	{
		auto msg = SCR_AIMessage_NoAmmo.Cast(message);
		if(!msg)
			return;
		
		auto activity = new SCR_AIResupplyActivity(utility, false, msg.m_entityToSupply, msg.m_MagazineWell, priorityLevel: utility.GetCurrentPriorityLevel());
		utility.AddAction(activity);
	}
};

[BaseContainerProps()]
class SCR_AIInfoReaction_UnderFire : SCR_AIInfoReaction
{
	// TODO: retreat to last safe position
};

[BaseContainerProps()]
class SCR_AIInfoReaction_Wounded : SCR_AIInfoReaction
{
	override void PerformReaction(notnull SCR_AIGroupUtilityComponent utility, SCR_AIMessageBase message)
	{
		auto msg = SCR_AIMessage_Wounded.Cast(message);
		if(!msg)
			return;
		AIAgent aiAgent = msg.GetSender();
		AIGroup aiGroup = utility.m_Owner;
		float priorityLevel = utility.GetCurrentPriorityLevel();
		if (aiAgent)
		{
			IEntity woundedEntity = aiAgent.GetControlledEntity();
			
			// Ignore message if we already have an activity to heal this soldier
			foreach (SCR_AIActionBase action : utility.m_aActions)
			{
				SCR_AIHealActivity healActivity = SCR_AIHealActivity.Cast(action);
				
				if (!healActivity)
					continue;
				
				// Return if we are already healing this soldier
				if (healActivity.m_EntityToHeal.m_Value == woundedEntity)
				{
					healActivity.SetPriorityLevel(priorityLevel);
					return;
				}
			}
			
			auto activity = new SCR_AIHealActivity(utility, false, aiAgent.GetControlledEntity(), priorityLevel: priorityLevel);
			utility.AddAction(activity);
		}
	}
};

[BaseContainerProps()]
class SCR_AIInfoReaction_FoundCorpse : SCR_AIInfoReaction
{
	// TODO: planning move to corpse location?
};