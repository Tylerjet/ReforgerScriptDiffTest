class SCR_RespawnTimer
{
	//! Time at which respawn timer was engaged
	protected float m_fStartTime;
	//! The duration of this timer in seconds
	protected float m_fDuration;
	
	/*!
		Start the timer by providing new start time.
		\param timeNow Current (synchronized) time to ask at.
	*/
	void Start(float timeNow)
	{
		m_fStartTime = timeNow;
	}
	
	/*!
		Returns current start time of this timer as seconds of synchronized time.
	*/
	float GetStartTime()
	{
		return m_fStartTime;
	}
	
	/*!
		\param timeNow Current (synchronized) time to ask at.
		\return Returns true in case timer is finished, false otherwise.
	*/
	bool IsFinished(float timeNow)
	{	
		return timeNow >= m_fStartTime + m_fDuration;
	}
	
	/*!
		\param timeNow Current (synchronized) time to ask at.
		\return Returns remaining time in seconds or 0 if finished.
	*/
	float GetRemainingTime(float timeNow)
	{
		float rem = (m_fStartTime + m_fDuration) - timeNow;
		if (rem <= 0)
			return 0.0;
	
		return rem;
	}
	
	/*!
		Returns duration in seconds of this timer.
	*/
	float GetDuration()
	{
		return m_fDuration;
	}
	
	/*!
		\param duration New duration in seconds to set.
	*/
	void SetDuration(float duration)
	{
		m_fDuration = duration;
	}
	
	/*!
		Serialize this class using provided ScriptBitWriter.
	*/
	bool RplSave(ScriptBitWriter writer)
	{
		writer.WriteFloat(m_fStartTime);
		writer.WriteFloat(m_fDuration);
		
		return true;
	}
	
	/*!
		Deserialize this class using provided ScriptBitWriter.
	*/
    bool RplLoad(ScriptBitReader reader)
	{
		reader.ReadFloat(m_fStartTime);
		reader.ReadFloat(m_fDuration);
		
		return true;
	}	
	
	//################################################################################################
	//! Codec methods
	//------------------------------------------------------------------------------------------------
	static void Encode(SSnapSerializerBase snapshot, ScriptCtx ctx, ScriptBitSerializer packet) 
	{
		snapshot.Serialize(packet, 8);
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Decode(ScriptBitSerializer packet, ScriptCtx ctx, SSnapSerializerBase snapshot) 
	{
		return snapshot.Serialize(packet, 8);
	}
	
	//------------------------------------------------------------------------------------------------
	static bool SnapCompare(SSnapSerializerBase lhs, SSnapSerializerBase rhs, ScriptCtx ctx) 
	{		
		return lhs.CompareSnapshots(rhs, 8);
	}

	//------------------------------------------------------------------------------------------------
	static bool PropCompare(SCR_RespawnTimer prop, SSnapSerializerBase snapshot, ScriptCtx ctx) 
	{
		return snapshot.Compare(prop.m_fStartTime, 4) 
			&& snapshot.Compare(prop.m_fDuration, 4);
			}
			
	//------------------------------------------------------------------------------------------------
	static bool Extract(SCR_RespawnTimer prop, ScriptCtx ctx, SSnapSerializerBase snapshot) 
	{
		snapshot.SerializeBytes(prop.m_fStartTime, 4);
		snapshot.SerializeBytes(prop.m_fDuration, 4);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Inject(SSnapSerializerBase snapshot, ScriptCtx ctx, SCR_RespawnTimer prop) 
	{
		snapshot.SerializeBytes(prop.m_fStartTime, 4);
		snapshot.SerializeBytes(prop.m_fDuration, 4);	
		return true;
	}
};