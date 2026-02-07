class SCR_TutorialDeployMobileAssembly : ScriptedUserAction
{
	protected bool m_bIsActive;
	
	//------------------------------------------------------------------------------------------------
	bool IsActive()
	{
		return m_bIsActive;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetActive(bool activate)
	{
		m_bIsActive = activate;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called when someone tries to perform the action, user entity is typically character
	override event void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		m_bIsActive = !m_bIsActive;
		
		SCR_VehicleSoundComponent sndComp = SCR_VehicleSoundComponent.Cast(pOwnerEntity.GetRootParent().FindComponent(SCR_VehicleSoundComponent));
		if(sndComp)
		{
			if(m_bIsActive)
				sndComp.SoundEvent(SCR_SoundEvent.SOUND_MHQ_DEPLOY);
			else
				sndComp.SoundEvent(SCR_SoundEvent.SOUND_MHQ_DISMANTLE);
		}	
	}
	
	//------------------------------------------------------------------------------------------------
	override event bool GetActionNameScript(out string outName)
	{
		if (!m_bIsActive)
			outName = "#AR-Campaign_Action_Deploy-UC";
		else
			outName = "#AR-Campaign_Action_Dismantle-UC";
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override event bool CanBePerformedScript(IEntity user)
	{
		return !m_bIsActive;
	};
}