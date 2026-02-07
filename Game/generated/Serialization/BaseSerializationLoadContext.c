/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Serialization
\{
*/

/*!
Base serialization context for loading data back to the game.
This class is passed parameter to SerializationBase::SerializeLoad for classes to read data from.
It works as an adapter between the game logic and serialized data.
*/
class BaseSerializationLoadContext: BaseSerializationContext
{
	proto bool ReadValue(string name, out void value);
	/*!
	helpers to serialize whole classes
	*/
	proto external bool ReadGameEntity(string name, GameEntity entity);
	proto external bool ReadGameComponent(string name, GameComponent component);
	proto external bool ReadEntitySlot(string name, EntitySlotInfo slot);
}

/*!
\}
*/
