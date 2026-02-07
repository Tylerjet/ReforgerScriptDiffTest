#ifdef WORKBENCH
[WorkbenchPluginAttribute(
	name: "World Prefabs Statistics",
	description: "Get Prefabs usage statistics in the currently-opened world",
	wbModules: { "WorldEditor" },
	awesomeFontCode: 0xF1FE)] // terrain-looking chart!
class SCR_PrefabStatisticsPlugin : WorkbenchPlugin
{
	[Attribute(defvalue: "0", desc: "Output results to a text file, otherwise Log Console", params: "0 inf")]
	protected bool m_bOutputToFile;

	[Attribute(defvalue: "0", desc: "Top X values to display\n0 = display everything", params: "0 inf")]
	protected int m_iMaxDisplayedEntries;

	[Attribute(defvalue: "0", desc: "Value under which entity counts are not listed", params: "0 inf")]
	protected int m_iDisplayThreshold;

	//! - 2 = all references in all layer/Prefab/conf files
	//! - 1 = analyse the current world
	//! - 0 = cancel
	protected float m_fMode;

	protected static const string OUTPUT_FILE_NAME = "PrefabStatistics.txt";

	//------------------------------------------------------------------------------------------------
	protected override void Run()
	{
		// if (!Workbench.ScriptDialog("", "Pick an option:\n- Scan ALL files: EXPENSIVE (and not implemented)\n- Scan world: analyse the currently loaded world (acceptable CPU load)\n- Close: does nothing (not expensive)", this))
		if (!Workbench.ScriptDialog("", "Get word entities statistics (or close the window)", this))
			return;

//		m_fMode = 1;

		if (m_fMode < 1)
			return;

		if (m_fMode == 2)
		{
			Debug.BeginTimeMeasure();
			ScanFiles();
			Debug.EndTimeMeasure("Files scan");
		}
		else // m_fMode == 1 being default
		{
			Debug.BeginTimeMeasure();
			ScanWorld();
			Debug.EndTimeMeasure("World scan");
		}
	}

	// TODO?
	protected void ScanFiles()
	{
		Workbench.Dialog("TODO?", "Not implemented, I said.");
	}

	//------------------------------------------------------------------------------------------------
	protected void ScanWorld()
	{
		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();
		if (!worldEditorAPI)
		{
			Print("World Editor API is not available", LogLevel.ERROR);
			return;
		}

		string worldPath;
		worldEditorAPI.GetWorldPath(worldPath);
		if (!worldPath) // .IsEmpty()
		{
			Workbench.Dialog("", "Be sure to load a world before using the Prefab Statistics plugin.");
			return;
		}

		BaseWorld world = worldEditorAPI.GetWorld();
		if (!world) // wat
		{
			Workbench.Dialog("", "Be sure to load a world before using the Prefab Statistics plugin.");
			return;
		}

		vector mins, maxes;
		world.GetBoundBox(mins, maxes);
		if (mins == maxes)
		{
			PrintFormat("World boundaries are invalid:\n%1\n%2", mins, maxes, level: LogLevel.ERROR);
			return;
		}

		map<ResourceName, int> prefabResult = new map<ResourceName, int>();
		map<ResourceName, int> prefabNoAncestorResult = new map<ResourceName, int>();
		map<ResourceName, int> emptyResult = new map<ResourceName, int>();
		map<ResourceName, int> genericResult = new map<ResourceName, int>();
		IEntitySource entitySource;
		IEntitySource ancestor;
		int noSourceFound;

		for (int i = worldEditorAPI.GetEditorEntityCount() - 1; i >= 0; --i)
		{
			entitySource = worldEditorAPI.GetEditorEntity(i);
			if (!entitySource) // wat
			{
				++noSourceFound;
				continue;
			}

			ancestor = entitySource.GetAncestor();
			if (ancestor)														// a Prefab
			{
				ResourceName resourceName = ancestor.GetResourceName();
				if (resourceName) // !.IsEmpty()
				{
					prefabResult.Set(resourceName, prefabResult.Get(resourceName) + 1);
				}
				else															// a Prefab without a name...
				{
					resourceName = entitySource.GetResourceName();
					if (resourceName) // !.IsEmpty()							// a Prefab with Prefab set on entity itself?
					{
						prefabNoAncestorResult.Set(resourceName, prefabNoAncestorResult.Get(resourceName) + 1);
					}
					else														// a Prefab without a Prefab path
					{
						string className = ancestor.GetClassName();
						emptyResult.Set(className, emptyResult.Get(className) + 1);
					}
				}
			}
			else																// a raw entity (spline, GenericEntity, etc)
			{
				string className = entitySource.GetClassName();
				genericResult.Set(className, genericResult.Get(className) + 1);
			}
		}

		if (m_bOutputToFile)
		{
			if (FileIO.FileExists(OUTPUT_FILE_NAME))
				FileIO.DeleteFile(OUTPUT_FILE_NAME);
		}

		OutputResult(prefabResult, "Prefab entities");
		OutputResult(prefabNoAncestorResult, "entities with Prefab path on EntitySource");
		OutputResult(emptyResult, "entities with empty Prefab path");
		OutputResult(genericResult, "generic entities");

		if (m_bOutputToFile)
		{
			string absPath;
			if (Workbench.GetAbsolutePath(OUTPUT_FILE_NAME, absPath, true))
				Workbench.RunCmd("notepad \"" + absPath + "\"");
		}

		if (noSourceFound > 0)
			PrintFormat("%1 entities do NOT have an Entity Source!", noSourceFound, level: LogLevel.WARNING);
	}

