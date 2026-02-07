//------------------------------------------------------------------------------------------------
class SCR_CampaignRadioMsg : ScriptedRadioMessage
{
	static const int INVALID_RADIO_MSG_PARAM = -1;
	
	protected SCR_ERadioMsg m_iRadioMsg;
	protected int m_iBaseCallsign =  SCR_CampaignBase.INVALID_BASE_INDEX;
	protected int m_iCallerCompanyID = SCR_CampaignBase.INVALID_PLAYER_INDEX;
	protected int m_iCallerPlatoonID = SCR_CampaignBase.INVALID_PLAYER_INDEX;
	protected int m_iCallerSquadID = SCR_CampaignBase.INVALID_PLAYER_INDEX;
	protected int m_iCalledCompanyID = SCR_CampaignBase.INVALID_PLAYER_INDEX;
	protected int m_iCalledPlatoonID = SCR_CampaignBase.INVALID_PLAYER_INDEX;
	protected int m_iCalledSquadID = SCR_CampaignBase.INVALID_PLAYER_INDEX;
	protected float m_fSeed = Math.RandomFloat01();
	protected bool m_bIsPublic = true;
	protected int m_iParam = INVALID_RADIO_MSG_PARAM;
	protected int m_iPlayerID = SCR_CampaignBase.INVALID_PLAYER_INDEX;
	
	//------------------------------------------------------------------------------------------------
	void SetRadioMsg(SCR_ERadioMsg msg)
	{
		m_iRadioMsg = msg;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetBaseCallsign(int callsign)
	{
		m_iBaseCallsign = callsign;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetCallerCallsign(int companyID, int platoonID, int squadID)
	{
		m_iCallerCompanyID = companyID;
		m_iCallerPlatoonID = platoonID;
		m_iCallerSquadID = squadID;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetCalledCallsign(int companyID, int platoonID, int squadID)
	{
		m_iCalledCompanyID = companyID;
		m_iCalledPlatoonID = platoonID;
		m_iCalledSquadID = squadID;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetIsPublic(bool public)
	{
		m_bIsPublic = public;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetParam(int param)
	{
		m_iParam = param;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetPlayerID(int playerID)
	{
		m_iPlayerID = playerID;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetSeed()
	{
		return m_fSeed;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDelivery(BaseRadioComponent radio, int freq, float quality, int transcvIdx)
	{
		IEntity owner = radio.GetOwner();
		
		ChimeraCharacter player;
		SCR_CampaignBase base;
		
		while (!player && !base)
		{
			player = ChimeraCharacter.Cast(owner);
			base = SCR_CampaignBase.Cast(owner);
			
			if (player || base)
				break;
			
			owner = owner.GetParent();
			
			if (!owner)
				return;
		}
		
		if (player)
		{
			int playerID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(owner);
			PlayerController controller = GetGame().GetPlayerManager().GetPlayerController(playerID);
			
			if (!controller)
				return;
			
			SCR_CampaignNetworkComponent comp = SCR_CampaignNetworkComponent.Cast(controller.FindComponent(SCR_CampaignNetworkComponent));
			
			if (comp)
				comp.PlayRadioMsg(m_iRadioMsg, m_iBaseCallsign, m_iCallerCompanyID, m_iCallerPlatoonID, m_iCallerSquadID, m_iCalledCompanyID, m_iCalledPlatoonID, m_iCalledSquadID, m_bIsPublic, m_iParam, m_fSeed, quality, m_iPlayerID);
		}
		else
		{
			// Received by HQ, ping player that the message has been received
			if (base.GetType() != CampaignBaseType.MAIN)
				return;
			
			PlayerController controller = GetGame().GetPlayerManager().GetPlayerController(m_iPlayerID);
			
			if (!controller)
				return;
			
			SCR_CampaignNetworkComponent comp = SCR_CampaignNetworkComponent.Cast(controller.FindComponent(SCR_CampaignNetworkComponent));
			
			if (comp)
				comp.SetLastHQRadioMessageTimestamp(Replication.Time());
		}
	}
};

enum SCR_ERadioMsg
{
	NONE,
	SEIZED_MAIN,
	SEIZED_MAJOR,
	SEIZED_SMALL,
	DEMOTION_RENEGADE,
	DEMOTION,
	PROMOTION_PRIVATE,
	PROMOTION_CORPORAL,
	PROMOTION_SERGEANT,
	PROMOTION_LIEUTENANT,
	PROMOTION_CAPTAIN,
	PROMOTION_MAJOR,
	REINFORCEMENTS,
	VICTORY,
	WINNING,
	DEFEAT,
	LOSING,
	RELAY,
	REQUEST_EVAC,
	REQUEST_FUEL,
	SUPPLIES,
	REQUEST_REINFORCEMENTS,
	REQUEST_TRANSPORT,
	CONFIRM,
	TASK_ASSIGN_SEIZE,
	TASK_UNASSIGN_REFUEL,
	TASK_UNASSIGN_TRANSPORT,
	TASK_UNASSIGN_EVAC,
	TASK_CANCEL_REQUEST,
	TASK_ASSIST,
	BASE_LOST,
	BASE_UNDER_ATTACK,
	BUILT_ARMORY,
	BUILT_FUEL,
	BUILT_REPAIR,
	BUILT_SUPPLY,
	BUILT_VEHICLES_LIGHT,
	BUILT_VEHICLES_HEAVY,
	BUILT_BARRACKS,
	BUILT_ANTENNA,
	BUILT_FIELD_HOSPITAL,
	DESTROYED_ARMORY,
	DESTROYED_FUEL,
	DESTROYED_REPAIR,
	REPAIRED_ARMORY,
	REPAIRED_FUEL,
	REPAIRED_REPAIR
};
