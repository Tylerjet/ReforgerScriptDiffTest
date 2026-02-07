class SCR_BaseContainerTools
{
	//------------------------------------------------------------------------------------------------
	/*!
	Get class name of the prefab.
	Use to check if prefab class is matching requirements before you spawn it.
	\param prefab Prefab path
	\return Class name
	*/
	static string GetContainerClassName(ResourceName prefab)
	{
		Resource prefabResource = Resource.Load(prefab);
		if (!prefabResource.IsValid())
			return string.Empty;

		return GetContainerClassName(prefabResource);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get class name of prefab resource.
	Use to check if prefab class is matching requirements before you spawn it.
	\param prefab Prefab resource
	\return Class name
	*/
	static string GetContainerClassName(Resource prefabResource)
	{
		if (!prefabResource.IsValid())
			return string.Empty;

		BaseResourceObject prefabContainer = prefabResource.GetResource();
		if (!prefabContainer)
			return string.Empty;

		BaseContainer prefabBase = prefabContainer.ToBaseContainer();
		if (!prefabBase)
			return string.Empty;

		return prefabBase.GetClassName();
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get enntity source from prefab resource.
	\param prefabResource Loaded entity prefab, use Resource.Load(prefab) to retrieve it from ResourceName
	\return Entity source
	*/
	static IEntitySource FindEntitySource(Resource prefabResource)
	{
		if (!prefabResource.IsValid())
			return null;

		BaseResourceObject prefabBase = prefabResource.GetResource();
		if (!prefabBase)
			return null;

		return prefabBase.ToEntitySource();
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get component source of given class from prefab resource.
	\param prefabResource Loaded entity prefab, use Resource.Load(prefab) to retrieve it from ResourceName
	\param componentClassName Class name of desired component
	\return Component source
	*/
	static IEntityComponentSource FindComponentSource(Resource prefabResource, string componentClassName)
	{
		if (!prefabResource.IsValid())
			return null;

		IEntitySource prefabEntity = FindEntitySource(prefabResource);
		if (!prefabEntity)
			return null;

		return FindComponentSource(prefabEntity, componentClassName);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get component source inherited from given class from prefab resource.
	\param prefabResource Loaded entity prefab, use Resource.Load(prefab) to retrieve it from ResourceName
	\param componentClassName Class of desired component
	\return Component source
	*/
	static IEntityComponentSource FindComponentSource(Resource prefabResource, typename componentClass)
	{
		if (!prefabResource.IsValid())
			return null;

		IEntitySource prefabEntity = FindEntitySource(prefabResource);
		if (!prefabEntity)
			return null;

		return FindComponentSource(prefabEntity, componentClass);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get component source of given class from entity source.
	\param prefabEntity Entity source
	\param componentClassName Class name of desired component
	\return Component source
	*/
	static IEntityComponentSource FindComponentSource(IEntitySource prefabEntity, string componentClassName)
	{
		if (!prefabEntity)
			return null;

		int componentsCount = prefabEntity.GetComponentCount();
		for (int i = 0; i < componentsCount; i++)
		{
			IEntityComponentSource componentSource = prefabEntity.GetComponent(i);
			if (componentSource.GetClassName() == componentClassName)
				return componentSource;
		}
		return null;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get component source inherited from given class from entity source.
	\param prefabEntity Entity source
	\param componentClass Class of desired component
	\return Component source
	*/
	static IEntityComponentSource FindComponentSource(IEntitySource prefabEntity, typename componentClass)
	{
		if (!prefabEntity)
			return null;

		int componentsCount = prefabEntity.GetComponentCount();
		for (int i = 0; i < componentsCount; i++)
		{
			IEntityComponentSource componentSource = prefabEntity.GetComponent(i);
			if (componentSource.GetClassName().ToType().IsInherited(componentClass))
				return componentSource;
		}
		return null;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get component sources of given classes from prefab resource.
	\param prefabResource Loaded entity prefab, use Resource.Load(prefab) to retrieve it from ResourceName
	\param componentClassNames Class names of desired components
	\param[out] Array to be filled with component sources (in the same order as componentClassNames)
	\return Number of components
	*/
	static int FindComponentSources(Resource prefabResource, notnull array<string> componentClassNames, notnull out array<ref array<IEntityComponentSource>> componentSources)
	{
		int classNamesCount = componentClassNames.Count();
		componentSources.Clear();
		componentSources.Resize(classNamesCount);

		if (!prefabResource.IsValid())
			return classNamesCount;

		IEntitySource prefabEntity = FindEntitySource(prefabResource);
		if (!prefabEntity)
			return classNamesCount;

		return FindComponentSources(prefabEntity, componentClassNames, componentSources);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get component sources of given classes from entity source.
	\param prefabEntity Entity source
	\param componentClassNames Class names of desired components
	\param[out] Array to be filled with component sources (in the same order as componentClassNames)
	\return Number of components
	*/
	static int FindComponentSources(IEntitySource prefabEntity, notnull array<string> componentClassNames, notnull out array<ref array<IEntityComponentSource>> componentSources)
	{
		int classNamesCount = componentClassNames.Count();
		componentSources.Clear();
		componentSources.Resize(classNamesCount);

		if (!prefabEntity)
			return classNamesCount;

		int componentsCount = prefabEntity.GetComponentCount();
		for (int i = 0; i < componentsCount; i++)
		{
			IEntityComponentSource componentSource = prefabEntity.GetComponent(i);
			string componentClassName = componentSource.GetClassName();
			for (int j = 0; j < classNamesCount; j++)
			{
				if (componentClassName == componentClassNames[j])
				{
					array<IEntityComponentSource> components = componentSources[j];
					if (!components)
					{
						components = new array<IEntityComponentSource>;
						componentSources.Set(j, components);
					}
					components.Insert(componentSource);
					break;
				}
			}
		}
		return classNamesCount;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Check of the comtainer contains any changes as opposed to its ancestor
	\param container Queried container
	\return True if the container has been modified
	* /
	static bool IsChanged(BaseContainer container)
	{
		string varName;
		BaseContainerList objectArray;
		for (int i = 0, varsCount = container.GetNumVars(); i < varsCount; i++)
		{
			varName = container.GetVarName(i);
			if (container.GetObject(varName))
			{
				if (IsChanged(container.GetObject(varName)))
					return true;
			}
			else if (container.GetObjectArray(varName))
			{
				objectArray = container.GetObjectArray(varName);
				for (int a = 0, arrayCount = objectArray.Count(); a < arrayCount; a++)
				{
					if (IsChanged(objectArray.Get(a)))
						return true;
				}
			}
			else
			{
				//PrintFormat("%1: %2: %3", container.GetClassName(), varName, container.IsVariableSetDirectly(varName));
				if (container.IsVariableSetDirectly(varName))
					return true;
			}
		}
		return false;
	}
	*/

	//------------------------------------------------------------------------------------------------
	/*!
	Get first container in hierarchy that comes from a prefab.
	\param container Evaluated container
	\return Container (or the input container when there is no prefab in the hierarchy)
	*/
	static BaseContainer GetPrefabContainer(BaseContainer container)
	{
		BaseContainer ancestor = container;
		while (ancestor)
		{
			if (container.GetResourceName().Contains("/"))
				return ancestor;
			ancestor = ancestor.GetAncestor();
		}
		return container;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get resource name of the first container in hierarchy that comes from a prefab.
	\param container Evaluated container
	\return Prefab resource name (or empty string when no prefab was found in the hierarchy)
	*/
	static ResourceName GetPrefabResourceName(BaseContainer container)
	{
		while (container)
		{
			if (container.GetResourceName().Contains("/"))
				return container.GetResourceName();
			container = container.GetAncestor();
		}
		return ResourceName.Empty;
	}

	//------------------------------------------------------------------------------------------------
	//! Get the topmost BaseContainer parent
	//! If it is the topmost, returns the provided baseContainer
	//! \param baseContainer
	//! \return topmost ancestor (e.g tree base)
	static BaseContainer GetTopMostAncestor(notnull BaseContainer baseContainer)
	{
		BaseContainer ancestorContainer = baseContainer.GetAncestor();
		if (!ancestorContainer)
			return baseContainer;

		while (ancestorContainer.GetAncestor())
		{
			ancestorContainer = ancestorContainer.GetAncestor();
		}

		return ancestorContainer;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get world coordinates of position local to given entity source.
	\param entitySource Entity source
	\param coords Local coordinates
	\return World coordinates
	*/
	static vector GetWorldCoords(IEntitySource entitySource, vector coords)
	{
		vector coordsEntity;
		while (entitySource)
		{
			entitySource.Get("coords", coordsEntity);
			coords += coordsEntity;
			entitySource = entitySource.GetParent();
		}
		return coords;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get coordinates local to given entity source.
	\param entitySource Entity source
	\param coords World coordinates
	\return Local coordinates
	*/
	static vector GetLocalCoords(IEntitySource entitySource, vector coords)
	{
		vector coordsEntity;
		while (entitySource)
		{
			entitySource.Get("coords", coordsEntity);
			coords -= coordsEntity;
			entitySource = entitySource.GetParent();
		}
		return coords;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Convert integer array to string in format acceptable by WorldEditorAPI.SetVariableValue
	\param values Input array
	\return Array converted to string
	*/
	static string GetArrayValue(notnull array<int> values)
	{
		string result;
		for (int i = 0, count = values.Count(); i < count; i++)
		{
			if (i > 0)
				result += ",";
			result += values[i].ToString();
		}
		return result;
	}
};
