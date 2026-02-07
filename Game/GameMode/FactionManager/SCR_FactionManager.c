//------------------------------------------------------------------------------------------------
class SCR_FactionManagerClass: FactionManagerClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_FactionManager : FactionManager
{
	[Attribute(defvalue: "1", desc: "Whether  or not the isPlayable state of a faction can be changed on run time")]
	protected bool m_bCanChangeFactionsPlayable;
	
	[Attribute("", UIWidgets.Object, "List of rank types")]
	protected ref array<ref SCR_RankID> m_aRanks;
	
	protected ref SCR_SortedArray<SCR_Faction> m_SortedFactions = new SCR_SortedArray<SCR_Faction>();
	protected ref map<string, ref array<string>> m_aAncestors = new map<string, ref array<string>>();
	
	//------------------------------------------------------------------------------------------------
	/*
	Get factions sorted according to their own custom order.
	\param[out] outFactions Array to be filled with factions
	\return Number of factions
	*/
	int GetSortedFactionsList(out notnull SCR_SortedArray<SCR_Faction> outFactions)
	{
		return outFactions.CopyFrom(m_SortedFactions);
	}
	
	//------------------------------------------------------------------------------------------------
	protected SCR_RankID GetRankByID(SCR_ECharacterRank rankID)
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
	array<ref SCR_RankID> GetAllAvailableRanks()
	{
		array<ref SCR_RankID> outArray = {};
		foreach (SCR_RankID rank: m_aRanks)
		{
			outArray.Insert(rank);
		}
		
		return outArray;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsRankRenegade(SCR_ECharacterRank rankID)
	{
		SCR_RankID rank = GetRankByID(rankID);
		
		if (rank)
			return rank.IsRankRenegade();
		else
			return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected SCR_ECharacterRank GetRenegadeRank()
	{	
		foreach (SCR_RankID rank: m_aRanks)
		{
			if (rank && rank.IsRankRenegade())
				return rank.GetRankID();
		}
		
		return SCR_ECharacterRank.INVALID;
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		array<string> ancestors;
		array<Faction> factions = new array<Faction>();
		GetFactionsList(factions);
		for (int i = factions.Count() - 1; i >= 0; i--)
		{
			SCR_Faction scriptedFaction = SCR_Faction.Cast(factions[i]);
			if (scriptedFaction)
			{
				if (m_aAncestors.Find(scriptedFaction.GetFactionKey(), ancestors))
					scriptedFaction.SetAncestors(ancestors);
				
				m_SortedFactions.Insert(scriptedFaction.GetOrder(), scriptedFaction);
				
				scriptedFaction.InitializeFaction();
			}
		}
		m_aAncestors = null; //--- Don't keep in the memory anymore, stored on factions now
		
		//--- Initialize components (OnPostInit doesn't work in them)
		SCR_BaseFactionManagerComponent component;
		array<Managed> components = {};
		for (int i = 0, count = owner.FindComponents(SCR_BaseFactionManagerComponent, components); i < count; i++)
		{
			component = SCR_BaseFactionManagerComponent.Cast(components[i]);
			component.OnFactionsInit(factions);
		}	
	}

	//------------------------------------------------------------------------------------------------
	void SCR_FactionManager(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
		
		//--- Save faction ancestors
		BaseContainerList factionSources = src.GetObjectArray("Factions");
		BaseContainer factionSource;
		string factionKey, parentKey;
		for (int i, count = factionSources.Count(); i < count; i++)
		{
			factionSource = factionSources.Get(i);
			factionSource.Get("FactionKey", factionKey);
			
			array<string> ancestors = {};
			while (factionSource)
			{
				factionSource.Get("FactionKey", parentKey);
				if (!ancestors.Contains(parentKey))
					ancestors.Insert(parentKey);
				
				factionSource = factionSource.GetAncestor();
			}
			
			m_aAncestors.Insert(factionKey, ancestors);
		}
	}
	
	/*!
	Check if the faction is playable.
	Non-playable factions will not appear in the respawn menu.
	\return True when playable
	*/
	bool CanChangeFactionsPlayable()
	{
		return m_bCanChangeFactionsPlayable;
	}
	
	//======================================== FACTION RELATIONS ========================================\\	
	/*!
	Set given factions friendly towards eachother (Server Only)
	It is possible to set the same faction friendly towards itself to prevent faction infighting
	\param factionA Faction to set friendly to factionB
	\param factionB Faction to set friendly to factionA
	\param playerChanged id of player who changed it to show notification. Leave -1 to not show notification
	*/
	void SetFactionsFriendly(notnull SCR_Faction factionA, notnull SCR_Faction factionB, int playerChanged = -1)
	{
		//~ Already friendly
		if (factionA.DoCheckIfFactionFriendly(factionB))
			return;
		
		//~ Only call once if setting self as friendly
		if (factionA == factionB)
		{
			factionA.SetFactionFriendly(factionA);
			
			if (playerChanged > 0)
				SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_FACTION_SET_FRIENDLY_TO_SELF, playerChanged, GetFactionIndex(factionA));
			
			RequestUpdateAllTargetsFactions();
			return;
		}
		
		factionA.SetFactionFriendly(factionB);
		factionB.SetFactionFriendly(factionA);
		
		RequestUpdateAllTargetsFactions();
		
		if (playerChanged > 0)
			SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_FACTION_SET_FRIENDLY_TO, playerChanged, GetFactionIndex(factionA), GetFactionIndex(factionB));
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Set given factions hostile towards eachother (Server Only)
	It is possible to set the same faction hostile towards itself to allow faction infighting
	\param factionA Faction to set hostile to factionB
	\param factionB Faction to set hostile to factionA
	\param playerChanged id of player who changed it to show notification. Leave -1 to not show notification
	*/
	void SetFactionsHostile(notnull SCR_Faction factionA, notnull SCR_Faction factionB, int playerChanged = -1)
	{
		//~ Already Hostile
		if (!factionA.DoCheckIfFactionFriendly(factionB))
			return;
		
		//~ Only call once if setting self as hostile
		if (factionA == factionB)
		{
			factionA.SetFactionHostile(factionA);
			
			if (playerChanged > 0)
				SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_FACTION_SET_HOSTILE_TO_SELF, playerChanged, GetFactionIndex(factionA));
			
			RequestUpdateAllTargetsFactions();
			return;
		}
		
		factionA.SetFactionHostile(factionB);
		factionB.SetFactionHostile(factionA);
		
		RequestUpdateAllTargetsFactions();
		
		if (playerChanged > 0)
			SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_FACTION_SET_HOSTILE_TO, playerChanged, GetFactionIndex(factionA), GetFactionIndex(factionB));
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Update all AI perception. 
	Used when faction friendly is changed to make sure AI in the area attack or stop attacking each other
	*/
	static void RequestUpdateAllTargetsFactions()
	{
		PerceptionManager pm = GetGame().GetPerceptionManager();
		if (pm)
			pm.RequestUpdateAllTargetsFactions();
	}
};
