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

	[Attribute(defvalue: "5.0", UIWidgets.Slider, params: "1.0 1000.0 0.5", desc: "Radius of the trigger area", category: "Trigger")]
	float m_fAreaRadius;
	
	[Attribute(defvalue: "1", UIWidgets.CheckBox, desc: "Activate the trigger once or everytime the activation condition is true?", category: "Trigger")]
	bool m_bOnce;
	
	[Attribute(desc: "Actions that will be activated when Trigger gets activated", category: "OnActivation")]
	ref array<ref SCR_ScenarioFrameworkActionBase>	m_aTriggerActions;
	
	[Attribute(desc: "Should the dynamic Spawn/Despawn based on distance from observer cameras be enabled?", category: "Activation")]
	bool m_bDynamicDespawn;

	[Attribute(defvalue: "750", desc: "How close at least one observer camera must be in order to trigger spawn", category: "Activation")]
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
	//! \return
	bool GetDynamicDespawnEnabled()
	{
		return m_bDynamicDespawn;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] enabled
	void SetDynamicDespawnEnabled(bool enabled)
	{
		m_bDynamicDespawn = enabled;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	int GetDynamicDespawnRange()
	{
		return m_iDynamicDespawnRange;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] range
	void SetDynamicDespawnRange(int range)
	{
		m_iDynamicDespawnRange = range;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_BaseTask GetTask()
	{
		return m_Task;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_ScenarioFrameworkSlotTask GetSlotTask()
	{
		return m_SlotTask;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] slotTask
	void SetSlotTask(SCR_ScenarioFrameworkSlotTask slotTask)
	{
		m_SlotTask = slotTask;
	}
	
	//------------------------------------------------------------------------------------------------
	override SCR_ScenarioFrameworkLayerTask GetLayerTask()
	{
		return m_LayerTask;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] layerTask
	void SetLayerTask(SCR_ScenarioFrameworkLayerTask layerTask)
	{
		m_LayerTask = layerTask;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	string GetLayerTaskName()
	{
		return m_LayerTask.GetOwner().GetName();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_ESFTaskType GetLayerTaskType()
	{
		return m_LayerTask.GetTaskType();
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	string GetDeliveryPointName()
	{
		return m_sItemDeliveryPointName;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] sDeliveryPointName
	void StoreDeliveryPoint(string sDeliveryPointName)
	{
		m_sItemDeliveryPointName = sDeliveryPointName;
		if (!SCR_TaskDeliver.Cast(m_Task))
			return;

		SCR_TaskDeliver.Cast(m_Task).SetTriggerNameToDeliver(sDeliveryPointName);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] task
	void StoreTaskToArea(SCR_BaseTask task)
	{
		m_Task = task;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	SCR_ScenarioFrameworkLayerTask Create()
	{
		array<SCR_ScenarioFrameworkLayerBase> aSlotsOut = {};
		GetAllSlots(aSlotsOut);
		if (aSlotsOut.IsEmpty())
			return null;

		Math.Randomize(-1);
		SCR_ScenarioFrameworkLayerBase layer = SCR_ScenarioFrameworkLayerBase.Cast(aSlotsOut.GetRandomElement());
		if (layer)
			layer.Init(this);

		return SCR_ScenarioFrameworkLayerTask.Cast(layer);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] eTaskType
	//! \return
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
	//!
	//! \param[in] layerTask
	void Create(SCR_ScenarioFrameworkLayerTask layerTask)
	{
		if (!layerTask)
			return;
		
		m_LayerTask = layerTask;
		m_LayerTask.Init(this, SCR_ScenarioFrameworkEActivationType.ON_TASKS_INIT);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_BaseTriggerEntity GetTrigger()
	{
		return m_Trigger;
	}

	//------------------------------------------------------------------------------------------------
	//!
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

		//m_Trigger.Show(false);
		m_Trigger.SetSphereRadius(m_fAreaRadius);
		m_Trigger.GetOnActivate().Insert(OnAreaTriggerActivated);
		
		SCR_CharacterTriggerEntity characterTrigger = SCR_CharacterTriggerEntity.Cast(m_Trigger);
		if (characterTrigger)
			characterTrigger.SetOnce(m_bOnce);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] task
	void MoveTaskIconToArea(notnull SCR_BaseTask task)
	{
		task.SetOrigin(GetOwner().GetOrigin());

	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] eTaskType
	//! \return
	bool GetIsTaskSuitableForArea(SCR_ESFTaskType eTaskType)
	{
		array<SCR_ESFTaskType> aTaskTypes = {};
		GetAvailableTaskTypes(aTaskTypes);
		if (aTaskTypes.IsEmpty())
			return false;

		return aTaskTypes.Find(eTaskType) != -1;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[out] aTaskTypes
	void GetAvailableTaskTypes(out array<SCR_ESFTaskType> aTaskTypes)
	{
		array<SCR_ScenarioFrameworkLayerBase> aSlots = {};
		GetAllSlots(aSlots);
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
	//! \param[out] aSlotsOut
	//! \param[in] eTaskType
	void GetSuitableLayersForTaskType(out notnull array<SCR_ScenarioFrameworkLayerBase> aSlotsOut, SCR_ESFTaskType eTaskType)
	{
		SCR_ESFTaskType eType;
		array<SCR_ScenarioFrameworkLayerBase> aSlots = {};
		GetAllSlots(aSlots);
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
	//! \param[out] aSlots
	void GetAllSlots(out array<SCR_ScenarioFrameworkLayerBase> aSlots)
	{
		SCR_ScenarioFrameworkLayerBase slotComponent;
		IEntity child = GetOwner().GetChildren();
		while (child)
		{
			slotComponent = SCR_ScenarioFrameworkLayerBase.Cast(child.FindComponent(SCR_ScenarioFrameworkLayerBase));
			if (slotComponent)
			{
				aSlots.Insert(slotComponent);
			}
			child = child.GetSibling();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[out] aLayerTasks
	void GetAllLayerTasks(out notnull array<SCR_ScenarioFrameworkLayerTask> aLayerTasks)
	{
		aLayerTasks = {};
		SCR_ScenarioFrameworkLayerTask layerTask;
		IEntity child = GetOwner().GetChildren();
		while (child)
		{
			layerTask = SCR_ScenarioFrameworkLayerTask.Cast(child.FindComponent(SCR_ScenarioFrameworkLayerTask));
			if (layerTask)
				aLayerTasks.Insert(layerTask);
			
			child = child.GetSibling();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] bSet
	void SetAreaSelected(bool bSet)
	{
		 m_bAreaSelected = bSet;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	bool GetIsAreaSelected()
	{
		return m_bAreaSelected;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] entity
	void OnAreaTriggerActivated(IEntity entity)
	{
		if (m_OnTriggerActivated)
		{
			m_OnTriggerActivated.Invoke(this, SCR_ScenarioFrameworkEActivationType.ON_AREA_TRIGGER_ACTIVATION, false);
			m_OnTriggerActivated.Clear();
			if (!m_bOnce)
				return;
			
			m_Trigger.Deactivate();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	ScriptInvoker GetOnAreaTriggerActivated()
	{
		if (!m_OnTriggerActivated)
			m_OnTriggerActivated = new ScriptInvoker<SCR_ScenarioFrameworkArea, SCR_ScenarioFrameworkEActivationType>();

		return m_OnTriggerActivated;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	ScriptInvoker GetOnAreaInit()
	{	
		return m_OnAreaInit;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	override void RestoreToDefault(bool includeChildren = false, bool reinitAfterRestoration = false)
	{
		m_Trigger = null;
		m_sItemDeliveryPointName = "";
		m_LayerTask = null;
		m_SlotTask = null;
		
		super.RestoreToDefault(includeChildren, reinitAfterRestoration);
	}
	
	//------------------------------------------------------------------------------------------------
	override void DynamicReinit()
	{
		Init();
	}
	
	//------------------------------------------------------------------------------------------------
	override void DynamicDespawn(SCR_ScenarioFrameworkLayerBase layer)
	{
		if (m_bExcludeFromDynamicDespawn)
			return;
		
		m_bInitiated = false;
		m_bDynamicallyDespawned = true;
		foreach (SCR_ScenarioFrameworkLayerBase child : m_aChildren)
		{
			child.DynamicDespawn(this);
		}
		
		m_aChildren.Clear();
	}

	//------------------------------------------------------------------------------------------------
	override void Init(SCR_ScenarioFrameworkArea area = null, SCR_ScenarioFrameworkEActivationType activation = SCR_ScenarioFrameworkEActivationType.SAME_AS_PARENT)
	{
		if (m_bInitiated || m_bIsTerminated)
			return;
		
		foreach (SCR_ScenarioFrameworkActivationConditionBase activationCondition : m_aActivationConditions)
		{
			//If just one condition is false, we don't continue and interrupt the init
			if (!activationCondition.Init(GetOwner()))
			{
				InvokeAllChildrenSpawned();
				return;
			}
		}

		if (m_eActivationType != SCR_ScenarioFrameworkEActivationType.ON_INIT)
			PrintFormat("ScenarioFramework: Area %1 is set to %2 activation type, but area will always spawn on Init as default", GetOwner().GetName(), activation, LogLevel.WARNING);

		if (!m_Trigger)
			SpawnTrigger();
		
		// Area is always spawned on the start
		super.Init(this, SCR_ScenarioFrameworkEActivationType.ON_INIT);
		
		if (m_Trigger)
		{
			foreach (SCR_ScenarioFrameworkActionBase triggerAction : m_aTriggerActions)
			{
				triggerAction.Init(m_Trigger);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gameMode)
			return;

		SCR_GameModeSFManager gameModeManager = SCR_GameModeSFManager.Cast(gameMode.FindComponent(SCR_GameModeSFManager));
		if (gameModeManager)
			gameModeManager.RegisterArea(this);
	}

	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
		
		super.OnPostInit(owner);
	}
	
	//------------------------------------------------------------------------------------------------
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
	override void _WB_AfterWorldUpdate(IEntity owner, float timeSlice)
	{
		DrawDebugShape(m_bShowDebugShapesInWorkbench);
	}
	
	//------------------------------------------------------------------------------------------------
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
