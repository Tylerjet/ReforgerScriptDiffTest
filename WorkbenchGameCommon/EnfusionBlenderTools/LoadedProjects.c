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
			//Find AbsPath to gproj
			Workbench.GetAbsolutePath("$" + addonName + ":", absPath, false);
			//TotalCommander uses small letters for disks so the path would be in lowercase as well
			absPath.ToLower();
			req.fbxPath.ToLower();
			//If fbx path contains the gproj path
			if(req.fbxPath.Contains(absPath) && absPath != "")
			{
				response.resourceInProject = true;
			}
		}
		return response;
	}

}