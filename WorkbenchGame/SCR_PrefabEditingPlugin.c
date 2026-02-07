#ifdef WORKBENCH
[WorkbenchPluginAttribute(name: "Edit Selected Prefab(s)", shortcut: "Ctrl+Shift+E", wbModules: { "WorldEditor" }, awesomeFontCode: 0xF1B2)]
class SCR_PrefabEditingPlugin : SCR_PrefabEditingPluginBase
{
	[Attribute(defvalue: "{F636AAB9EE015E5F}Configs/Workbench/PrefabEditingPlugin/PrefabEditingPluginConfig.conf", params: "conf", desc: "Config with rules defining which worlds will be used for which folders.")]
	protected ResourceName m_Config;

	//------------------------------------------------------------------------------------------------
	protected string GetFileName(ResourceName prefab)
	{
		string name = FilePath.StripPath(prefab);
		return FilePath.StripExtension(name);
	}

	//------------------------------------------------------------------------------------------------
	protected IEntitySource CreateEntity(WorldEditorAPI api, ResourceName prefab, vector position, vector rotation)
	{
		string extension;
		FilePath.StripExtension(prefab, extension);
		if (extension == "ct")
		{
			IEntitySource entity = api.CreateEntity("GenericEntity", GetFileName(prefab), api.GetCurrentEntityLayerId(), null, position, rotation);
			api.CreateComponent(entity, prefab);
			return entity;
		}
		else
		{
			return api.CreateEntity(prefab, GetFileName(prefab), api.GetCurrentEntityLayerId(), null, position, rotation);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool CreateWorldFiles(ResourceName worldPrefab, string targetPath)
	{
		//--- ToDo: Use more legit way to create the world?
		string worldPath = worldPrefab.GetPath();

		if (!FileIO.FileExists(worldPath))
		{
			Print(string.Format("Cannot load prefab, selected world '%2' doesn't exist!", worldPath), LogLevel.ERROR);
			return false;
		}

		FileIO.MakeDirectory("$profile:worlds");

		//--- World file
		FileHandle file = FileIO.OpenFile(targetPath + ".ent", FileMode.WRITE);
		file.WriteLine("SubScene {");
		file.WriteLine(string.Format(" Parent \"%1\"", worldPrefab));
		file.WriteLine("}");
		file.WriteLine("Layer default {");
		file.WriteLine(" Index 0");
		file.WriteLine("}");
		file.Close();

		//--- Layer file
		file = FileIO.OpenFile(targetPath + "_Layers/default.layer", FileMode.WRITE);
		file.Close();

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void SetCamera(WorldEditorAPI api, IEntity entity)
	{
		IEntitySource source = api.EntityToSource(entity);
		vector boundsMin = Vector(float.MAX, float.MAX, float.MAX);
		vector boundsMax = vector.Zero;
		GetEntitySourceBounds(api, source, boundsMin, boundsMax);

		vector size = boundsMax - boundsMin;
		float distance = Math.Max(size.Length() * 1.5, 1);
		if (distance > 100)
		{
			size = vector.Zero;
			distance = 100;
		}

		vector rotation = Vector(-135, -30, 0).AnglesToVector();
		vector pivot = entity.GetOrigin() + Vector(0, size[1] / 2, 0);
		api.SetCamera(pivot - rotation * distance, rotation);
	}

	//------------------------------------------------------------------------------------------------
	protected void GetEntitySourceBounds(WorldEditorAPI api, IEntitySource source, out vector boundsMin, out vector boundsMax)
	{
		IEntity entity = api.SourceToEntity(source);
		if (!entity)
			return;

		if (entity.GetVObject())
		{
			vector entityMin, entityMax;
			entity.GetBounds(entityMin, entityMax);
			entityMin = entity.CoordToParent(entityMin);
			entityMax = entity.CoordToParent(entityMax);

			boundsMin[0] = Math.Min(boundsMin[0], entityMin[0]);
			boundsMin[1] = Math.Min(boundsMin[1], entityMin[1]);
			boundsMin[2] = Math.Min(boundsMin[2], entityMin[2]);

			boundsMax[0] = Math.Max(boundsMax[0], entityMax[0]);
			boundsMax[1] = Math.Max(boundsMax[1], entityMax[1]);
			boundsMax[2] = Math.Max(boundsMax[2], entityMax[2]);
		}

		for (int i, childrenCount = source.GetNumChildren(); i < childrenCount; i++)
		{
			GetEntitySourceBounds(api, source.GetChild(i), boundsMin, boundsMax);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected PrefabEditingPluginConfig GetConfig(ResourceName configPath)
	{
		Resource configResource = Resource.Load(configPath);
		if (!configResource.IsValid())
		{
			Print(string.Format("Cannot load config '%1'!", configPath), LogLevel.ERROR);
			return null;
		}

		BaseResourceObject configContainer = configResource.GetResource();
		if (!configContainer)
			return null;

		BaseContainer configBase = configContainer.ToBaseContainer();
		if (!configBase)
			return null;

		if (configBase.GetClassName() != "PrefabEditingPluginConfig")
		{
			Print(string.Format("Config '%1' is of type '%2', must be 'PrefabEditingPluginConfig'!", configPath, configBase.GetClassName()), LogLevel.ERROR);
			return null;
		}

		return PrefabEditingPluginConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(configBase));
	}

	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		if (!SCR_Global.IsEditMode() || !Workbench.OpenModule(WorldEditor))
			return;

		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		if (!worldEditor)
			return;

		WorldEditorAPI api = worldEditor.GetApi();
		if (!api)
			return;

		array<ResourceName> compatiblePrefabs = {};
		if (!GetPrefabs(compatiblePrefabs, true))
			return;

		//--- Load config
		PrefabEditingPluginConfig config = GetConfig(m_Config);
		if (!config)
			return;

		PrefabEditingPluginConfigFolder folderSettings = config.GetFolderSettings(compatiblePrefabs[0]);
		if (!folderSettings)
			return;

		//--- Create world
		string targetPath = config.GetPath();
		if (!CreateWorldFiles(folderSettings.GetWorld(), targetPath))
			return;

		if (!worldEditor.SetOpenedResource(targetPath + ".ent"))
		{
			Print(string.Format("Cannot load world '%1'!", folderSettings.GetWorld()), LogLevel.ERROR);
			return;
		}

		//--- Create entities
		api.BeginEntityAction();
		api.ClearEntitySelection();
		array<IEntitySource> entities = {};

		vector pos, rot, size, boundsMin, boundsMax, boundsSize;
		pos = folderSettings.GetPosition();
		rot = folderSettings.GetRotation();
		size = vector.Zero;
		bool usePrefabPosition = folderSettings.UsePrefabPosition();

		Resource resource;
		IEntitySource source;
		foreach (int i, ResourceName prefab : compatiblePrefabs)
		{
			resource = Resource.Load(prefab);
			if (!resource.IsValid())
			{
				Print(string.Format("Cannot load prefab '%1'!", prefab), LogLevel.ERROR);
				api.EndEntityAction();
				return;
			}

			if (usePrefabPosition)
			{
				source = SCR_BaseContainerTools.FindEntitySource(resource);
				source.Get("coords", pos);
				rot = vector.Zero;
			}

			source = CreateEntity(api, prefab, pos, rot); //--- ToDo: Smarter placement of multiple entities?
			if (!source)
			{
				Print(string.Format("Cannot create entity for prefab '%1'!", prefab), LogLevel.ERROR);
				api.EndEntityAction();
				return;
			}

			entities.Insert(source);

			//--- Get bounding box
			boundsMin = Vector(float.MAX, float.MAX, float.MAX);
			boundsMax = -Vector(float.MAX, float.MAX, float.MAX);
			GetEntitySourceBounds(api, source, boundsMin, boundsMax);
			boundsSize = boundsMax - boundsMin;
			for (int n = 0; n < 3; n++)
			{
				if (size[n] < boundsSize[n])
					size[n] = boundsSize[n];
			}
		}

		float spacing = folderSettings.GetSpacing();
		size += { spacing, spacing, spacing };

		//--- Select entities (start from the back, because the last one has transformation gizmo on it)
		int entitiesCount = entities.Count();
		int length = Math.Ceil(Math.Sqrt(entitiesCount));
		int row = 0;
		int column = 0;
		for (int i = entitiesCount; i--; i >= 0)
		{
			row = Math.Floor(i / length);
			column = i % length;
			IEntitySource e = entities[i];
			api.AddToEntitySelection(e);
			if (!usePrefabPosition)
				api.SetVariableValue(e, null, "coords", string.Format("%1 %2 %3", pos[0] - size[0] * column, pos[1], pos[2] - size[2] * row));
		}

		SetCamera(api, api.SourceToEntity(entities[0]));
		api.EndEntityAction();

		//--- Save to make sure that only user changes are marked
		worldEditor.Save();
	}

	//------------------------------------------------------------------------------------------------
	override void Configure()
	{
		Workbench.ScriptDialog("Configure 'Edit Selected Prefab(s)' plugin", "", this);
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Close")]
	protected bool ButtonClose()
	{
	}
}

[WorkbenchPluginAttribute(name: "Edit Selected Prefab(s)", shortcut: "Ctrl+Shift+E", wbModules: { "ResourceManager" }, category: "Prefabs", awesomeFontCode: 0xF1B2)]
class SCR_PrefabEditingPluginResourceManager : SCR_PrefabEditingPlugin
{
	//------------------------------------------------------------------------------------------------
	override void GetSelected(out array<ResourceName> selection)
	{
		GetSelectedResourceBrowser(selection);
	}
}

[BaseContainerProps(configRoot: true)]
class PrefabEditingPluginConfig
{
	[Attribute(defvalue: "$profile:worlds/PrefabEditingPlugin")]
	protected string m_sPath;

	[Attribute()]
	protected ref array<ref PrefabEditingPluginConfigFolder> m_Folders;

	//------------------------------------------------------------------------------------------------
	//! \return
	string GetPath()
	{
		return m_sPath;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] prefab
	//! \return
	PrefabEditingPluginConfigFolder GetFolderSettings(ResourceName prefab)
	{
		string prefabPath = prefab.GetPath();

		PrefabEditingPluginConfigFolder folderSettings = null;
		int pathLengthMax = -1;
		foreach (PrefabEditingPluginConfigFolder folderSettingsTemp : m_Folders)
		{
			string folderPath = folderSettingsTemp.GetFolder();
			int pathLength = folderPath.Length();
			if (pathLength > pathLengthMax && prefabPath.StartsWith(folderPath))
			{
				folderSettings = folderSettingsTemp;
				pathLengthMax = pathLength;
			}
		}

		return folderSettings;
	}
}

[BaseContainerProps(), SCR_BaseContainerCustomTitleResourceName("m_Folder")]
class PrefabEditingPluginConfigFolder
{
	[Attribute(params: "folders")]
	protected ResourceName m_Folder;

	[Attribute(defvalue: "{70ABDE49D02BA4F4}worlds/TestMaps/PrefabEditingPlugin/PrefabEditingPlugin.ent", uiwidget: UIWidgets.ResourceNamePicker, params: "ent")]
	protected ResourceName m_World;

	[Attribute(desc: "When enabled, prefabs will be used on their own coordinates.")]
	protected bool m_bUsePrefabPosition;

	[Attribute()]
	protected vector m_vPosition;

	[Attribute(desc: "Rotation pitch, yaw, roll")]
	protected vector m_vRotation;

	[Attribute(defvalue: "0.1", desc: "Minimal spacing [m] between spawned entities")]
	protected float m_fSpacing;

	//------------------------------------------------------------------------------------------------
	//! \return
	string GetFolder()
	{
		return m_Folder.GetPath();
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	ResourceName GetWorld()
	{
		return m_World;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	bool UsePrefabPosition()
	{
		return m_bUsePrefabPosition;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	vector GetPosition()
	{
		return m_vPosition;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	vector GetRotation()
	{
		return m_vRotation;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetSpacing()
	{
		return m_fSpacing;
	}
}
#endif // WORKBENCH
