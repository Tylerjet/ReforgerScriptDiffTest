
class ExportEmatRequest : JsonApiStruct
{
	// since JsonApiStruct doesnt support maps or 2D arrays to be send
	// uses string array where
	// i = property ; i+1 = Value
	//i+1 is never out of index
	ref array<string> properties = new array<string>;
	string path;
	bool create;
	bool xobAssignedMaterial;
	string shaderClass;
	string materialName;

	void ExportEmatRequest()
	{
		RegV("properties");
		RegV("path");
		RegV("create");
		RegV("xobAssignedMaterial");
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
		ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
		
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
					string importPath = properties[i+1];
					MetaFile meta = resourceManager.GetMetaFile(importPath);
					
					// if its texture get its ResourceID
					if (FilePath.IsAbsolutePath(properties[i+ 1]))
					{
						RegisterResourceUtils utils = new RegisterResourceUtils();
						
						if(!LoadedProjects.InLoadedProjects(importPath))
						{
							// Get importPath to emat folder
							Workbench.GetAbsolutePath(path.GetPath(), importPath);
							importPath = FilePath.StripExtension(importPath);
							importPath = FilePath.StripFileName(importPath);
							
							// add texture name
							importPath += FilePath.StripPath(properties[i+1]);
							
							// import 
							FileIO.CopyFile(properties[i+1], importPath);
							
							meta = resourceManager.GetMetaFile(importPath);
							// Register the texture that was copied to dataFolder
							utils.Register(importPath);
							
							
						}	
						

						properties[i+1] = meta.GetResourceID();
					}

					// if its UVTransform
					if (properties[i+ 1] == EBTContainerFields.UVTransform)
					{
						// create MatUVTransform class
						string transformType = properties[i];
						Resource matUVTransform = BaseContainerTools.CreateContainer(EBTContainerFields.UVTransform);
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
		// path to dataFolder
		string dataFolder = save + "\\Data\\";
		
		// absPath to emat
		save = dataFolder + matName + ".emat";
		
		// creating Emat container with correct class
		Resource res = BaseContainerTools.CreateContainer(classname);
		BaseContainer cont = res.GetResource().ToBaseContainer();
		string defaultVal;
		bool defaultObject = true;

		// looping through properties from blender
		for (int i = 0; i < properties.Count(); i+=2)
		{
			// if its texture set the ResourceID
			Print("HUH");
			Print(properties[i+1]);
			if (FilePath.IsAbsolutePath(properties[i+ 1]))
			{
				RegisterResourceUtils utils = new RegisterResourceUtils();
				ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
				
				
				string importPath = properties[i+1];
				Print(importPath);
				if(!LoadedProjects.InLoadedProjects(importPath))
				{
					importPath = dataFolder + FilePath.StripPath(properties[i+1]);
					
					// import 
					FileIO.CopyFile(properties[i+1], importPath);
				}	
				MetaFile meta = resourceManager.GetMetaFile(importPath);
						
				// Register the texture that was copied to dataFolder
				utils.Register(importPath);
				
				// get ResourceName from metaFile and assign it
				meta = resourceManager.GetMetaFile(importPath);
				properties[i+1] = meta.GetResourceID();
			}

			// if its uvTransforms
			if (properties[i+ 1] == EBTContainerFields.UVTransform)
			{
				// create new container
				defaultObject = true;
				Resource matUVTransform = BaseContainerTools.CreateContainer(EBTContainerFields.UVTransform);
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

		const int WAITING_TIME = 3000;
		
		ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
		MetaFile meta = resourceManager.RegisterResourceFile(save);
		meta.Save();
		resourceManager.RebuildResourceFile(save,"PC", true);
		resourceManager.WaitForFile(save,WAITING_TIME);
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

		response.inProject = LoadedProjects.InLoadedProjects(req.path);
		if (!response.inProject)
		{
			return response;
		}
		
					
		if (req.create)
		{
			Print("CREATE");
			response.guid = utils.CreateEmat(req.path, req.shaderClass, req.properties, req.materialName, response);
			
		}
		else
		{
			Print("EDIT XOB");
			utils.EditEmat(utils.FindEmat(req.path, true, req.materialName), req.shaderClass, req.properties);
		}
		return response;
	}
}
