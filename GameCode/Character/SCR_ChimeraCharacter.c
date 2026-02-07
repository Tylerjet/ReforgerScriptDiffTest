

class SCR_ChimeraCharacterClass: ChimeraCharacterClass
{
};

class SCR_ChimeraCharacter : ChimeraCharacter
{
	float m_fFaceAlphaTest = 0; // Used to fade away the head when getting close with the 3rd person camera

	FactionAffiliationComponent m_pFactionComponent;
	
	override void EOnInit(IEntity owner)
	{
	}
	
	void SCR_ChimeraCharacter(IEntitySource src, IEntity parent)
	{
	}
	
	void ~SCR_ChimeraCharacter()
	{
		m_pFactionComponent = null;
	}
	
	Faction GetFaction()
	{
		if (!m_pFactionComponent)
			m_pFactionComponent = FactionAffiliationComponent.Cast(FindComponent(FactionAffiliationComponent));
		
		if (m_pFactionComponent)
			return m_pFactionComponent.GetAffiliatedFaction();
		
		return null;
	}
	
	string GetFactionKey()
	{
		Faction faction = GetFaction();
		if (!faction)
			return string.Empty;
		
		return faction.GetFactionKey();
	}
};