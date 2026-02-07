[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionChangeTriggerActivationPresence : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Target layer that spawns Trigger to change")]
	ref SCR_ScenarioFrameworkGetLayerBase m_Getter;

	[Attribute(uiwidget: UIWidgets.ComboBox, desc: "By whom the trigger is activated", params: ParamEnumArray.FromEnum(SCR_EScenarioFrameworkTriggerActivation).ToString(), category: "Trigger")]
	SCR_EScenarioFrameworkTriggerActivation m_eActivationPresence;

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] object
	//! \param[out] trigger
	//! \return
	bool CanActivateTriggerVariant(IEntity object, out SCR_ScenarioFrameworkTriggerEntity trigger)
	{
		trigger = SCR_ScenarioFrameworkTriggerEntity.Cast(object);
		if (m_iMaxNumberOfActivations != -1 && m_iNumberOfActivations >= m_iMaxNumberOfActivations)
		{
			if (trigger)
			{
				trigger.GetOnActivate().Remove(OnActivate);
				trigger.GetOnDeactivate().Remove(OnActivate);
			}

			return false;
		}

		m_iNumberOfActivations++;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		SCR_ScenarioFrameworkTriggerEntity trigger;
		if (!CanActivateTriggerVariant(object, trigger))
			return;

		if (trigger)
		{
			trigger.SetActivationPresence(m_eActivationPresence);
			return;
		}

		if (!m_Getter)
		{
			Print(string.Format("ScenarioFramework Action: Getter not found for Action %1.", this), LogLevel.ERROR);
			return;
		}

		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
		if (!entityWrapper)
		{
			Print(string.Format("ScenarioFramework Action: Issue with Getter detected for Action %1.", this), LogLevel.ERROR);
			return;
		}

		IEntity entityFrom = entityWrapper.GetValue();
		if (!entityFrom)
		{
			Print(string.Format("ScenarioFramework Action: Entity not found for Action %1.", this), LogLevel.ERROR);
			return;
		}

		SCR_ScenarioFrameworkLayerBase layer = SCR_ScenarioFrameworkLayerBase.Cast(entityFrom.FindComponent(SCR_ScenarioFrameworkLayerBase));
		if (!layer)
		{
			Print(string.Format("ScenarioFramework Action: Entity is not Layer Base for Action %1.", this), LogLevel.ERROR);
			return;
		}

		SCR_ScenarioFrameworkArea area = SCR_ScenarioFrameworkArea.Cast(layer);
		if (area)
		{
			trigger = SCR_ScenarioFrameworkTriggerEntity.Cast(area.GetTrigger());
		}
		else
		{
			IEntity entity = layer.GetSpawnedEntity();
			if (!BaseGameTriggerEntity.Cast(entity))
			{
				Print(string.Format("ScenarioFramework Action: SlotTrigger - The selected prefab is not trigger!"), LogLevel.ERROR);
				return;
			}
			trigger = SCR_ScenarioFrameworkTriggerEntity.Cast(entity);
		}

		trigger.SetActivationPresence(m_eActivationPresence);
	}
}