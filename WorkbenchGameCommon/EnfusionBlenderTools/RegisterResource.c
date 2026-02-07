class RegisterResourceRequest : JsonApiStruct
{
	ref array<string> path = new array<string>;
	string textureImportSize;

	void RegisterResourceRequest()
	{
		RegV("path");
		RegV("textureImportSize");
	}
}

class RegisterResourceResponse : JsonApiStruct
{
	bool Output;

	void RegisterResourceResponse()
	{
		RegV("Output");
	}
}

// Copied over from TextureImportTool
bool IsImage(string className)
{
	return
		className == "PNGResourceClass" ||
		className == "DDSResourceClass" ||
		className == "TGAResourceClass" ||
		className == "TIFFResourceClass" ||
		className == "PNGResourceClass" ||
		className == "HDRResourceClass" ||
		className == "PAAResourceClass" ||
		className == "JPGResourceClass";
}

class RegisterResourceUtils
{
	bool Register(string absPath, int textureImportSize = -1)
	{
		const int WAITING_TIME = 3000;
		ResourceManager rm = Workbench.GetModule(ResourceManager);
		MetaFile meta = rm.GetMetaFile(absPath);
		bool needsMetaUpdate = false;
		
		// check if meta doesnt exist already
		if (!meta)
		{
			// create if not
			meta = rm.RegisterResourceFile(absPath);
			needsMetaUpdate = true;
			// if creation was succesful
			if (!meta)	
				return false;
		}
		
		// save and rebuild
		BaseContainerList configurations = meta.GetObjectArray("Configurations");
		
		if (IsImage(configurations[0].GetClassName()))
		{
			needsMetaUpdate = true;
			array<ref TextureType> textureTypes = new array<ref TextureType>;
			TextureType.RegisterTypes(textureTypes);	
			FixTextureMetaFile(meta, absPath, textureTypes);
			
			if (textureImportSize > 0)
			{
				for (int c = 0; c < configurations.Count(); c++)
				{
					BaseContainer cfg = configurations.Get(c);
					
					cfg.Set("MaxSize", textureImportSize);
				}
			}
			else
			{
				for (int c = 0; c < configurations.Count(); c++)
				{
					BaseContainer cfg = configurations.Get(c);
					cfg.ClearVariable("MaxSize");
				}
			}
				
		}
		
		if (needsMetaUpdate)
		{
			meta.Save();
		
			rm.RebuildResourceFile(absPath, "PC", true);
			rm.WaitForFile(absPath, WAITING_TIME);
		}
		
		return true;

	}
}

class RegisterResource : NetApiHandler
{
	override JsonApiStruct GetRequest()
	{
		return new RegisterResourceRequest();
	}

	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		RegisterResourceRequest req = RegisterResourceRequest.Cast(request);
		RegisterResourceResponse response = new RegisterResourceResponse();
		RegisterResourceUtils utils = new RegisterResourceUtils();
		
		int textureImportSize = -1;
		if (req.textureImportSize != "")
			textureImportSize = req.textureImportSize.ToInt();

		// for each export path
		for (int i = 0; i < req.path.Count(); i++)
		{
			response.Output = utils.Register(req.path[i], textureImportSize);
		}
		return response;
	}
}
