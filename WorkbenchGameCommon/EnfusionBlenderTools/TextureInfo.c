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
	string GetEdds(ResourceName path, bool guidFormat, TextureResponse response = null)
	{
		string textureType;	
		string colorSpace;
		string conversion;
		BaseContainer eddsCont;
		
		if(!(FilePath.IsAbsolutePath(path)))
		{
			Workbench.GetAbsolutePath(path.GetPath(), path);
		}
		ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
		
		MetaFile meta = resourceManager.GetMetaFile(path);
		if(meta == false)
		{
			response.Issue = true;
			return "";
		}
		
		
		BaseContainerList configurations = meta.GetObjectArray("Configurations");
		BaseContainer cfg = configurations.Get(0);
		//Getting info
		cfg.Get("Conversion",conversion);
		cfg.Get("ColorSpace",colorSpace);
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
			return (colorSpace + "|" + conversion + "|" + meta.GetResourceID());
		}
		else
		{
			return (path + "|" + cfg.GetClassName() + "|" + colorSpace + "|" + conversion + " ");
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
		textureInfo[2] = FilePath.StripExtension(textureInfo[2]) + "_" + guid;
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
		string output = textureUtils.GetEdds(edds, true, response);
		if(response.Issue != true)
		{
			response.Output = textureUtils.ArrayToString(textureUtils.TextureFormat(output));
		}
		return response;
	}
}