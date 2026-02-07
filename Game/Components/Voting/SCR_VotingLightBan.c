//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EVotingType, "LIGHTBAN")]
class SCR_VotingLightBan: SCR_VotingKick
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
			
			GetGame().GetPlayerManager().KickPlayer(winner, PlayerManagerKickReason.TEMP_BAN, durationInSeconds);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_VotingLightBan()
	{
		m_bFactionSpecific = true;
	}
};