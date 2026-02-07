class MaterialPropertiesRequest: JsonApiStruct
{
	string path;
	
	void MaterialPropertiesRequest()
	{
		RegV("path");
	}
};

class MaterialPropertiesResponse: JsonApiStruct
{
	string mat_props;
	
	void MaterialPropertiesResponse()
	{
		RegV("mat_props");
	}
};

class MaterialPropertiesUtils
{
	void GetEmat(ResourceName path, MaterialPropertiesResponse props)
	{
		BaseContainer ematCont;
		BaseContainer uvTransform;
		BaseContainer empty;
		string temp = ".";	
		string textureType;
		TextureUtils tUtils = new TextureUtils();
		
		Resource resource = Resource.Load(path);
				
				
		ematCont = resource.GetResource().ToBaseContainer();
		
		
		//Main loop for props		
		for(int i = 0; i < ematCont.GetNumVars(); i++)
		{	
			//If its a class -> Tiling
			if(ematCont.GetObject(ematCont.GetVarName(i)) != empty) //Uv Tiling
			{
				props.mat_props += ematCont.GetVarName(i)  + " " + "1 ";
				//Getting child and same thing as up
				uvTransform = ematCont.GetObject(ematCont.GetVarName(i));
				for(int j = 0; j < uvTransform.GetNumVars(); j++)
				{
					//Getting every tiling/rotation etc.
					uvTransform.Get(uvTransform.GetVarName(j), temp);
					if(temp != "")
					{
						//Output is stylized for processing in blender | Example: "DirtUVTransform_TilingU 5"
						props.mat_props += ematCont.GetVarName(i) + "_" + uvTransform.GetVarName(j) + " " + temp + " ";
					}
				}
			}
			//If its not class
			ematCont.Get(ematCont.GetVarName(i),temp);
			if(temp != "")
			{
				//Add Varname -> Color/Metalness/etc.
				props.mat_props += ematCont.GetVarName(i) + " ";
				if(temp.Contains(".edds"))
				{
					//If texture
					props.mat_props += tUtils.GetEdds(temp, false);
				}
				else if(temp.Contains(" "))
				{
					//Conversion from spaces to |
					temp.Replace(" ", "|");
					props.mat_props += temp + " ";
				}
				else
				{	
					//If nothing of that then just output
					props.mat_props += temp + " ";
				}
				temp = "";
			}

		}
		return;
	}

};

class MaterialProperties: NetApiHandler
{
	override JsonApiStruct GetRequest()
	{
		return new MaterialPropertiesRequest();
	}
	
	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		MaterialPropertiesRequest req = MaterialPropertiesRequest.Cast(request);
		MaterialPropertiesResponse props = new MaterialPropertiesResponse();
		MaterialPropertiesUtils utils = new MaterialPropertiesUtils();
		ResourceName path = req.path;
		
		
		utils.GetEmat(path, props);
		return props;
	}
}