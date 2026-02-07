[EntityEditorProps(category: "GameScripted/ScenarioFramework/Slot", description: "")]
class SCR_ScenarioFrameworkSlotTriggerClass : SCR_ScenarioFrameworkSlotBaseClass
{
}

class SCR_ScenarioFrameworkSlotTrigger : SCR_ScenarioFrameworkSlotBase
{
	[Attribute(desc: "Actions that will be performed after trigger conditions are true and the trigger itself activates (not the slot itself)", category: "OnActivation")];
	protected ref array<ref SCR_ScenarioFrameworkActionBase>	m_aTriggerActions;
	
	//------------------------------------------------------------------------------------------------
	//!
	override void RestoreToDefault(bool includeChildren = false, bool reinitAfterRestoration = false)
	{
		foreach (SCR_ScenarioFrameworkActionBase activationAction : m_aTriggerActions)
		{
			activationAction.m_iNumberOfActivations = 0;
		}
		
		super.RestoreToDefault(includeChildren, reinitAfterRestoration);
	}
	
	//------------------------------------------------------------------------------------------------
	override void FinishInit()
	{
		BaseGameTriggerEntity trigger = BaseGameTriggerEntity.Cast(m_Entity);
		if (trigger)
		{
			trigger.EnablePeriodicQueries(false);
			
			SCR_ScenarioFrameworkTriggerEntity frameworkTrigger = SCR_ScenarioFrameworkTriggerEntity.Cast(trigger);
			if (frameworkTrigger)
				frameworkTrigger.SetInitSequenceDone(false);
		}
		
		super.FinishInit();
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
		
		foreach (SCR_ScenarioFrameworkActionBase triggerAction : m_aTriggerActions)
		{
			triggerAction.Init(m_Entity);
		}

		if (m_fRepeatedSpawnTimer >= 0)
			RepeatedSpawn();
		
		if (m_Area)
			m_Area.GetOnAllChildrenSpawned().Remove(AfterParentAreaChildrenSpawned);
		
		if (m_Entity)
		{
			BaseGameTriggerEntity trigger = BaseGameTriggerEntity.Cast(m_Entity);
			if (trigger)
			{
				trigger.EnablePeriodicQueries(true);
			
				SCR_ScenarioFrameworkTriggerEntity frameworkTrigger = SCR_ScenarioFrameworkTriggerEntity.Cast(trigger);
				if (frameworkTrigger)
					frameworkTrigger.SetInitSequenceDone(true);
			}
		}
	}
}
