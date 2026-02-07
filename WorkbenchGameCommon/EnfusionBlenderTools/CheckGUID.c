class CheckGUIDRequest: JsonApiStruct
{
	string matName;
	
	void CheckGUIDRequest()
	{
		RegV("matName");
	}
};

class CheckGUIDResponse: JsonApiStruct
{
	bool exists = false;
	
	void CheckGUIDResponse()
	{
		RegV("exists");
	}
};

class CheckGUID: NetApiHandler
{
	override JsonApiStruct GetRequest()
	{
		return new CheckGUIDRequest();
	}
	
	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		CheckGUIDRequest req = CheckGUIDRequest.Cast(request);
		CheckGUIDResponse response = new CheckGUIDResponse();		
		
		if(req.matName.Contains("_"))
		{
			ResourceName relPath = "{" + req.matName.Substring(req.matName.LastIndexOf("_")+1, req.matName.Length() - 1 - req.matName.LastIndexOf("_")) + "}";
			if(relPath.Length() == 18)
			{
				string resName = Workbench.GetResourceName(relPath);
				//If getResourceName won't find GUID it will print the same GUID as it get
				if(relPath != resName)
				{
					response.exists = true;
				}
			}
		}
		
		return response;
	}

}