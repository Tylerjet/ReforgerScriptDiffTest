class SCR_ChimeraAIAgentClass: ChimeraAIAgentClass
{
};

class SCR_ChimeraAIAgent : ChimeraAIAgent
{
	// Current waypoint of our group
	AIWaypoint m_GroupWaypoint;
	
	protected EventHandlerManagerComponent	m_EventHandlerManagerComponent;
	protected FactionAffiliationComponent m_FactionAffiliationComponent;
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner) 
	{
		if (!GetControlledEntity())
			return;
		
		IEntity controlledEntity = GetControlledEntity();
		
		m_EventHandlerManagerComponent = EventHandlerManagerComponent.Cast(controlledEntity.FindComponent(EventHandlerManagerComponent));
		if (m_EventHandlerManagerComponent)
			m_EventHandlerManagerComponent.RegisterScriptHandler("OnConsciousnessChanged", this, this.OnConsciousnessChanged, true);
		
		m_FactionAffiliationComponent = FactionAffiliationComponent.Cast(controlledEntity.FindComponent(FactionAffiliationComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_ChimeraAIAgent()
	{
		if (m_EventHandlerManagerComponent)
			m_EventHandlerManagerComponent.RemoveScriptHandler("OnConsciousnessChanged", this, this.OnConsciousnessChanged, true);
	}

	//------------------------------------------------------------------------------------------------	
	protected void SendWoundedMsg()
	{
		SCR_CharacterDamageManagerComponent damageMgr = SCR_CharacterDamageManagerComponent.Cast(GetControlledEntity().FindComponent(SCR_CharacterDamageManagerComponent));
		if (!damageMgr)
			return;
	
		if (!damageMgr.IsDamagedOverTime(EDamageType.BLEEDING))
			return;
		
		AIGroup myGroup = GetParentGroup();
		if (!myGroup)
			return;
		
		AICommunicationComponent comms = GetCommunicationComponent();
		if (!comms)
			return;
		
		SCR_AIMessage_Wounded msg = SCR_AIMessage_Wounded.Create(GetControlledEntity());
		msg.SetReceiver(myGroup);
		comms.RequestBroadcast(msg, myGroup);
	} 
	
	//------------------------------------------------------------------------------------------------
	void OnConsciousnessChanged(bool conscious)
	{
		if (!conscious)
		{
			// first send message and then deactivate otherwise message won't be sent
			SendWoundedMsg();
			GetControlComponent().DeactivateAI();
		}
		else
		{
			if (!EntityUtils.IsPlayer(GetControlledEntity()))
				GetControlComponent().ActivateAI();
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
};