class RegisterResourceRequest : JsonApiStruct
{
	ref array<string> path = new array<string>;
	string textureImportSize;
	bool hasMorphs;
	bool hasEmpty;
	bool hasArmature;
	
	void RegisterResourceRequest()
	{
		RegV("path");
		RegV("textureImportSize");
		RegV("hasMorphs");
		RegV("hasEmpty");
		RegV("hasArmature");
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
	bool Register(string absPath, int textureImportSize = -1, bool hasMorphs = false, bool hasEmpty = false, bool hasArmature = false)
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
			
			// if creation was succesful
			if (!meta)	
				return false;
		}
	
		
		// save and rebuild
		BaseContainerList configurations = meta.GetObjectArray("Configurations");
		BaseContainer cfg = configurations.Get(0);
		
		if(hasMorphs)
		{				
			cfg.Set("ExportMorphs", 1);
		}
		else
		{
			cfg.ClearVariable("ExportMorphs");
		}

		if(hasEmpty)
		{				
			cfg.Set("ExportSceneHierarchy", 1);
		}
		else
		{
			cfg.ClearVariable("ExportSceneHierarchy");
		}
		
		if(hasArmature)
		{				
			cfg.Set("ExportSkinning", 1);
		}
		else
		{
			cfg.ClearVariable("ExportSkinning");
		} 
		needsMetaUpdate = true;

		if (IsImage(configurations[0].GetClassName()))
		{
			needsMetaUpdate = true;
			TextureTypes textureTypes = new TextureTypes();
			textureTypes.DoChecks(TextureIssueOp.Fix, meta.GetResourceID(), meta);
			
			/*if (textureImportSize > 0)
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
			}*/
				
		}
		
		if (needsMetaUpdate)
		{
			meta.Save();
			//Print("Meta updated, saving changes.");
			
			rm.RebuildResourceFile(absPath, "PC", true);
			rm.WaitForFile(absPath, WAITING_TIME);
		}
		/*else
        {
            //Print("No changes made to Meta file.");
        }*/
		
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
			response.Output = utils.Register(req.path[i], textureImportSize, req.hasMorphs, req.hasEmpty, req.hasArmature);
		}
		return response;
	}
}