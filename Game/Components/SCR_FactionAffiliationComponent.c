class SCR_FactionAffiliationComponentClass: FactionAffiliationComponentClass
{
	
};

class SCR_FactionAffiliationComponent: FactionAffiliationComponent
{
	//! Local invokers
	private ref ScriptInvoker m_OnFactionUpdate;
	
	//--------------------------------------------------------------------------------------------------------------------------
	override protected void OnFactionChanged()
	{
		if (m_OnFactionUpdate)
			m_OnFactionUpdate.Invoke();
	}
	
	// may return to DefaultAffiliatedFaction in some cases - override this
	//--------------------------------------------------------------------------------------------------------------------------
	void ClearAffiliatedFaction()
	{
		SetAffiliatedFaction(null); 
	}
	
	//--------------------------------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnFactionUpdate()
	{
		if (!m_OnFactionUpdate)
			m_OnFactionUpdate = new ScriptInvoker();
		return m_OnFactionUpdate;
	}
};