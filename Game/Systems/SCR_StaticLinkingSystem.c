//------------------------------------------------------------------------------------------------
class SCR_StaticLinkingEntry : Managed
{
	int m_iID;
	IEntity m_Entity;
	
	void ~SCR_StaticLinkingEntry()
	{
		
	}
	
	void SCR_StaticLinkingEntry()
	{
		
	}
};

//------------------------------------------------------------------------------------------------
class SCR_StaticLinkingSystem
{
	static int highestID = 0;
	private static ref SCR_StaticLinkingSystem instance;
	
	ref map<int, IEntity> m_mEntries = new ref map<int, IEntity>();
	
	//------------------------------------------------------------------------------------------------
	IEntity FindEntry(int id)
	{
		// Get should return either value or null if not found
		return m_mEntries.Get(id);
	}
	
	//------------------------------------------------------------------------------------------------
	int FindID(IEntity entity)
	{
		int id = m_mEntries.GetKeyByValue(entity);
		return id;
	}
	
	//------------------------------------------------------------------------------------------------
	// no need to create an entity for this system
	/*static bool CreateStaticLinkingSystemInstance()
	{
		ArmaReforgerScripted game = GetGame();
		GenericWorldEntity worldEntity = game.GetWorldEntity();
		BaseWorld world = game.GetWorld();
		if (world)
		{
			IEntity staticLinkingSystem = world.FindEntityByName("StaticLinkingSystem");
		}
		if (game)
		{
			#ifdef WORKBENCH
			WorldEditorAPI api;
			if (worldEntity)
				api = worldEntity._WB_GetEditorAPI();
			if (api)
				api.CreateEntity("SCR_StaticLinkingSystem", "StaticLinkingSystem", api.GetCurrentEntityLayerId(), null, vector.Zero, vector.Zero);
			#endif
			game.SpawnEntity(SCR_StaticLinkingSystem);
			return true;
		}
		return false;
	}*/
	
	//------------------------------------------------------------------------------------------------
	void AddEntry(IEntity entity, int id)
	{
		if (!entity)
		{
			Print("Entity is null (SCR_StaticLinkingEntry.AddEntry(), returning.", LogLevel.WARNING);
			return;
		}
		
		if (id > highestID)
			highestID = id;
		else
		{
			IEntity foundEntity = FindEntry(id);
			if (foundEntity)
			{
				if (foundEntity != entity)
				{
					Print("ID: " + id + " is already used, please change ID of " + entity, LogLevel.WARNING);
					return;
				}
				return;
			}
		}
		
		if (m_mEntries)
			m_mEntries.Insert(id, entity);
	}
	
	//------------------------------------------------------------------------------------------------
	int AddEntry(IEntity entity)
	{
		if (!entity)
		{
			Print("Entity is null (SCR_StaticLinkingEntry.AddEntry(), returning -1.", LogLevel.WARNING);
			return -1;
		}
		
		highestID++;
		
		if (m_mEntries)
			m_mEntries.Insert(highestID, entity);
		
		return highestID;
	}
	
	//------------------------------------------------------------------------------------------------
	void RemoveEntry(int id)
	{
		m_mEntries.Remove(id);
		// don't worry about this, keep it alive
		/*if (m_mEntries.Count() <= 0) //No entries, delete the as it's no longer required instance
			delete this;*/
	}

	//------------------------------------------------------------------------------------------------
	static SCR_StaticLinkingSystem GetInstance()
	{
		// this makes sure instance is created the first time GetInstance() is called
		// after that the system is always available
		if (!instance)
		{
			instance = new SCR_StaticLinkingSystem;
		}
		
		return instance;
	}
	
	//------------------------------------------------------------------------------------------------
	private void SCR_StaticLinkingSystem()
	{
		// no need to do the instance/delete/etc. code here
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_StaticLinkingSystem()
	{
		if (m_mEntries)
			m_mEntries.Clear();
		m_mEntries = null;
		
		highestID = 0;
	}

};