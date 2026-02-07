class TextureRequest: JsonApiStruct
{
	string Edds;
	
	void TextureRequest()
	{
		RegV("Edds");
	}
};

class TextureResponse: JsonApiStruct
{
	string Output;
	bool Issue;
	
	void TextureResponse()
	{
		RegV("Output");
		RegV("Issue");
	}
};

class TextureUtils
{
	string GetEdds(ResourceName path, bool guidFormat)
	{
		string textureType;	
		string colorSpace;
		bool containsMips;
		string conversion;
		BaseContainer eddsCont;
		

		if(!(FilePath.IsAbsolutePath(path)))
		{
			Workbench.GetAbsolutePath(path.GetPath(), path, true);
			//If GUID wont be found
			//It will find the resource by its RelPath and add " 0" to it, so remove the 0 and use the path
			if(path.EndsWith(" 0"))
			{
				path = path.Substring(0, path.Length() - 2);
			}
		}
		ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
		MetaFile meta = resourceManager.GetMetaFile(path);
		//If it won't find the meta anyways, Add Missing texture 
		if(!meta)
		{
			return "";
		}
		
		BaseContainerList configurations = meta.GetObjectArray("Configurations");
		BaseContainer cfg = configurations.Get(0);
		//Getting info
		cfg.Get("Conversion",conversion);
		cfg.Get("ColorSpace",colorSpace);
		cfg.Get("ContainsMips", containsMips);
		
		string resourceType = cfg.GetClassName();
		//Translating int conversion to string
		switch(conversion)
		{
			case "3":
				conversion = "RedGreenHQCompression";
				break;
			case "8":
				conversion = "ColorHQCompression";
				break;
			default:
				conversion = "*"; //If needed add more types of Compression
				break;		
		}
		//Translating int color-space to string 
		if(colorSpace == "1")
		{
			colorSpace = "TosRGB";
		}
		else
		{
			colorSpace = "ToLinear";
		}
		if(guidFormat)
		{
			return (colorSpace + "|" + conversion + "|" + meta.GetResourceID()+ "|" + containsMips);
		}
		else
		{
			return (path + "|" + cfg.GetClassName() + "|" + colorSpace + "|" + conversion + "|" + containsMips + "||");	
		}
	} 	
	array<string> TextureFormat(string texture)
	{
		array<string> textureInfo = new array<string>;
		texture.Split("|", textureInfo, true);
		// ColorSpace format for blender
		if(textureInfo[0] == "ToLinear")
		{
			textureInfo[0] = "Raw";
		}
		else
		{
			textureInfo[0] = "sRGB";
		}
		//Getting GUID from Name
		string guid = textureInfo[2].Substring(textureInfo[2].IndexOf("{") + 1, textureInfo[2].IndexOf("}") - 1);
		//Stripping Path and Extesion to get just a name of the file
		textureInfo[2] = FilePath.StripPath(textureInfo[2]);
		textureInfo[2] = FilePath.StripExtension(textureInfo[2]);
		return textureInfo;
	}
	string ArrayToString(array<string> arr)
	{
		string output;
		for(int i = 0; i < arr.Count(); i++)
		{
			if(i + 1 != arr.Count())
			{
				output += arr[i] + " ";
			}
			else
			{	
				output += arr[i];
			}
		}
		return output;
	}
};

class TextureInfo: NetApiHandler
{
	override JsonApiStruct GetRequest()
	{
		return new TextureRequest();
	}
	
	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		TextureRequest req = TextureRequest.Cast(request);
		TextureResponse response = new TextureResponse();		
		TextureUtils textureUtils = new TextureUtils();
		
		ResourceName edds = req.Edds;	
		//Format: Color_Space Conversion GUID
		string output = textureUtils.GetEdds(edds, true);
		if(output != "")
		{
			response.Output = textureUtils.ArrayToString(textureUtils.TextureFormat(output));
		}
		else
		{
			response.Issue = true;
		}
		return response;
	}
}