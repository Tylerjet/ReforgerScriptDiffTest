class FactionChatChannel : BaseChatChannel
{

	
	//------------------------------------------------------------------------------------------------
	override bool IsDelivering(BaseChatComponent sender, BaseChatComponent receiver)
	{
		SCR_PlayerController senderPC = SCR_PlayerController.Cast(sender.GetOwner());		
		SCR_PlayerController receiverPC = SCR_PlayerController.Cast(receiver.GetOwner());
		if (!receiverPC || !senderPC)
			return false;
		
		SCR_RespawnSystemComponent respawnSystemComponent = SCR_RespawnSystemComponent.GetInstance();
		if (!respawnSystemComponent)
			return false;
		
		// Compare factions
		if (respawnSystemComponent.GetPlayerFaction(senderPC.GetPlayerId()) == respawnSystemComponent.GetPlayerFaction(receiverPC.GetPlayerId()))
			return true;
		
		// Receiver faction is not the same as sender faction
		return false;
	}
	
	
	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(BaseChatComponent sender)
	{
		
		SCR_PlayerController senderPC =SCR_PlayerController.Cast(sender.GetOwner());
		if (!senderPC)
			return false;
		
		SCR_RespawnSystemComponent respawnSystemComponent = SCR_RespawnSystemComponent.GetInstance();
		if (!respawnSystemComponent)
			return false;
		
		if (respawnSystemComponent.GetPlayerFaction(senderPC.GetPlayerId()))
			return true;
		else 
			return false;
	}
};