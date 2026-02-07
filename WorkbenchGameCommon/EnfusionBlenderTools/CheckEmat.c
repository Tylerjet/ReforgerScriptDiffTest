class CheckEmatRequest: JsonApiStruct
{
	string xobPath;
	string matName;
	bool fbx;
	
	void CheckEmatRequest()
	{
		RegV("xobPath");
		RegV("matName");
		RegV("fbx");
	}
};

class CheckEmatResponse: JsonApiStruct
{
	bool exists = false;
	string guid;
	string message;
	
	void CheckEmatResponse()
	{
		RegV("exists");
		RegV("guid");
		RegV("message");
	}
};

class CheckEmat: NetApiHandler
{
	override JsonApiStruct GetRequest()
	{
		return new CheckEmatRequest();
	}
	
	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		CheckEmatRequest req = CheckEmatRequest.Cast(request);
		CheckEmatResponse response = new CheckEmatResponse();		
		//Default as false
				ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
		MetaFile meta = resourceManager.GetMetaFile(req.xobPath);
		if(!meta)
		{
			return response;
		}
		BaseContainerList configurations = meta.GetObjectArray("Configurations");
		BaseContainer cfg = configurations.Get(0);
		string materialAssigns;
		array<string> pairs = new array<string>;
		cfg.Get("MaterialAssigns", materialAssigns);
		materialAssigns.Split(";", pairs, true);
		map<string, string> materials = new map<string, string>();
		
		foreach (string pair : pairs)
		{
			array<string> keyValue = new array<string>;
		   	pair.Split(",", keyValue, true);
		    materials.Set(keyValue[0], keyValue[1]);
		}
		
		
		for(int i = 0; i < materials.Count(); i++)
		{
			if(req.matName == materials.GetKey(i))
			{
				response.exists = true;
			}
			else
		}

		
		if(req.matName.Contains("_"))
		{
			ResourceName relPath = "{" + req.matName.Substring(req.matName.LastIndexOf("_")+1, req.matName.Length() - 1 - req.matName.LastIndexOf("_")) + "}";
			if(relPath.Length() == 18)
			{
				string absPath;
				Workbench.GetAbsolutePath(relPath.GetPath(), absPath, true);
				if(req.fbx)
				{
					response.exists = true;
				}
			}
		}
		return response;
	}

}