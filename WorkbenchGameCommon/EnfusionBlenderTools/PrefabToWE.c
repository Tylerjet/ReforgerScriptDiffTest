class PrefabToWERequest: JsonApiStruct
{
	ref array<string> etPaths = new array<string>;
	ref array<int> numObjects = new array<int>;
	ref array<float> transformations = new array<float>;
	
	void PrefabToWERequest()
	{
		RegV("etPaths");
		RegV("numObjects");
		RegV("transformations");
	}
}

class PrefabToWEResponse: JsonApiStruct
{
	string Output;
	
	void PrefabToWEResponse()
	{
		RegV("Output");
	}
}

class PrefabToWEUtils
{
	bool CheckFloats(PrefabToWERequest req)
	{
		int totalNumInstances = 0;
		for(int c = 0; c < req.numObjects.Count(); c++)
		{
			totalNumInstances += req.numObjects[c];
		}
		if(req.transformations.Count() < totalNumInstances*6)
		{
			return false;
		}
		return true;
	}
	
	//From abs to Rel
	protected string GetPrefabResource(ResourceName absolutePath, PrefabToWEResponse response)
	{
		if(absolutePath.Contains("\\"))
		{
			absolutePath.Replace("\\","/");
		}
		ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
		MetaFile meta = resourceManager.GetMetaFile(absolutePath);
		if(!meta)
		{
			response.Output = "Wrong path to " + absolutePath + " or prefab is not registered!"; 
		}
		return (meta.GetResourceID());
	}
	//Main function
	void CreateEntity(ResourceName absPath, int numObjects, array<float> transformations, PrefabToWEResponse response, WorldEditorAPI api, string newLayer)
	{
		IEntity entity;
		//Position vec
		vector pos, rot;
		float scale;
		
		//Prefab
		ResourceName prefab = GetPrefabResource(absPath, response);
		//Start creating Entity
		int count = 0;
		//ClassName is ResourceName
		pos[0] = transformations[count];
		//Swap Y and Z axis because of Blender
		pos[1] = 0;
		pos[2] = transformations[count+1]; 
		//Degrees Also swapping axis
		rot[0] = transformations[count+2];
		rot[2] = transformations[count+3];
		rot[1] = transformations[count+4];
		scale = transformations[count+5];
		count += 6;
		entity = api.CreateEntity(prefab, "", api.GetEntityLayerId(0, newLayer), null, pos, rot);
		entity.SetScale(scale);
		//If not created
		if(!entity)
		{
			response.Output = "Entity couldn't be created! See console";
			return;
		}
		//Automaticaly select added prefab
		api.AddToEntitySelection(entity);
		return;
	}
}

class PrefabToWE: NetApiHandler
{
	override JsonApiStruct GetRequest()
	{
		return new PrefabToWERequest();
	}
	
	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		WorldEditorAPI api = worldEditor.GetApi();
		PrefabToWERequest req = PrefabToWERequest.Cast(request);
		PrefabToWEResponse response = new PrefabToWEResponse();		
		PrefabToWEUtils utils = new PrefabToWEUtils();

		if(!utils.CheckFloats(req))
		{
			response.Output = "Number of transformations doesn't match with total number of instances (Probably not every transformation is a float)";
			return response;
		}
		string newLayer = "NewLayer";
		api.BeginEntityAction();
		api.CreateSubsceneLayer(0,newLayer);
		for(int i = 0; i < req.etPaths.Count(); i++)
		{
			for(int j = 0; j < req.numObjects[i]; j++)
			{
				utils.CreateEntity(req.etPaths[i], req.numObjects[i], req.transformations, response, api, newLayer);
				for(int k = 0; k <= 5; k++)
				{
					req.transformations.RemoveOrdered(0);
				}
			}
		}
		api.EndEntityAction();
		response.Output = "Prefab was succesfully imported to WE!";
		return response;
	}

}