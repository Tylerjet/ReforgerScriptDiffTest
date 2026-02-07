class TextureValidationRequest: JsonApiStruct
{
	ref array<string> mats = new array<string>;
	string fbxPath; 
	
	void TextureValidationRequest()
	{
		RegV("mats");
		RegV("fbxPath");
	}
};

class TextureValidationResponse: JsonApiStruct
{
	ref array<string> invalidTextures = new array<string>;
	bool xob = true;
	ref array<string> xobMats = new array<string>;
	
	void TextureValidationResponse()
	{
		RegV("invalidTextures");
		RegV("xob");
		RegV("xobMats");
	}
};

class TextureValidationUtils
{
	map<string,ResourceName> GetMaterialAssigns(string xobPath, TextureValidationResponse response)
	{
		string sourceMatText;
		array<string> sourceMats;
		map<string, ResourceName> matAssigns = new map<string, ResourceName>();
		ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
		MetaFile meta = resourceManager.GetMetaFile(xobPath);
		if(!meta)
		{
			return matAssigns;
		}
		BaseContainerList configurations = meta.GetObjectArray("Configurations");
		BaseContainer cfg = configurations.Get(0);

		string materialAssigns;
		array<string> pairs = new array<string>;
		cfg.Get("MaterialAssigns", materialAssigns);
		materialAssigns.Split(";", pairs, true);
		
		foreach (string pair : pairs)
		{
			array<string> keyValue = new array<string>;
		   	pair.Split(",", keyValue, true);
		    matAssigns.Set(keyValue[0], keyValue[1]);
			response.xobMats.Insert(keyValue[0]);
		}
		
		return matAssigns;
	}
	
	//I'll read all textures and foreach texture check it's GUID if the GUID is wrong add it to the list
	array<string> GetInvalidTextures(ResourceName emat)
	{
		ResourceName temp;
		array<string> invalidText = {};
		ResourceName ematGuid = emat.Substring(0,emat.IndexOf("}")+1);
		if(ematGuid.Length() == 18)
		{
			string resName = Workbench.GetResourceName(ematGuid);
			if(ematGuid == resName)
			{
				invalidText.Insert("Emat does not have valid GUID in XOB file");
				return invalidText;
			}
		}
		Resource resource = Resource.Load(emat);
		BaseContainer ematCont = resource.GetResource().ToBaseContainer();
		
		for(int i = 0; i < ematCont.GetNumVars(); i++)
		{
			ematCont.Get(ematCont.GetVarName(i),temp);
			if(ematCont.IsVariableSet(ematCont.GetVarName(i)))
			{
				if(temp.Contains(".edds"))
				{
					//Not great way, but works
					ResourceName texGuid = temp.Substring(0,temp.IndexOf("}")+1);
					if(texGuid.Length() == 18)
					{
						string resName = Workbench.GetResourceName(texGuid);
						if(texGuid == resName)
						{
							invalidText.Insert(temp.Substring(0,temp.Length() - 2));
						}
					}
				}
			}
		}
		return invalidText;
	}
};

class TextureValidation: NetApiHandler
{
	override JsonApiStruct GetRequest()
	{
		return new TextureValidationRequest();
	}
	
	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		TextureValidationRequest req = TextureValidationRequest.Cast(request);
		TextureValidationResponse response = new TextureValidationResponse();		
		TextureValidationUtils utils = new TextureValidationUtils();
		array<string> invalidTextArr =  {};
		//Here get XOB no need to get it everytime in the loop
		req.fbxPath.Replace(".fbx",".xob");
		map<string,ResourceName> matAssigns = utils.GetMaterialAssigns(req.fbxPath, response);
		if(matAssigns.IsEmpty())
		{
			response.xob = false;
			return response;
		}
		foreach(string mat: req.mats)
		{
			string invalidTextStr = string.Empty;
			ResourceName emat = matAssigns.Get(mat);
			if(emat == string.Empty)
			{
				response.invalidTextures.Insert("Material is in FBX but not found in XOB");
				continue;
			}
	
			//List of all invalid textures for that material
			invalidTextArr = utils.GetInvalidTextures(emat);
			for(int i = 0; i < invalidTextArr.Count(); i++)
			{
				invalidTextStr += invalidTextArr[i];
				if(i != invalidTextArr.Count() - 1)
				{
					invalidTextStr += ", ";
				}
			}
			response.invalidTextures.Insert(invalidTextStr);
		}
		return response;
	}
}