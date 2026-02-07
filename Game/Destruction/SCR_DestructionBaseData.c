//------------------------------------------------------------------------------------------------
class SCR_DestructionBaseData
{
	// This integer is public as we don't want to have unnecessary getter & setter calls for it
	int m_iNextFreeIndex = -1;
	protected float m_fPreviousHealth;
	protected ref SCR_DestructionHitInfo m_HitInfo;
	protected ref ScriptInvoker m_OnDamage;
	
	//------------------------------------------------------------------------------------------------
	void Reset()
	{
		m_fPreviousHealth = 0;
		delete m_HitInfo;
		delete m_OnDamage;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetPreviousHealth()
	{
		return m_fPreviousHealth;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetPreviousHealth(float previousHealth)
	{
		m_fPreviousHealth = previousHealth;
	}
	
	//------------------------------------------------------------------------------------------------
	void DeleteHitInfo()
	{
		SCR_DestructionHitInfo hitinfo = m_HitInfo;
		m_HitInfo = null;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_DestructionHitInfo GetHitInfo(bool createNew = true)
	{
		if (!m_HitInfo && createNew)
			m_HitInfo = new SCR_DestructionHitInfo();
		return m_HitInfo;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnDamage(bool createNew = true)
	{
		if (!m_OnDamage && createNew)
			m_OnDamage = new ScriptInvoker();
		return m_OnDamage;
	}
};