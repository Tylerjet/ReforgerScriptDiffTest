//------------------------------------------------------------------------------------------------
//! Map module base class
[BaseContainerProps()]
class SCR_MapModuleBase : Managed
{
	protected SCR_MapEntity m_MapEntity;
	
	//------------------------------------------------------------------------------------------------
	//! SCR_MapEntity event
	protected void OnMapOpen(MapConfiguration config)
	{}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_MapEntity event
	protected void OnMapClose(MapConfiguration config)
	{}
	
	//------------------------------------------------------------------------------------------------
	//! Enable open/close events
	void SetActive(bool active)
	{
		if (active)
		{
			m_MapEntity.GetOnMapOpen().Insert(OnMapOpen);
			m_MapEntity.GetOnMapClose().Insert(OnMapClose);
		}
		else 
		{
			m_MapEntity.GetOnMapOpen().Remove(OnMapOpen);
			m_MapEntity.GetOnMapClose().Remove(OnMapClose);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Init method for cases where all modules and components should be loaded already so constructor cannot be used, called once after creation
	void Init()
	{}
	
	//------------------------------------------------------------------------------------------------
	//! Update method for frame operations
	void Update(float timeSlice)
	{}
	
	//------------------------------------------------------------------------------------------------
	//! Dont use arguments in the contructor for this class or its children
	void SCR_MapModuleBase()
	{
		m_MapEntity = SCR_MapEntity.GetMapInstance();
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_MapModuleBase() 
	{
		if (m_MapEntity)
		{
			m_MapEntity.GetOnMapOpen().Remove(OnMapOpen);
			m_MapEntity.GetOnMapClose().Remove(OnMapClose);
		}
	}
};
