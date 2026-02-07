//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EVotingType, "HEAVYBAN")]
class SCR_VotingHeavyBan: SCR_VotingKick
{
	//------------------------------------------------------------------------------------------------
	override void OnVotingEnd(int value = DEFAULT_VALUE, int winner = DEFAULT_VALUE)
	{
		if (winner != DEFAULT_VALUE)
		{
			SCR_DataCollectorCrimesModule crimesModule = SCR_DataCollectorCrimesModule.Cast(GetGame().GetDataCollector().FindModule(SCR_DataCollectorCrimesModule));
			if (!crimesModule)
			{
				GetGame().GetPlayerManager().KickPlayer(winner, PlayerManagerKickReason.KICK, 0);
				return;
			}
			
			int durationInSeconds = crimesModule.FindDurationPunishmentInQueue(winner);
			
			if (durationInSeconds <= 0)
			{
				GetGame().GetPlayerManager().KickPlayer(winner, PlayerManagerKickReason.KICK, 0);
				return;
			}
			
			GetGame().GetBackendApi().PlayerBanCreate("Heavy ban", durationInSeconds, winner);
			GetGame().GetPlayerManager().KickPlayer(winner, PlayerManagerKickReason.BAN, 0);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_VotingHeavyBan()
	{
		m_bFactionSpecific = true;
	}
};