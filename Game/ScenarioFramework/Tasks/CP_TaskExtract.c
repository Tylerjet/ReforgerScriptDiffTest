//------------------------------------------------------------------------------------------------
class CP_TaskExtractClass: CP_TaskAreaClass
{
};

//------------------------------------------------------------------------------------------------
class CP_TaskExtract : CP_TaskArea
{	
	//------------------------------------------------------------------------------------------------
	override void OnTriggerActivated()
	{
		//TODO: task related things
		m_pSupportEntity.FinishTask(this);	
	}
}
