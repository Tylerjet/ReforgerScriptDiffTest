//------------------------------------------------------------------------------------------------
class SCR_FactionManagerClass: FactionManagerClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_FactionManager : FactionManager
{
	[Attribute(defvalue: "1", desc: "Whether  or not the isPlayable state of a faction can be changed on run time")]
	protected bool m_bCanChangeFactionsPlayable;
	
	protected ref map<string, ref array<string>> m_aAncestors = new map<string, ref array<string>>();
	
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

		ClearFlags(EntityFlags.ACTIVE, false);
	}

	//------------------------------------------------------------------------------------------------
	void SCR_FactionManager(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
		SetFlags(EntityFlags.ACTIVE, false);
		
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
};
