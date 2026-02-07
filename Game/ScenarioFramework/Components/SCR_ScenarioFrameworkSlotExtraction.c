[EntityEditorProps(category: "GameScripted/ScenarioFramework/Slot", description: "")]
class SCR_ScenarioFrameworkSlotExtractionClass : SCR_ScenarioFrameworkSlotTaskClass
{
}

class SCR_ScenarioFrameworkSlotExtraction : SCR_ScenarioFrameworkSlotTask
{
	//------------------------------------------------------------------------------------------------
	override void Init(SCR_ScenarioFrameworkArea area = null, SCR_ScenarioFrameworkEActivationType activation = SCR_ScenarioFrameworkEActivationType.SAME_AS_PARENT)
	{
        if (m_eActivationType != activation)
		{
			if (m_ParentLayer)
				m_ParentLayer.CheckAllChildrenSpawned(this);
			
			return;
		}
		
		foreach (SCR_ScenarioFrameworkActivationConditionBase activationCondition : m_aActivationConditions)
		{
			//If just one condition is false, we don't continue and interrupt the init
			if (!activationCondition.Init(GetOwner()))
			{
				InvokeAllChildrenSpawned();
				return;
			}
		}
			
		super.Init(area, activation);
		
		SCR_CharacterTriggerEntity trigger = SCR_CharacterTriggerEntity.Cast(m_Entity);
		if (!trigger)
			return;

		trigger.SetOwnerFaction(m_sFactionKey);
	}
}
