[WorkbenchToolAttribute(
	name: "Window Frame/Sash/Glass Generator Tool",
	description:
		"- Fills sashes (window panel) with found glass panes\n"
	+	"- soon™: Fills window frames with sashes\n",
	awesomeFontCode: 0xF2D2)]
class WindowFrameSashGlassGeneratorTool : WorldEditorTool
{
	[Attribute("1", UIWidgets.ComboBox, "File system where new prefabs will be created", "", ParamEnumAddons.FromEnum(), "General")]
	protected int m_iModToUse;

	[Attribute("1", desc: "Remove sub-entities from the processed prefabs before anything", category: "General")]
	protected bool m_bRemoveExistingChildren;

	[Attribute("_WFSG", desc: "If empty, overrides the provided prefabs", category: "General")]
	protected string m_sSuffix;

	protected WorldEditor m_WorldEditor;
	protected ResourceManager m_ResourceManager;
	protected static ref array<ResourceName> s_aPrefabs;		// static due to search callback - prefabPaths (with GUID)
	protected static ref map<string, string> s_mGlassPrefabs;	// static due to search callback - filename/prefabPath (with GUID)

	protected static string HIERARCHY_COMPONENT_CLASSNAME = "Hierarchy";

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Generate Glass panes for selected prefabs")]
	void Execute()
	{
		m_WorldEditor = Workbench.GetModule(WorldEditor);
		m_ResourceManager = Workbench.GetModule(ResourceManager);

		s_aPrefabs = {};
		m_WorldEditor.GetResourceBrowserSelection(PrefabSelectionCallback, true);

		if (s_aPrefabs.IsEmpty())
		{
			Print("Nothing selected - quitting", LogLevel.NORMAL);
			return;
		}

		Print("Processing " + s_aPrefabs.Count() + " selected prefabs", LogLevel.NORMAL);
		foreach (ResourceName resourceName : s_aPrefabs)
		{
			Print("- " + resourceName, LogLevel.DEBUG);
		}

		// mod prefix
		string modPrefix = SCR_AddonTool.ToFileSystem(SCR_AddonTool.GetAddonIndex(m_iModToUse));

		Debug.BeginTimeMeasure();
		FillGlasses(); // cheers
		Debug.EndTimeMeasure("Filling glasses");

		// START
		m_API.BeginEntityAction();
		Debug.BeginTimeMeasure();

		foreach (ResourceName prefabPath : s_aPrefabs)
		{
			if (prefabPath.IsEmpty())
				continue;

			string prefabRelativePath = prefabPath.GetPath();
			string prefabDir = FilePath.StripFileName(prefabRelativePath); // keeps the trailing slash
			string prefabFileName = FilePath.StripPath(prefabPath);

			IEntity entity = m_API.CreateEntity(prefabPath, string.Empty, m_API.GetCurrentEntityLayerId(), null, vector.Zero, vector.Zero);
			IEntitySource entitySource = m_API.EntityToSource(entity);
			array<string> boneNames = {};
			entity.GetBoneNames(boneNames);

			if (m_bRemoveExistingChildren)
				entitySource = RemoveExistingChildren(entitySource);

			if (boneNames.IsEmpty())
			{
				Print("Skipping " + prefabPath + " - no bones found", LogLevel.NORMAL);
				continue;
			}

			Print("Processing " + prefabPath, LogLevel.NORMAL);
			int createdGlasses;
			int bones;
			boneNames.Sort();
			foreach (string boneName : boneNames)
			{
				if (CreateGlass(entitySource, boneName))
					createdGlasses++;
				bones++;
			}
			Print(createdGlasses.ToString() + " glasses created (" + bones + " bones)", LogLevel.DEBUG);

			string destination;
			if (m_sSuffix.IsEmpty())
				m_sSuffix = "_WFSG";
//				destination = modPrefix + prefabDir + prefabFileName;
//			else
				destination = modPrefix + prefabDir + FilePath.StripExtension(prefabFileName) + m_sSuffix + ".et";

			string absolutePath;
			Workbench.GetAbsolutePath(destination, absolutePath, false);

			if (absolutePath.IsEmpty())
				Print("Absolute path is empty, avoiding stupid error - (destination: " + destination + ")");
			else
				m_API.CreateEntityTemplate(entitySource, absolutePath);

			m_API.DeleteEntity(m_API.SourceToEntity(entitySource));
		}

		Debug.EndTimeMeasure("Processed " + s_aPrefabs.Count() + " prefabs");
		m_API.EndEntityAction();
		// FINISH
	}

