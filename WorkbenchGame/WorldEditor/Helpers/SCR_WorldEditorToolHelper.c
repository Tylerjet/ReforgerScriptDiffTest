#ifdef WORKBENCH
class SCR_WorldEditorToolHelper
{
	protected static ref array<IEntity> s_aTempEntities;

	//------------------------------------------------------------------------------------------------
	//! Get the ResourceManager object
	//! \return available ResourceManager or null if unavailable
	static ResourceManager GetResourceManager()
	{
		return Workbench.GetModule(ResourceManager);
	}

	//------------------------------------------------------------------------------------------------
	//! Get the WorldEditor object
	//! \return available WorldEditor or null if unavailable
	static WorldEditor GetWorldEditor()
	{
		return Workbench.GetModule(WorldEditor);
	}

	//------------------------------------------------------------------------------------------------
	//! Get the World Editor API
	//! \return available WorldEditorAPI or null if unavailable
	static WorldEditorAPI GetWorldEditorAPI()
	{
		WorldEditor worldEditor = GetWorldEditor();
		if (!worldEditor)
			return null;

		return worldEditor.GetApi();
	}

	//------------------------------------------------------------------------------------------------
	//! Return the currently opened Prefab's entity with Edit Prefab button in Resource Manager
	//! \return the entity's source, null if error
	static IEntitySource GetPrefabEditModeEntitySource()
	{
		WorldEditor worldEditor = GetWorldEditor();
		if (!worldEditor)
			return null;

		if (!worldEditor.IsPrefabEditMode())
			return null;

		WorldEditorAPI worldEditorAPI = worldEditor.GetApi();
		if (worldEditorAPI.GetEditorEntityCount() != 2) // world and Entity
			return null;

		return worldEditorAPI.GetEditorEntity(1);
	}

	//------------------------------------------------------------------------------------------------
	//! Return the currently opened Prefab's entity with Edit Prefab button in Resource Manager
	//! \return the entity's source, null if error
	static ResourceName GetPrefabEditModeResourceName()
	{
		IEntitySource entitySource = GetPrefabEditModeEntitySource();
		if (!entitySource)
			return string.Empty;

		BaseContainer ancestor = entitySource.GetAncestor();
		if (!ancestor)
			return string.Empty;

		return ancestor.GetResourceName();
	}

	//------------------------------------------------------------------------------------------------
	//! Get a registered file's ResourceName (requires the .meta file)
	//! \return the registered file's ResourceName
	static ResourceName GetResourceNameFromFile(string absoluteFilePath)
	{
		MetaFile metaFile = GetResourceManager().GetMetaFile(absoluteFilePath);
		if (!metaFile)
			return string.Empty;

		return metaFile.GetResourceID();
	}

