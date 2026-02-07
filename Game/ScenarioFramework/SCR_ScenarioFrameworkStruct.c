//------------------------------------------------------------------------------------------------
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
	override bool Serialize()
	{
		SCR_GameModeSFManager manager = SCR_GameModeSFManager.Cast(GetGame().GetGameMode().FindComponent(SCR_GameModeSFManager));
		if (!manager)
			return false;
		
		m_bMatchOver = manager.GetIsMatchOver();
		
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
		
		m_eGameOverType = manager.m_eGameOverType;
		StoreAreaStates(manager, m_aAreasStructs);
		ClearEmptyAreaStructs(m_aAreasStructs);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool Deserialize()
	{
		if (m_aAreasStructs.IsEmpty())
			return false;
		
		SCR_GameModeSFManager manager = SCR_GameModeSFManager.Cast(GetGame().GetGameMode().FindComponent(SCR_GameModeSFManager));
		if (!manager)
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
		
		manager.m_eGameOverType = m_eGameOverType;
		LoadAreaStates(manager, m_aAreasStructs);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[out] outEntries
	void StoreAreaStates(notnull SCR_GameModeSFManager manager, out notnull array<ref SCR_ScenarioFrameworkAreaStruct> outEntries)
	{
		if (!manager.m_aAreas)
			return;
		
		SCR_ScenarioFrameworkAreaStruct struct;
		for (int i = manager.m_aAreas.Count() - 1; i >= 0; i--)
		{
			struct = new SCR_ScenarioFrameworkAreaStruct();
			struct.StoreState(manager.m_aAreas[i], struct);
			outEntries.Insert(struct);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] loadedAreaStruct
	void LoadAreaStates(notnull SCR_GameModeSFManager manager, notnull array<ref SCR_ScenarioFrameworkAreaStruct> loadedAreaStruct)
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
			
			LoadAreaStructs(manager, area, areaStruct);
			LoadNestedAreaStructs(areaStruct);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void LoadAreaStructs(notnull SCR_GameModeSFManager manager, notnull SCR_ScenarioFrameworkArea area, notnull SCR_ScenarioFrameworkAreaStruct areaStruct)
	{
		if (areaStruct.GetAreaSelected())
		{
			manager.m_aAreasTasksToSpawn.Insert(areaStruct.GetName());
			manager.m_aLayersTaskToSpawn.Insert(areaStruct.GetLayerTaskname());
		}
		
		if (areaStruct.GetDeliveryPointNameForItem())
			area.StoreDeliveryPoint(areaStruct.GetDeliveryPointNameForItem());
			
		if (areaStruct.GetRandomlySpawnedChildren())
			area.SetRandomlySpawnedChildren(areaStruct.GetRandomlySpawnedChildren());
			
		LoadRepeatedSpawnAreaStructs(area, areaStruct);
		
	}
	
	//------------------------------------------------------------------------------------------------
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
	protected void LoadNestedAreaStructs(notnull SCR_ScenarioFrameworkAreaStruct areaStruct)
	{
		if (areaStruct.GetLayerStructs())
			LoadLayer(areaStruct.GetLayerStructs());
			
		if (areaStruct.GetLogicStructs())
			LoadLogic(areaStruct.GetLogicStructs());
	}
	
	//------------------------------------------------------------------------------------------------
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
	protected void LoadRepeatedSpawnLayerStructs(notnull SCR_ScenarioFrameworkLayerBase layer, notnull  SCR_ScenarioFrameworkLayerStruct layerStruct)
	{
		if (!layerStruct.GetEnableRepeatedSpawn())
			return;
		
		layer.SetEnableRepeatedSpawn(true);
		layer.SetRepeatedSpawnNumber(layerStruct.GetRepeatedSpawnNumber());
	}
	
	//------------------------------------------------------------------------------------------------
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
	protected void LoadNestedLayerStructs(notnull SCR_ScenarioFrameworkLayerStruct layerStruct)
	{
		if (layerStruct.GetLayerStructs())
			LoadLayer(layerStruct.GetLayerStructs());
			
		if (layerStruct.GetLogicStructs())
			LoadLogic(layerStruct.GetLogicStructs());
	}
	
	//------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------
class SCR_ScenarioFrameworkAreaStruct : SCR_ScenarioFrameworkLayerStruct
{
	protected bool m_bAreaSelected;
	protected string m_sItemDeliveryPointName;
	protected string m_sLayerTaskName;
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[out] areaStruct
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
	//! Handles area selection
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
	//! Delivery point handling
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
	//! Stores the Activation type
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
	//! Marks if this was terminated - either by death or deletion
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
	//! Repeated spawn handling
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
	//! Layer Task handling
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
	//! Children handling
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
	//! Logics handling
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
	//! //Cleaning empty layers
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
	//! //Cleaning empty layers
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
	//! //Cleaning empty structs that are unnecessary to be saved and removing other variables that are there due to inheritance
	protected void CleanAreaStructs(notnull SCR_ScenarioFrameworkAreaStruct areaStruct)
	{
		areaStruct.ClearEmptyLayerStructs(areaStruct.GetLayerStructs());
		areaStruct.ClearEmptyLogicStructs(areaStruct.GetLogicStructs());

		areaStruct.UnregV("m_aAIPrefabsForRemoval");
		areaStruct.UnregV("m_sRandomlySpawnedObject");
		areaStruct.UnregV("m_iLayerTaskState");
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetAreaSelected()
	{
		return m_bAreaSelected;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetDeliveryPointNameForItem()
	{
		return m_sItemDeliveryPointName;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetLayerTaskname()
	{
		return m_sLayerTaskName;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetAreaSelected(bool selected)
	{
		m_bAreaSelected = selected;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetDeliveryPointNameForItem(string name)
	{
		m_sItemDeliveryPointName = name;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetLayerTaskName(string layerTaskName)
	{
		m_sLayerTaskName = layerTaskName;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_ScenarioFrameworkAreaStruct()
	{
		RegV("m_bAreaSelected");
		RegV("m_sLayerTaskName");
		RegV("m_sItemDeliveryPointName");
	}
};

//------------------------------------------------------------------------------------------------
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
	//! Marks if this was terminated - either by death or deletion
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
	//! Marks if this was terminated - either by death or deletion
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
	//! Repeated spawn handling
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
	//! Layer Task handling
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
	//! Children handling
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
	//! Logics handling
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
	//! Logics handling
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
	//! //Cleaning empty layers
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
	//! //Cleaning empty Logic
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
	void CleanEmptyStructs()
	{
		//Clears empty layer structs which are unnecessary to be saved
		ClearEmptyLayerStructs(m_aLayersStructs);
		
		//Clears empty logic structs which are unnecessary to be saved
		ClearEmptyLogicStructs(m_aLogicStructs);
	
	}
	
	//------------------------------------------------------------------------------------------------
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
	//! Handling logic counter value
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
	//! Marks if this was terminated - either by death or deletion
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
	void IncreaseStructVarCount()
	{
		m_iStructVarCount += 1;
	} 
	
	//------------------------------------------------------------------------------------------------
	void DecreaseStructVarCount()
	{
		m_iStructVarCount -= 1;
	} 
	
	//------------------------------------------------------------------------------------------------
	array<ref SCR_ScenarioFrameworkLayerStruct> GetLayerStructs()
	{
		return m_aLayersStructs;
	}
	
	//------------------------------------------------------------------------------------------------
	array<ref SCR_ScenarioFrameworkLogicStruct> GetLogicStructs()
	{
		return m_aLogicStructs;
	}
	
	//------------------------------------------------------------------------------------------------
	array<string> GetRandomlySpawnedChildren()
	{
		return m_aRandomlySpawnedChildren;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetStructVarCount()
	{
		return m_iStructVarCount;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_ScenarioFrameworkEActivationType GetActivationType()
	{
		return m_eActivationType;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetIsTerminated()
	{
		return m_bIsTerminated;
	}
	
	//------------------------------------------------------------------------------------------------
	ref array<ResourceName> GetAIPrefabsForRemoval()
	{
		return m_aAIPrefabsForRemoval;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetName()
	{
		return m_sName;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetLayerTaskState()
	{
		return m_iLayerTaskState;
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetRandomlySpawnedObject()
	{
		return m_sRandomlySpawnedObject;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetRepeatedSpawnNumber()
	{
		return m_iRepeatedSpawnNumber;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetEnableRepeatedSpawn()
	{
		return m_bEnableRepeatedSpawn;
	}
	
	//--------------------- Setters:
	
	//------------------------------------------------------------------------------------------------
	void InsertRandomlySpawnedChildren(string child)
	{
		m_aRandomlySpawnedChildren.Insert(child);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetActivationType(SCR_ScenarioFrameworkEActivationType type)
	{
		m_eActivationType = type;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetIsTerminated(bool state)
	{
		m_bIsTerminated = state;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetName(string name)
	{
		m_sName = name;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetLayerTaskState(int state)
	{
		m_iLayerTaskState = state;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetAIPrefabsForRemoval(array<ResourceName> arrayForRemoval)
	{
		m_aAIPrefabsForRemoval = arrayForRemoval;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetRandomlySpawnedObject(ResourceName name)
	{
		m_sRandomlySpawnedObject = name;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetRepeatedSpawnNumber(int number)
	{
		m_iRepeatedSpawnNumber = number;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetEnableRepeatedSpawn(bool value)
	{
		m_bEnableRepeatedSpawn = value;
	}
	
	//------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------------------------
class SCR_ScenarioFrameworkLogicStruct : SCR_JsonApiStruct
{
	protected string m_sName;
	protected int m_iCounterValue;
	protected bool m_bIsTerminated; //Marks if this was terminated - either by death or deletion
	
	protected int m_iStructVarCount;
	
	//------------------------------------------------------------------------------------------------
	void IncreaseStructVarCount()
	{
		m_iStructVarCount += 1;
	} 
	
	//------------------------------------------------------------------------------------------------
	int GetStructVarCount()
	{
		return m_iStructVarCount;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetIsTerminated()
	{
		return m_bIsTerminated;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetName()
	{
		return m_sName;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetCounterValue()
	{
		return m_iCounterValue;
	}
	
	//--------------------- Setters:
	
	//------------------------------------------------------------------------------------------------
	void SetIsTerminated(bool state)
	{
		m_bIsTerminated = state;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetName(string name)
	{
		m_sName = name;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetCounterValue(int value)
	{
		m_iCounterValue = value;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_ScenarioFrameworkLogicStruct()
	{
		RegV("m_sName");
		RegV("m_bIsTerminated");
		RegV("m_iCounterValue");	
	}
};