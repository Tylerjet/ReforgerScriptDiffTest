
// Convenience Save/Load contexts to work with json format

/*-----------------------------------------------------------
 Context to save data to given serliazation container
*/
class SCR_JsonSaveContext : ContainerSaveContext
{
	private ref JsonSaveContainer container;

	void SCR_JsonSaveContext()
	{
		container = new JsonSaveContainer();
		SetContainer(container);
	}

	void ~SCR_JsonSaveContext()
	{
	}

	bool SaveToFile(string filePath)
	{
		return container.SaveToFile(filePath);
	}
	
	string ExportToString()
	{
		return container.ExportToString();
	}

};


/*-----------------------------------------------------------
 Context to load data from given serliazation container
*/
class SCR_JsonLoadContext : ContainerLoadContext
{
	private ref JsonLoadContainer container;
	
	void SCR_JsonLoadContext()
	{
		container = new JsonLoadContainer();
		SetContainer(container);
	}

	void ~SCR_JsonLoadContext()
	{
	}
	
	bool ImportFromString(string jsonData)
	{
		return container.ImportFromString(jsonData);
	}
	
	bool LoadFromFile(string fileName)
	{
		return container.LoadFromFile(fileName);
	}
	
};