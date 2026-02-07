[EntityEditorProps(category: "GameScripted/ScenarioFramework/Slot", description: "")]
class SCR_ScenarioFrameworkSlotClearAreaClass : SCR_ScenarioFrameworkSlotTaskClass
{
}

class SCR_ScenarioFrameworkSlotClearArea : SCR_ScenarioFrameworkSlotTask
{
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
