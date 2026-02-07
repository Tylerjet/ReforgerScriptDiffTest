[EntityEditorProps(category: "GameScripted/ScenarioFramework", description: "")]
class SCR_ScenarioFrameworkAreaClass : SCR_ScenarioFrameworkLayerBaseClass
{
}

// Helper class for designer to specify what tasks will be available in this area
[BaseContainerProps()]
class SCR_ScenarioFrameworkTaskType
{
	[Attribute("Type of task", UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_ESFTaskType))]
	protected SCR_ESFTaskType m_eTypeOfTask;

	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_ESFTaskType GetTaskType()
	{
		return m_eTypeOfTask;
	}
}

//! Class is managing the area including the Slots (prefab Slot.et).
//! The Slots should be placed in its hierarchy.
//! GameModePatrolManager will select one instance of this type and will start to populate it based on the selected task
class SCR_ScenarioFrameworkArea : SCR_ScenarioFrameworkLayerBase
{
	[Attribute(uiwidget: UIWidgets.ResourcePickerThumbnail, "Trigger for area", category: "Trigger")]
	ResourceName m_sTriggerResource;

	[Attribute(defvalue: "5.0", UIWidgets.Slider, params: "0 inf", desc: "Radius of the trigger area", category: "Trigger")]
	float m_fAreaRadius;
	
	[Attribute(defvalue: "1", UIWidgets.CheckBox, desc: "Activate the trigger once or everytime the activation condition is true?", category: "Trigger")]
	bool m_bOnce;
	
	[Attribute(desc: "Actions that will be activated when Trigger gets activated", category: "OnActivation")]
	ref array<ref SCR_ScenarioFrameworkActionBase>	m_aTriggerActions;
	
	[Attribute(desc: "Should the dynamic Spawn/Despawn based on distance from observer cameras be enabled?", category: "Activation")]
	bool m_bDynamicDespawn;

	[Attribute(defvalue: "750", params: "0 inf", desc: "How close at least one observer camera must be in order to trigger spawn", category: "Activation")]
	int m_iDynamicDespawnRange;

	SCR_BaseTriggerEntity m_Trigger;
	ref ScriptInvoker<SCR_ScenarioFrameworkArea, SCR_ScenarioFrameworkEActivationType>	m_OnTriggerActivated;
	ref ScriptInvoker m_OnAreaInit = new ScriptInvoker();
	bool m_bAreaSelected;
	SCR_BaseTask m_Task;
	string m_sItemDeliveryPointName;
	SCR_ScenarioFrameworkLayerTask m_LayerTask;
	SCR_ScenarioFrameworkSlotTask m_SlotTask; 				//storing this one in order to get the task title and description

	[Attribute(defvalue: "0", desc: "Show the debug shapes in Workbench", category: "Debug")]
	protected bool m_bShowDebugShapesInWorkbench;

