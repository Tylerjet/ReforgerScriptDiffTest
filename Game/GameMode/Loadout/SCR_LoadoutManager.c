[EntityEditorProps(category: "GameScripted/GameMode", description: "temp loadout selection.", color: "0 0 255 255")]
class SCR_LoadoutManagerClass: GenericEntityClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_LoadoutManager : GenericEntity
{
	static const int INVALID_LOADOUT_INDEX = -1;
	
	[Attribute("", UIWidgets.Object, category: "Loadout Manager")]
	ref array<ref SCR_BasePlayerLoadout> m_aPlayerLoadouts;
	
	//------------------------------------------------------------------------------------------------
	array<ref SCR_BasePlayerLoadout> GetPlayerLoadouts()
	{ 
		return m_aPlayerLoadouts;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
		Returns the number of loadouts provided by this manager or 0 if none.
	*/
	int GetLoadoutCount()
	{
		if (!m_aPlayerLoadouts)
			return 0;
		
		return m_aPlayerLoadouts.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
		Returns index of provided loadout or -1 if none.
	*/
	int GetLoadoutIndex(SCR_BasePlayerLoadout loadout)
	{
		int i = 0;
		foreach (auto inst : m_aPlayerLoadouts)
		{
			if (inst == loadout)
				return i;
			
			i++;
		}		
		return -1;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
		Returns loadout at provided index or null if none.
	*/
	SCR_BasePlayerLoadout GetLoadoutByIndex(int index)
	{
		if (index < 0 || index >= m_aPlayerLoadouts.Count())
			return null;
		
		return m_aPlayerLoadouts[index];
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
		Returns the first loadout with the provided name, or null if none were found.
	*/
	SCR_BasePlayerLoadout GetLoadoutByName(string name)
	{
		int count = m_aPlayerLoadouts.Count();
		for (int i = 0; i < count; i++)
		{
			SCR_BasePlayerLoadout candidate = m_aPlayerLoadouts[i];
			
			if (candidate.GetLoadoutName() == name)
				return candidate;
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns random loadout that belongs to provided faction or null if none.
	SCR_BasePlayerLoadout GetRandomFactionLoadout(Faction faction)
	{
		array<ref SCR_BasePlayerLoadout> loadouts = {};
		int count = GetPlayerLoadoutsByFaction(faction, loadouts);
		if (count == 0)
			return null;
		
		int randomIndex = Math.RandomInt(0, count);
		return loadouts[randomIndex];
	}
	
	//------------------------------------------------------------------------------------------------
	int GetPlayerLoadoutsByFaction(Faction faction, out notnull array<ref SCR_BasePlayerLoadout> outLoadouts)
	{
		outLoadouts.Clear();
		
		if (!m_aPlayerLoadouts)
			return 0;
		
		ArmaReforgerScripted game = GetGame();
		if (!game)
			return 0;
		
		FactionManager factionManager = game.GetFactionManager();
		if (!factionManager)
			return 0;
		
		int outCount = 0;
			
		int count = m_aPlayerLoadouts.Count();
		for (int i = 0; i < count; i++)
		{
			SCR_FactionPlayerLoadout factionLoadout = SCR_FactionPlayerLoadout.Cast(m_aPlayerLoadouts[i]);
			if (factionLoadout)
			{
				Faction ldFaction = factionManager.GetFactionByKey(factionLoadout.m_sAffiliatedFaction);
				if (faction == ldFaction)
				{
					outLoadouts.Insert(factionLoadout);
					outCount++;
				}
			}
		}
		
		return outCount;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetPlayerLoadouts(out notnull array<SCR_BasePlayerLoadout> outLoadouts)
	{
		int outCount = 0;
		outLoadouts.Clear();
		
		if (!m_aPlayerLoadouts)
			return 0;	
			
		int count = m_aPlayerLoadouts.Count();
		for (int i = 0; i < count; i++)
		{
			outLoadouts.Insert(m_aPlayerLoadouts[i]);
			outCount++;
		}
		
		return outCount;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetRandomLoadoutIndex(Faction faction)
	{
		if (!m_aPlayerLoadouts)
			return -1;	
			
		array<ref SCR_BasePlayerLoadout> loadouts = new array<ref SCR_BasePlayerLoadout>();
		int count = GetPlayerLoadoutsByFaction(faction, loadouts);
		if (count <= 0)
			return -1;
		
		int randomIndex = Math.RandomInt(0, count);
		return randomIndex;
	}

	
	//------------------------------------------------------------------------------------------------
	int GetRandomLoadoutIndex()
	{
		if (!m_aPlayerLoadouts)
			return -1;	
			
		int count = m_aPlayerLoadouts.Count();
		if (count <= 0)
			return -1;
		
		int randomIndex = Math.RandomInt(0, count);
		return randomIndex;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_BasePlayerLoadout GetRandomLoadout()
	{
		int randomIndex = GetRandomLoadoutIndex();
		if (randomIndex < 0)
			return null;
		return m_aPlayerLoadouts[randomIndex];
	}

	//------------------------------------------------------------------------------------------------
	void SCR_LoadoutManager(IEntitySource src, IEntity parent)
	{
		GetGame().RegisterLoadoutManager(this);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_LoadoutManager()
	{
		GetGame().UnregisterLoadoutManager(this);
	}
};