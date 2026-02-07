//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_RankIDCampaign: SCR_RankID
{	
	[Attribute("100", UIWidgets.EditBox, "XP required to get promoted to this rank.")]
	protected int m_iXP;
	
	[Attribute("10", UIWidgets.EditBox, "How long does this rank has to wait between requests (sec).")]
	protected int m_iRequestCD;
	
	[Attribute("30", UIWidgets.EditBox, "Respawn timer when deploying on this unit while it's carrying a radio.")]
	protected int m_iRadioRespawnCooldown;
	
	//------------------------------------------------------------------------------------------------
	int GetRankXP()
	{
		return m_iXP;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetRankRequestCooldown()
	{
		return m_iRequestCD * 1000;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetRankRadioRespawnCooldown()
	{
		return m_iRadioRespawnCooldown;
	}
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignFactionManagerClass: SCR_MilitaryFactionManagerClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignFactionManager : SCR_MilitaryFactionManager
{
	protected static SCR_CampaignFactionManager s_Instance;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Slot Flat Small", "et")]
	private ResourceName m_SlotFlatSmall;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Slot Flat Medium", "et")]
	private ResourceName m_SlotFlatMedium;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Slot Flat Large", "et")]
	private ResourceName m_SlotFlatLarge;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Slot Checkpoint Small", "et")]
	private ResourceName m_SlotCheckpointSmall;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Slot Checkpoint Medium", "et")]
	private ResourceName m_SlotCheckpointMedium;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Slot Checkpoint Large", "et")]
	private ResourceName m_SlotCheckpointLarge;
	
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetSlotsResource(SCR_ESlotTypesEnum type)
	{
		switch (type)
		{
			case SCR_ESlotTypesEnum.FlatSmall: {return m_SlotFlatSmall;};
			case SCR_ESlotTypesEnum.FlatMedium: {return m_SlotFlatMedium;};
			case SCR_ESlotTypesEnum.FlatLarge: {return m_SlotFlatLarge;};
			case SCR_ESlotTypesEnum.CheckpointSmall: {return m_SlotCheckpointSmall;};
			case SCR_ESlotTypesEnum.CheckpointMedium: {return m_SlotCheckpointMedium;};
			case SCR_ESlotTypesEnum.CheckpointLarge: {return m_SlotCheckpointLarge;};
			default: {return m_SlotFlatSmall;};
		}
		
		return ResourceName.Empty;
	}

	//------------------------------------------------------------------------------------------------
	static notnull SCR_CampaignFactionManager GetInstance()
	{
		if (!s_Instance)
		{
			FactionManager factionManager = GetGame().GetFactionManager();
			if (factionManager)
			{
				SCR_CampaignFactionManager campaignFactionManager = SCR_CampaignFactionManager.Cast(factionManager);
				if (!campaignFactionManager)
					Print("SCR_CampaignFactionManager.s_Instance is null, but there's another faction manager! Something is very wrong, check the map!", LogLevel.ERROR);
				else
					s_Instance = campaignFactionManager;
			}
			else
				GetGame().SpawnEntityPrefab(SCR_GameModeCampaignMP.GetFactionManagerResource(), GetGame().GetWorld());
		}
		
		return s_Instance;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_CampaignFaction GetEnemyFaction(notnull SCR_CampaignFaction alliedFaction)
	{
		array<Faction> factions = new array<Faction>();
		GetFactionsList(factions);
		
		for (int i = factions.Count() - 1; i >= 0; i--)
		{
			SCR_Faction factionCast = SCR_Faction.Cast(factions[i]);
			
			if (factionCast && factionCast.IsPlayable() && factionCast != alliedFaction)
				return SCR_CampaignFaction.Cast(factionCast);
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_CampaignFaction GetCampaignFactionByKey(string factionKey)
	{
		Faction faction = GetFactionByKey(factionKey);
		if (faction)
			return SCR_CampaignFaction.Cast(faction);
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_CampaignFaction GetCampaignFactionByIndex(int index)
	{
		return SCR_CampaignFaction.Cast(GetFactionByIndex(index));
	}
	
	//------------------------------------------------------------------------------------------------
	int GetRankXP(ECharacterRank rankID)
	{
		SCR_RankIDCampaign rank = SCR_RankIDCampaign.Cast(GetRankByID(rankID));
		
		if (!rank)
			return int.MAX;
			
		return rank.GetRankXP();
	}
	
	//------------------------------------------------------------------------------------------------
	int GetRankRequestCooldown(ECharacterRank rankID)
	{
		SCR_RankIDCampaign rank = SCR_RankIDCampaign.Cast(GetRankByID(rankID));
		
		if (!rank)
			return int.MAX;
		
		return rank.GetRankRequestCooldown();
	}
	
	//------------------------------------------------------------------------------------------------
	int GetRankRadioRespawnCooldown(ECharacterRank rankID)
	{
		SCR_RankIDCampaign rank = SCR_RankIDCampaign.Cast(GetRankByID(rankID));
		
		if (!rank)
			return int.MAX;
		
		return rank.GetRankRadioRespawnCooldown();
	}
	
	//------------------------------------------------------------------------------------------------
	ECharacterRank GetRankByXP(int XP)
	{
		if (!m_aRanks)
			return ECharacterRank.INVALID;
		
		int maxFoundXP = -100000;
		ECharacterRank rankFound = GetRenegadeRank();
		
		foreach (SCR_RankID rank: m_aRanks)
		{
			int reqXP = GetRankXP(rank.GetRankID());
			
			if (reqXP <= XP && reqXP > maxFoundXP)
			{
				maxFoundXP = reqXP;
				rankFound = rank.GetRankID();
			}
		}
		
		return rankFound;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the next higher rank
	ECharacterRank GetRankNext(ECharacterRank rank)
	{
		int rankXP = GetRankXP(rank);
		int higherXP = 99999;
		ECharacterRank foundID = ECharacterRank.INVALID;
		
		foreach (SCR_RankID r: m_aRanks)
		{
			if (!r)
				continue;
			
			ECharacterRank ID = r.GetRankID();
			int thisXP = GetRankXP(ID);
			
			if (thisXP > rankXP && thisXP < higherXP)
			{
				higherXP = thisXP;
				foundID = ID;
			}
		}
		
		return foundID;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the next lower rank
	ECharacterRank GetRankPrev(ECharacterRank rank)
	{
		int rankXP = GetRankXP(rank);
		int lowerXP = -99999;
		ECharacterRank foundID = ECharacterRank.INVALID;
		
		foreach (SCR_RankID r: m_aRanks)
		{
			if (!r)
				continue;
			
			ECharacterRank ID = r.GetRankID();
			int thisXP = GetRankXP(ID);
			
			if (thisXP < rankXP && thisXP > lowerXP)
			{
				lowerXP = thisXP;
				foundID = ID;
			}
		}
		
		return foundID;
	}
	//------------------------------------------------------------------------------------------------
	void SCR_CampaignFactionManager(IEntitySource src, IEntity parent)
	{
		if (SCR_GameModeCampaignMP.NotPlaying())
			return;
	}
};
