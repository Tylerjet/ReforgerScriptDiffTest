class SCR_AISemaphoreOut: SCR_AISemaphoreIn
{
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
    {
		if (m_lockComponent)
		{
			//Print("Jsem :" + m_lockName2); 
			bool tryCloseLock = m_lockComponent.LeaveKey(m_lockName);
			if (!tryCloseLock)
			{ 
				Print("Error: tried to close lock that was not opened!");
				return ENodeResult.FAIL;
			}	
			else 
				return ENodeResult.SUCCESS;		
		}
		return ENodeResult.FAIL;
	}		
};




