
class ExportEmatRequest : JsonApiStruct
{
	// since JsonApiStruct doesnt support maps or 2D arrays to be send
	// uses string array where
	// i = property ; i+1 = Value
	//i+1 is never out of index
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
}

class ExportEmatResponse : JsonApiStruct
{
	string guid;
	bool firstExport = false;
	bool inProject;

	void ExportEmatResponse()
	{
		RegV("guid");
		RegV("firstExport");
		RegV("inProject");
	}
}


class ExportEmatUtils
{

	// get absolute path to a material from a mat name
	string FindEmat(ResourceName xob, bool relPath, string materialName)
	{		
		map<string, ResourceName> materials = new map<string,ResourceName>();
		ResourceName ematPath, absPath;
		
		EBTEmatUtils ematUtils = new EBTEmatUtils();
		ematUtils.GetMaterials(xob, materials);
		
		// match the export material with assigned materials
		for (int i = 0; i < materials.Count(); i++)
		{
			if(materials.GetKey(i) == materialName)
			{
				ematPath = materials.Get(materials.GetKey(i));
			}
		}

		if (relPath)
		{
			return ematPath;
		}
		// convert to absolute path
		Workbench.GetAbsolutePath(ematPath.GetPath(), absPath);
		return absPath;
	}

	void EditEmat(ResourceName path, string classname, array<string> properties)
	{
		// load emat
		Resource resource = Resource.Load(path);
		BaseContainer cont = resource.GetResource().ToBaseContainer();

		string defaultValue;
		string value;
		bool uvTrans = false;

		// looping through properties from Blender
		for (int i = 0; i < properties.Count(); i+=2)
		{
			// looping through properties in .emat
			for (int j = 0; j < cont.GetNumVars(); j++)
			{
				// matching the property
				if (properties[i] == cont.GetVarName(j))
				{
					// if its texture get its ResourceID
					if (FilePath.IsAbsolutePath(properties[i+ 1]))
					{
						ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
						MetaFile meta = resourceManager.GetMetaFile(properties[i+ 1]);
						properties[i+ 1] = meta.GetResourceID();
					}

					// if its UVTransform
					if (properties[i+ 1] == EBTConfig.UVTransform)
					{
						// create MatUVTransform class
						string transformType = properties[i];
						Resource matUVTransform = BaseContainerTools.CreateContainer(EBTConfig.UVTransform);
						BaseContainer uvTransform = matUVTransform.GetResource();

						// looping through the UVTransforms and setting its values
						for (int a = 0; a < uvTransform.GetNumVars(); a++)
						{
							// moving to the correct property since its a new container and the setting is here
							i += 2;
							uvTransform.Set(uvTransform.GetVarName(a), properties[i+ 1]);
						}
						// setting the MatUVTransform to the .emat and moving the index of emat props +1 since it's already set and the properties was moved as well
						cont.SetObject(transformType, uvTransform);
						j++;
					}
					else
					{
						// getting default value of the property
						cont.Get(cont.GetVarName(j), value);
						cont.GetDefaultAsString(cont.GetVarName(j), defaultValue);
						// if value is NONE or default clear the parameter from the emat
						if (properties[i + 1] == "NONE" || properties[i + 1] == defaultValue)
						{
							cont.ClearVariable(cont.GetVarName(j));
						}
						// if its float/vector/string property and not default set it
						else if (properties[i+ 1] != value)
						{
							cont.Set(cont.GetVarName(j), properties[i+ 1]);
						}
					}
				}
			}
		}
		// save
		BaseContainerTools.SaveContainer(cont, path);
		return;
	}

	string CreateEmat(string save, string classname, array<string> properties, string matName, ExportEmatResponse response)
	{
		// create Data folder if one doesn't exists
		if (!FileIO.FileExists(save + "\\Data"))
		{
			FileIO.MakeDirectory(save + "\\Data");
		}
		save = save + "\\Data\\" + matName + ".emat";

		// creating Emat container with correct class
		Resource res = BaseContainerTools.CreateContainer(classname);
		BaseContainer cont = res.GetResource().ToBaseContainer();
		string defaultVal;
		bool defaultObject = true;

		// looping through properties from blender
		for (int i = 0; i < properties.Count(); i+=2)
		{
			// if its texture set the ResourceID
			if (FilePath.IsAbsolutePath(properties[i+ 1]))
			{
				ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
				MetaFile meta = resourceManager.GetMetaFile(properties[i+ 1]);
				properties[i+ 1] = meta.GetResourceID();
			}

			// if its uvTransforms
			if (properties[i+ 1] == EBTConfig.UVTransform)
			{
				// create new container
				defaultObject = true;
				Resource matUVTransform = BaseContainerTools.CreateContainer(EBTConfig.UVTransform);
				BaseContainer uvTransform = matUVTransform.GetResource().ToBaseContainer();
				string transformType = properties[i];
				// loop through transforms
				for (int j = 0; j < uvTransform.GetNumVars(); j++)
				{
					// moving to new property (UVTransform) since the property setting is in this loop, since its a different container
					i += 2;
					// if the transform is not default set it
					uvTransform.GetDefaultAsString(uvTransform.GetVarName(j), defaultVal);
					if (defaultVal.ToFloat() != properties[i+ 1].ToFloat())
					{
						defaultObject = false;
						uvTransform.Set(uvTransform.GetVarName(j), properties[i+ 1]);
					}
				// if atleast one of the transforms wasn't default set the MatUVTransform container to the emat
				}
				if (!defaultObject)
				{
					cont.SetObject(transformType, uvTransform);
				}
			}
			// setting all other properties
			else
			{
				cont.Set(properties[i], properties[i+ 1]);
			}
		}
		// saving the emat file and register meta
		BaseContainerTools.SaveContainer(cont, "", save);

		ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
		MetaFile meta = resourceManager.RegisterResourceFile(save);
		meta.Save();
		resourceManager.RebuildResourceFile(save,"PC", true);
		response.firstExport = true;
		return meta.GetResourceID();
	}

}


class ExportEmat : NetApiHandler
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

		// check if the export path is in LoadedProjects
		response.inProject = LoadedProjects.InLoadedProjects(req.path);
		if (!response.inProject)
		{
			return response;
		}


		if (req.create)
		{
			response.guid = utils.CreateEmat(req.path, req.shaderClass, req.properties, req.materialName, response);
		}
		else
		{
			utils.EditEmat(utils.FindEmat(req.path, true, req.materialName), req.shaderClass, req.properties);
		}
		return response;
	}
}
