#ifdef WORKBENCH
class SCR_WorldEditorToolHelper
{
	protected static ResourceManager s_ResourceManager;
	protected static WorldEditor s_WorldEditor;
	protected static WorldEditorAPI s_WorldEditorAPI;

	protected static ref array<ResourceName> s_aTempResourceNames;

	//------------------------------------------------------------------------------------------------
	static ResourceManager GetResourceManager()
	{
		if (!s_ResourceManager)
			s_ResourceManager = Workbench.GetModule(ResourceManager);

		return s_ResourceManager;
	}

	//------------------------------------------------------------------------------------------------
	static WorldEditor GetWorldEditor()
	{
		if (!s_WorldEditor)
			s_WorldEditor = Workbench.GetModule(WorldEditor);

		return s_WorldEditor;
	}

	//------------------------------------------------------------------------------------------------
	static WorldEditorAPI GetWorldEditorAPI()
	{
		if (!s_WorldEditorAPI)
		{
			WorldEditor worldEditor = GetWorldEditor();
			if (worldEditor)
				s_WorldEditorAPI = worldEditor.GetApi();
		}

		return s_WorldEditorAPI;
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
	static array<ResourceName> GetSelectedOrOpenedResources(string wantedExtension = "", array<string> keywords = null)
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

		array<ResourceName> result = {};

		foreach (ResourceName resourceName : tempResult)
		{
			string extension;
			string resourceNameLC = FilePath.StripExtension(resourceName, extension);

			if (!wantedExtension.IsEmpty() && extension != wantedExtension)
				continue;

			resourceNameLC = FilePath.StripPath(resourceNameLC);
			resourceNameLC.ToLower();

			if (!keywords || SCR_StringHelper.ContainsEvery(resourceNameLC, keywords))
				result.Insert(resourceName);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	static array<ResourceName> GetSelectedResources(bool recursive = true)
	{
		WorldEditor worldEditor = GetWorldEditor();
		if (!worldEditor)
			return null;

		array<ResourceName> result = {};

		s_aTempResourceNames = {};
		worldEditor.GetResourceBrowserSelection(ResourceNameCallback, recursive);

		result.Copy(s_aTempResourceNames);
		s_aTempResourceNames = null;

		return result;
	}

	//------------------------------------------------------------------------------------------------
	static array<ResourceName> SearchWorkbenchResources(array<string> fileExtensions = null, array<string> searchStrArray = null, string rootPath = "", bool recursive = true)
	{
		array<ResourceName> result = {};

		s_aTempResourceNames = {};
		Workbench.SearchResources(ResourceNameCallback, fileExtensions, searchStrArray, rootPath, recursive);

		result.Copy(s_aTempResourceNames);
		s_aTempResourceNames = null;

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Delete the entitySource's entity IF it exists
	//! the provided entitySource can be null as well
	static void DeleteEntityFromSource(IEntitySource entitySource)
	{
		if (!entitySource)
			return;

		IEntity entity = GetWorldEditorAPI().SourceToEntity(entitySource);
		if (entity)
		{
			bool manageEditAction = BeginEntityAction();

			GetWorldEditorAPI().DeleteEntity(entity);

			EndEntityAction(manageEditAction);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Begin an Entity Action in World Editor API if required
	//! \return true if a BeginEntityAction has been called and EndEntityAction has to be called by EndEntityAction(), false otherwise
	static bool BeginEntityAction()
	{
		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();
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
	//! \param manageEditAction if World Editor Entity Action should be terminated, result of an earlier BeginEntityAction call
	static void EndEntityAction(bool manageEditAction)
	{
		if (manageEditAction)
			SCR_WorldEditorToolHelper.GetWorldEditorAPI().EndEntityAction();
	}

	/*
		Callbacks
	*/

	//------------------------------------------------------------------------------------------------
	//! WorkbenchSearchResourcesCallback method
	protected static void ResourceNameCallback(ResourceName resName, string filePath = "")
	{
		s_aTempResourceNames.Insert(resName);
	}
};
#endif
