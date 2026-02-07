class TextureRequest : JsonApiStruct
{
	string Edds;

	void TextureRequest()
	{
		RegV("Edds");
	}
}

class TextureResponse : JsonApiStruct
{
	ref array<string> textureInfo = new array<string>();
	bool Issue;

	void TextureResponse()
	{
		RegV("Issue");
		RegV("textureInfo");
	}
}

class TextureUtils
{
	array<string> GetEdds(ResourceName path, bool guidFormat)
	{
		string colorSpace;
		bool containsMips;
		string conversion;
		
		if (!(FilePath.IsAbsolutePath(path)))
		{
			Workbench.GetAbsolutePath(path.GetPath(), path, true);
			// if GUID wont be found
			// it will find the resource by its RelPath and add " 0" to it, so remove the 0 and use the path
			if (path.EndsWith(" 0"))
			{
				path = path.Substring(0, path.Length() - 2);
			}
		}
		ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
		MetaFile meta = resourceManager.GetMetaFile(path);
		// if it won't find the meta anyways, Add empty texture
		if (!meta)
		{
			return {};
		}

		BaseContainerList configurations = meta.GetObjectArray(EBTContainerFields.conf);
		BaseContainer cfg = configurations.Get(0);
		// getting info
		cfg.Get(EBTContainerFields.conversion, conversion);
		cfg.Get(EBTContainerFields.colorSpace, colorSpace);
		cfg.Get(EBTContainerFields.mips, containsMips);

		string resourceType = cfg.GetClassName();

		// translating int color-space to string
		// don't need anything else than TosRGB and ToLinear
		if (colorSpace == "1")
		{
			colorSpace = EBTContainerFields.tosRGB;
		}
		else
		{
			colorSpace = EBTContainerFields.toLinear;
		}
		
		// translating int conversion to string
		switch (conversion)
		{
			case "3":
				conversion = EBTContainerFields.rgHQCompression;
				break;
			case "8":
				conversion = EBTContainerFields.colorHQCompression;
				break;
			default:
				conversion = "*"; // If needed add more types of Compression
				break;
		}
		
		// if GUID is needed
		if (guidFormat)
		{
			return {colorSpace, conversion, meta.GetResourceID(), containsMips.ToString()};
		}
		else
		{
			return {path, cfg.GetClassName(), colorSpace, conversion, containsMips.ToString()};
			
		}
	}
	
}

class TextureInfo : NetApiHandler
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
		//Format: Color_Space,Conversion,ResourceName,Mips
		response.textureInfo = textureUtils.GetEdds(edds, true);
		
		// empty if meta not found
		if(response.textureInfo.Count() == 0)
		{
			response.Issue = true;
			return response;
		}
		
		// convert Color space for Blender
		if (response.textureInfo[0] == EBTContainerFields.toLinear)
		{
			response.textureInfo[0] = EBTContainerFields.raw;
		}
		else
		{
			response.textureInfo[0] = EBTContainerFields.sRGB;
		}
		
		// Strip path and extension from ResourceName, only Name and suffix needed
		response.textureInfo[2] = FilePath.StripPath(response.textureInfo[2]);
		response.textureInfo[2] = FilePath.StripExtension(response.textureInfo[2]);
		return response;
	}
}
