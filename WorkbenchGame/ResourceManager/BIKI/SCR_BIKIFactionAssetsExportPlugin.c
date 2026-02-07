#ifdef WORKBENCH
//! see https://community.bistudio.com/wiki/Arma_Reforger:Assets
[WorkbenchPluginAttribute(name: "Export Assets", description: "Exports https:/" + "/community.bistudio.com/wiki/Arma_Reforger:Assets' content", wbModules: { "ResourceManager" }, category: "BIKI", awesomeFontCode: 0xF70E)]
class SCR_BIKIFactionAssetsExportPlugin : SCR_BIKIExportBasePlugin
{
	[Attribute(params: "conf class=SCR_Faction")]
	protected ref array<ResourceName> m_aFactionConfigs;

	[Attribute(uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EEntityCatalogType))]
	protected ref array<EEntityCatalogType> m_aCatalogTypes;

	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		if (!Workbench.ScriptDialog("Faction to export", "", this))
			return;

		if (!m_aFactionConfigs || m_aFactionConfigs.IsEmpty())
			m_aFactionConfigs = { "{5EB46557DF2AA24F}Configs/Factions/US.conf", "{09727032415AC39B}Configs/Factions/USSR.conf", "{8053DB656DD18B14}Configs/Factions/FIA.conf" };

		if (!m_aCatalogTypes || m_aCatalogTypes.IsEmpty())
			m_aCatalogTypes = { EEntityCatalogType.CHARACTER, EEntityCatalogType.GROUP, EEntityCatalogType.VEHICLE, EEntityCatalogType.WEAPONS_TRIPOD };

		string result;
		foreach (ResourceName factionConfig : m_aFactionConfigs)
		{
			SCR_Faction faction = SCR_Faction.Cast(SCR_BaseContainerTools.CreateInstanceFromPrefab(factionConfig));

			result += GetFactionString(faction, m_aCatalogTypes);
		}

		result.TrimInPlace();

		result += "\n\n{{GameCategory|armaR|Content}}";

		ShowResult(result);
	}

	//------------------------------------------------------------------------------------------------
	protected string GetFactionString(notnull SCR_Faction faction, array<EEntityCatalogType> catalogTypes)
	{
		SCR_BIKIStringBuilder stringBuilder = new SCR_BIKIStringBuilder();

		stringBuilder.AddTitle(2, faction.GetFactionKey());

		array<SCR_EntityCatalogEntry> entityList = {};
		array<string> pieces = {};
		foreach (EEntityCatalogType catalogType : catalogTypes)
		{
			SCR_EntityCatalog catalog = SCR_Faction_BIKI.GetEntityCatalog(faction, catalogType);
			if (!catalog)
				continue;

			stringBuilder.AddTitle(3, SCR_StringHelper.FormatSnakeCaseToUserFriendly(typename.EnumToString(EEntityCatalogType, catalogType)));
			stringBuilder.BeginTable("min-width: 60em");
			stringBuilder.AddTableHeader("Name", "min-width: 50%");
			stringBuilder.AddTableHeader("Prefab");

			catalog.GetEntityList(entityList);

			foreach (SCR_EntityCatalogEntry entry : entityList)
			{
				ResourceName prefab = entry.GetPrefab();
				stringBuilder.AddTableDataCellsLine(SCR_StringHelper.FormatResourceNameToUserFriendly(prefab), "{{Link|enfusion:/" + "/ResourceManager/~ArmaReforger:" + prefab.GetPath() + "}}");
			}

			stringBuilder.EndTable();
		}

		return stringBuilder.GetContent();
	}
}

class SCR_Faction_BIKI : SCR_Faction
{
	//------------------------------------------------------------------------------------------------
	//! \param[in] faction
	//! \param[in] catalogType
	//! \return
	static SCR_EntityCatalog GetEntityCatalog(notnull SCR_Faction faction, EEntityCatalogType catalogType)
	{
		foreach (SCR_EntityCatalog catalog : faction.m_aEntityCatalogs)
		{
			if (catalog.GetCatalogType() == catalogType)
				return catalog;
		}

		return null;
	}
}
#endif // WORKBENCH
