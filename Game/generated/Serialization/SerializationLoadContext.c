/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Serialization
* @{
*/

/**
/////////////////////////////////////////////////////////////
// Interface class for load context..
// This class serves as interface and doesn't contain
// any implementation, choose specific child class to create
// instance of what context should be used.
/////////////////////////////////////////////////////////////
*/
class SerializationLoadContext: SerializationContextBase
{
	void SerializationLoadContext() {}
	void ~SerializationLoadContext() {}
	
	// read data as key/value pairs
	proto bool ReadBoolValue(string name, out bool value);
	proto bool ReadIntValue(string name, out int value);
	proto bool ReadFloatValue(string name, out float value);
	proto bool ReadStrValue(string name, out string value);
	proto bool ReadVectorValue(string name, out vector value);
	/**
	helpers to serialize whole classes
	*/
	proto external bool ReadGameEntity(string name, GameEntity entity);
	proto external bool ReadGameComponent(string name, GameComponent component);
	proto external bool ReadEntitySlot(string name, EntitySlotInfo slot);
};

/** @}*/
