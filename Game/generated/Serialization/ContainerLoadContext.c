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
// Context to load data from given serliazation container.
// This class serves as interface and doesn't contain
// any implementation, choose specific child class to create
// instance of what context should be used.
/////////////////////////////////////////////////////////////
*/
class ContainerLoadContext: SerializationLoadContext
{
	void ContainerLoadContext() {}
	void ~ContainerLoadContext() {}
	
	//! Returns the assigned container
	proto external BaseLoadContainer GetContainer();
	/*!
	Set the new assigned container.
	\param container The new container.
	*/
	proto external void SetContainer(BaseLoadContainer container);
};

/** @}*/