	//------------------------------------------------------------------------------------------------
	//! Print statistics results in log console
	//! \param[in] result
	//! \param[in] string description
	protected void OutputResult(notnull map<ResourceName, int> result, string description)
	{
		array<string> resultLines = GetResultLines(result, description);

		if (m_bOutputToFile)	// write to txt
		{
			if (resultLines.IsEmpty())
			{
				if (FileIO.FileExists(OUTPUT_FILE_NAME))
					FileIO.DeleteFile(OUTPUT_FILE_NAME);

				Print("Export failed: file would have been empty", LogLevel.WARNING);
			}
			else
			{
				if (SCR_FileIOHelper.AppendFileContent(OUTPUT_FILE_NAME, resultLines))
				{
					Print("Successfully wrote " + description + " to file", LogLevel.NORMAL);
				}
				else
				{
					Print("Export failed: cannot write to file - printing instead", LogLevel.WARNING);
					foreach (string line : resultLines)
					{
						Print("" + line, LogLevel.NORMAL);
					}
				}
			}
		}
		else					// print to log console
		{
			foreach (string line : resultLines)
			{
				Print("" + line, LogLevel.NORMAL);
			}

			Print("\n", LogLevel.NORMAL);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Print statistics results in log console
	//! \param[in] result
	//! \param[in] description
	protected array<string> GetResultLines(notnull map<ResourceName, int> result, string description)
	{
		int count = result.Count();
		if (count < 1)
			return { "No " + description + " found" };

		array<string> resultLines = {};

		array<int> values = {};
		values.Reserve(count);
		int entitiesCount;

		// standard entities

		foreach (string key, int value : result)
		{
			values.Insert(value);
			entitiesCount += value;
		}

		resultLines.Insert(string.Format("Found and processed %1 %2", entitiesCount, description)); // no ':' colon as the following lines can be hidden by filters

		values.Sort(true); // DESC
		if (m_iMaxDisplayedEntries > 0)
			values.Resize(m_iMaxDisplayedEntries);

		foreach (int value : values)
		{
			if (value < m_iDisplayThreshold)
				break;

			ResourceName key = result.GetKeyByValue(value);
			if (key.IsEmpty())
				//continue;
				Print("empty key found", LogLevel.WARNING);

			result.Remove(key); // fix duplicates
			resultLines.Insert(string.Format("%1× %2", value, key));
		}

		return resultLines;
	}

//	//------------------------------------------------------------------------------------------------
//	[ButtonAttribute("Scan ALL files")]
//	protected int ButtonFiles()
//	{
//		m_fMode = 2;
//
//		return m_fMode;
//	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Analyse world", true)]
	protected int ButtonWorld()
	{
		m_fMode = 1;

		return m_fMode;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Close")]
	protected int ButtonClose()
	{
		m_fMode = 0;

		return m_fMode;
	}
}
#endif
