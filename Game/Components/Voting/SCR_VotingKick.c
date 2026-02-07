[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EVotingType, "m_Type")]
class SCR_VotingKick: SCR_VotingReferendum
{
	[Attribute("0", desc: "How long before kicked-out player can reconnect.\n-1 means permanent ban (at least until exe restart)")]
	protected int m_iKickTimeout;
	
	[Attribute(uiwidget: UIWidgets.SearchComboBox, desc: "Reason for kicking shown to kicked-out player.", enums: ParamEnumArray.FromEnum(SCR_PlayerManagerKickReason))]
	protected SCR_PlayerManagerKickReason m_KickReason;
	
	[Attribute(desc: "When true, only players on the same faction as target of the vote can vote.")]
	protected bool m_bFactionSpecific;
	
	protected Faction GetPlayerFaction(int playerID)
	{
		SCR_RespawnSystemComponent respawnSystem = SCR_RespawnSystemComponent.GetInstance();
		if (respawnSystem)
			return respawnSystem.GetPlayerFaction(playerID);
		else
			return null;
	}
	
	override bool IsAvailable(int value, bool isOngoing)
	{
		return true;
		//--- Cannot kick yourself
		if (value == SCR_PlayerController.GetLocalPlayerId())
			return false;
		
		//--- Cannot kick admin
		if (SCR_Global.IsAdmin(value))
			return false;
		
		//--- When faction specific, allow voting only for players on the same faction as target of the vote
		if (m_bFactionSpecific)
		{
			Faction targetFaction = GetPlayerFaction(value); //--- value is ID of the player who is target of the vote
			Faction playerFaction = GetPlayerFaction(SCR_PlayerController.GetLocalPlayerId());
			if (targetFaction != playerFaction)
				return false;
		}
		
		//--- Cannot vote to kick out session host
		SCR_VotingManagerComponent votingManager = SCR_VotingManagerComponent.GetInstance();
		return votingManager && votingManager.GetHostPlayerID() != value;
	}
	override protected int GetPlayerCount()
	{
		int playerCount;
		if (m_bFactionSpecific)
		{
			//--- When faction specific, count only players on the same faction as target of the vote
			//--- e.g., with 50% vote limit, only half of BLUFOR players will have to vote, not half of all playerd
			Faction targetFaction;
			Faction playerFaction = GetPlayerFaction(SCR_PlayerController.GetLocalPlayerId());
			
			array<int> players = {};
			for (int i = 0, count = GetGame().GetPlayerManager().GetPlayers(players); i < count; i++)
			{
				if (players[i] != m_iValue)
				{
					targetFaction = GetPlayerFaction(i);
					if (targetFaction == playerFaction)
						playerCount++;
				}
			}
		}
		else
		{
			//--- Ignore target player by subtracting 1
			playerCount = super.GetPlayerCount() - 1;
		}
		//--- 2 is a limit to prevent instant completion in a session with just 2 people
		return Math.Max(playerCount, 2);
	}
	override void OnVotingEnd(int value = DEFAULT_VALUE, int winner = DEFAULT_VALUE)
	{
		if (winner != DEFAULT_VALUE)
			GetGame().GetPlayerManager().KickPlayer(winner, m_KickReason, m_iKickTimeout);
	}
};