	//------------------------------------------------------------------------------------------------
	//! Get selected directories - does not get subdirectories, only selected ones (in their order of selection)
	//! \return resource names of directory that are selected in World Editor's Resource Browser
	static array<ResourceName> GetSelectedDirectories()
	{
		array<ResourceName> result = GetSelectedResources(true);
		if (!result)
			return null;

		for (int i = result.Count() - 1; i >= 0; i--)
		{
			ResourceName selectedResource = result[i];
			if (FilePath.StripExtension(selectedResource) != selectedResource)
				result.RemoveOrdered(i);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Wrapper for the method below
	//! \param[in] wantedExtension (case-insensitive)
	//! \param[in] keywords words that should be present in the file name (case-insensitive)
	//! \return an array of found ResourceNames - cannot be null
	static array<ResourceName> GetSelectedOrOpenedResources(string wantedExtension, array<string> keywords = null)
	{
		return GetSelectedOrOpenedResources({ wantedExtension }, keywords);
	}

	//------------------------------------------------------------------------------------------------
	//! \return an array of selected entities's IEntitySources, null on error (e.g WorldEditorAPI not available)
	static array<IEntitySource> GetSelectedWorldEntitySources()
	{
		WorldEditorAPI worldEditorAPI = GetWorldEditorAPI();
		if (!worldEditorAPI)
			return null;

		array<IEntitySource> result = {};
		IEntitySource entitySource;
		for (int i, count = worldEditorAPI.GetSelectedEntitiesCount(); i < count; ++i)
		{
			entitySource = worldEditorAPI.GetSelectedEntity(i);
			if (entitySource)
				result.Insert(entitySource);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Get selected or opened resources
	//! \param[in] acceptedExtensions accepted extensions (case-insensitive)
	//! \param[in] keywords words that should be present in the file name (case-insensitive)
	//! \return an array of found ResourceNames - cannot be null
	static array<ResourceName> GetSelectedOrOpenedResources(array<string> acceptedExtensions = null, array<string> keywords = null)
	{
		array<ResourceName> tempResult = GetSelectedResources(true);
		if (tempResult.IsEmpty())
		{
			ResourceName openedResource = GetPrefabEditModeResourceName();
			if (!openedResource.IsEmpty())
				tempResult.Insert(openedResource);
		}

		if (tempResult.IsEmpty())
			return tempResult;

		if (acceptedExtensions && acceptedExtensions.IsEmpty())
			acceptedExtensions = null;

		if (acceptedExtensions)
		{
			foreach (int i, string wantedExtension : acceptedExtensions)
			{
				wantedExtension.ToLower();
				acceptedExtensions[i] = wantedExtension;
			}
		}

		array<string> keywordsLC;
		if (keywords && !keywords.IsEmpty())
		{
			keywordsLC = {};
			foreach (string keyword : keywords)
			{
				if (keyword.IsEmpty())
					continue;

				keyword.ToLower();
				keywordsLC.Insert(keyword);
			}
		}

		array<ResourceName> result = {};

		foreach (ResourceName resourceName : tempResult)
		{
			string resourceNameLC = resourceName;
			resourceNameLC.ToLower();

			string extensionLC;
			resourceNameLC = FilePath.StripExtension(resourceName, extensionLC);
			extensionLC.ToLower();

			if (acceptedExtensions && !acceptedExtensions.Contains(extensionLC))
				continue;

			resourceNameLC = FilePath.StripPath(resourceNameLC);

			if (!keywordsLC || SCR_StringHelper.ContainsEvery(resourceNameLC, keywordsLC))
				result.Insert(resourceName);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Get all ResourceName that are selected in the Resource Browser
	//! \param[in] recursive true to get a selected directory's files, false to stop at the directory
	//! \return array of ResourceName of selected resources or null on error (e.g World Editor is not available)
	static array<ResourceName> GetSelectedResources(bool recursive = true)
	{
		WorldEditor worldEditor = GetWorldEditor();
		if (!worldEditor)
			return null;

		array<ResourceName> result = {};
		worldEditor.GetResourceBrowserSelection(result.Insert, recursive);
		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Search Workbench-available files by extension and filters inside a provided directory
	//! \param[in] fileExtensions
	//! \param[in] searchStrArray
	//! \param[in] rootPath format $addon:Workbench/Directory
	//! \param[in] recursive
	//! \return found resources
	// TODO: move to an eventual SCR_WorkbenchHelper
	static array<ResourceName> SearchWorkbenchResources(array<string> fileExtensions = null, array<string> searchStrArray = null, string rootPath = "", bool recursive = true)
	{
		SearchResourcesFilter filter = new SearchResourcesFilter();
		filter.fileExtensions = fileExtensions;
		filter.recursive = recursive;
		filter.rootPath = rootPath;
		filter.searchStr = searchStrArray;

		array<ResourceName> result = {};
		ResourceDatabase.SearchResources(filter, result.Insert);
		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Delete the entitySource's entity IF it exists
	//! \entitySource can be null
	static void DeleteEntityFromSource(IEntitySource entitySource)
	{
		if (!entitySource)
			return;

		WorldEditorAPI worldEditorAPI = GetWorldEditorAPI();
		if (!worldEditorAPI)
		{
			Print("WorldEditorAPI is not available", LogLevel.ERROR);
			return;
		}

		bool manageEditAction = BeginEntityAction();

		worldEditorAPI.DeleteEntity(entitySource);

		EndEntityAction(manageEditAction);
	}

	//------------------------------------------------------------------------------------------------
	//! Begin an Entity Action in World Editor API if required
	//! \return true if a BeginEntityAction has been called and EndEntityAction has to be called by EndEntityAction(), false otherwise
	static bool BeginEntityAction()
	{
		WorldEditorAPI worldEditorAPI = GetWorldEditorAPI();
		if (!worldEditorAPI)
		{
			Print("WorldEditorAPI is not available", LogLevel.ERROR);
			return false;
		}

		if (worldEditorAPI.IsDoingEditAction())
		{
			return false;
		}
		else
		{
			worldEditorAPI.BeginEntityAction();
			return true;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! End an Entity Action in World Editor API if required
	//! \param[in] manageEditAction if World Editor Entity Action should be terminated, result of an earlier BeginEntityAction call
	static void EndEntityAction(bool manageEditAction)
	{
		WorldEditorAPI worldEditorAPI = GetWorldEditorAPI();
		if (!worldEditorAPI)
		{
			Print("WorldEditorAPI is not available", LogLevel.ERROR);
			return;
		}

		if (manageEditAction)
			worldEditorAPI.EndEntityAction();
	}

	//------------------------------------------------------------------------------------------------
	//! Queries entities within an AABB in the world, returns results in an array
	//! \param[in] world represents the game's environment in which entities exist, used for querying entities within an Axis-Aligned Bounding Box
	//! \param[in] mins min bounds for an axis-aligned bounding box (AABB) query in 3D space
	//! \param[in] maxs max bounds for an axis-aligned bounding box (AABB) query in 3D space
	//! \param[in] queryFlags is an enumeration representing filter options for entity query in AABB - see BaseWorld.QueryEntitiesByAABB
	//! \return an array of entities within the specified AABB (Axis-Aligned Bounding Box) in the world
	static array<IEntity> QueryEntitiesByAABB(notnull World world, vector mins, vector maxs, EQueryEntitiesFlags queryFlags = EQueryEntitiesFlags.ALL)
	{
		array<IEntity> result = {};
		s_aTempEntities = {};
		world.QueryEntitiesByAABB(mins, maxs, QueryEntitiesCallbackMethod, null, queryFlags);
		result.Copy(s_aTempEntities);
		s_aTempEntities = null;
		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Queries entities within a sphere in the world, returns them in an array.
	//! \param[in] world World represents the game's environment in which entities exist, used for querying entities within a specified sphere radius in the method
	//! \param[in] worldPos represents the center point for the sphere query in 3D space
	//! \param[in] radius represents the distance from the center point (worldPos) within which entities are searched in the method
	//! \param[in] queryFlags specifies query flags for entity selection criteria - see BaseWorld.QueryEntitiesBySphere
	//! \return an array of entities within specified radius from world position
	static array<IEntity> QueryEntitiesBySphere(notnull World world, vector worldPos, float radius, EQueryEntitiesFlags queryFlags = EQueryEntitiesFlags.ALL)
	{
		array<IEntity> result = {};
		s_aTempEntities = {};
		world.QueryEntitiesBySphere(worldPos, radius, QueryEntitiesCallbackMethod, null, queryFlags);
		result.Copy(s_aTempEntities);
		s_aTempEntities = null;
		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Return all entities found by moved sphere trace
	//! \param[in] traceSphere
	//! \param[in] world
	//! \return found entities or null on error
	static array<IEntity> TraceMoveEntitiesBySphere(notnull World world, notnull TraceSphere traceSphere)
	{
		array<IEntity> result = {};

		s_aTempEntities = {};
		world.TraceMove(traceSphere, TraceCallbackMethod);

		result.Copy(s_aTempEntities);
		s_aTempEntities = null;

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Return all entities found by sphere trace
	//! \param[in] traceSphere
	//! \param[in] world
	//! \return found entities or null on error
	static array<IEntity> TracePositionEntitiesBySphere(notnull World world, notnull TraceSphere traceSphere)
	{
		array<IEntity> result = {};

		s_aTempEntities = {};
		world.TracePosition(traceSphere, TraceCallbackMethod);

		result.Copy(s_aTempEntities);
		s_aTempEntities = null;

		return result;
	}

	/*
		Callbacks
	*/

	//------------------------------------------------------------------------------------------------
	//! QueryEntitiesCallback method used for Entity querying
	protected static bool QueryEntitiesCallbackMethod(IEntity e)
	{
		s_aTempEntities.Insert(e);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! TraceEntitiesCallback method used for World tracing
	protected static bool TraceCallbackMethod(notnull IEntity e, vector start = "0 0 0", vector dir = "0 0 0")
	{
		s_aTempEntities.Insert(e);
		return true;
	}
}
#endif // WORKBENCH
