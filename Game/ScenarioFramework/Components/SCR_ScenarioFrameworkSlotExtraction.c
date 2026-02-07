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
			
		super.Init(area, activation);
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return;

		if (!SCR_PlayersPresentTriggerEntity.Cast(m_Entity))
			return;

		SCR_PlayersPresentTriggerEntity.Cast(m_Entity).SetOwnerFaction(factionManager.GetFactionByKey(m_sFactionKey));
	}
}
