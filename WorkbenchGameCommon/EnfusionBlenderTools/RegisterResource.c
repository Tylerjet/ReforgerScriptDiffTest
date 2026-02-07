class RegisterResourceRequest: JsonApiStruct
{
	ref array<string> path = new array<string>;
	
	void RegisterResourceRequest()
	{
		RegV("path");
	}
}

class RegisterResourceResponse: JsonApiStruct
{
	bool Output;
	
	void RegisterResourceResponse()
	{
		RegV("Output");
	}
}

class RegisterResourceUtils
{
	void Register(string absPath ,RegisterResourceResponse response)
	{
		//Get Resource
		ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
		MetaFile meta = resourceManager.RegisterResourceFile(absPath);
		//Check if metafile was created
		if (meta == false)
		{
			response.Output = false;
			return;
		}
		else response.Output = true;
		//Rebuild
		meta.Save();
		resourceManager.RebuildResourceFile(absPath,"PC", true);
		return;
	}
}

class RegisterResource: NetApiHandler
{
	override JsonApiStruct GetRequest()
	{
		return new RegisterResourceRequest();
	}
	
	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		RegisterResourceRequest req = RegisterResourceRequest.Cast(request);
		RegisterResourceResponse response = new RegisterResourceResponse();
 		RegisterResourceUtils utils = new RegisterResourceUtils();
		//To fbx which is in WB files
		for(int i = 0; i < req.path.Count(); i++)
		{
			Print(req.path[i]);
			utils.Register(req.path[i], response);
		}
		return response;
	}
}