/*************************************************************************************************
*	PROVIDE ACTION -> TAKING THE NOZZLE FROM PROVIDER
**************************************************************************************************/

class SCR_FuelProviderUserAction : SCR_FuelUserAction
{
	
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{		
	}	

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_FuelProviderUserAction()
	{
		m_bIsProvider = true;
	}
	
};
