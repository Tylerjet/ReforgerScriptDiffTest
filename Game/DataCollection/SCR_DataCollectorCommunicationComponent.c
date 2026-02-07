[EntityEditorProps(category: "GameScripted/DataCollection/", description: "Component used to send data to specific clients.")]
class SCR_DataCollectorCommunicationComponentClass : ScriptComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_DataCollectorCommunicationComponent : ScriptComponent
{
	protected ref ScriptInvoker m_OnDataReceived;
	
	//------------------------------------------------------------------------------------------------
	notnull ScriptInvoker GetOnDataReceived()
	{
		if (!m_OnDataReceived)
			m_OnDataReceived = new ScriptInvoker();
		
		return m_OnDataReceived;
	}
	
	//------------------------------------------------------------------------------------------------
	void SendData(SCR_PlayerData playerData)
	{
		Rpc(Rpc_DoSendData, playerData.GetStats(), playerData.GetPreviousStats());
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void Rpc_DoSendData(array<float> stats, array<float> previousStats)
	{
		for (int i = 0, count = stats.Count(); i < count; i++)
		{
			Print(stats[i], LogLevel.DEBUG);
		}
		for (int i = 0, count = previousStats.Count(); i < count; i++)
		{
			Print(previousStats[i], LogLevel.DEBUG);
		}
		
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return;
		
		SCR_DataCollectorComponent dataCollector = GetGame().GetDataCollector();
		if (!dataCollector)
			return;
		
		SCR_PlayerData playerData = dataCollector.GetPlayerData(0, true, false); // Use 0 as ID for local player, as this is always on the client
		playerData.SetStats(stats);
		playerData.SetPreviousStats(previousStats);
		
		if (m_OnDataReceived)
			m_OnDataReceived.Invoke(playerData);
	}
};
