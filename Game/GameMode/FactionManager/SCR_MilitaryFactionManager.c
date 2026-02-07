//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_RankID
{
	[Attribute(defvalue: "1", uiwidget: UIWidgets.ComboBox, desc: "Rank ID", enums: ParamEnumArray.FromEnum(ECharacterRank))]
	protected ECharacterRank m_iRank;
	
	[Attribute("0", UIWidgets.CheckBox, "Renegade", "Is this rank considered hostile by friendlies?")]
	protected bool m_bIsRenegade;
	
	//------------------------------------------------------------------------------------------------
	ECharacterRank GetRankID()
	{
		return m_iRank;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsRankRenegade()
	{
		return m_bIsRenegade;
	}
};

//------------------------------------------------------------------------------------------------
class SCR_MilitaryFactionManagerClass: SCR_FactionManagerClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_MilitaryFactionManager : SCR_FactionManager
{
	[Attribute("", UIWidgets.Object, "List of rank types")]
	protected ref array<ref SCR_RankID> m_aRanks;
	
	//------------------------------------------------------------------------------------------------
	protected SCR_RankID GetRankByID(ECharacterRank rankID)
	{		
		if (!m_aRanks)
			return null;
		
		foreach (SCR_RankID rank: m_aRanks)
		{	
			if (rank && rank.GetRankID() == rankID)
				return rank;
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsRankRenegade(ECharacterRank rankID)
	{
		SCR_RankID rank = GetRankByID(rankID);
		
		if (!rank)
			return false;
			
		return rank.IsRankRenegade();
	}
	
	//------------------------------------------------------------------------------------------------
	protected ECharacterRank GetRenegadeRank()
	{	
		foreach (SCR_RankID rank: m_aRanks)
		{
			if (rank && rank.IsRankRenegade())
				return rank.GetRankID();
		}
		
		return ECharacterRank.INVALID;
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_MilitaryFactionManager()
	{
		if (m_aRanks)
			m_aRanks.Clear();
		
		m_aRanks = null;
	}
};

enum ECharacterRank
{
	RENEGADE,
	PRIVATE,
	CORPORAL,
	SERGEANT,
	LIEUTENANT,
	CAPTAIN,
	MAJOR,
	COLONEL,
	CUSTOM1,
	CUSTOM2,
	CUSTOM3,
	CUSTOM4,
	CUSTOM5,
	CUSTOM6,
	CUSTOM7,
	CUSTOM8,
	CUSTOM9,
	CUSTOM10,
	INVALID
};