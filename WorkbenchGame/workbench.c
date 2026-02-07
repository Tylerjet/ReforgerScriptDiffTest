//! General Workbench functions
// TODO: replace with helper methods
class SCR_WorkbenchSearchResourcesCallbackArray
{
	protected array<ResourceName> m_aSelection;

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] resName
	//! \param[in] filePath
	void Insert(ResourceName resName, string filePath)
	{
		m_aSelection.Insert(resName);
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[out] selection
	void SCR_WorkbenchSearchResourcesCallbackArray(notnull out array<ResourceName> selection)
	{
		m_aSelection = selection;
	}
}

class SCR_WorkbenchIEntityQueryCallbackArray
{
	protected array<IEntity> m_aEntities;

	//------------------------------------------------------------------------------------------------
	//! QueryEntitiesCallback method
	bool Insert(IEntity e)
	{
		m_aEntities.Insert(e);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_WorkbenchIEntityQueryCallbackArray(notnull out array<IEntity> entitiesArray)
	{
		m_aEntities = entitiesArray;
	}
}
