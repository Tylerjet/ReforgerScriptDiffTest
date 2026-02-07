class ParamEnumAddons : array<ref ParamEnum>
{
	//------------------------------------------------------------------------------------------------
	static ParamEnumArray FromEnum()
	{
		ParamEnumArray params = new ParamEnumArray();
		array<string> addons = {};
		GameProject.GetLoadedAddons(addons);
		int cnt = addons.Count();

		for (int i = 0; i < cnt; i++)
		{
			params.Insert(new ParamEnum(GameProject.GetAddonID(addons[i]), i.ToString()));
		}

		return params;
	}
};

class SCR_AddonTool
{
	static const autoptr array<string> IGNORED_ADDONS = { "core", "ArmaReforger" };

	//------------------------------------------------------------------------------------------------
	/*!
	Returns an array of addons where given resource is present or modified.
	\param prefab Prefab path
	\param ignoreCoreAddons if true then ArmaReforger and Core are ignored
	\return Aray of addon ID strings
	*/
	static array<string> GetResourceAddons(ResourceName prefab, bool ignoreCoreAddons = false)
	{
		array<string> addonNames = {};

		if (prefab.IsEmpty())
			return addonNames;

		Resource prefabResource = BaseContainerTools.LoadContainer(prefab);
		if (!prefabResource)
			return addonNames;

		BaseResourceObject configContainer;
		configContainer = prefabResource.GetResource();
		if (!configContainer)
			return addonNames;

		BaseContainer configBase = configContainer.ToBaseContainer();
		if (!configBase)
			return addonNames;

		configBase.GetSourceAddons(addonNames);

		if (ignoreCoreAddons && !addonNames.IsEmpty())
		{
			foreach (string item : IGNORED_ADDONS)
			{
				addonNames.RemoveItem(item);
			}
		}

		return addonNames;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Returns last addon where given resource is present.
	\param prefab Prefab path
	\return Class name
	*/
	static string GetResourceLastAddon(ResourceName prefab)
	{
		array<string> addonNames = GetResourceAddons(prefab);
		if (addonNames.IsEmpty())
			return string.Empty;

		return addonNames[addonNames.Count() - 1];
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get addon name by providing index of the addon.
	Can be used in tandem with ParamEnumAddons.
	\param index Index number
	\return Addon ID
	*/
	static string GetAddonIndex(int index)
	{
		array<string> addons = {};
		GameProject.GetLoadedAddons(addons);

		return GameProject.GetAddonID(addons[index]);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Convert addon name to file system format.
	For instance, "ArmaReforger" will get converted to "$ArmaReforger:".
	\param addon Addon ID
	\return Class name
	*/
	static string ToFileSystem(string addon)
	{
		return "$" + addon + ":";
	}
};
