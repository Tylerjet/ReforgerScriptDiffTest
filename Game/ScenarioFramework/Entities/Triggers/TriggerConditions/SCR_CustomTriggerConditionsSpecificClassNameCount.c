[BaseContainerProps()]
class SCR_CustomTriggerConditionsSpecificClassNameCount : SCR_CustomTriggerConditions
{
	[Attribute(desc: "If SPECIFIC_CLASS is selected, fill the class name here.", category: "Trigger")]
	ref array<string> 	m_aSpecificClassNames;

	[Attribute(defvalue: "1", desc: "How many entities of specific prefab are requiered to be inside the trigger", params: "0 100000 1", category: "Trigger")]
	int 	m_iClassnameCount;

	//------------------------------------------------------------------------------------------------
	//! Initializes specific class names for scenario trigger entity, adding them as class types.
	//! \param[in] trigger Trigger entity for scenario framework initialization.
	override void Init(SCR_ScenarioFrameworkTriggerEntity trigger)
	{
		trigger.SetSpecificClassName(m_aSpecificClassNames);

		typename type;
		foreach (string className : m_aSpecificClassNames)
		{
			type = className.ToType();
			if (type)
				trigger.AddClassType(type);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Checks how many times the specific classname is present inside the trigger and sets trigger conditions accordingly
	override void CustomTriggerConditions(SCR_ScenarioFrameworkTriggerEntity trigger)
	{
		bool triggerStatus;
		foreach (string className : m_aSpecificClassNames)
		{
			if (trigger.GetSpecificClassCountInsideTrigger(className) >= m_iClassnameCount)
			{
				triggerStatus = true;
			}
			else
			{
				triggerStatus = false;
				break;
			}
		}

		trigger.SetTriggerConditionsStatus(triggerStatus);
	}
}