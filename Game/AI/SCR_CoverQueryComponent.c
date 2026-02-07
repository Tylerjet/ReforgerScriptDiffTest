// unused?
class SCR_CoverQueryComponentClass : CoverQueryComponentClass
{
}

class SCR_CoverQueryComponent : CoverQueryComponent
{
	// Constants for amount of covers to check for high priority cover queries and low priority cover queries
	const int MAX_COVERS_HIGH_PRIORITY = 25;
	const int MAX_COVERS_LOW_PRIORITY = 15;
	
	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_CoverQueryComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
	}
}
