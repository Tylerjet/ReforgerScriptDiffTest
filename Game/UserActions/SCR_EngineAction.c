class SCR_EngineAction : SCR_VehicleActionBase
{
	protected CarControllerComponent m_pCarController;
	protected CarControllerComponent_SA m_pCarController_SA;

	
	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		if(GetGame().GetIsClientAuthority())
			m_pCarController = CarControllerComponent.Cast(pOwnerEntity.FindComponent(CarControllerComponent));
		else
			m_pCarController_SA = CarControllerComponent_SA.Cast(pOwnerEntity.FindComponent(CarControllerComponent_SA));
	}
	
	//------------------------------------------------------------------------------------------------
	override bool HasLocalEffectOnlyScript()
	{
		if(GetGame().GetIsClientAuthority())
		{
			return false;
		}
		else
		{
			// Local only - prevents its automatic networking
			return true;
		}
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
		
		if (m_pCarController_SA)
			m_pCarController_SA.TryStartEngine();
	}
	
	//! Method called from scripted interaction handler when an action is started (progress bar appeared)
	//! \param pUserEntity The entity that started performing this action
	override void OnActionStart(IEntity pUserEntity)
	{
		if (m_pCarController)
			m_pCarController.TryStartEngine();
		
		if (m_pCarController_SA)
			m_pCarController_SA.TryStartEngine();
	}

	//! Action canceled
	//! \param pUserEntity The entity that started performing this action
	override void OnActionCanceled(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!m_bIsToggle && !m_bTargetState)
			return;
		
		if (m_pCarController && !m_pCarController.IsEngineOn())
			m_pCarController.CancelStart();
		
		if (m_pCarController_SA && !m_pCarController_SA.IsEngineOn())
			m_pCarController_SA.CancelStart();
		
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if(GetGame().GetIsClientAuthority())
			return m_pCarController && super.CanBeShownScript(user) && CanBePerformedScript(user);
		else
			return m_pCarController_SA && super.CanBeShownScript(user) && CanBePerformedScript(user);

	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetState()
	{
		if(GetGame().GetIsClientAuthority())
			return m_pCarController && m_pCarController.IsEngineOn();
		else
			return m_pCarController_SA && m_pCarController_SA.IsEngineOn();
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetState(bool enable)
	{
		if(GetGame().GetIsClientAuthority())
		{
			if (!m_pCarController)
				return;
			
			if (enable)
				m_pCarController.StartEngine();
			else
				m_pCarController.StopEngine();
		}
		else
		{
			if (!m_pCarController_SA)
				return;
			
			if (enable)
				m_pCarController_SA.StartEngine();
			else
				m_pCarController_SA.StopEngine();
		}
	}
};