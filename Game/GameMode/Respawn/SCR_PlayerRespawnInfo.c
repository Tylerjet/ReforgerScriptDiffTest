//------------------------------------------------------------------------------------------------
//! Helper class that holds data for a player respawn.
class SCR_PlayerRespawnInfo
{
	static const int RESPAWN_INFO_INVALID_INDEX = -1;
	
	protected int m_iPlayerID = RESPAWN_INFO_INVALID_INDEX;
	protected int m_iPlayerLoadoutIndex = RESPAWN_INFO_INVALID_INDEX;
	protected int m_iPlayerFactionIndex = RESPAWN_INFO_INVALID_INDEX;
	protected RplId m_iPlayerSpawnPointIdentity = RplId.Invalid();
	
	//------------------------------------------------------------------------------------------------
	int GetPlayerID()
	{
		return m_iPlayerID;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetPlayerID(int playerId)
	{
		m_iPlayerID = playerId;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetPlayerLoadoutIndex(int index)
	{
		m_iPlayerLoadoutIndex = index;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetPlayerFactionIndex(int index)
	{
		m_iPlayerFactionIndex = index;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetPlayerSpawnPointIdentity(RplId identity)
	{
		m_iPlayerSpawnPointIdentity = identity;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetPlayerLoadoutIndex()
	{
		return m_iPlayerLoadoutIndex;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetPlayerFactionIndex()
	{
		return m_iPlayerFactionIndex;
	}
	
	//------------------------------------------------------------------------------------------------
	RplId GetPlayerSpawnPointIdentity()
	{
		return m_iPlayerSpawnPointIdentity;
	}
	
	
	//################################################################################################
	//! Codec methods
	//------------------------------------------------------------------------------------------------
	static void Encode(SSnapSerializerBase snapshot, ScriptCtx ctx, ScriptBitSerializer packet) 
	{
		snapshot.Serialize(packet, 16);
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Decode(ScriptBitSerializer packet, ScriptCtx ctx, SSnapSerializerBase snapshot) 
	{
		return snapshot.Serialize(packet, 16);
	}
	
	//------------------------------------------------------------------------------------------------
	static bool SnapCompare(SSnapSerializerBase lhs, SSnapSerializerBase rhs, ScriptCtx ctx) 
	{	
		return lhs.CompareSnapshots(rhs, 16);
	}
	
	//------------------------------------------------------------------------------------------------
	static bool PropCompare(SCR_PlayerRespawnInfo prop, SSnapSerializerBase snapshot, ScriptCtx ctx) 
	{
		return snapshot.Compare(prop.m_iPlayerID, 4) 
			&& snapshot.Compare(prop.m_iPlayerLoadoutIndex, 4) 
			&& snapshot.Compare(prop.m_iPlayerFactionIndex, 4)
			&& snapshot.Compare(prop.m_iPlayerSpawnPointIdentity, 4);
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Extract(SCR_PlayerRespawnInfo prop, ScriptCtx ctx, SSnapSerializerBase snapshot) 
	{		
		snapshot.SerializeBytes(prop.m_iPlayerID, 4);
		snapshot.SerializeBytes(prop.m_iPlayerLoadoutIndex, 4);
		snapshot.SerializeBytes(prop.m_iPlayerFactionIndex, 4);
		snapshot.SerializeBytes(prop.m_iPlayerSpawnPointIdentity, 4);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Inject(SSnapSerializerBase snapshot, ScriptCtx ctx, SCR_PlayerRespawnInfo prop) 
	{
		snapshot.SerializeBytes(prop.m_iPlayerID, 4);
		snapshot.SerializeBytes(prop.m_iPlayerLoadoutIndex, 4);
		snapshot.SerializeBytes(prop.m_iPlayerFactionIndex, 4);
		snapshot.SerializeBytes(prop.m_iPlayerSpawnPointIdentity, 4);
		
		return true;
	}
	//################################################################################################
	
	
	//------------------------------------------------------------------------------------------------
	void SCR_PlayerRespawnInfo()
	{
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_PlayerRespawnInfo()
	{
	}

};
