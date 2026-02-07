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
// Context to save data to given serliazation container.
// This class serves as interface and doesn't contain
// any implementation, choose specific child class to create
// instance of what context should be used.
/////////////////////////////////////////////////////////////
*/
class ContainerSaveContext: SerializationSaveContext
{
	void ContainerSaveContext() {}
	void ~ContainerSaveContext() {}
	
	//! Returns the assigned container
	proto external BaseSaveContainer GetContainer();
	/*!
	Set the new assigned container.
	\param container The new container.
	*/
	proto external void SetContainer(BaseSaveContainer newContainer);
};

/** @}*/
