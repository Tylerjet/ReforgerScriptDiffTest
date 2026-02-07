[EntityEditorProps(category: "GameScripted/Campaign", description: "This allows only vehicle requester enther the vehicle as driver for defined period of time.")]
class SCR_VehicleSpawnProtectionComponentClass : ScriptComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_VehicleSpawnProtectionComponent : SCR_BaseLockComponent
{
	const int NO_OWNER = -1;
	const int NO_TIME_LIMIT = 0;
	
	[RplProp()]
	private int m_iVehicleOwnerID = NO_OWNER;
	
	protected int m_iTimeOfProtection;
	private SCR_CharacterControllerComponent m_CharControlComp;
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{			
		ClearEventMask(owner, EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	private void ReleaseProtection()
	{
		RemoveEventHandlers();
		m_iVehicleOwnerID = NO_OWNER;
		Replication.BumpMe();
		
		GetGame().GetCallqueue().Remove(ReleaseProtection);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetProtectionTime(int protectionTime)
	{
		if (protectionTime != NO_TIME_LIMIT)
		{
			m_iTimeOfProtection = protectionTime;
			GetGame().GetCallqueue().CallLater(ReleaseProtection, m_iTimeOfProtection * 1000, false);
		}		
	}
	
	//------------------------------------------------------------------------------------------------
	void SetVehicleOwner(int playerID)
	{
		IEntity playerEnt = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);
		if (!playerEnt)
			return;

		m_CharControlComp = SCR_CharacterControllerComponent.Cast(playerEnt.FindComponent(SCR_CharacterControllerComponent));
		if (!m_CharControlComp)
			return;
		
		// Add EH
		m_CharControlComp.m_OnPlayerDeath.Insert(ReleaseProtection);	
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (gameMode)
			gameMode.GetOnPlayerDisconnected().Insert(OnPlayerDisconected);

		// Set owner of this vehicle
		m_iVehicleOwnerID = playerID;		
		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	int GetVehicleOwner()
	{
		return m_iVehicleOwnerID;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsProtected(notnull IEntity playerEntering, notnull BaseCompartmentSlot compartmentSlot)
	{
		// Test if it is a driver seat or not.
		if (!PilotCompartmentSlot.Cast(compartmentSlot))
    		return false;
				
		int testPlayerID = SCR_PossessingManagerComponent.GetPlayerIdFromMainEntity(playerEntering);
		// Check vehicle owenr ID
		if (m_iVehicleOwnerID == NO_OWNER || m_iVehicleOwnerID == testPlayerID /*m_Lobby.GetPlayerIdFromControlledEntity(playerEntering)*/)
		{
			#ifdef VEHICLE_LOCK
			PrintFormat("Player ID is %1, vehicle owner ID is %2. Vehicle is unlocked.",testPlayerID,m_iVehicleOwnerID);
			#endif
			return false;
		}
		#ifdef VEHICLE_LOCK
		PrintFormat("Player ID is %1, vehicle owner ID is %2. Vehicle is LOCKED.",testPlayerID,m_iVehicleOwnerID);
		#endif
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void OnPlayerDisconected(int playerID)
	{
		if (playerID == m_iVehicleOwnerID)
			ReleaseProtection();
	}	
	//------------------------------------------------------------------------------------------------
	LocalizedString GetReasonText(IEntity user)
	{
		if (!user)
			return string.Empty;

		return "#AR-Campaign_Action_CannotEnterVehicle-UC";
	}
	
	//------------------------------------------------------------------------------------------------
	void RemoveEventHandlers()
	{
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (gameMode)
			gameMode.GetOnPlayerDisconnected().Remove(OnPlayerDisconected);
		
		if (m_CharControlComp)
			m_CharControlComp.m_OnPlayerDeath.Remove(ReleaseProtection);
	}
};
