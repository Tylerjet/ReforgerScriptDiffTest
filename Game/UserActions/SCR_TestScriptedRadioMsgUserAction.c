class SCR_TestScriptedRadioMsgUserAction : ScriptedUserAction
{
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{		
		GenericEntity owner = GenericEntity.Cast(pOwnerEntity);
		BaseRadioComponent radioComp = BaseRadioComponent.Cast(owner.FindComponent(BaseRadioComponent));
		
		if (radioComp)
		{
			//ScriptedRadioMessage msg();
			SCR_RequestTransportMessage msg();
			radioComp.Transmit(msg);
		}
	}
};