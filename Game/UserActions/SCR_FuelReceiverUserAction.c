/*************************************************************************************************
*	RECEIVE ACTION -> RECEIVING FROM PROVIDER TO OWNER OF THE ACTION (i.e. provider -> vehicle)
**************************************************************************************************/

class SCR_FuelReceiverUserAction : SCR_FuelUserAction
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
	void SCR_FuelReceiverUserAction()
	{
		m_bIsProvider = false;
	}

};
