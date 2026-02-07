//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_CharacterRank
{
	[Attribute(defvalue: "1", uiwidget: UIWidgets.ComboBox, desc: "Rank ID set in FactionManager", enums: ParamEnumArray.FromEnum(ECharacterRank))]
	protected ECharacterRank m_iRank;
	
	[Attribute(defvalue: "", uiwidget: UIWidgets.EditBox, desc: "Rank name")]
	protected string m_sRankName;
	
	[Attribute(defvalue: "", uiwidget: UIWidgets.EditBox, desc: "Rank name (upper case)")]
	protected string m_sRankNameUpper;
	
	[Attribute(defvalue: "", uiwidget: UIWidgets.EditBox, desc: "Rank name (short)")]
	protected string m_sRankNameShort;
	
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Rank insignia", params: "edds")]
	protected ResourceName m_sInsignia;
	//------------------------------------------------------------------------------------------------
	ECharacterRank GetRankID()
	{
		return m_iRank;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetRankName()
	{
		return m_sRankName;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetRankNameUpperCase()
	{
		return m_sRankNameUpper;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetRankNameShort()
	{
		return m_sRankNameShort;
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetRankInsignia()
	{
		return m_sInsignia;
	}
};

//------------------------------------------------------------------------------------------------
class SCR_MilitaryFaction : SCR_Faction
{
	[Attribute("", UIWidgets.Object, "List of ranks")]
	protected ref array<ref SCR_CharacterRank> m_aRanks;
	
	[Attribute("", uiwidget: UIWidgets.EditBox)]
	protected string m_sFactionRadioEncryptionKey;
	
	[Attribute("0", uiwidget: UIWidgets.EditBox)]
	protected int m_iFactionRadioFrequency;
	
	//------------------------------------------------------------------------------------------------
	protected SCR_CharacterRank GetRankByID(ECharacterRank rankID)
	{
		if (!m_aRanks)
			return null;

		foreach (SCR_CharacterRank rank: m_aRanks)
		{
			if (rank && rank.GetRankID() == rankID)
				return rank;
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetRankName(ECharacterRank rankID)
	{
		SCR_CharacterRank rank = GetRankByID(rankID);
		
		if (!rank)
			return "";
			
		return rank.GetRankName();
	}
	
	//------------------------------------------------------------------------------------------------
	string GetRankNameUpperCase(ECharacterRank rankID)
	{
		SCR_CharacterRank rank = GetRankByID(rankID);
		
		if (!rank)
			return "";
			
		return rank.GetRankNameUpperCase();
	}
	
	//------------------------------------------------------------------------------------------------
	string GetRankNameShort(ECharacterRank rankID)
	{
		SCR_CharacterRank rank = GetRankByID(rankID);
		
		if (!rank)
			return "";
			
		return rank.GetRankNameShort();
	}
	
	//------------------------------------------------------------------------------------------------
	string GetFactionRadioEncryptionKey()
	{
		return m_sFactionRadioEncryptionKey;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetFactionRadioFrequency()
	{
		return m_iFactionRadioFrequency;
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetRankInsignia(ECharacterRank rankID)
	{
		SCR_CharacterRank rank = GetRankByID(rankID);
		
		if (!rank)
			return "";
			
		return rank.GetRankInsignia();
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_MilitaryFaction()
	{
		if (m_aRanks)
			m_aRanks.Clear();
		
		m_aRanks = null;
	}
};