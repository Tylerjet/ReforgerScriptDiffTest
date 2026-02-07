class SCR_ChimeraAIAgentClass: ChimeraAIAgentClass
{
};

class SCR_ChimeraAIAgent : ChimeraAIAgent
{
	// Current waypoint of our group
	AIWaypoint m_GroupWaypoint;
	SCR_AIUtilityComponent m_UtilityComponent;
	SCR_AIInfoComponent m_InfoComponent;
	
	protected SCR_CharacterControllerComponent m_CharacterController;
	protected FactionAffiliationComponent m_FactionAffiliationComponent;
	protected int m_iPendingPlayerId;
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner) 
	{
		IEntity controlledEntity = GetControlledEntity();
		if (!controlledEntity)
			return;
		
		ChimeraCharacter character = ChimeraCharacter.Cast(controlledEntity);
		if (character)
		{
			m_CharacterController = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
			if (m_CharacterController)
				m_CharacterController.m_OnLifeStateChanged.Insert(OnLifeStateChanged);
		}
			
		GetGame().GetCallqueue().CallLater(EnsureAILimit, 1, false);
		
		m_FactionAffiliationComponent = FactionAffiliationComponent.Cast(controlledEntity.FindComponent(FactionAffiliationComponent));
		m_InfoComponent = SCR_AIInfoComponent.Cast(FindComponent(SCR_AIInfoComponent));
		m_UtilityComponent = SCR_AIUtilityComponent.Cast(FindComponent(SCR_AIUtilityComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_ChimeraAIAgent()
	{
		if (m_CharacterController)
			m_CharacterController.m_OnLifeStateChanged.Remove(OnLifeStateChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	void EnsureAILimit()
	{
		auto aiWorld = GetGame().GetAIWorld();
		if (!aiWorld)
			return;
		
		if (aiWorld.CanLimitedAIBeAdded())
			return;
		
		IEntity controlledEntity = GetControlledEntity();
		if (!EntityUtils.IsPlayer(controlledEntity) && !IsPlayerPending_S()) // Ensure that pending players can spawn
		{
			SCR_EntityHelper.DeleteEntityAndChildren(controlledEntity);
		}
	}

	//------------------------------------------------------------------------------------------------	
	protected void SendWoundedMsg()
	{
		SCR_CharacterDamageManagerComponent damageMgr = SCR_CharacterDamageManagerComponent.Cast(GetControlledEntity().FindComponent(SCR_CharacterDamageManagerComponent));
		if (!damageMgr)
			return;
	
		if (!damageMgr.IsDamagedOverTime(EDamageType.BLEEDING))
			return;
		
		SCR_AIGroup msgReceiverGroup = null;
		
		SCR_AIGroup myGroup = SCR_AIGroup.Cast(GetParentGroup());
		if (myGroup)
		{
			SCR_AIGroup slaveGroup = myGroup.GetSlave();
			if (slaveGroup)
				msgReceiverGroup = slaveGroup;	// Send to our slave group - this is the one which has AIs and will heal us
			else
				msgReceiverGroup = myGroup;	// Send to our group
		}
		
		if (!msgReceiverGroup)
			return;
		
		// Inject message to the group mailbox directly.
		// This bypasses problems of our own mailbox being disabled (because character is possessed by GM, or because we are unconscious)
		AICommunicationComponent comms = msgReceiverGroup.GetCommunicationComponent();
		if (!comms)
			return;
		
		SCR_AIMessage_Wounded msg = SCR_AIMessage_Wounded.Create(GetControlledEntity());
		msg.SetReceiver(msgReceiverGroup);
		comms.RequestBroadcast(msg, msgReceiverGroup);
	} 
	
	//------------------------------------------------------------------------------------------------
	void OnLifeStateChanged(ECharacterLifeState previousLifeState, ECharacterLifeState newLifeState)
	{
		if (newLifeState == ECharacterLifeState.INCAPACITATED)
		{
			// first send message and then deactivate otherwise message won't be sent
			SendWoundedMsg();
			GetControlComponent().DeactivateAI();
			
			SCR_AICommsHandler commsHandler = m_UtilityComponent.m_CommsHandler;
			if (commsHandler)
				commsHandler.SetSuspended(true);
			
			return;
		}
		else if (newLifeState == ECharacterLifeState.DEAD)
		{
			GetControlComponent().DeactivateAI();
			SCR_AICommsHandler commsHandler = m_UtilityComponent.m_CommsHandler;
			if (commsHandler)
				commsHandler.Reset();
			
			return;
		}
		else if (newLifeState == ECharacterLifeState.ALIVE)
		{
			bool possesingMainEntity = false;
			array<int> players = new array<int>;
			GetGame().GetPlayerManager().GetPlayers(players);
			foreach (int playerId: players)
			{
				if(SCR_PossessingManagerComponent.GetPlayerMainEntity(playerId) == GetControlledEntity())
				{
					possesingMainEntity = true;
					break;
				}
			}
			
			if (!EntityUtils.IsPlayer(GetControlledEntity()) && !possesingMainEntity)
				GetControlComponent().ActivateAI();
			
			AICommunicationComponent comms = GetCommunicationComponent();
			if (comms)
			{
				// Clear orders, it doesn't matter if we have missed them
				comms.ClearOrders();
				
				// Don't clear messages, we will process them to catch up with goals from group
				// This is crucial to resume to group orders
				
				// Clear danger events, it doesn't matter if we have missed them
				ClearDangerEvents(GetDangerEventsCount());
			}
			
			SCR_AICommsHandler commsHandler = m_UtilityComponent.m_CommsHandler;
			if (commsHandler)
				commsHandler.SetSuspended(false);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	Faction GetFaction(IEntity entity)
	{
		// Common case first
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(entity);
		if (character && character.m_pFactionComponent)
		{
			return character.m_pFactionComponent.GetAffiliatedFaction();
		}
		
		Vehicle vehicle = Vehicle.Cast(entity);
		if (vehicle && vehicle.m_pFactionComponent)
		{
			return vehicle.m_pFactionComponent.GetAffiliatedFaction();
		}
		
		// Worst case - some other entity, perform component search
		FactionAffiliationComponent factionAffiliation = FactionAffiliationComponent.Cast(entity.FindComponent(FactionAffiliationComponent));
		if (factionAffiliation)
			return factionAffiliation.GetAffiliatedFaction();
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsEnemy(IEntity entity)
	{
		Faction otherFaction = GetFaction(entity);
		
		Faction myFaction;
		if (m_FactionAffiliationComponent)
			myFaction = m_FactionAffiliationComponent.GetAffiliatedFaction();
		
		if (!otherFaction || !myFaction)
			return false;
				
		return myFaction.IsFactionEnemy(otherFaction);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnGroupWaypointChanged(AIWaypoint newWaypoint)
	{
		m_GroupWaypoint = newWaypoint;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetPlayerPending_S(int playerId)
	{
		if (m_iPendingPlayerId != 0 && playerId != 0)
		{
			Print("Trying to set pending player on an already pending agent!", LogLevel.ERROR);
			return;
		}
		
		m_iPendingPlayerId = playerId;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsPlayerPending_S()
	{
		return m_iPendingPlayerId != 0;
	}
};