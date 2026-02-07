
// Convenience Save/Load contexts to work with binary format

/*-----------------------------------------------------------
 Context to save data to given serliazation container
*/
class SCR_BinSaveContext : ContainerSerializationSaveContext
{
	private ref BinSerializationSaveContainer m_Container;

	void SCR_BinSaveContext(bool skipEmptyObjects = true)
	{
		m_Container = new BinSerializationSaveContainer();
		SetContainer(m_Container);
	}

	void ~SCR_BinSaveContext()
	{
	}

	bool SaveToFile(string filePath)
	{
		return m_Container.SaveToFile(filePath);
	}
	
	BinSerializationContainer SaveToContainer()
	{
		return m_Container.SaveToContainer();
	}
};

/*-----------------------------------------------------------
 Context to load data from given serliazation container
*/
class SCR_BinLoadContext : ContainerSerializationLoadContext
{
	private ref BinSerializationLoadContainer m_Container;
	
	void SCR_BinLoadContext(bool skipEmptyObjects = true)
	{
		m_Container = new BinSerializationLoadContainer();
		SetContainer(m_Container);
	}

	void ~SCR_BinLoadContext()
	{
	}
	
	bool LoadFromFile(string fileName)
	{
		return m_Container.LoadFromFile(fileName);
	}
	
	bool LoadFromContainer(notnull BinSerializationContainer container)
	{
		return m_Container.LoadFromContainer(container);
	}
};