	//------------------------------------------------------------------------------------------------
	protected static void PrefabSelectionCallback(ResourceName resName, string filePath = "")
	{
		string toLowerResName = resName;
		toLowerResName.ToLower();
		if (toLowerResName.EndsWith(".et") && toLowerResName.Contains("win_"))
			s_aPrefabs.Insert(resName);
	}

	//------------------------------------------------------------------------------------------------
	//! This method fills s_mGlassPrefabs with found glass prefabs to pick from it when needed
	protected void FillGlasses()
	{
		s_mGlassPrefabs = new map<string, string>();
		Workbench.SearchResources(GlassSearchCallback, { "et" }, { "glass", "x" });
	}

	//------------------------------------------------------------------------------------------------
	protected static void GlassSearchCallback(ResourceName resName, string filePath = "")
	{
		string toLowerFileNameWithoutExtension = FilePath.StripExtension(FilePath.StripPath(resName));
		toLowerFileNameWithoutExtension.ToLower();
		s_mGlassPrefabs.Insert(toLowerFileNameWithoutExtension, resName);
		Print("adding " + toLowerFileNameWithoutExtension + " - " + resName, LogLevel.DEBUG);
	}

	//------------------------------------------------------------------------------------------------
	protected string FindAdaptedGlassPrefab(string boneName)
	{
		if (boneName.IsEmpty())
			return string.Empty;

		boneName.ToLower();
		if (boneName == "scene_root")
		{
			Print("Skipping Scene_Root", LogLevel.DEBUG);
			return string.Empty;
		}

		foreach (string key, string value : s_mGlassPrefabs)
		{
			if (boneName.Contains(key))
			{
				Print("Found " + key + " for bone " + boneName, LogLevel.DEBUG);
				return value;
			}
		}

		Print("Did not find glass for bone " + boneName, LogLevel.DEBUG);
		return string.Empty;
	}

	//------------------------------------------------------------------------------------------------
	protected IEntity CreateGlass(notnull IEntitySource parent, string boneName)
	{
		if (boneName.IsEmpty())
			return null;

		string glassPrefab = FindAdaptedGlassPrefab(boneName);
		if (glassPrefab.IsEmpty())
			return null;

		IEntity result = m_API.CreateEntity(glassPrefab, string.Empty, m_API.GetCurrentEntityLayerId(), parent, vector.Zero, vector.Up);
		if (!result)
			return null;

		IEntitySource resultSource = m_API.EntityToSource(result);
		if (!resultSource)
		{
			delete result;
			return null;
		}

		IEntityComponentSource hierarchyComponent;
		for (int i = 0, count = resultSource.GetComponentCount(); i < count; i++)
		{
			if (resultSource.GetComponent(i).GetClassName() == HIERARCHY_COMPONENT_CLASSNAME)
			{
				hierarchyComponent = resultSource.GetComponent(i);
				break;
			}
		}

		if (!hierarchyComponent)
		{
			// do not create the component, directly throw an error
			Print("No Hierarchy component - fix by adding a Hierarchy component to " + resultSource + " first", LogLevel.WARNING);
			delete result;
			return null;
		}

		m_API.SetVariableValue(resultSource, { new ContainerIdPathEntry(HIERARCHY_COMPONENT_CLASSNAME) }, "PivotID", boneName);

		return m_API.SourceToEntity(resultSource);
	}

	//------------------------------------------------------------------------------------------------
	protected IEntitySource RemoveExistingChildren(notnull IEntitySource entitySource)
	{
		IEntity entity = m_API.SourceToEntity(entitySource);

		IEntity child = entity.GetChildren();
		IEntity sibling;

		while (child)
		{
			sibling = child.GetSibling();
			delete child;
			child = sibling;
		}

		return m_API.EntityToSource(entity);
	}
};
