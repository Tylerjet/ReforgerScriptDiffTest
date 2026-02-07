/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Components
* @{
*/

class BaseChatComponentClass: GameComponentClass
{
};

//! This component takes care of sending chat messages
class BaseChatComponent: GameComponent
{
	//! Returns the parent entity of this component.
	proto external IEntity GetOwner();
	/*!
	Enables/disables chat-channel. System channel (channelId = 0) is enabled by default
	\param channelId: Defined by BaseChatEntity. Indexed from 0.
	\param enabled: Enable/disable receiving of messages in channel
	*/
	proto external void SetChannel(int channelId, bool enabled);
	/*!
	\param channelId: Defined by BaseChatEntity. Indexed from 0.
	\return true for enabled channel, false otherwise
	*/
	proto external bool GetChannelState(int channelId);
	/*!
	Send message to specified channel
	\param msgStr: Message to be sent
	*/
	proto external void SendMessage(string msgStr, int channelIdx);
	/*!
	Send message to specific player
	\param msgStr: Message to be sent
	\param recipient: Recipient playerId
	*/
	proto external void SendPrivateMessage(string msgStr, int recipient);
	
	// callbacks
	
	/*!
	Event rised on every message delivered to BaseChatComponent
	\param msg: Payload
	\param channel: Defined by BaseChatEntity. Indexed from 0.
	\param playerId: Senders Players ID compatible with lobby
	*/
	event protected void OnNewMessage(string msg, int channel, int senderId);
	/*!
	Event rised on every private message delivered to BaseChatComponent
	\param msg: Payload
	\param sender: Senders playerId
	\param receiver: Receiver playerId
	*/
	event protected void OnNewPrivateMessage(string msg, int senderId, int receiverId);
	event protected void ShowMessage(string msg);
};

/** @}*/
