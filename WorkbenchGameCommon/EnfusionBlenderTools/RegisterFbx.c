class RegisterFbxRequest: JsonApiStruct
{
	string Fbx;
	
	void RegisterFbxRequest()
	{
		RegV("Fbx");
	}
};

class RegisterFbxResponse: JsonApiStruct
{
	bool Output;
	
	void RegisterFbxResponse()
	{
		RegV("Output");
	}
};

class RegisterFbxUtils
{
	void Register(string absPath ,RegisterFbxResponse response)
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
};

class RegisterFbx: NetApiHandler
{
	override JsonApiStruct GetRequest()
	{
		return new RegisterFbxRequest();
	}
	
	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		RegisterFbxRequest req = RegisterFbxRequest.Cast(request);
		RegisterFbxResponse response = new RegisterFbxResponse();
 		RegisterFbxUtils utils = new RegisterFbxUtils();
		
		//To fbx which is in WB files
		string absPath = req.Fbx;
		utils.Register(absPath, response);
		return response;
	}
}