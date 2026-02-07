[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkPluginOnDestroyEvent : SCR_ScenarioFrameworkPlugin
{
	[Attribute(UIWidgets.Auto, desc: "What to do once object gets destroyed", category: "OnDestroy")]
	ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActionsOnDestroy;

	IEntity m_Asset;

	//------------------------------------------------------------------------------------------------
	override void Init(SCR_ScenarioFrameworkLayerBase object)
	{
		if (!object)
			return;

		super.Init(object);
		IEntity entity = object.GetSpawnedEntity();
		if (!entity)
			return;

		m_Asset = entity;
		SCR_DamageManagerComponent objectDmgManager = SCR_DamageManagerComponent.GetDamageManager(m_Asset);
		if (objectDmgManager)
			objectDmgManager.GetOnDamageStateChanged().Insert(OnObjectDamage);
		else
			PrintFormat("ScenarioFramework: Registering OnDestroy of entity %1 failed! The entity doesn't have damage manager", entity, LogLevel.ERROR);

		if (Vehicle.Cast(m_Asset))
		{
			VehicleControllerComponent vehicleController = VehicleControllerComponent.Cast(m_Asset.FindComponent(VehicleControllerComponent));
			if (vehicleController)
				vehicleController.GetOnEngineStop().Insert(CheckEngineDrowned);

			// Since there is no invoker and no reliable way how to tackle drowned vehicles, in order to make it reliable,
			// We cannot solely rely on GetOnEngineStop because vehicle could have been pushed/moved into the water without started engine.
			GetGame().GetCallqueue().CallLater(CheckEngineDrowned, 5000, true);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] state
	void OnObjectDamage(EDamageState state)
	{
		if (state != EDamageState.DESTROYED || !m_Asset)
			return;

		SCR_DamageManagerComponent objectDmgManager = SCR_DamageManagerComponent.GetDamageManager(m_Asset);
		if (objectDmgManager)
		{
			objectDmgManager.GetOnDamageStateChanged().Remove(OnObjectDamage);
			GetGame().GetCallqueue().Remove(CheckEngineDrowned);

			VehicleControllerComponent vehicleController = VehicleControllerComponent.Cast(m_Asset.FindComponent(VehicleControllerComponent));
			if (vehicleController)
				vehicleController.GetOnEngineStop().Remove(CheckEngineDrowned);
		}

		foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnDestroy)
		{
			action.OnActivate(m_Asset);
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	void CheckEngineDrowned()
	{
		if (!m_Asset)
			return;

		VehicleControllerComponent vehicleController = VehicleControllerComponent.Cast(m_Asset.FindComponent(VehicleControllerComponent));
		if (vehicleController && vehicleController.GetEngineDrowned())
		{
			vehicleController.GetOnEngineStop().Remove(CheckEngineDrowned);
			GetGame().GetCallqueue().Remove(CheckEngineDrowned);

			SCR_DamageManagerComponent objectDmgManager = SCR_DamageManagerComponent.GetDamageManager(m_Asset);
			if (objectDmgManager)
		 		objectDmgManager.GetOnDamageStateChanged().Remove(OnObjectDamage);

			foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnDestroy)
			{
				action.OnActivate(m_Asset);
			}
		}
	}
}