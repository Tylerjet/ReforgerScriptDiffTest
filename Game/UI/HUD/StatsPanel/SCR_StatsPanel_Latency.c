class SCR_StatsPanel_Latency : SCR_StatsPanelBase
{
	//------------------------------------------------------------------------------------------------
	override protected float GetValue()
	{
		RplConnectionStats stats = Replication.GetConnectionStats(m_RplIdentity);
		
		return stats.GetRoundTripTimeInMs();
	}
}