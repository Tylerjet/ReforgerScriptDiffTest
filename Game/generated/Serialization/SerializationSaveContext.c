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
// Interface class for save context.
// This class serves as interface and doesn't contain
// any implementation, choose specific child class to create
// instance of what context should be used.
/////////////////////////////////////////////////////////////
*/
class SerializationSaveContext: SerializationContextBase
{
	void SerializationSaveContext() {}
	void ~SerializationSaveContext() {}
	
	// write data as key/value pairs
	proto external bool WriteBoolValue(string name, bool value);
	proto external bool WriteIntValue(string name, int value);
	proto external bool WriteFloatValue(string name, float value);
	proto external bool WriteStrValue(string name, string value);
	proto external bool WriteVectorValue(string name, vector value);
	/**
	helpers to serialize whole classes
	*/
	proto external bool WriteGameEntity(string name, GameEntity entity);
	proto external bool WriteGameComponent(string name, GameComponent component);
	proto external bool WriteEntitySlot(string name, EntitySlotInfo slot);
};

/** @}*/
