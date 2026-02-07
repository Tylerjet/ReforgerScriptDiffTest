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
		
		if (!SCR_AIIsAlive.IsAlive(msg.m_Target))
			return;
		
		if (utility.AddKnownEnemy(msg.m_Target))
		{
			auto activity = new SCR_AIAttackActivity(utility, false, false, msg.m_Target, msg.m_LastSeenPosition, msg.GetSender());
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
		
		utility.RemoveKnownEnemy(msg.m_Target);
		if (!utility.IsSomeEnemyKnown())
		{
			utility.SetStateAllActionsOfType(EAIActionType.ATTACK,EAIActionState.FAILED);
			//SetStateAllActionsOfType(EAIActionType.MOVE_COMBAT,EAIActionState.COMPLETED);
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
		utility.RemoveKnownEnemy(msg.m_Target);
		if (!utility.IsSomeEnemyKnown())
		{
			utility.SetStateAllActionsOfType(EAIActionType.ATTACK,EAIActionState.COMPLETED);
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
		
		auto activity = new SCR_AIResupplyActivity(utility, false, false, msg.m_entityToSupply, msg.m_MagazineWell);
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
		if (aiAgent)
		{
			auto activity = new SCR_AIHealActivity(utility, false, false, aiAgent.GetControlledEntity());
			utility.AddAction(activity);
		}		
	}	
};

[BaseContainerProps()]
class SCR_AIInfoReaction_FoundCorpse : SCR_AIInfoReaction
{
	// TODO: planning move to corpse location?
};