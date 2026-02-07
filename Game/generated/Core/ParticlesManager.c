/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Core
* @{
*/

class ParticlesManager
{
	proto external bool HasReservationSCR(ResourceName effect);
	/*!
	Release the ownership of the particle
	\param pEntity entity with an attached particle
	\return ValidityCheck which tells you if you still owns the particle
	*/
	proto external void RecycleEntity(IEntity entity);
	/*!
	Make a managed entity from an unmanaged one
	\param pEntity entity with an attached particle
	\return ValidityCheck which tells you if you still owns the particle
	*/
	proto external void InsertToUpdate(IEntity entity);
	proto external void RegisterParticleEntitiesByResource(ResourceName resource, ResourceName effect, int reservedCount, bool resizable);
	proto external void RegisterParticleEntitiesByClass(string resource, ResourceName effect, int reservedCount, bool resizable);
	proto external void Clear();
};

/** @}*/
