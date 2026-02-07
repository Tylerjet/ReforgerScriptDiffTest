/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Serialization
* @{
*/

/*!
Base serialization context for saving data from game.
It works as an adapter between the game logic and serialized data.
*/
class BaseSerializationSaveContext: BaseSerializationContext
{
	proto bool WriteValue(string name, void value);
	/**
	helpers to serialize whole classes
	*/
	proto external bool WriteGameEntity(string name, GameEntity entity);
	proto external bool WriteGameComponent(string name, GameComponent component);
	proto external bool WriteEntitySlot(string name, EntitySlotInfo slot);
};

/** @}*/
