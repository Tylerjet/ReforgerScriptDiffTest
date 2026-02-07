[BaseContainerProps()]
class SCR_ScenarioFrameworkStruct : SCR_JsonApiStruct
{
	protected int m_iHours = -1;
	protected int m_iMinutes = -1;
	protected int m_iSeconds = -1;
	protected bool m_bMatchOver;
	protected string m_sWeatherState;
	protected EGameOverTypes m_eGameOverType = EGameOverTypes.COMBATPATROL_DRAW;
	protected ref array<ref SCR_ScenarioFrameworkAreaStruct> m_aAreasStructs = {};
	
	//------------------------------------------------------------------------------------------------
	//! Serializes scenario state
	//! \return true if serialization is successful.
	override bool Serialize()
	{
		SCR_ScenarioFrameworkSystem scenarioFrameworkSystem = SCR_ScenarioFrameworkSystem.GetInstance();
		if (!scenarioFrameworkSystem)
			return false;
		
		m_bMatchOver = scenarioFrameworkSystem.GetIsMatchOver();
		
		m_aAreasStructs.Clear();
		
		ChimeraWorld world = GetGame().GetWorld();
		TimeAndWeatherManagerEntity timeManager = world.GetTimeAndWeatherManager();
		
		if (timeManager)
		{
			timeManager.GetHoursMinutesSeconds(m_iHours, m_iMinutes, m_iSeconds);
			BaseWeatherStateTransitionManager transitionManager = timeManager.GetTransitionManager();
			
			if (transitionManager)
				m_sWeatherState = transitionManager.GetCurrentState().GetStateName();
		}
		
		m_eGameOverType = scenarioFrameworkSystem.m_eGameOverType;
		StoreAreaStates(scenarioFrameworkSystem, m_aAreasStructs);
		ClearEmptyAreaStructs(m_aAreasStructs);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets up daytime, weather, and game over type, then loads area states for scenario framework system.
	//! \return true if deserialization is successful, false otherwise.
	override bool Deserialize()
	{
		if (m_aAreasStructs.IsEmpty())
			return false;
		
		SCR_ScenarioFrameworkSystem scenarioFrameworkSystem = SCR_ScenarioFrameworkSystem.GetInstance();
		if (!scenarioFrameworkSystem)
			return false;
		
		// Game was saved after match was over, don't load
		if (m_bMatchOver)
			return false;
		
		SCR_TimeAndWeatherHandlerComponent timeHandler = SCR_TimeAndWeatherHandlerComponent.GetInstance();
		
		// Weather has to be changed after init
		if (timeHandler && m_iHours >= 0 && m_iMinutes >= 0)
		{
			GetGame().GetCallqueue().Remove(timeHandler.SetupDaytimeAndWeather);
			GetGame().GetCallqueue().CallLater(timeHandler.SetupDaytimeAndWeather, 500, false, m_iHours, m_iMinutes, m_iSeconds, m_sWeatherState, true);
		}
		
		SCR_ScenarioFrameworkSystem.GetCallQueue().Clear();
		scenarioFrameworkSystem.m_bDebugInit = true;
		scenarioFrameworkSystem.m_iCurrentlySpawnedLayerTasks = 0;

		scenarioFrameworkSystem.m_aSelectedAreas.Clear();
		scenarioFrameworkSystem.m_aLayerTasksToBeInitialized.Clear();
		scenarioFrameworkSystem.m_aLayerTasksForRandomization.Clear();
		scenarioFrameworkSystem.m_aAreasTasksToSpawn.Clear();
		scenarioFrameworkSystem.m_aLayersTaskToSpawn.Clear();
		scenarioFrameworkSystem.m_aSlotsTaskToSpawn.Clear();
		scenarioFrameworkSystem.m_aESFTaskTypesAvailable.Clear();
		scenarioFrameworkSystem.m_aESFTaskTypeForRandomization.Clear();
		scenarioFrameworkSystem.m_aSpawnedAreas.Clear();
		scenarioFrameworkSystem.m_aDespawnedAreas.Clear();
		scenarioFrameworkSystem.m_VariableMap.Clear();

		foreach (SCR_ScenarioFrameworkArea registeredArea : scenarioFrameworkSystem.m_aAreas)
		{
			registeredArea.RestoreToDefault(true, false, false);
		}
		
		scenarioFrameworkSystem.m_eGameOverType = m_eGameOverType;
		LoadAreaStates(scenarioFrameworkSystem, m_aAreasStructs);
		scenarioFrameworkSystem.Init();
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Stores states of all areas in scenario framework system into an array.
	//! \param[in] scenarioFrameworkSystem Represents the scenario's area system, containing area data for storing states.
	//! \param[out] outEntries Array of stored area states.
	void StoreAreaStates(notnull SCR_ScenarioFrameworkSystem scenarioFrameworkSystem, out notnull array<ref SCR_ScenarioFrameworkAreaStruct> outEntries)
	{
		if (!scenarioFrameworkSystem.m_aAreas)
			return;
		
		SCR_ScenarioFrameworkAreaStruct struct;
		for (int i = scenarioFrameworkSystem.m_aAreas.Count() - 1; i >= 0; i--)
		{
			struct = new SCR_ScenarioFrameworkAreaStruct();
			struct.StoreState(scenarioFrameworkSystem.m_aAreas[i], struct);
			outEntries.Insert(struct);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Loads area states from loadedAreaStruct array into scenarioFrameworkSystem, handling nested areas.
	//! \param[in] scenarioFrameworkSystem Represents the scenario framework system for managing areas in the game.
	//! \param[in] loadedAreaStruct Array of loaded area structures representing areas in the scenario.
	void LoadAreaStates(notnull SCR_ScenarioFrameworkSystem scenarioFrameworkSystem, notnull array<ref SCR_ScenarioFrameworkAreaStruct> loadedAreaStruct)
	{
		IEntity entity;
		SCR_ScenarioFrameworkArea area;
		foreach (SCR_ScenarioFrameworkAreaStruct areaStruct : loadedAreaStruct)
		{
			entity = GetGame().GetWorld().FindEntityByName(areaStruct.GetName());
			if (!entity)
				continue;

			area = SCR_ScenarioFrameworkArea.Cast(entity.FindComponent(SCR_ScenarioFrameworkArea));
			if (!area)
				continue;
			
			LoadAreaStructs(scenarioFrameworkSystem, area, areaStruct);
			LoadNestedAreaStructs(areaStruct);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Loads area structures, adds tasks to spawn, sets delivery points, sets randomly spawned children, and loads repeated spawn area
	//! \param[in] scenarioFrameworkSystem Represents scenario framework system for managing area tasks, layers, and spawning structures.
	//! \param[in] area Loads area structures, sets spawn tasks, delivery points, and randomly spawned children for an area.
	//! \param[in] areaStruct Represents an area structure with spawning tasks, delivery points, and randomly spawned children for an area in the scenario framework
	protected void LoadAreaStructs(notnull SCR_ScenarioFrameworkSystem scenarioFrameworkSystem, notnull SCR_ScenarioFrameworkArea area, notnull SCR_ScenarioFrameworkAreaStruct areaStruct)
	{
		if (areaStruct.GetAreaSelected())
		{
			scenarioFrameworkSystem.m_aAreasTasksToSpawn.Insert(areaStruct.GetName());
			scenarioFrameworkSystem.m_aLayersTaskToSpawn.Insert(areaStruct.GetLayerTaskname());
		}
		
		if (areaStruct.GetDeliveryPointNameForItem())
			area.StoreDeliveryPoint(areaStruct.GetDeliveryPointNameForItem());
			
		if (areaStruct.GetRandomlySpawnedChildren())
			area.SetRandomlySpawnedChildren(areaStruct.GetRandomlySpawnedChildren());
			
		LoadRepeatedSpawnAreaStructs(area, areaStruct);
		
	}
	
	//------------------------------------------------------------------------------------------------
	//! Loads repeated spawn settings from area struct to area.
	//! \param[in] area Loads repeated spawn settings from area struct into scenario area.
	//! \param[in] areaStruct Represents spawn area structure with repeatable spawn settings for scenario framework area.
	protected void LoadRepeatedSpawnAreaStructs(notnull SCR_ScenarioFrameworkArea area, notnull SCR_ScenarioFrameworkAreaStruct areaStruct)
	{
		if (areaStruct.GetEnableRepeatedSpawn())
		{
			area.SetEnableRepeatedSpawn(areaStruct.GetEnableRepeatedSpawn());
			if (areaStruct.GetRepeatedSpawnNumber())
				area.SetRepeatedSpawnNumber(areaStruct.GetRepeatedSpawnNumber());
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Loads nested area layer and logic structs for given area struct.
	//! \param[in] areaStruct Represents an area structure containing nested layer and logic structures in the scenario framework.
	protected void LoadNestedAreaStructs(notnull SCR_ScenarioFrameworkAreaStruct areaStruct)
	{
		if (areaStruct.GetLayerStructs())
			LoadLayer(areaStruct.GetLayerStructs());
			
		if (areaStruct.GetLogicStructs())
			LoadLogic(areaStruct.GetLogicStructs());
	}
	
	//------------------------------------------------------------------------------------------------
	//! Loads layers from an array of structures, finds entities, casts them to layers, and loads their structures.
	//! \param[in] loadedLayerStruct Loads an array of layer structures, representing individual layers in the scenario.
	protected void LoadLayer(notnull array<ref SCR_ScenarioFrameworkLayerStruct> loadedLayerStruct)
	{
		IEntity entity;
		SCR_ScenarioFrameworkLayerBase layer;
		BaseWorld world = GetGame().GetWorld();
		foreach (SCR_ScenarioFrameworkLayerStruct layerStruct : loadedLayerStruct)
		{
			entity = world.FindEntityByName(layerStruct.GetName());
			if (!entity)
				continue;
			
			layer = SCR_ScenarioFrameworkLayerBase.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerBase));
			if (!layer)
				continue;
			
			LoadLayerStructs(layer, layerStruct);
			LoadNestedLayerStructs(layerStruct);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Loads layer structures, sets activation type, termination status, layer task state, randomly spawned children, and nested layer
	//! \param[in] layer Loads layer structs, sets activation type, termination, spawn layer structs, layer task state, randomly spawned
	//! \param[in] layerStruct Represents layer's configuration data for the scenario framework layer.
	protected void LoadLayerStructs(notnull SCR_ScenarioFrameworkLayerBase layer, notnull  SCR_ScenarioFrameworkLayerStruct layerStruct)
	{
		if (layerStruct.GetActivationType() != -1)
			layer.SetActivationType(layerStruct.GetActivationType());
		
		if (layerStruct.GetIsTerminated())
		{
			layer.SetIsTerminated(layerStruct.GetIsTerminated());
			return;
		}
			
		LoadRepeatedSpawnLayerStructs(layer, layerStruct);
		LoadLayerStructSlots(layer, layerStruct);
			
		//Layer task handling
		SCR_ScenarioFrameworkLayerTask layerTask = SCR_ScenarioFrameworkLayerTask.Cast(layer);
		if (layerTask)
			layerTask.SetLayerTaskState(layerStruct.GetLayerTaskState());
			
		if (layerStruct.GetRandomlySpawnedChildren())
			layer.SetRandomlySpawnedChildren(layerStruct.GetRandomlySpawnedChildren());
			
		LoadNestedLayerStructs(layerStruct);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Enables repeated spawn for a scenario layer based on provided layer struct.
	//! \param[in] layer Enables repeated spawning for the layer based on provided layer struct.
	//! \param[in] layerStruct Represents spawning configuration for repeating layer in scenario.
	protected void LoadRepeatedSpawnLayerStructs(notnull SCR_ScenarioFrameworkLayerBase layer, notnull  SCR_ScenarioFrameworkLayerStruct layerStruct)
	{
		if (!layerStruct.GetEnableRepeatedSpawn())
			return;
		
		layer.SetEnableRepeatedSpawn(true);
		layer.SetRepeatedSpawnNumber(layerStruct.GetRepeatedSpawnNumber());
	}
	
	//------------------------------------------------------------------------------------------------
	//! Loads layer slots with AI prefab removal and random spawned object settings from layer struct.
	//! \param[in] layer Loads layer's slots with AI prefab removal and randomly spawned object data from layer struct.
	//! \param[in] layerStruct Represents layer's struct configuration for AI prefab removal and randomly spawned objects.
	protected void LoadLayerStructSlots(notnull SCR_ScenarioFrameworkLayerBase layer, notnull  SCR_ScenarioFrameworkLayerStruct layerStruct)
	{
		SCR_ScenarioFrameworkSlotBase slot;
		SCR_ScenarioFrameworkSlotAI slotAI;
		slot = SCR_ScenarioFrameworkSlotBase.Cast(layer.FindComponent(SCR_ScenarioFrameworkSlotBase));
		if (slot)
		{
			slotAI = SCR_ScenarioFrameworkSlotAI.Cast(slot.FindComponent(SCR_ScenarioFrameworkSlotAI));
			if (slotAI)
			{
				if (layerStruct.GetAIPrefabsForRemoval())
					slotAI.SetAIPrefabsForRemoval(layerStruct.GetAIPrefabsForRemoval());
			}
				
			if (layerStruct.GetRandomlySpawnedObject())
				slot.SetRandomlySpawnedObject(layerStruct.GetRandomlySpawnedObject());
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Loads nested layer and logic structs from provided layer struct.
	//! \param[in] layerStruct Loads nested layer structures and logic structures from provided layerStruct object.
	protected void LoadNestedLayerStructs(notnull SCR_ScenarioFrameworkLayerStruct layerStruct)
	{
		if (layerStruct.GetLayerStructs())
			LoadLayer(layerStruct.GetLayerStructs());
			
		if (layerStruct.GetLogicStructs())
			LoadLogic(layerStruct.GetLogicStructs());
	}
	
	//------------------------------------------------------------------------------------------------
	//! Loads scenario logic entries, sets termination status, and updates counters for each logic entity in the scenario.
	//! \param[in] entries The entries represent an array of scenario logic structures, which contain information about scenario logic entities, their states, and counter values.
	protected void LoadLogic(notnull array<ref SCR_ScenarioFrameworkLogicStruct> entries)
	{
		IEntity entity;
		BaseWorld world = GetGame().GetWorld();
		SCR_ScenarioFrameworkLogic logic;
		SCR_ScenarioFrameworkLogicCounter logicCounter;
		foreach (SCR_ScenarioFrameworkLogicStruct logicInfo : entries)
		{
			entity = world.FindEntityByName(logicInfo.GetName());
			if (!entity)
				continue;
			
			logic = SCR_ScenarioFrameworkLogic.Cast(entity);
			if (!logic)
				continue;
			
			if (logicInfo.GetIsTerminated())
			{
				logic.SetIsTerminated(logicInfo.GetIsTerminated());
				continue;
			}
			
			logicCounter = SCR_ScenarioFrameworkLogicCounter.Cast(entity);
			if (!logicCounter)
				continue;
			
			logicCounter.SetCounterValue(logicInfo.GetCounterValue());
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Removes empty area structs from array.
	//! \param[in] areaStructsToClear Clears empty area structs from areaStructsToClear array.
	protected void ClearEmptyAreaStructs(notnull array<ref SCR_ScenarioFrameworkAreaStruct> areaStructsToClear)
	{
		array<ref SCR_ScenarioFrameworkAreaStruct> areasStructsCopy = {};
		foreach (SCR_ScenarioFrameworkAreaStruct areaStructToCopy : areaStructsToClear)
		{
			areasStructsCopy.Insert(areaStructToCopy);
		}
		
		foreach (SCR_ScenarioFrameworkAreaStruct areaStruct : areasStructsCopy)
		{
			if (areaStruct.GetStructVarCount() < 1)
			{
				areaStructsToClear.RemoveItem(areaStruct);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Registers variables for scenario framework structure.
	void SCR_ScenarioFrameworkStruct()
	{
		RegV("m_iHours");
		RegV("m_iMinutes");
		RegV("m_iSeconds");
		RegV("m_bMatchOver");
		RegV("m_sWeatherState");
		RegV("m_eGameOverType");
		RegV("m_aAreasStructs");
	}
};

class SCR_ScenarioFrameworkAreaStruct : SCR_ScenarioFrameworkLayerStruct
{
	protected bool m_bAreaSelected;
	protected string m_sItemDeliveryPointName;
	protected string m_sLayerTaskName;
	
	//------------------------------------------------------------------------------------------------
	//! Stores scenario area data into struct, cleans empty layers and logic, and stores children areas.
	//! \param[in] area Stores scenario area data into struct for further processing.
	//! \param[out] areaStruct Represents stored scenario area data structure for further processing.
	void StoreState(notnull SCR_ScenarioFrameworkArea area, out notnull SCR_ScenarioFrameworkAreaStruct areaStruct)
	{
		//Checks if area is named. If it is not, we cannot use it for serialization
		if (area.GetName().IsEmpty())
		{
			delete areaStruct;
			return;
		}
		
		areaStruct.SetName(area.GetName());
		
		StoreSelectedArea(area, areaStruct);
		StoreDeliveryPoint(area, areaStruct);
		
		StoreActivationTypeStatus(area, areaStruct);
		StoreTerminationStatus(area, areaStruct);
		StoreRepeatedSpawn(area, areaStruct);
		StoreLayerTask(area, areaStruct);
		
		StoreChildren(area, areaStruct);
		StoreLogic(area, areaStruct);
		
		CleanEmptyStoredLayers(areaStruct);
		CleanEmptyStoredLogic(areaStruct);
		CleanAreaStructs(areaStruct);
	}

	//------------------------------------------------------------------------------------------------
	//! Selects or deselects area in scenario framework area structure based on its selection status.
	//! \param[in] area Selects area for struct if it's selected, increments struct var count otherwise unregisters selection.
	//! \param[in] areaStruct Represents selected area state in scenario framework structure.
	protected void StoreSelectedArea(notnull SCR_ScenarioFrameworkArea area, notnull SCR_ScenarioFrameworkAreaStruct areaStruct)
	{
		if (area.m_bAreaSelected)
		{
			areaStruct.IncreaseStructVarCount();
			areaStruct.SetAreaSelected(1);
		}
		else
		{
			areaStruct.UnregV("m_bAreaSelected");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Sets delivery point for an item in scenario area structure based on area's delivery point name.
	//! \param[in] area Sets delivery point name for item in scenario framework area structure based on area's delivery point name.
	//! \param[in] areaStruct Represents an area's structure in the scenario, used for item delivery point management.
	protected void StoreDeliveryPoint(notnull SCR_ScenarioFrameworkArea area, notnull SCR_ScenarioFrameworkAreaStruct areaStruct)
	{
		if (!area.m_sItemDeliveryPointName.IsEmpty())
		{
			areaStruct.IncreaseStructVarCount();
			areaStruct.SetDeliveryPointNameForItem(area.GetDeliveryPointName());
		}
		else 
		{
			areaStruct.UnregV("m_sItemDeliveryPointName");
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Updates area activation type in scenario framework area structure if it differs from default, otherwise removes it from structure.
	//! \param[in] area Updates area's activation type in scenario framework area structure, adjusting count if necessary.
	//! \param[in] areaStruct Represents scenario framework area structure with activation type status.
	void StoreActivationTypeStatus(notnull SCR_ScenarioFrameworkArea area, notnull SCR_ScenarioFrameworkAreaStruct areaStruct)
	{
		if (area.m_eActivationTypeDefault != area.m_eActivationType)
		{
			areaStruct.SetActivationType(area.m_eActivationType);
			areaStruct.IncreaseStructVarCount();
		}
		else
		{
			areaStruct.UnregV("m_eActivationType");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Updates termination status in scenario framework area structure based on current termination status of area.
	//! \param[in] area Updates area termination status in scenario framework area structure.
	//! \param[in] areaStruct Represents scenario framework area's state data structure.
	protected void StoreTerminationStatus(notnull SCR_ScenarioFrameworkArea area, notnull SCR_ScenarioFrameworkAreaStruct areaStruct)
	{
		if (area.GetIsTerminated())
		{
			areaStruct.IncreaseStructVarCount();
			areaStruct.SetIsTerminated(true);
		}
		else
		{
			areaStruct.UnregV("m_bIsTerminated");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Updates area struct with repeated spawn settings from area.
	//! \param[in] area Stores repeated spawn settings from area to area struct.
	//! \param[in] areaStruct Represents area's repeated spawn configuration data in scenario framework.
	protected void StoreRepeatedSpawn(notnull SCR_ScenarioFrameworkArea area, notnull SCR_ScenarioFrameworkAreaStruct areaStruct)
	{
		if (area.GetEnableRepeatedSpawn())
		{
			areaStruct.IncreaseStructVarCount();
			areaStruct.SetEnableRepeatedSpawn(area.GetEnableRepeatedSpawn());
			if (area.GetRepeatedSpawnNumber() != -1)
			{
				areaStruct.IncreaseStructVarCount();
				areaStruct.SetRepeatedSpawnNumber(area.GetRepeatedSpawnNumber());
			}
			else
			{
				areaStruct.UnregV("m_iRepeatedSpawnNumber");
			}
		}
		else
		{
			areaStruct.UnregV("m_bEnableRepeatedSpawn");
			areaStruct.UnregV("m_iRepeatedSpawnNumber");
		}	
	}

	//------------------------------------------------------------------------------------------------
	//! Sets layer task name in area struct if layer task exists in area, otherwise removes it from area struct.
	//! \param[in] area Sets layer task name in area struct if layer task exists in area, otherwise removes it from area struct.
	//! \param[in] areaStruct Represents area's layer task structure in scenario framework.
	protected void StoreLayerTask(notnull SCR_ScenarioFrameworkArea area, notnull SCR_ScenarioFrameworkAreaStruct areaStruct)
	{
		if (area.m_LayerTask)
		{
			areaStruct.SetLayerTaskName(area.GetLayerTaskName());
			areaStruct.IncreaseStructVarCount();
		}
		else	
		{
			areaStruct.UnregV("m_sLayerTaskName");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Stores children layers and their states in area struct, handles randomly spawned children if applicable.
	//! \param[in] area Stores children layers and their states in area structure.
	//! \param[in] areaStruct Represents a structure containing information about scenario layers in an area for storing and retrieving layer states.
	protected void StoreChildren(notnull SCR_ScenarioFrameworkArea area, notnull SCR_ScenarioFrameworkAreaStruct areaStruct)
	{
		array<SCR_ScenarioFrameworkLayerBase> children = {};
		area.GetChildren(children);
		
		if (children.IsEmpty())
		{
			areaStruct.UnregV("m_aLayersStructs");
			return;
		}

		if (area.GetSpawnChildrenType() != SCR_EScenarioFrameworkSpawnChildrenType.ALL)
		{
			array<SCR_ScenarioFrameworkLayerBase> m_aRandomlySpawnedChildrenLayerBases = area.GetRandomlySpawnedChildren();
			foreach (SCR_ScenarioFrameworkLayerBase child : m_aRandomlySpawnedChildrenLayerBases)
			{
				areaStruct.InsertRandomlySpawnedChildren(child.GetName());
			}
			if (!m_aRandomlySpawnedChildrenLayerBases.IsEmpty())
				areaStruct.IncreaseStructVarCount();
		}
		else
		{
			areaStruct.UnregV("m_aRandomlySpawnedChildren");
		}
			
		foreach (SCR_ScenarioFrameworkLayerBase layer : children)
		{
			areaStruct.StoreLayerState(layer);
		}
			
		foreach (SCR_ScenarioFrameworkLayerStruct childLayerStruct : m_aLayersStructs)
		{
			if (childLayerStruct.GetStructVarCount() >= 1)
			{
				areaStruct.IncreaseStructVarCount();
				return;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Stores logic states for scenario area and updates struct count if necessary.
	//! \param[in] area Stores logic states for scenario areas in an array, updates struct variables count if necessary.
	//! \param[in] areaStruct Stores logic states for area in scenario framework structure.
	protected void StoreLogic(notnull SCR_ScenarioFrameworkArea area, notnull SCR_ScenarioFrameworkAreaStruct areaStruct)
	{
		array<SCR_ScenarioFrameworkLogic> logics = {};
		area.GetLogics(logics);
		
		if (logics.IsEmpty())
		{
			areaStruct.UnregV("m_aLogicStructs");
		}
		else
		{
			foreach (SCR_ScenarioFrameworkLogic logic : logics)
			{
				areaStruct.StoreLogicState(logic);
			}
			
			if (!m_aLogic.IsEmpty())
				areaStruct.IncreaseStructVarCount();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Removes empty layer structs from area struct.
	//! \param[in] areaStruct AreaStruct represents an area in the scenario with its own layer structures, which can be cleaned up if empty or non-ex
	protected void CleanEmptyStoredLayers(notnull SCR_ScenarioFrameworkAreaStruct areaStruct)
	{
		if (areaStruct.GetLayerStructs())
		{
			if (areaStruct.GetLayerStructs().IsEmpty())
				areaStruct.UnregV("m_aLayersStructs");
		}
		else
		{
			areaStruct.UnregV("m_aLayersStructs");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Removes empty logic structs from area struct.
	//! \param[in] areaStruct AreaStruct represents an area in the scenario with its own logic structures.
	protected void CleanEmptyStoredLogic(notnull SCR_ScenarioFrameworkAreaStruct areaStruct)
	{
		if (areaStruct.GetLogicStructs())
		{
			if (areaStruct.GetLogicStructs().IsEmpty())
				areaStruct.UnregV("m_aLogicStructs");
		}
		else
		{
			areaStruct.UnregV("m_aLogicStructs");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Clears empty layer and logic structs, removes AI prefab references, and resets random spawned object and layer
	//! \param[in] areaStruct Clears empty layer and logic structures, removes AI prefabs, random objects, and layer task state from area struct.
	protected void CleanAreaStructs(notnull SCR_ScenarioFrameworkAreaStruct areaStruct)
	{
		areaStruct.ClearEmptyLayerStructs(areaStruct.GetLayerStructs());
		areaStruct.ClearEmptyLogicStructs(areaStruct.GetLogicStructs());

		areaStruct.UnregV("m_aAIPrefabsForRemoval");
		areaStruct.UnregV("m_sRandomlySpawnedObject");
		areaStruct.UnregV("m_iLayerTaskState");
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return whether an area is selected or not.
	bool GetAreaSelected()
	{
		return m_bAreaSelected;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return the name of the delivery point.
	string GetDeliveryPointNameForItem()
	{
		return m_sItemDeliveryPointName;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Represents the name of the layer task.
	string GetLayerTaskname()
	{
		return m_sLayerTaskName;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] selected Sets whether an area is selected or not.
	void SetAreaSelected(bool selected)
	{
		m_bAreaSelected = selected;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] name Sets item delivery point name.
	void SetDeliveryPointNameForItem(string name)
	{
		m_sItemDeliveryPointName = name;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] layerTaskName Sets the name for the layer task.
	void SetLayerTaskName(string layerTaskName)
	{
		m_sLayerTaskName = layerTaskName;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Registers variables for area.
	void SCR_ScenarioFrameworkAreaStruct()
	{
		RegV("m_bAreaSelected");
		RegV("m_sLayerTaskName");
		RegV("m_sItemDeliveryPointName");
	}
};

class SCR_ScenarioFrameworkLayerStruct : SCR_JsonApiStruct
{
	protected string											m_sName;
	protected ResourceName 										m_sRandomlySpawnedObject;
	protected ref array<ref SCR_ScenarioFrameworkLayerStruct>	m_aLayersStructs = {};
	protected ref array<ref SCR_ScenarioFrameworkLogicStruct> 	m_aLogicStructs = {};
	protected ref array<SCR_ScenarioFrameworkLayerBase>			m_aChildren = {};
	protected ref array<SCR_ScenarioFrameworkLogic>				m_aLogic = {};
	protected ref array<ResourceName>							m_aAIPrefabsForRemoval = {};
	protected ref array<string>									m_aRandomlySpawnedChildren = {};
	protected int 												m_iLayerTaskState;
	protected SCR_ScenarioFrameworkEActivationType				m_eActivationType = -1; // We put default value as -1 here because 0 has other implications down the line
	protected bool 												m_bIsTerminated; //Marks if this was terminated - either by death or deletion
	protected int 												m_iRepeatedSpawnNumber;
	protected bool 												m_bEnableRepeatedSpawn;
	protected int												m_iStructVarCount;
	
	//------------------------------------------------------------------------------------------------
	//! Stores layer state data into structs, cleans empty structs, and inserts non-empty ones into list.
	//! \param[in] layer Stores layer state data for further processing in the scenario framework.
	void StoreLayerState(notnull SCR_ScenarioFrameworkLayerBase layer)
	{
		SCR_ScenarioFrameworkLayerStruct layerStruct = new SCR_ScenarioFrameworkLayerStruct();
		
		//Checks if layer is named. If it is not, we cannot use it for serialization
		if (layer.GetName().IsEmpty())
		{
			delete layerStruct;
			return;
		}
		
		layerStruct.SetName(layer.GetName());
		
		StoreActivationTypeStatus(layer, layerStruct);
		StoreTerminationStatus(layer, layerStruct);
		StoreRepeatedSpawn(layer, layerStruct);
		StoreLayerTask(layer, layerStruct);
		
		StoreChildren(layer, layerStruct);
		StoreLogic(layer, layerStruct);
		
		StoreSlotAndRandomObject(layer, layerStruct);
		
		CleanEmptyStoredLayers(layer, layerStruct);
		CleanEmptyStoredLogic(layer, layerStruct);
		
		//Final insertion
		if (layerStruct && layerStruct.GetStructVarCount() > 0)
			m_aLayersStructs.Insert(layerStruct);
		
		CleanEmptyStructs();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Updates layer's activation type in scenario structure if it differs from default, otherwise removes it from structure.
	//! \param[in] layer Represents scenario layer's activation type status in the scenario framework.
	//! \param[in] layerStruct Represents the scenario framework layer structure where activation type changes are stored.
	void StoreActivationTypeStatus(notnull SCR_ScenarioFrameworkLayerBase layer, notnull SCR_ScenarioFrameworkLayerStruct layerStruct)
	{
		if (layer.m_eActivationTypeDefault != layer.m_eActivationType)
		{
			layerStruct.SetActivationType(layer.m_eActivationType);
			layerStruct.IncreaseStructVarCount();
		}
		else
		{
			layerStruct.UnregV("m_eActivationType");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Updates termination status in scenario layer structure based on current termination status in scenario layer.
	//! \param[in] layer Represents scenario layer's termination status in the method, updates termination flag in layer struct.
	//! \param[in] layerStruct Represents scenario termination status data structure.
	void StoreTerminationStatus(notnull SCR_ScenarioFrameworkLayerBase layer, notnull SCR_ScenarioFrameworkLayerStruct layerStruct)
	{
		if (layer.GetIsTerminated())
		{
			layerStruct.SetIsTerminated(true);
			layerStruct.IncreaseStructVarCount();
		}
		else
		{
			layerStruct.UnregV("m_bIsTerminated");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Updates scenario layer's repeated spawn settings from layer struct.
	//! \param[in] layer Stores repeated spawn settings from layer to layer struct.
	//! \param[in] layerStruct Represents scenario layer structure with repeated spawn settings.
	void StoreRepeatedSpawn(notnull SCR_ScenarioFrameworkLayerBase layer, notnull SCR_ScenarioFrameworkLayerStruct layerStruct)
	{
		if (layer.GetEnableRepeatedSpawn())
		{
			layerStruct.SetEnableRepeatedSpawn(layer.GetEnableRepeatedSpawn());
			layerStruct.IncreaseStructVarCount();
			if (layer.GetRepeatedSpawnNumber() != -1)
			{
				layerStruct.IncreaseStructVarCount();
				layerStruct.SetRepeatedSpawnNumber(layer.GetRepeatedSpawnNumber());
			}
			else
			{
				layerStruct.UnregV("m_iRepeatedSpawnNumber");
			}
		}
		else
		{
			layerStruct.UnregV("m_bEnableRepeatedSpawn");
			layerStruct.UnregV("m_iRepeatedSpawnNumber");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Updates layer task state in scenario layer structure if it exists, otherwise removes it.
	//! \param[in] layer Updates layer state in scenario framework layer structure.
	//! \param[in] layerStruct Represents layer's state data structure in scenario framework.
	void StoreLayerTask(notnull SCR_ScenarioFrameworkLayerBase layer, notnull SCR_ScenarioFrameworkLayerStruct layerStruct)
	{
		SCR_ScenarioFrameworkLayerTask layerTask = SCR_ScenarioFrameworkLayerTask.Cast(layer);
		if (layerTask && layerTask.GetLayerTaskState() != 0)
		{
			layerStruct.SetLayerTaskState(layerTask.GetLayerTaskState());
			layerStruct.IncreaseStructVarCount();
		}
		else
		{
			layerStruct.UnregV("m_iLayerTaskState");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Stores children layers, handles random spawning, and updates layer states in scenario framework layer structure.
	//! \param[in] layer Stores children layers and their states, updates struct variables count based on conditions.
	//! \param[in] layerStruct Stores layer's children and their states, updates struct variables count based on conditions.
	void StoreChildren(notnull SCR_ScenarioFrameworkLayerBase layer, notnull SCR_ScenarioFrameworkLayerStruct layerStruct)
	{
		layer.GetChildren(m_aChildren);
		if (m_aChildren.IsEmpty())
		{
			layerStruct.UnregV("m_aLayersStructs");
			return;
		}
		
		if (layer.GetSpawnChildrenType() != SCR_EScenarioFrameworkSpawnChildrenType.ALL)
		{
			array <SCR_ScenarioFrameworkLayerBase> m_aRandomlySpawnedChildrenLayerBases = layer.GetRandomlySpawnedChildren();
			foreach (SCR_ScenarioFrameworkLayerBase child : m_aRandomlySpawnedChildrenLayerBases)
			{
				layerStruct.InsertRandomlySpawnedChildren(child.GetName());
			}
				
			if (!m_aRandomlySpawnedChildrenLayerBases.IsEmpty())
				layerStruct.IncreaseStructVarCount();
		}
		else
			layerStruct.UnregV("m_aRandomlySpawnedChildren");
			
		foreach (SCR_ScenarioFrameworkLayerBase layerToCycle : m_aChildren)
		{
			layerStruct.StoreLayerState(layerToCycle);
		}
			
		foreach (SCR_ScenarioFrameworkLayerStruct childLayerStruct : m_aLayersStructs)
		{
			// We need to check if just one child layer struct has something saved
			if (childLayerStruct.GetStructVarCount() >= 1)
			{
				// Then we can increase this layer struct and return it
				layerStruct.IncreaseStructVarCount();
				return;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Stores logic states for scenario framework layer.
	//! \param[in] layer Stores logic data for scenario layer.
	//! \param[in] layerStruct Stores logic state for scenario framework layer structures.
	void StoreLogic(notnull SCR_ScenarioFrameworkLayerBase layer, notnull SCR_ScenarioFrameworkLayerStruct layerStruct)
	{
		layer.GetLogics(m_aLogic);
		if (m_aLogic.IsEmpty())
		{
			layerStruct.UnregV("m_aLogicStructs");
		}
		else
		{
			foreach (SCR_ScenarioFrameworkLogic logic : m_aLogic)
			{
				layerStruct.StoreLogicState(logic);
			}
			
			if (!m_aLogic.IsEmpty())
				layerStruct.IncreaseStructVarCount();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Stores AI prefab removal list and random object from scenario layer to scenario layer struct.
	//! \param[in] layer Stores AI prefab removal list and random object for scenario layer.
	//! \param[in] layerStruct Stores AI prefab removal list and random object for scenario layer.
	void StoreSlotAndRandomObject(notnull SCR_ScenarioFrameworkLayerBase layer, notnull SCR_ScenarioFrameworkLayerStruct layerStruct)
	{
		SCR_ScenarioFrameworkSlotBase slot = SCR_ScenarioFrameworkSlotBase.Cast(layer);
		if (slot)
		{
			SCR_ScenarioFrameworkSlotAI slotAI = SCR_ScenarioFrameworkSlotAI.Cast(slot);
			if (slotAI && !slotAI.GetAIPrefabsForRemoval().IsEmpty() && !slotAI.GetIsTerminated())
			{
				layerStruct.SetAIPrefabsForRemoval(slotAI.GetAIPrefabsForRemoval());
				layerStruct.IncreaseStructVarCount();
			}
			else
			{
				layerStruct.UnregV("m_aAIPrefabsForRemoval");
			}
			
			if (slot.GetRandomlySpawnedObject())
			{
				layerStruct.SetRandomlySpawnedObject(slot.GetRandomlySpawnedObject());
				layerStruct.IncreaseStructVarCount();
			}
			else
			{
				layerStruct.UnregV("m_sRandomlySpawnedObject");
			}
		}
		else
		{
			layerStruct.UnregV("m_aAIPrefabsForRemoval");
			layerStruct.UnregV("m_sRandomlySpawnedObject");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Removes empty layer structs from scenario layer.
	//! \param[in] layer Removes empty layer structs from scenario framework layer.
	//! \param[in] layerStruct Represents a container for storing layers in scenario framework.
	void CleanEmptyStoredLayers(notnull SCR_ScenarioFrameworkLayerBase layer, notnull SCR_ScenarioFrameworkLayerStruct layerStruct)
	{
		if (layerStruct.GetLayerStructs())
		{
			if (layerStruct.GetLayerStructs().IsEmpty())
				layerStruct.UnregV("m_aLayersStructs");
		}
		else
		{
			layerStruct.UnregV("m_aLayersStructs");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Clears empty logic structs from scenario layer.
	//! \param[in] layer Clears empty logic structs from scenario layer.
	//! \param[in] layerStruct Represents scenario framework layer structure with logic structs.
	void CleanEmptyStoredLogic(notnull SCR_ScenarioFrameworkLayerBase layer, notnull SCR_ScenarioFrameworkLayerStruct layerStruct)
	{
		if (layerStruct.GetLogicStructs())
		{
			if (layerStruct.GetLogicStructs().IsEmpty())
				layerStruct.UnregV("m_aLogicStructs");
		}
		else
		{
			layerStruct.UnregV("m_aLogicStructs");
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Clears empty layer and logic structs in the object.
	void CleanEmptyStructs()
	{
		//Clears empty layer structs which are unnecessary to be saved
		ClearEmptyLayerStructs(m_aLayersStructs);
		
		//Clears empty logic structs which are unnecessary to be saved
		ClearEmptyLogicStructs(m_aLogicStructs);
	
	}
	
	//------------------------------------------------------------------------------------------------
	//! Stores scenario logic state, handles counter and struct variables, and adds to list if struct count is more than 1.
	//! \param[in] logic Stores scenario framework logic state, handles counter values, names, and adds to list if multiple variables exist.
	void StoreLogicState(notnull SCR_ScenarioFrameworkLogic logic)
	{
		SCR_ScenarioFrameworkLogicStruct logicStruct = new SCR_ScenarioFrameworkLogicStruct();
		
		SCR_ScenarioFrameworkLogicCounter logicCounter = SCR_ScenarioFrameworkLogicCounter.Cast(logic);
		if (logicCounter)
		{
			//Checks if layer is named. If it is not, we cannot use it for serialization
			if (logicCounter.GetName())
			{
				logicStruct.IncreaseStructVarCount();
				logicStruct.SetName(logicCounter.GetName());
			}
			else
			{
				delete logicStruct;
				return;
			}
			
			StoreLogicCounterValue(logicCounter, logicStruct);
			StoreLogicCounterTermination(logicCounter, logicStruct);
			
			if (logicStruct && logicStruct.GetStructVarCount() > 1)
			{
				m_aLogicStructs.Insert(logicStruct);
				return;	
			}
		}
		
		delete logicStruct;
	}

	//------------------------------------------------------------------------------------------------
	//! Updates logic struct with counter value if non-zero, otherwise unregisters it.
	//! \param[in] logicCounter Represents a counter value in the scenario logic.
	//! \param[in] logicStruct Represents a struct containing variables and counter value for scenario logic.
	void StoreLogicCounterValue(notnull SCR_ScenarioFrameworkLogicCounter logicCounter, notnull SCR_ScenarioFrameworkLogicStruct logicStruct)
	{
		if (logicCounter.GetCounterValue() == 0)
		{
			logicStruct.UnregV("m_iCounterValue");
		}
		else
		{
			logicStruct.IncreaseStructVarCount();
			logicStruct.SetCounterValue(logicCounter.GetCounterValue());
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Checks if counter is terminated, increments struct var count if terminated, sets termination flag in struct if not
	//! \param[in] logicCounter represents a scenario counter with termination status, used for incrementing struct's termination count if it's terminated
	//! \param[in] logicStruct Represents scenario framework logic structure, tracks termination state.
	void StoreLogicCounterTermination(notnull SCR_ScenarioFrameworkLogicCounter logicCounter, notnull SCR_ScenarioFrameworkLogicStruct logicStruct)
	{
		if (logicCounter.GetIsTerminated())
		{
			logicStruct.IncreaseStructVarCount();
			logicStruct.SetIsTerminated(true);
		}
		else
		{
			logicStruct.UnregV("m_bIsTerminated");
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Removes empty layer structs from an array by recursively checking their contents.
	//! \param[in] layerStructsToClear Array of layer structs to clear empty structures from.
	void ClearEmptyLayerStructs(notnull array<ref SCR_ScenarioFrameworkLayerStruct> layerStructsToClear)
	{
		array<ref SCR_ScenarioFrameworkLayerStruct> layersStructsCopy = {};
		foreach (SCR_ScenarioFrameworkLayerStruct layerStructToCopy : layerStructsToClear)
		{
			layersStructsCopy.Insert(layerStructToCopy);
		}
		
		foreach (SCR_ScenarioFrameworkLayerStruct layerStruct : layersStructsCopy)
		{
			if (layerStruct.GetStructVarCount() < 1)
				layerStructsToClear.RemoveItem(layerStruct);
			else
				ClearEmptyLayerStructs(layerStruct.GetLayerStructs());
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Clears empty logic structs from an array by copying them into another array, then removing those with less than 2
	//! \param[in] logicStructsToClear Array of scenario framework logic structures to clear empty ones from.
	void ClearEmptyLogicStructs(notnull array<ref SCR_ScenarioFrameworkLogicStruct> logicStructsToClear)
	{
		array<ref SCR_ScenarioFrameworkLogicStruct> logicsStructsCopy = {};
		foreach (SCR_ScenarioFrameworkLogicStruct logicStructToCopy : logicStructsToClear)
		{
			logicsStructsCopy.Insert(logicStructToCopy);
		}
		
		foreach (SCR_ScenarioFrameworkLogicStruct logicStruct : logicsStructsCopy)
		{
			if (logicStruct.GetStructVarCount() <= 1)
				logicStructsToClear.RemoveItem(logicStruct);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Increases struct variable count by 1.
	void IncreaseStructVarCount()
	{
		m_iStructVarCount += 1;
	} 
	
	//------------------------------------------------------------------------------------------------
	//! Decreases struct variable count by 1.
	void DecreaseStructVarCount()
	{
		m_iStructVarCount -= 1;
	} 
	
	//------------------------------------------------------------------------------------------------
	//! \return Array of layer structure references.
	array<ref SCR_ScenarioFrameworkLayerStruct> GetLayerStructs()
	{
		return m_aLayersStructs;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Array of logic structs for scenario framework logic.
	array<ref SCR_ScenarioFrameworkLogicStruct> GetLogicStructs()
	{
		return m_aLogicStructs;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Randomly spawned children array.
	array<string> GetRandomlySpawnedChildren()
	{
		return m_aRandomlySpawnedChildren;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return The number of structure variables in the class.
	int GetStructVarCount()
	{
		return m_iStructVarCount;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Represents the activation type for the scenario event action.
	SCR_ScenarioFrameworkEActivationType GetActivationType()
	{
		return m_eActivationType;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return whether the object is terminated or not.
	bool GetIsTerminated()
	{
		return m_bIsTerminated;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Array of AI prefab resources marked for removal.
	ref array<ResourceName> GetAIPrefabsForRemoval()
	{
		return m_aAIPrefabsForRemoval;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return the name stored in the variable m_sName.
	string GetName()
	{
		return m_sName;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Represents current layer task state.
	int GetLayerTaskState()
	{
		return m_iLayerTaskState;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Randomly spawned object ResourceName.
	ResourceName GetRandomlySpawnedObject()
	{
		return m_sRandomlySpawnedObject;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Repetition count for spawning an object.
	int GetRepeatedSpawnNumber()
	{
		return m_iRepeatedSpawnNumber;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Represents whether repeated spawning is enabled or not.
	bool GetEnableRepeatedSpawn()
	{
		return m_bEnableRepeatedSpawn;
	}
	
	//--------------------- Setters:
	
	//------------------------------------------------------------------------------------------------
	//! Adds child to randomly spawned children list.
	//! \param[in] child Randomly spawned child entity added to list.
	void InsertRandomlySpawnedChildren(string child)
	{
		m_aRandomlySpawnedChildren.Insert(child);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] type Sets the activation type for the scenario event action.
	void SetActivationType(SCR_ScenarioFrameworkEActivationType type)
	{
		m_eActivationType = type;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] state  indicating termination status.
	void SetIsTerminated(bool state)
	{
		m_bIsTerminated = state;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] name Sets layer name.
	void SetName(string name)
	{
		m_sName = name;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] state Sets layer task state, controls layer visibility and functionality.
	void SetLayerTaskState(int state)
	{
		m_iLayerTaskState = state;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] arrayForRemoval Array of resource names for AI prefab removal.
	void SetAIPrefabsForRemoval(array<ResourceName> arrayForRemoval)
	{
		m_aAIPrefabsForRemoval = arrayForRemoval;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] name Represents an object to spawn randomly in the world.
	void SetRandomlySpawnedObject(ResourceName name)
	{
		m_sRandomlySpawnedObject = name;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] number Represents the number of repeated spawns for an entity in the game.
	void SetRepeatedSpawnNumber(int number)
	{
		m_iRepeatedSpawnNumber = number;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] value Enables or disables repeated spawning of units in the mission.
	void SetEnableRepeatedSpawn(bool value)
	{
		m_bEnableRepeatedSpawn = value;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Registers variables for scenario framework layer structure.
	void SCR_ScenarioFrameworkLayerStruct()
	{
		RegV("m_sName");
		RegV("m_sRandomlySpawnedObject");
		RegV("m_eActivationType");
		RegV("m_bIsTerminated");
		RegV("m_aLayersStructs");	
		RegV("m_aLogicStructs");
		RegV("m_iRepeatedSpawnNumber");
		RegV("m_aAIPrefabsForRemoval");
		RegV("m_bEnableRepeatedSpawn");	
		RegV("m_aRandomlySpawnedChildren");	
		RegV("m_iLayerTaskState");	
	}
};

class SCR_ScenarioFrameworkLogicStruct : SCR_JsonApiStruct
{
	protected string m_sName;
	protected int m_iCounterValue;
	protected bool m_bIsTerminated; //Marks if this was terminated - either by death or deletion
	
	protected int m_iStructVarCount;
	
	//------------------------------------------------------------------------------------------------
	//! Increases struct variable count by 1.
	void IncreaseStructVarCount()
	{
		m_iStructVarCount += 1;
	} 
	
	//------------------------------------------------------------------------------------------------
	//! \return The number of structure variables in the class.
	int GetStructVarCount()
	{
		return m_iStructVarCount;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return whether the object is terminated or not.
	bool GetIsTerminated()
	{
		return m_bIsTerminated;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return the name stored in the variable m_sName.
	string GetName()
	{
		return m_sName;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return The return value represents the current count value stored in m_iCounterValue variable.
	int GetCounterValue()
	{
		return m_iCounterValue;
	}
	
	//--------------------- Setters:
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] state indicating termination status.
	void SetIsTerminated(bool state)
	{
		m_bIsTerminated = state;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] name Sets layer name.
	void SetName(string name)
	{
		m_sName = name;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] value Sets the counter value for further use in the script.
	void SetCounterValue(int value)
	{
		m_iCounterValue = value;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Registers scenario framework variables: name, termination status, counter value.
	void SCR_ScenarioFrameworkLogicStruct()
	{
		RegV("m_sName");
		RegV("m_bIsTerminated");
		RegV("m_iCounterValue");	
	}
};