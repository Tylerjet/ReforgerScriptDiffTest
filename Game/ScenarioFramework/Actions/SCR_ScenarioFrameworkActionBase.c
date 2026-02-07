[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionBase
{
	[Attribute(defvalue: "-1", uiwidget: UIWidgets.Graph, params: "-1 10000 1", desc: "How many times this action can be performed if this gets triggered? Value -1 for infinity")]
	int					m_iMaxNumberOfActivations;

	IEntity				m_Entity;
	int					m_iNumberOfActivations;

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] entity
	void Init(IEntity entity)
	{
		if (!SCR_BaseTriggerEntity.Cast(entity))
		{
			OnActivate(entity);
			m_Entity = entity;
			return;
		}

		m_Entity = entity;
		ScriptInvoker pOnActivateInvoker = SCR_BaseTriggerEntity.Cast(entity).GetOnActivate();
		if (pOnActivateInvoker)
			pOnActivateInvoker.Insert(OnActivate);

		ScriptInvoker pOnDeactivateInvoker = SCR_BaseTriggerEntity.Cast(entity).GetOnDeactivate();
		if (pOnDeactivateInvoker)
			pOnDeactivateInvoker.Insert(OnActivate);		//registering OnDeactivate to OnActivate - we need both changes
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	bool CanActivate()
	{
		if (m_iMaxNumberOfActivations != -1 && m_iNumberOfActivations >= m_iMaxNumberOfActivations)
		{
			if (m_Entity)
				Print(string.Format("ScenarioFramework Action: Maximum number of activations reached for Action %1 attached on %2. Action won't do anything.", this, m_Entity.GetName()), LogLevel.ERROR);
			else
				Print(string.Format("ScenarioFramework Action: Maximum number of activations reached for Action %1. Action won't do anything.", this), LogLevel.ERROR);

			return false;
		}

		m_iNumberOfActivations++;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \out entity
	//! \return
	bool ValidateInputEntity(IEntity object, SCR_ScenarioFrameworkGet getter, out IEntity entity)
	{
		if (!getter && object)
		{
			SCR_ScenarioFrameworkLayerBase layer = SCR_ScenarioFrameworkLayerBase.Cast(object.FindComponent(SCR_ScenarioFrameworkLayerBase));
			if (!layer)
			{
				Print(string.Format("ScenarioFramework Action: Action %1 attached on %2 is not called from layer and won't do anything.", this, object.GetName()), LogLevel.ERROR);
				return false;
			}

			entity = layer.GetSpawnedEntity();
		}
		else
		{
			SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(getter.Get());
			if (!entityWrapper)
			{
				if (object)
					Print(string.Format("ScenarioFramework Action: Issue with Getter detected for Action %1 attached on %2.", this, object.GetName()), LogLevel.ERROR);
				else
					Print(string.Format("ScenarioFramework Action: Issue with Getter detected for Action %1.", this), LogLevel.ERROR);

				return false;
			}

			entity = entityWrapper.GetValue();
		}

		if (!entity)
		{
			if (object)
				Print(string.Format("ScenarioFramework Action: Entity not found for Action %1 attached on %2.", this, object.GetName()), LogLevel.ERROR);
			else
				Print(string.Format("ScenarioFramework Action: Entity not found for Action %1.", this), LogLevel.ERROR);

			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] object
	void OnActivate(IEntity object);

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] aObjectsNames
	//! \param[in] eActivationType
	void SpawnObjects(notnull array<string> aObjectsNames, SCR_ScenarioFrameworkEActivationType eActivationType)
	{
		IEntity object;
		SCR_ScenarioFrameworkLayerBase layer;

		foreach (string sObjectName : aObjectsNames)
		{
			object = GetGame().GetWorld().FindEntityByName(sObjectName);
			if (!object)
			{
				Print(string.Format("ScenarioFramework Action: Can't spawn object set in slot %1. Slot doesn't exist", sObjectName), LogLevel.ERROR);
				continue;
			}

			layer = SCR_ScenarioFrameworkLayerBase.Cast(object.FindComponent(SCR_ScenarioFrameworkLayerBase));
			if (!layer)
			{
				Print(string.Format("Can't spawn object %1 - the slot doesn't have SCR_ScenarioFrameworkLayerBase component", sObjectName), LogLevel.ERROR);
				continue;
			}
			
			if (layer.GetActivationType() != eActivationType)
			{
				Print(string.Format("Can't spawn object %1 - the slot has %2 activation type set instead of %3", sObjectName, layer.GetActivationType(), eActivationType), LogLevel.ERROR);
				continue;
			}

			layer.Init(null, eActivationType);
			layer.SetActivationType(SCR_ScenarioFrameworkEActivationType.SAME_AS_PARENT);
		}
	}
}