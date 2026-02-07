[BaseContainerProps(category: "Respawn")]
class SCR_MenuSpawnLogic : SCR_SpawnLogic
{
	//------------------------------------------------------------------------------------------------
	override void OnPlayerRegistered_S(int playerId)
	{
		super.OnPlayerRegistered_S(playerId);

		// Probe reconnection component first
		IEntity returnedEntity;
		if (ResolveReconnection(playerId, returnedEntity))
		{
			// User was reconnected, their entity was returned
			return;
		}

		// Send a notification to registered client:
		// Always ensure to hook OnLocalPlayer callbacks prior to sending such notification,
		// otherwise the notification will be disregarded
		GetPlayerRespawnComponent_S(playerId).NotifyReadyForSpawn_S();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPlayerEntityLost_S(int playerId)
	{
		super.OnPlayerEntityLost_S(playerId);

		GetPlayerRespawnComponent_S(playerId).NotifyReadyForSpawn_S();
	}
};