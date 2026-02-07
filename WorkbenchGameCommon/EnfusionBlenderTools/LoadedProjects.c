class LoadedProjectsRequest: JsonApiStruct
{
	string fbxPath;
	
	void LoadedProjectsRequest()
	{
		RegV("fbxPath");
	}
};

class LoadedProjectsResponse: JsonApiStruct
{
	bool resourceInProject;
	
	void LoadedProjectsResponse()
	{
		RegV("resourceInProject");
	}
};

class LoadedProjects: NetApiHandler
{
	override JsonApiStruct GetRequest()
	{
		return new LoadedProjectsRequest();
	}
	
	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		LoadedProjectsRequest req = LoadedProjectsRequest.Cast(request);
		LoadedProjectsResponse response = new LoadedProjectsResponse();		
		
		//Default as false
		response.resourceInProject = false;
		array<string> addons = new array<string>;
		//Get GUIDs of loaded addons
		GameProject.GetLoadedAddons(addons);
		string addonName, absPath;
		
		//Go throught the addons
		for(int i = 0; i < addons.Count(); i++)
		{
			//ID from the GUID
			addonName = GameProject.GetAddonID(addons[i]);
			//Find AbsPath to all loaded gprojs
			Workbench.GetAbsolutePath("$" + addonName + ":", absPath, false);
			//If any of the paths is contained in fbx path
			if(req.fbxPath.Contains(absPath) && absPath != "")
			{
				response.resourceInProject = true;
			}
		}
		Print(response.resourceInProject);
		return response;
	}

}