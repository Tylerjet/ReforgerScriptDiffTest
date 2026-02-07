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
		

		ResourceName guid = "{" + req.matName + "}";
		string resName = Workbench.GetResourceName(guid);

		if(resName != guid && guid.Length() == 18)
		{
			response.exists = true;
		}
		
		return response;
	}

}