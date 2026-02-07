[EntityEditorProps(category: "GameScripted/ScenarioFramework/Slot", description: "")]
class SCR_ScenarioFrameworkSlotClearAreaClass : SCR_ScenarioFrameworkSlotTaskClass
{
}

class SCR_ScenarioFrameworkSlotClearArea : SCR_ScenarioFrameworkSlotTask
{
	//------------------------------------------------------------------------------------------------
	override void Init(SCR_ScenarioFrameworkArea area = null, SCR_ScenarioFrameworkEActivationType activation = SCR_ScenarioFrameworkEActivationType.SAME_AS_PARENT)
	{
		if (m_bInitiated)
			return;
		
		if (!m_bDynamicallyDespawned && activation != m_eActivationType)
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
		
		bool tempTerminated = m_bIsTerminated;
		m_bIsTerminated = false;
		
		if (m_Entity && !m_bEnableRepeatedSpawn)
		{
			IEntity entity = GetOwner().GetParent();
			if (!entity)
				return;
				
			SCR_ScenarioFrameworkLayerBase layerBase = SCR_ScenarioFrameworkLayerBase.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerBase));
			if (!layerBase)
				return;
				
			if (!layerBase.GetEnableRepeatedSpawn())
			{
				Print(string.Format("ScenarioFramework: Object %1 already exists and won't be spawned for %2, exiting...", m_Entity, GetOwner().GetName()), LogLevel.ERROR);
				return;
			}
		}
		
		// Handles inheritance of faction settings from parents
		if (m_sFactionKey.IsEmpty() && m_ParentLayer && !m_ParentLayer.GetFactionKey().IsEmpty())
			SetFactionKey(m_ParentLayer.GetFactionKey());
		
		if (!m_bUseExistingWorldAsset)
			m_Entity = SpawnAsset();
		else
			QueryObjectsInRange();	//sets the m_Entity in subsequent callback
		
		GetOnAllChildrenSpawned().Insert(AfterAllChildrenSpawned);
		
		if (m_Entity)
		{
			BaseGameTriggerEntity trigger = BaseGameTriggerEntity.Cast(m_Entity);
			if (trigger)
				trigger.EnablePeriodicQueries(false);
		}
		else
		{
			InvokeAllChildrenSpawned();
			return;
		}
		
		if (!m_sID.IsEmpty())
			m_Entity.SetName(m_sID);	
		
		SCR_DamageManagerComponent objectDmgManager = SCR_DamageManagerComponent.GetDamageManager(m_Entity);
		if (objectDmgManager)
			objectDmgManager.GetOnDamageStateChanged().Insert(OnObjectDamage);
		
		if (Vehicle.Cast(m_Entity))
		{
			EventHandlerManagerComponent ehManager = EventHandlerManagerComponent.Cast(m_Entity.FindComponent(EventHandlerManagerComponent));
			if (ehManager)
				ehManager.RegisterScriptHandler("OnCompartmentEntered", this, OnCompartmentEntered, false, true);
		}
			
		StoreTaskSubjectToParentTaskLayer();
		m_bIsTerminated = tempTerminated;
		
		InvokeAllChildrenSpawned();
	}
	
	//------------------------------------------------------------------------------------------------
	override void AfterAllChildrenSpawned(SCR_ScenarioFrameworkLayerBase layer)
	{
		m_bInitiated = true;
		
		if (m_ParentLayer)
			m_ParentLayer.CheckAllChildrenSpawned(this);
		
		if (!m_Area)
			m_Area = GetParentArea();
		
		if (m_Area)
		{
			m_Area.GetOnAllChildrenSpawned().Insert(AfterParentAreaChildrenSpawned);
			m_Area.CheckAllChildrenSpawned(this);
		}

		GetOnAllChildrenSpawned().Remove(AfterAllChildrenSpawned);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void AfterParentAreaChildrenSpawned(SCR_ScenarioFrameworkLayerBase layer)
	{
		foreach (SCR_ScenarioFrameworkPlugin plugin : m_aPlugins)
		{
			plugin.Init(this);
		}
		
		foreach (SCR_ScenarioFrameworkActionBase activationAction : m_aActivationActions)
		{
			activationAction.Init(GetOwner());
		}
		
		if (m_Area)
			m_Area.GetOnAllChildrenSpawned().Remove(AfterParentAreaChildrenSpawned);
		
		if (m_Entity)
		{
			BaseGameTriggerEntity trigger = BaseGameTriggerEntity.Cast(m_Entity);
			if (trigger)
				trigger.EnablePeriodicQueries(true);
		}
	}
}
