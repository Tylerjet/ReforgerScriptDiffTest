class SCR_AISendMessageGenerated : AITaskScripted
{
	static const string PORT_RECEIVER = "Receiver";
	
	[Attribute("", UIWidgets.EditBox)]
	protected string m_sText;
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------
	protected SCR_AIActivityBase GetRelatedActivity(AIAgent owner)
	{
		SCR_AIBaseUtilityComponent utilityComp = SCR_AIBaseUtilityComponent.Cast(owner.FindComponent(SCR_AIBaseUtilityComponent));
		if (!utilityComp)
			return null;
		
		if (SCR_AIGroup.Cast(owner))
			return SCR_AIActivityBase.Cast(utilityComp.GetCurrentAction());
		else
			return SCR_AIBehaviorBase.Cast(utilityComp.GetCurrentAction()).m_RelatedGroupActivity;
		
		return null;
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------
	protected bool SendMessage(AIAgent owner, AIAgent receiver, AIMessage msg)
	{
		AICommunicationComponent comms = owner.GetCommunicationComponent();
		if (!comms || !receiver)
			return false;
		
		return comms.RequestBroadcast(msg, receiver);
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------
	protected AIAgent GetReceiverAgent(AIAgent owner)
	{
		AIAgent receiver;
		if (!GetVariableIn(PORT_RECEIVER, receiver))
			receiver = owner;
		
		return receiver;
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------
	override bool VisibleInPalette() { return false; }	
}