	//------------------------------------------------------------------------------------------------
	//! \return Indicates whether dynamic despawn is enabled or not.
	bool GetDynamicDespawnEnabled()
	{
		return m_bDynamicDespawn;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] enabled Enables or disables dynamic despawning.
	void SetDynamicDespawnEnabled(bool enabled)
	{
		m_bDynamicDespawn = enabled;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Represents dynamic despawn range.
	int GetDynamicDespawnRange()
	{
		return m_iDynamicDespawnRange;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] range Represents distance in meters for dynamic despawning.
	void SetDynamicDespawnRange(int range)
	{
		m_iDynamicDespawnRange = range;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return the current task assigned to the area.
	SCR_BaseTask GetTask()
	{
		return m_Task;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return slot task associated with this scenario framework area.
	SCR_ScenarioFrameworkSlotTask GetSlotTask()
	{
		return m_SlotTask;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] slotTask associated with this scenario framework area.
	void SetSlotTask(SCR_ScenarioFrameworkSlotTask slotTask)
	{
		m_SlotTask = slotTask;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return the layer task associated with this scenario framework area.
	override SCR_ScenarioFrameworkLayerTask GetLayerTask()
	{
		return m_LayerTask;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] layerTask associated with this scenario framework area.
	void SetLayerTask(SCR_ScenarioFrameworkLayerTask layerTask)
	{
		m_LayerTask = layerTask;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return the name of the owner entity of the layer task.
	string GetLayerTaskName()
	{
		return m_LayerTask.GetOwner().GetName();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return the task type associated with the layer task.
	SCR_ESFTaskType GetLayerTaskType()
	{
		return m_LayerTask.GetTaskType();
	}

	//------------------------------------------------------------------------------------------------
	//! \return the name of the delivery point for an item.
	string GetDeliveryPointName()
	{
		return m_sItemDeliveryPointName;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets delivery point name for task delivery and triggers it with given name.
	//! \param[in] sDeliveryPointName Represents the name of the delivery point for an item in the mission.
	void StoreDeliveryPoint(string sDeliveryPointName)
	{
		m_sItemDeliveryPointName = sDeliveryPointName;
		if (!SCR_TaskDeliver.Cast(m_Task))
			return;

		SCR_TaskDeliver.Cast(m_Task).SetTriggerNameToDeliver(sDeliveryPointName);
	}

	//------------------------------------------------------------------------------------------------
	//! Store Task in area.
	//! \param[in] task
	void StoreTaskToArea(SCR_BaseTask task)
	{
		m_Task = task;
	}

	//------------------------------------------------------------------------------------------------
	//! Randomly selects an available scenario layer from all available slots and initializes it.
	//! \return Randomly selected Layer Task
	SCR_ScenarioFrameworkLayerTask Create()
	{
		array<SCR_ScenarioFrameworkLayerBase> aSlotsOut = {};
		GetChildren(aSlotsOut);
		if (aSlotsOut.IsEmpty())
			return null;

		Math.Randomize(-1);
		SCR_ScenarioFrameworkLayerBase layer = aSlotsOut.GetRandomElement();
		if (layer)
			layer.Init(this);

		return SCR_ScenarioFrameworkLayerTask.Cast(layer);
	}

	//------------------------------------------------------------------------------------------------
	//! Creates a suitable layer for a given task type, initializes it, and returns it.
	//! \param[in] eTaskType eTaskType represents the type of task for which suitable layers are searched in the method.
	//! \return a random suitable layer task for the given task type.
	SCR_ScenarioFrameworkLayerTask Create(SCR_ESFTaskType eTaskType)
	{
		array<SCR_ScenarioFrameworkLayerBase> aSlotsOut = {};
		GetSuitableLayersForTaskType(aSlotsOut, eTaskType);
		if (aSlotsOut.IsEmpty())
			return null;

		//there might be more layers in the area conforming to the task type (i.e. 2x the Truck task)
		Math.Randomize(-1);
		m_LayerTask = SCR_ScenarioFrameworkLayerTask.Cast(aSlotsOut.GetRandomElement());
		if (m_LayerTask)
			m_LayerTask.Init(this, SCR_ScenarioFrameworkEActivationType.ON_TASKS_INIT);

		return m_LayerTask;
	}

	//------------------------------------------------------------------------------------------------
	//! Initializes layer task with random spawned child slot task on tasks init.
	//! \param[in] layerTask LayerTask represents the parent task for spawning child tasks randomly in the scenario.
	//! \param[in] slotTask SlotTask represents a task within a layer, which can be added randomly to the layer by the method.
	void Create(SCR_ScenarioFrameworkLayerTask layerTask, SCR_ScenarioFrameworkSlotTask slotTask = null)
	{
		if (!layerTask)
			return;
		
		m_LayerTask = layerTask;
		m_LayerTask.AddRandomlySpawnedChild(slotTask);
		m_LayerTask.Init(this, SCR_ScenarioFrameworkEActivationType.ON_TASKS_INIT);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Represents the trigger entity associated with this area.
	SCR_BaseTriggerEntity GetTrigger()
	{
		return m_Trigger;
	}

	//------------------------------------------------------------------------------------------------
	//! Spawns a trigger entity with specified resource, sets its radius, and attaches an activation event.
	void SpawnTrigger()
	{
		Resource resource = Resource.Load(m_sTriggerResource);
		if (!resource)
			return;

		EntitySpawnParams spawnParams = new EntitySpawnParams();
		spawnParams.TransformMode = ETransformMode.WORLD;
		GetOwner().GetWorldTransform(spawnParams.Transform);

		//--- Apply rotation
		vector angles = Math3D.MatrixToAngles(spawnParams.Transform);
		Math3D.AnglesToMatrix(angles, spawnParams.Transform);


		//--- Spawn the prefab
		BaseResourceObject resourceObject = resource.GetResource();
		if (!resourceObject)
			return;

		string resourceName = resourceObject.GetResourceName();
		m_Trigger = SCR_BaseTriggerEntity.Cast(GetGame().SpawnEntityPrefab(resource, GetGame().GetWorld(), spawnParams));
		if (!m_Trigger)
			return;

		m_Trigger.SetSphereRadius(m_fAreaRadius);
		m_Trigger.GetOnActivate().Insert(OnAreaTriggerActivated);
		
		SCR_ScenarioFrameworkTriggerEntity characterTrigger = SCR_ScenarioFrameworkTriggerEntity.Cast(m_Trigger);
		if (characterTrigger)
			characterTrigger.SetOnce(m_bOnce);
		
		if (m_Trigger)
		{
			foreach (SCR_ScenarioFrameworkActionBase triggerAction : m_aTriggerActions)
			{
				triggerAction.Init(m_Trigger);
	}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Moves task icon to area position.
	//! \param[in] task
	void MoveTaskIconToArea(notnull SCR_BaseTask task)
	{
		task.SetOrigin(GetOwner().GetOrigin());

	}

	//------------------------------------------------------------------------------------------------
	//! Checks if given task type is suitable for current area by checking if it's in available task types list.
	//! \param[in] eTaskType Task type enum representing possible tasks in the area.
	//! \return true if the given task type is suitable for the current area, false otherwise.
	bool GetIsTaskSuitableForArea(SCR_ESFTaskType eTaskType)
	{
		array<SCR_ESFTaskType> aTaskTypes = {};
		GetAvailableTaskTypes(aTaskTypes);
		if (aTaskTypes.IsEmpty())
			return false;

		return aTaskTypes.Find(eTaskType) != -1;
	}

	//------------------------------------------------------------------------------------------------
	//! Gets available task types from scenario layers and adds them to an array.
	//! \param[out] aTaskTypes The array of available task types for the scenario framework layer.
	void GetAvailableTaskTypes(out array<SCR_ESFTaskType> aTaskTypes)
	{
		array<SCR_ScenarioFrameworkLayerBase> aSlots = {};
		GetChildren(aSlots);
		SCR_ESFTaskType eType;
		SCR_ScenarioFrameworkLayerTask pos;
		foreach (SCR_ScenarioFrameworkLayerBase layer : aSlots)
		{
			pos = SCR_ScenarioFrameworkLayerTask.Cast(layer);
			if (!pos)
				continue;

			eType = pos.GetTaskType();
			if (aTaskTypes.Find(eType) == -1)
				aTaskTypes.Insert(eType);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Filters suitable layers for given task type, populates output array with matching layers.
	//! \param[out] aSlotsOut Array of suitable layers for given task type.
	//! \param[in] eTaskType eTaskType is the task type parameter used to filter suitable layers for the given task type in the scenario framework.
	void GetSuitableLayersForTaskType(out notnull array<SCR_ScenarioFrameworkLayerBase> aSlotsOut, SCR_ESFTaskType eTaskType)
	{
		SCR_ESFTaskType eType;
		array<SCR_ScenarioFrameworkLayerBase> aSlots = {};
		GetChildren(aSlots);
		SCR_ScenarioFrameworkLayerTask pos;
		foreach (SCR_ScenarioFrameworkLayerBase layer : aSlots)
		{
			pos = SCR_ScenarioFrameworkLayerTask.Cast(layer);
			if (!pos)
				continue;

			eType = pos.GetTaskType();
			if (eTaskType == eType)
				aSlotsOut.Insert(pos);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] bSet is a boolean parameter indicating whether to set or unset the area selection.
	void SetAreaSelected(bool bSet)
	{
		 m_bAreaSelected = bSet;
	}

	//------------------------------------------------------------------------------------------------
	//! \return whether an area is selected or not.
	bool GetIsAreaSelected()
	{
		return m_bAreaSelected;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] entity Triggers area activation event for scenario framework, enabling periodic queries if not already done once.
	void OnAreaTriggerActivated(IEntity entity)
	{
		if (m_OnTriggerActivated)
		{
			m_OnTriggerActivated.Invoke(this, SCR_ScenarioFrameworkEActivationType.ON_AREA_TRIGGER_ACTIVATION, false);
			m_OnTriggerActivated.Clear();
			if (!m_bOnce)
				return;
			
			m_Trigger.EnablePeriodicQueries(false);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \return a ScriptInvoker object for handling area trigger activation events.
	ScriptInvoker GetOnAreaTriggerActivated()
	{
		if (!m_OnTriggerActivated)
			m_OnTriggerActivated = new ScriptInvoker<SCR_ScenarioFrameworkArea, SCR_ScenarioFrameworkEActivationType>();

		return m_OnTriggerActivated;
	}

	//------------------------------------------------------------------------------------------------
	//! \return the event handler for area initialization.
	ScriptInvoker GetOnAreaInit()
	{	
		return m_OnAreaInit;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Restores default settings, deselects area, nullifies triggers, resets item delivery point, and calls base class
	//! \param[in] includeChildren Restores default settings, optionally including children objects.
	//! \param[in] reinitAfterRestoration Restores object state, optionally reinitializes after restoration.
	//! \param[in] affectRandomization Affects randomization settings during restoration.
	override void RestoreToDefault(bool includeChildren = false, bool reinitAfterRestoration = false, bool affectRandomization = true)
	{
		m_bAreaSelected = false;
		m_Trigger = null;
		m_sItemDeliveryPointName = "";
		m_LayerTask = null;
		m_SlotTask = null;
		
		super.RestoreToDefault(includeChildren, reinitAfterRestoration, affectRandomization);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Initializes dynamic reinitialization by calling Init() method.
	override void DynamicReinit()
	{
		Init();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Removes all children layers from the scenario framework layer and despawns itself if not excluded from dynamic despawning.
	//! \param[in] layer Removes layer from dynamic despawning, clears children layers.
	override void DynamicDespawn(SCR_ScenarioFrameworkLayerBase layer)
	{
		if (m_bExcludeFromDynamicDespawn)
			return;
		
		m_bInitiated = false;
		m_bDynamicallyDespawned = true;
		m_aChildren.RemoveItem(null);
		foreach (SCR_ScenarioFrameworkLayerBase child : m_aChildren)
		{
			child.DynamicDespawn(this);
		}
		
		m_aChildren.Clear();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Initializes parent layer with true value.
	//! \return true if parent layer initialization is successful.
	override bool InitParentLayer()
	{
		// Areas do not have parents
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Initializes area, spawns trigger if not already spawned, initializes trigger actions if trigger is present, and sets activation type
	//! \param[in] area parent
	//! \param[in] activation Activation type determines how scenario area is initiated, either on init or based on specific conditions.
	override void Init(SCR_ScenarioFrameworkArea area = null, SCR_ScenarioFrameworkEActivationType activation = SCR_ScenarioFrameworkEActivationType.SAME_AS_PARENT)
	{
		if (m_eActivationType != SCR_ScenarioFrameworkEActivationType.ON_INIT)
			PrintFormat("ScenarioFramework: Area %1 is set to %2 activation type, but area will always spawn on Init as default", GetOwner().GetName(), activation, LogLevel.WARNING);

		// Area is always spawned on the start
		super.Init(this, SCR_ScenarioFrameworkEActivationType.ON_INIT);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Initializes Trigger
	//! \param[in] layer for which this method is called.
	override void AfterAllChildrenSpawned(SCR_ScenarioFrameworkLayerBase layer)
		{
		super.AfterAllChildrenSpawned(layer);
		
		if (!SCR_StringHelper.IsEmptyOrWhiteSpace(m_sTriggerResource))
			SpawnTrigger();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Initializes area registration with ScenarioFrameworkSystem when game mode is valid.
	//! \param[in] owner The owner represents the entity that initializes this script on its EOnInit event.
	override void EOnInit(IEntity owner)
	{
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gameMode)
			return;

		SCR_ScenarioFrameworkSystem scenarioFrameworkSystem = SCR_ScenarioFrameworkSystem.GetInstance();
		if (scenarioFrameworkSystem)
			scenarioFrameworkSystem.RegisterArea(this);
		
		array<SCR_ScenarioFrameworkLayerBase> children = {};
		GetAllLayers(children, SCR_ScenarioFrameworkEActivationType.ON_INIT);
		
		foreach (SCR_ScenarioFrameworkLayerBase child : children)
		{
			child.Init(this, SCR_ScenarioFrameworkEActivationType.ON_INIT);
			child.SetActivationType(SCR_ScenarioFrameworkEActivationType.SAME_AS_PARENT);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] owner The owner represents the entity being initialized in the method.
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
		
		super.OnPostInit(owner);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Draws debug shapes for debugging
	//! \param[in] draw debug shape for the object if draw parameter is true, otherwise does not draw anything.
	override protected void DrawDebugShape(bool draw)
	{
		Shape dbgShape = null;
		if (!draw)
			return;

		dbgShape = Shape.CreateSphere(
										m_iDebugShapeColor,
										ShapeFlags.TRANSP | ShapeFlags.DOUBLESIDE | ShapeFlags.NOZWRITE | ShapeFlags.ONCE | ShapeFlags.NOOUTLINE,
										GetOwner().GetOrigin(),
										m_fDebugShapeRadius
								);
		
		if (m_sTriggerResource.IsEmpty())
			return;
		
		Shape triggerdbgShape = null;
		triggerdbgShape = Shape.CreateSphere(
										ARGB(100, 0x99, 0x10, 0xF2),
										ShapeFlags.TRANSP | ShapeFlags.DOUBLESIDE | ShapeFlags.NOZWRITE | ShapeFlags.ONCE | ShapeFlags.NOOUTLINE,
										GetOwner().GetOrigin(),
										m_fAreaRadius
								);
	}
	
#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	//! Draws debug shape based on m_bShowDebugShapeInWorkbench setting in Workbench after world update.
	//! \param[in] owner The owner represents the entity (object) calling the method.
	//! \param[in] timeSlice represents the time interval for which the method is called during each frame update.
	override void _WB_AfterWorldUpdate(IEntity owner, float timeSlice)
	{
		DrawDebugShape(m_bShowDebugShapesInWorkbench);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Controls debug shape visibility in Workbench based on user input.
	//! \param[in] owner The owner represents the entity (object) in the game world that triggers the method when its key value changes.
	//! \param[in] src BaseContainer src represents input parameter containing key-value pairs related to the key changes in the Workbench.
	//! \param[in] key Controls whether debug shapes are drawn in Workbench.
	//! \param[in] ownerContainers Represents a list of containers related to the owner entity in the method.
	//! \param[in] parent Parent represents the entity that owns the key change event, used for context in the method.
	//! \return: Controls whether debug shapes are drawn in Workbench.
	override bool _WB_OnKeyChanged(IEntity owner, BaseContainer src, string key, BaseContainerList ownerContainers, IEntity parent)
	{
		if (key == "m_bShowDebugShapesInWorkbench")
			DrawDebugShape(m_bShowDebugShapesInWorkbench);
		
		return false;
	}
#endif	
	
	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_ScenarioFrameworkArea(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_fDebugShapeRadius = m_iDynamicDespawnRange;
#ifdef WORKBENCH	
		m_iDebugShapeColor = ARGB(32, 0x99, 0xF3, 0x12);
#endif		
	}
}
