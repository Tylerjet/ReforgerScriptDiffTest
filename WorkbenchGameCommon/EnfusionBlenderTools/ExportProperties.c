class ExportPropertiesRequest: JsonApiStruct
{
	string path;
	
	void ExportPropertiesRequest()
	{
		RegV("path");
	}
}

class ExportPropertiesResponse: JsonApiStruct
{
	// List of GeometryParam Names
	ref array<string> geoParamName = new array<string>;
	//List of LayerPresets
	ref array<string> layPres = new array<string>;
	// List of SurfaceProperties
	ref array<ResourceName> surProps = new array<ResourceName>;
	// List of different SurfaceProp list sizes
	ref array<int> spCount = new array<int>;
	
	void ExportPropertiesResponse()
	{
		RegV("geoParamName");
		RegV("layPres");
		RegV("surProps");
		RegV("spCount");
	}
}

class ExportPropertiesUtils
{
	bool GetProps(string absPath, ExportPropertiesResponse response)
	{
		const int WAITING_TIME = 3000;
		ResourceManager rm = Workbench.GetModule(ResourceManager);
		MetaFile meta = rm.GetMetaFile(absPath);

		// check if meta exists
		if (!meta)
		{
			return false;
		}
	
		
		// get data
		BaseContainerList configurations = meta.GetObjectArray("Configurations");
		BaseContainer cfg = configurations.Get(0);
		BaseContainerList geometryParams = cfg.GetObjectArray("GeometryParams");
		
		for(int i = 0; i < geometryParams.Count(); i++)
		{
			// get obj name
			string gpName = geometryParams[i].GetName();
			// get layer preset
			string layerPreset = "";
			geometryParams[i].Get("LayerPreset", layerPreset);
			// get all gamemats (surface properties) for this object
			array<ResourceName> surfaceProps = {};
			geometryParams[i].Get("SurfaceProperties", surfaceProps);
			
			// write into response
			response.geoParamName.Insert(gpName);
			response.layPres.Insert(layerPreset);
			response.surProps.InsertAll(surfaceProps);
			response.spCount.Insert(surfaceProps.Count());
			
		}
		
		// edit surface properties to have a correct format
		for(int i = 0; i < response.surProps.Count(); i++)
		{
			string guid;
			string name;
			if (response.surProps[i].Contains("/"))
			{
				name = response.surProps[i].Substring(response.surProps[i].LastIndexOf("/") + 1, response.surProps[i].IndexOf(".") - response.surProps[i].LastIndexOf("/") - 1);
			}
			else
			{
				name = response.surProps[i].Substring(response.surProps[i].IndexOf("}") + 1, response.surProps[i].IndexOf(".") - response.surProps[i].IndexOf("}") - 1);
			}
			// getting GUID
			guid = response.surProps[i].Substring(response.surProps[i].IndexOf("{") + 1, response.surProps[i].IndexOf("}") - 1);
			// adding them to the format
			response.surProps[i] = (name + "_" + guid);
		}
		return true;		
	}	
}
	
class ExportProperties : NetApiHandler
{
	override JsonApiStruct GetRequest()
	{
		return new ExportPropertiesRequest();
	}

	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		ExportPropertiesRequest req = ExportPropertiesRequest.Cast(request);
		ExportPropertiesResponse response = new ExportPropertiesResponse();
		ExportPropertiesUtils utils = new ExportPropertiesUtils();
		
		utils.GetProps(req.path, response);
		return response;
	}
}