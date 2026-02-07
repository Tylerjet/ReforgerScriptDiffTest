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
Base class for save/load contexts.
This class serves as interface and doesn't contain 
any implementation, choose specific child class to create
instance of what context should be used.
*/
class SerializationContextBase: Managed
{
	// Injects
	void SerializationContextBase()	{}
	void ~SerializationContextBase() {}
	
	proto external bool IsValid();
	/*!
	Functions for switching to/from sub-object
	*/
	proto external bool StartObject(string name);
	proto external bool EndObject();
	proto external int GetSaveVersion();
};

/** @}*/
