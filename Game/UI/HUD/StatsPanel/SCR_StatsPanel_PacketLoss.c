class SCR_StatsPanel_PacketLoss : SCR_StatsPanelBase
{
	//------------------------------------------------------------------------------------------------
	override protected float GetValue()
	{
		RplConnectionStats stats = Replication.GetConnectionStats(m_RplIdentity);
		
		return stats.GetPacketLoss();
	}
}