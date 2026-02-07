[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EVotingType, "m_Type")]
class SCR_VotingKick: SCR_VotingReferendum
{
	override bool IsAvailable(int value, bool isOngoing)
	{
		//--- Cannot kick yourself
		if (value == SCR_PlayerController.GetLocalPlayerId())
			return false;
		
		//--- Cannot kick admin
		if (SCR_Global.IsAdmin(value))
			return false;
		
		//--- Cannot vote to kick out session host
		SCR_VotingManagerComponent votingManager = SCR_VotingManagerComponent.GetInstance();
		return votingManager && votingManager.GetHostPlayerID() != value;
	}
	override protected int GetPlayerCount()
	{
		return Math.Max(super.GetPlayerCount() - 1, 2); //--- Ignore target player. 2 is a limit to prevent instant completion in a session with just 2 people.
	}
	override void OnVotingEnd(int value = DEFAULT_VALUE, int winner = DEFAULT_VALUE)
	{
		if (winner != DEFAULT_VALUE)
		{
			SCR_PlayerPenaltyComponent playerPenaltyComponent = SCR_PlayerPenaltyComponent.Cast(SCR_PlayerPenaltyComponent.GetInstance());
			
			if (playerPenaltyComponent)
				playerPenaltyComponent.KickPlayer(winner, SCR_PlayerManagerKickReason.KICK_VOTED);
			else 
				GetGame().GetPlayerManager().KickPlayer(winner, PlayerManagerKickReason.KICK_VOTED);
		}
	}
};