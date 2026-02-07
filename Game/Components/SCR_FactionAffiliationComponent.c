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
			m_OnFactionUpdate.Invoke(GetAffiliatedFaction());
	}
	
	// may return to DefaultAffiliatedFaction in some cases - override this
	//--------------------------------------------------------------------------------------------------------------------------
	void ClearAffiliatedFaction()
	{
		SetAffiliatedFaction(null); 
	}
	
	//--------------------------------------------------------------------------------------------------------------------------
	/*
	Set faction.
	\param IEntity Owner
	\param faction Desired faction
	*/
	static void SetFaction(IEntity owner, Faction faction)
	{
		if (!owner || !faction )
			return;

		SCR_FactionAffiliationComponent factionComponent = SCR_FactionAffiliationComponent.Cast(owner.FindComponent(SCR_FactionAffiliationComponent));
		if (factionComponent)
			factionComponent.SetAffiliatedFaction(faction);
	}
	
	//--------------------------------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnFactionUpdate()
	{
		if (!m_OnFactionUpdate)
			m_OnFactionUpdate = new ScriptInvoker();
		return m_OnFactionUpdate;
	}
};