class CheckEmatRequest : JsonApiStruct
{
	string xobPath;
	string matName;

	void CheckEmatRequest()
	{
		RegV("xobPath");
		RegV("matName");
	}
}

class CheckEmatResponse : JsonApiStruct
{
	bool exists = false;

	void CheckEmatResponse()
	{
		RegV("exists");
	}
}

class CheckEmat : NetApiHandler
{
	override JsonApiStruct GetRequest()
	{
		return new CheckEmatRequest();
	}

	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		CheckEmatRequest req = CheckEmatRequest.Cast(request);
		CheckEmatResponse response = new CheckEmatResponse();
		
		// check if the material exists in Xob Meta
		EBTEmatUtils ematUtils = new EBTEmatUtils();
		map<string,ResourceName> materials = new map<string, ResourceName>();
		ematUtils.GetMaterials(req.xobPath, materials);
		for (int i = 0; i < materials.Count(); i++)
		{
			if (req.matName == materials.GetKey(i))
			{
				response.exists = true;
			}
		}
		return response;
	}

}
