class SCR_EditableFactionStruct: JsonApiStruct
{
	protected string m_sFactionKey;
	protected ref array<ref SCR_EditorAttributeStruct> m_aAttributes = {};
	
	static void SerializeFactions(out notnull array<ref SCR_EditableFactionStruct> outEntries, SCR_EditorAttributeList attributeList)
	{
		SCR_DelegateFactionManagerComponent delegateManager = SCR_DelegateFactionManagerComponent.GetInstance();
		if (!delegateManager)
			return;
		
		//--- Clear existing array
		outEntries.Clear();
		
		SCR_SortedArray<SCR_EditableFactionComponent> delegates = new SCR_SortedArray<SCR_EditableFactionComponent>();
		for (int i, count = delegateManager.GetSortedFactionDelegates(delegates); i < count; i++)
		{
			SerializeFaction(delegates[i], outEntries, attributeList);
		}
	}
	protected static void SerializeFaction(SCR_EditableFactionComponent faction, out notnull array<ref SCR_EditableFactionStruct> outEntries, SCR_EditorAttributeList attributeList)
	{
		SCR_EditableFactionStruct entry = new SCR_EditableFactionStruct();
		entry.m_sFactionKey = faction.GetFaction().GetFactionKey();
		outEntries.Insert(entry);
		
		SCR_EditorAttributeStruct.SerializeAttributes(entry.m_aAttributes, attributeList, faction);
	}
	static void DeserializeFactions(notnull array<ref SCR_EditableFactionStruct> entries, SCR_EditorAttributeList attributeList = null)
	{
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return;
		
		SCR_DelegateFactionManagerComponent delegateManager = SCR_DelegateFactionManagerComponent.GetInstance();
		if (!delegateManager)
			return;
		
		Faction faction;
		SCR_EditableFactionComponent factionDelegate;
		foreach (int id, SCR_EditableFactionStruct entry: entries)
		{
			faction = factionManager.GetFactionByKey(entry.m_sFactionKey);
			factionDelegate = delegateManager.GetFactionDelegate(faction);
			if (factionDelegate)
			{
				SCR_EditorAttributeStruct.DeserializeAttributes(entry.m_aAttributes, attributeList, factionDelegate);
			}
			else
			{
				Print(string.Format("SCR_EditableFactionStruct: Cannot load faction '%1', it's not configured in FactionManager!", entry.m_sFactionKey), LogLevel.WARNING);
			}
		}
		
		SCR_SortedArray<SCR_EditableFactionComponent> delegates = new SCR_SortedArray<SCR_EditableFactionComponent>();
		for (int i, count = delegateManager.GetSortedFactionDelegates(delegates); i < count; i++)
		{
		}
	}
	static void LogFactions(notnull array<ref SCR_EditableFactionStruct> entries, SCR_EditorAttributeList attributeList = null)
	{
		Print("  SCR_EditableFactionStruct: " + entries.Count());
		foreach (int id, SCR_EditableFactionStruct entry: entries)
		{
			PrintFormat("    %1", entry.m_sFactionKey);
			
			SCR_EditorAttributeStruct.LogAttributes(entry.m_aAttributes, attributeList, "    ");
		}
	}
};