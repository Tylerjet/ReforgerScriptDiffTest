class ExportEmatRequest: JsonApiStruct
{
	ref array<string> properties = new array<string>;
	string path;
	bool create;
	string shaderClass;
	string materialName;
	
	void ExportEmatRequest()
	{
		RegV("properties");
		RegV("path");
		RegV("create");
		RegV("shaderClass");
		RegV("materialName");
	}
};

class ExportEmatResponse: JsonApiStruct
{
	string guid;
	bool inProject;
	
	void ExportEmatResponse()
	{
		RegV("guid");
		RegV("inProject");
	}
};


class ExportEmatUtils
{
	
	string GetEmat(ResourceName xob, bool relPath, string materialName)
	{
		ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
		MetaFile meta = resourceManager.GetMetaFile(xob);
		BaseContainerList configurations = meta.GetObjectArray("Configurations");
		BaseContainer cfg = configurations.Get(0);
		string sourceMatText;
		array<string> sourceMat = new array<string>;
		string absPath;
		ResourceName ematPath;
		
		cfg.Get("MaterialAssigns", sourceMatText);
		sourceMatText.Split(";", sourceMat, false);
		
		for(int i = 0; i < sourceMat.Count(); i++)
		{
			if(sourceMat[i].Substring(0,sourceMat[i].IndexOf(",")) == materialName)
			{
				ematPath = sourceMat[i].Substring(sourceMat[i].IndexOf(",") + 1, sourceMat[i].Length() - sourceMat[i].IndexOf(",") - 1);
			}
		}

		if(relPath)
		{
			return ematPath;
		}
		Workbench.GetAbsolutePath(ematPath.GetPath(), absPath);
		return absPath;
	}
	
	void EditEmat(ResourceName path, string classname, array<string> properties)
	{	
		Resource resource = Resource.Load(path);
		BaseContainer cont = resource.GetResource().ToBaseContainer();
		

		string defaultValue;
		string value;
		bool uvTrans = false;

		for(int i = 0; i < properties.Count(); i+=2)
		{
			for(int j = 0; j < cont.GetNumVars(); j++)
			{
				if(properties[i] == cont.GetVarName(j))
				{
							
					if(FilePath.IsAbsolutePath(properties[i+1]))
					{
						ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
						MetaFile meta = resourceManager.GetMetaFile(properties[i+1]);	
						properties[i+1] = meta.GetResourceID();
					}
							
					if(properties[i+1] == "MatUVTransform")
					{
						string transformType = properties[i];
						Resource matUVTransform = BaseContainerTools.CreateContainer("MatUVTransform");
						BaseContainer uvTransform = BaseContainer.Cast(matUVTransform.GetResource());
						for(int a = 0; a < uvTransform.GetNumVars(); a++)
						{
							i += 2;
							uvTransform.Set(uvTransform.GetVarName(a), properties[i+1]);		
						}
						cont.SetObject(transformType, uvTransform);
						j++;
					}
					cont.Get(cont.GetVarName(j), value);
					cont.GetDefaultAsString(cont.GetVarName(j), defaultValue);
					if(properties[i + 1] == "NONE" || properties[i + 1] == defaultValue)
					{
						cont.ClearVariable(cont.GetVarName(j));
					}
					else if(properties[i+1] != value)
					{	
						cont.Set(cont.GetVarName(j), properties[i+1]);
					}
				}		
			}
		}
		BaseContainerTools.SaveContainer(cont, path);
		return;
	}
					
	string CreateEmat(string save, string classname, array<string> properties, string matName, ExportEmatResponse response)
	{
		if(!FileIO.FileExists(save + "\\Data"))
		{
			FileIO.MakeDirectory(save + "\\Data");
		}
		save = save + "\\Data\\" + matName + ".emat";

		
		Resource res = BaseContainerTools.CreateContainer(classname);
		BaseContainer cont = res.GetResource().ToBaseContainer();
		string defaultVal;
		bool defaultObject = true;

		for(int i = 0; i < properties.Count(); i+=2)
		{
			if(FilePath.IsAbsolutePath(properties[i+1]))
			{
				ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
				MetaFile meta = resourceManager.GetMetaFile(properties[i+1]);	
				properties[i+1] = meta.GetResourceID();
			}

			if(properties[i+1] == "MatUVTransform")
			{
				defaultObject = true;
				Resource matUVTransform = BaseContainerTools.CreateContainer("MatUVTransform");
				BaseContainer uvTransform = matUVTransform.GetResource().ToBaseContainer();
				string transformType = properties[i];
				for(int j = 0; j < uvTransform.GetNumVars(); j++)
				{
					i += 2;	
					uvTransform.GetDefaultAsString(uvTransform.GetVarName(j), defaultVal);
					if(defaultVal.ToFloat() != properties[i+1].ToFloat())
					{
						defaultObject = false;
						uvTransform.Set(uvTransform.GetVarName(j), properties[i+1]);
					}			
				}
				if(!defaultObject)
				{
					cont.SetObject(transformType, uvTransform);		
				}
			}
			else
			{
				cont.Set(properties[i], properties[i+1]);		
			}
		}
		BaseContainerTools.SaveContainer(cont, "", save);

		ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
		MetaFile meta = resourceManager.RegisterResourceFile(save);
		meta.Save();
		resourceManager.RebuildResourceFile(save,"PC", true);
		return "1"+meta.GetResourceID();	
	}

};


class ExportEmat: NetApiHandler
{
	override JsonApiStruct GetRequest()
	{
		return new ExportEmatRequest();
	}
	
	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		ExportEmatRequest req = ExportEmatRequest.Cast(request);
		ExportEmatResponse response = new ExportEmatResponse();
		ExportEmatUtils utils = new ExportEmatUtils();
		
		
		response.inProject = LoadedProjects.InLoadedProjects(req.path);
		if(!response.inProject)
		{
			return response;
		}

		
		
		if(req.create)
		{
			response.guid = utils.CreateEmat(req.path,req.shaderClass, req.properties, req.materialName, response);
		}
		else
		{
			utils.EditEmat(utils.GetEmat(req.path, true, req.materialName), req.shaderClass, req.properties);	
		}
		return response;
	}
}