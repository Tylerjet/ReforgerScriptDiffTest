[WorkbenchPluginAttribute(name: "Create Images of Editable Entities", category: "In-game Editor", shortcut: "", wbModules: {"WorldEditor"}, awesomeFontCode: 0xf03e)]
class CreateEditableEntityImagesPlugin : PrefabEditingPluginBase
{
	//[Attribute(defvalue: "{90B4D95CF3610F3D}Configs/Workbench/EditablePrefabs/EditablePrefabsConfig.conf", desc: "", params: "conf")]
	//private ResourceName m_Config;

	//[Attribute("800")]
	int	m_iWidth = 800;

	//[Attribute("800")]
	int	m_iHeight = 800;

	[Attribute(defvalue: "{3229DBC2536F0E71}worlds/Editor/Test/EditableEntityImages.ent", desc: "World where entities will be created.")]
	private ResourceName m_SourceWorldPath;

	protected ref array<IEntity> m_Entities;

	protected bool ValidateScreen(WorldEditorAPI api)
	{
		//--- Verify screen dimensions (ToDo: Force them from script)
		float screenW = api.GetScreenWidth();
		float screenH = api.GetScreenHeight();

		if (screenW == m_iWidth && screenH == m_iHeight)
			return true;

		Print(string.Format("Cannot generate images, workspace window must be %1x%2, is %3x%4! Please resize the viewport.", m_iWidth, m_iHeight, screenW, screenH), LogLevel.WARNING);
		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected bool ValidateExtension()
	{

		string testFilePath = "CreateEditableEntityImagesPlugin";
		System.MakeScreenshot("$ArmaReforger:" + testFilePath);

		Sleep(1);
		bool exists = FileIO.FileExist(FilePath.AppendExtension(testFilePath, "png"));
		FileIO.DeleteFile(FilePath.AppendExtension(testFilePath, "png"));
		FileIO.DeleteFile(FilePath.AppendExtension(testFilePath, "bmp"));

		if (exists)
			return true;

		Print(string.Format("Cannot generate images of editable entities, please set 'Game > Compressed format in WB' to 'true'"), LogLevel.WARNING);
		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected bool ValidatePrefabs(out notnull map<ResourceName, string> outPrefabs)
	{
		//--- Get selected prefabs
		array<ResourceName> selectedPrefabs = {};
		if (!GetPrefabs(selectedPrefabs))
			return false;

		//--- Identify compatible prefabs
		Resource prefabResource;
		IEntityComponentSource componentSource;
		ResourceName imagePath;
		SCR_EditableEntityUIInfo info;
		foreach (ResourceName prefab : selectedPrefabs)
		{
			prefabResource = Resource.Load(prefab);
			if (!prefabResource || !prefabResource.IsValid())
				continue;

			componentSource = SCR_EditableEntityComponentClass.GetEditableEntitySource(prefabResource);
			if (!componentSource)
			{
				Print(string.Format("Cannot generate image, prefab '%1' does not contain SCR_EditableEntityComponent!", prefab.GetPath()), LogLevel.WARNING);
				continue;
			}

			info = SCR_EditableEntityUIInfo.Cast(SCR_EditableEntityComponentClass.GetInfo(componentSource));
			if (!info)
			{
				Print(string.Format("Cannot generate image, prefab '%1' does not have UI info defined in SCR_EditableEntityComponent!", prefab.GetPath()), LogLevel.WARNING);
				continue;
			}

			imagePath = info.GetImage();
			if (imagePath.IsEmpty())
			{
				Print(string.Format("Cannot generate image, prefab '%1' does not have image path defined in UI info of SCR_EditableEntityComponent!", prefab.GetPath()), LogLevel.WARNING);
				continue;
			}

			outPrefabs.Insert(prefab, FilePath.StripExtension(imagePath.GetPath()));
			Print(string.Format("Prefab '%1' marked for image auto-generation.", prefab.GetPath()), LogLevel.DEBUG);
		}

		if (!outPrefabs.IsEmpty())
			return true;

		Print("Cannot generate images, no compatible prefabs found!", LogLevel.WARNING);
		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected void CaptureImages(WorldEditorAPI api, map<ResourceName, string> prefabs)
	{
		ArmaReforgerScripted game = GetGame();
		BaseWorld world = game.GetWorld();

		IEntity player = SCR_PlayerController.GetLocalControlledEntity();
		if (player)
			player.ClearFlags(EntityFlags.VISIBLE, true);

		SCR_CameraBase camera = SCR_CameraBase.Cast(GetGame().SpawnEntity(SCR_CameraBase, world));
		CameraManager cameraManager = game.GetCameraManager();
		cameraManager.SetCamera(camera);

		EntitySpawnParams spawnParams = new EntitySpawnParams;
		Math3D.MatrixIdentity3(spawnParams.Transform);
		spawnParams.Transform[3] = Vector(64, 0, 64);

		Sleep(1000); //--- Give the world time to load

		m_Entities = new array<IEntity>;
		array<IEntity> entitiesBase = new array<IEntity>;
		GetEntities(world, entitiesBase);

		array<IEntity> entities = {};
		float boundingSize;
		vector pos;
		vector transform[4];
		string addon;

		foreach (ResourceName prefab, string imagePath : prefabs)
		{
			if (!api.IsGameMode())
				return; //--- Terminated by user

			ClearEntities(world, entities);
			Sleep(1);

			game.SpawnEntityPrefab(Resource.Load(prefab), world, spawnParams);
			Sleep(1);

			GetEntities(world, entities, entitiesBase);
			boundingSize = GetBoundingSize(entities, pos) * 1.5;

			Math3D.AnglesToMatrix(Vector(-135, -30, 0), transform);
			transform[3] = pos - transform[2] * boundingSize;
			camera.SetTransform(transform);
			Sleep(1000);

			if (!api.IsGameMode())
				return; //--- Terminated by user

			addon = SCR_AddonTool.GetResourceLastAddon(prefab);
			addon = SCR_AddonTool.ToFileSystem(addon);

			System.MakeScreenshot(addon + imagePath);
			PrintFormat("Image of prefab '%1' saved to @\"%2\"", FilePath.StripPath(prefab), imagePath);
			Sleep(1);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool QueryEntitiesCallback(IEntity e)
	{
		m_Entities.Insert(e);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void GetEntities(BaseWorld world, out notnull array<IEntity> outEntities, array<IEntity> baseEntities = null)
	{
		m_Entities = new array<IEntity>;
		world.QueryEntitiesByAABB(vector.Zero, vector.One * 128, QueryEntitiesCallback);

		if (baseEntities)
		{
			foreach (IEntity entity : m_Entities)
			{
				if (baseEntities.Find(entity) == -1)
					outEntities.Insert(entity);
			}
		}
		else
		{
			outEntities.Copy(m_Entities);
		}

		m_Entities = null;
	}

	//------------------------------------------------------------------------------------------------
	protected void ClearEntities(BaseWorld world, notnull array<IEntity> entities)
	{
		foreach (IEntity entity : entities)
		{
			delete entity;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected float GetBoundingSize(notnull array<IEntity> entities, out vector pos)
	{
		vector totalMins = vector.One * float.MAX;
		vector totalMaxs = vector.One * -float.MAX;
		vector mins, maxs;
		VObject object;
		string ext;
		bool hasShape;
		foreach (IEntity entity : entities)
		{
			if (!entity)
				continue;

			object = entity.GetVObject();
			if (!object)
				continue;

			FilePath.StripExtension(object.GetResourceName(), ext);
			if (ext != "xob")
				continue;

			hasShape = true;
			entity.GetBounds(mins, maxs);
			mins = entity.CoordToParent(mins);
			maxs = entity.CoordToParent(maxs);

			totalMins[0] = Math.Min(totalMins[0], mins[0]);
			totalMins[1] = Math.Min(totalMins[1], mins[1]);
			totalMins[2] = Math.Min(totalMins[2], mins[2]);

			totalMaxs[0] = Math.Max(totalMaxs[0], maxs[0]);
			totalMaxs[1] = Math.Max(totalMaxs[1], maxs[1]);
			totalMaxs[2] = Math.Max(totalMaxs[2], maxs[2]);

			PrintFormat("%1\n%2 - %3\n%4 - %5", FilePath.StripPath(entity.GetVObject().GetResourceName().GetPath()), mins, maxs, totalMins, totalMaxs);
		}

		if (!hasShape)
			return 1;

		Print(totalMins, LogLevel.DEBUG);
		Print(totalMaxs, LogLevel.DEBUG);
		vector total = totalMaxs - totalMins;
		pos = totalMins + total / 2;
		return total.Length();
	}

	//------------------------------------------------------------------------------------------------
	protected void Generate()
	{
		Workbench.OpenModule(WorldEditor);
		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		if (!worldEditor)
			return;

		WorldEditorAPI api = worldEditor.GetApi();
		if (!api)
			return;

		if (!ValidateScreen(api))
			return;

		if (!ValidateExtension())
			return;

		map<ResourceName, string> prefabs = new map<ResourceName, string>;
		if (!ValidatePrefabs(prefabs))
			return;

		if (!worldEditor.SetOpenedResource(m_SourceWorldPath.GetPath()))
			return;

		worldEditor.SwitchToGameMode();
		worldEditor.WaitForGameMode();
		CaptureImages(api, prefabs);
		worldEditor.SwitchToEditMode();
	}

	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		thread Generate();
	}

	//------------------------------------------------------------------------------------------------
	override void Configure()
	{
		Workbench.ScriptDialog("", "", this);
	}
};
