class SCR_EngineAction : SCR_VehicleActionBase
{
	protected CarControllerComponent m_pCarController;
	
	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_pCarController = CarControllerComponent.Cast(pOwnerEntity.FindComponent(CarControllerComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	override bool HasLocalEffectOnlyScript()
	{
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBroadcastScript()
	{
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override void PerformContinuousAction(IEntity pOwnerEntity, IEntity pUserEntity, float timeSlice)
	{
		if (m_pCarController)
			m_pCarController.TryStartEngine();
	}
	
	//! Method called from scripted interaction handler when an action is started (progress bar appeared)
	//! \param pUserEntity The entity that started performing this action
	override void OnActionStart(IEntity pUserEntity)
	{
		if (m_pCarController)
			m_pCarController.TryStartEngine();
	}

	//! Action canceled
	//! \param pUserEntity The entity that started performing this action
	override void OnActionCanceled(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!m_bIsToggle && !m_bTargetState)
			return;
		
		if (m_pCarController && !m_pCarController.IsEngineOn())
			m_pCarController.CancelStart();
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		return m_pCarController && super.CanBeShownScript(user) && CanBePerformedScript(user);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetState()
	{
		return m_pCarController && m_pCarController.IsEngineOn();
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetState(bool enable)
	{
		if (!m_pCarController)
			return;
		
		if (enable)
			m_pCarController.StartEngine();
		else
			m_pCarController.StopEngine();
	}
};