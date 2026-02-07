class PrefabImporterRequest: JsonApiStruct
{
	string absPath;
	int start;
	
	void PrefabImporterRequest()
	{
		RegV("absPath");
		RegV("start");
	}
};


class PrefabImporterResponse: JsonApiStruct
{
	ref array<string> fbx = new array<string>;
	ref array<string> coords = new array<string>;
	ref array<string> angles = new array<string>;
	ref array<string> scales = new array<string>;
	ref array<string> pivots = new array<string>;
	ref array<string> ids = new array<string>;
	ref array<string> sourceMat = new array<string>;
	ref array<string> ematPath = new array<string>;
	ref array<int> parents = new array<int>;
	int stopped;
	int size;
	
	void PrefabImporterResponse()
	{
		RegV("fbx");
		RegV("coords");
		RegV("angles");
		RegV("scales");
		RegV("pivots");
		RegV("ids");
		RegV("sourceMat");
		RegV("ematPath");
		RegV("stopped");
		RegV("size");
		RegV("parents");
	}
};


class PrefabImporterUtils
{	
	
	int GetArrayIndex(BaseContainer prefab, string name)
	{	
		BaseContainerList srcList = prefab.GetObjectArray("components");
		for(int i = 0; i < srcList.Count(); i++)
		{
			prefab = srcList.Get(i);
			if(prefab.GetClassName() == name)
			{
				return i;
			}
		}
		return -1;
	}
	
	void GetPrefabParams(IEntitySource prefab, PrefabImporterResponse response)
	{
		BaseContainer empty;
		BaseContainer ancestorSource = prefab.GetAncestor();
		ResourceName ancestor = ancestorSource.GetResourceName();
		IEntitySource parent = prefab.GetParent();
		string pivotID = "";
		//If null object?
		if(ancestor == "")
		{
			return;
		}
		
		//Get Hierarchy 
		int hierarchy = GetArrayIndex(prefab, "Hierarchy");
		if(hierarchy != -1)
		{
			//Set pivots if exists
			BaseContainerList srcList = prefab.GetObjectArray("components");		
			BaseContainer prefabSocket = srcList.Get(hierarchy);
			ResourceName id = prefabSocket.GetResourceName();
			prefabSocket.Get("PivotID", pivotID);
			pivotID.ToLower();
			int index = 0;

			/*if(!pivotID.Contains("socket_door") && pivotID != "")
			{
				
				for(int i = 0; i < response.ids.Count(); i++)
				{
					if(response.ids[i] == prefab.GetID().ToString())
					{
						index += 1;
					}
				}
				if(index > 0)
				{
					pivotID = pivotID + "." + index.ToString(3);
				}
			}*/
			
			response.ids.Insert(prefab.GetID().ToString());
			response.pivots.Insert(pivotID);
		}
		else
		{
			//Fill if not
			response.pivots.Insert("xxx");
		}
		//All angles (Cuz every angle is one VarName)
		array<string> angles = {"angleX","angleY","angleZ"};
		
		//<-----------------Getting params----------------->\\
		string coords;
		string scale;
		
		if(prefab.IsVariableSet("coords"))
		{
			prefab.Get("coords", coords);
		}
		else
		{
			coords = "0 0 0";
		}
		
		if(prefab.IsVariableSet("scale"))
		{
			prefab.Get("scale", scale);
		}
		else
		{
			scale = "1";
		}
		string allAngles;
		for(int j = 0; j < 3; j++)
		{
			string angle;
			//If angle is set
			if(prefab.IsVariableSet(angles[j]))
			{
				//Get angle and add it to string
				prefab.Get(angles[j], angle);
				allAngles += " " + angle;
			}	
			else
			{
				//If not add 0
				allAngles += " 0";
			}
		}
		
		
		
		if(parent != empty)
		{
			int parentObjectID = GetArrayIndex(parent, "MeshObject");
			if(parentObjectID != -1)
			{
				string parentFbx = GetFBXPath(parent, parentObjectID, "Object", response, false);
				for(int i = response.fbx.Count() - 1; i > 0; i--)
				{
					if(response.fbx.Count() > 1 && parentFbx == response.fbx[i])
					{
						response.parents.Insert(i);
						/*array<string> currentVec = new array<string>;
						array<string> parentVec = new array<string>;
						coords.Split(" ", currentVec, true);
						response.coords[i].Split(" ", parentVec, true);
						coords = "";
						for(int x = 0; x < currentVec.Count(); x++)
						{
							string coord = (currentVec[x].ToFloat() + parentVec[x].ToFloat()).ToString();
							coords += coord + " ";
						}
						allAngles.Split(" ", currentVec, true);
						response.angles[i].Split(" ", parentVec, true);
						allAngles = "";
						for(int x = 0; x < currentVec.Count(); x++)
						{
							string angle = (currentVec[x].ToFloat() + parentVec[x].ToFloat()).ToString();
							allAngles += angle + " ";
						}*/
						break;
					}
				}
			}
		}
		
		//Insert
		response.coords.Insert(coords);
		response.scales.Insert(scale);
		response.angles.Insert(allAngles);
		//Clear string
		allAngles = "";
		
		
		if(response.parents.Count() != response.coords.Count())
		{
			response.parents.Insert(-1);
		}
		//<-----------------Getting params----------------->\\
		
		
		//I need to somehow ignore Pivot, cuz Ill get pivot from here but it will push another one from the PrefabParams
		/*BaseContainerList srcList = ancestorSource.GetObjectArray("components");
		for(int i = 0; i < srcList.Count(); i++)
		{
			BaseContainer test = srcList.Get(i);
			if(test.GetClassName() == "BaseSlotComponent")
			{
				ResourceName glass;
				test.Get("EntityPrefab",glass);
				
				Resource resource = Resource.Load(glass);
				BaseContainer glassCont = resource.GetResource().ToBaseContainer();
				if(glass != "")
				{
					BaseContainer attachType = test.GetObject("AttachType");
					ResourceName pivot;
					attachType.Get("PivotID", pivot);
					response.pivots.Insert(pivot);
				}
				GetPrefabParams(glassCont, response);
			}
		}*/

		
		
		
		int meshObjectID = GetArrayIndex(prefab, "MeshObject");
		if(meshObjectID != -1)
		{
			response.fbx.Insert(GetFBXPath(prefab, meshObjectID, "Object", response, true));
			ReadAncestor(ancestorSource, response, true);
		}
		else
		{
			response.fbx.Insert("Collection-" + FilePath.StripExtension(FilePath.StripPath(prefab.GetResourceName())));
			ReadAncestor(ancestorSource, response, true);
		}
		
		
		
		return;
	}
	
	
	string GetFBXPath(IEntitySource prefab, int meshObjectID,string meshVarName, PrefabImporterResponse response, bool getEmat)
	{
		ResourceName absPath, output, relXob, ematPath, absPathEmat;
		string matName;
		array<string> sourceMat = new array<string>;
		Workbench.GetAbsolutePath(prefab.GetResourceName().GetPath(), absPath);
		BaseContainerList srcList = prefab.GetObjectArray("components");
		IEntitySource prefabMesh = srcList.Get(meshObjectID);
		if(getEmat)
		{
			GetPrefabEmat(prefabMesh, response);
		}
		prefabMesh.Get(meshVarName,relXob);
		Workbench.GetAbsolutePath(relXob.GetPath(), output); // Boolean
		output.Replace("xob","fbx");
		//This will get me the children of prefab but there is a problem with coords
		/*for(int i = 0; i < prefab.GetNumChildren(); i++)
		{
			IEntitySource child = prefab.GetChild(i).GetAncestor();	
			GetPrefabParams(child, response);
		}*/
		return output;
	}
	
	
	
	void VehicleParams(BaseContainer prefab, int scrEntity, PrefabImporterResponse response)
	{
		BaseContainerList srcList = prefab.GetObjectArray("components");
		prefab = srcList.Get(scrEntity);
		srcList = prefab.GetObjectArray("m_aEntries");
		
		ResourceName xob;
		string fbx;
		string position, angle, scale, pivot; 
		for(int i = 0; i < srcList.Count(); i++)
		{
			prefab = srcList.Get(i);
			prefab.Get("m_Mesh",xob);
			prefab.Get("m_fScale", scale);
			prefab.Get("m_vPostion",position);
			prefab.Get("m_vAngles", angle);
			prefab.Get("m_iPivotID", pivot);
			
			Workbench.GetAbsolutePath(xob.GetPath(), fbx); // Boolean
			fbx.Replace("xob","fbx");
			response.fbx.Insert(fbx);
			response.coords.Insert(position);
			response.angles.Insert(angle);
			response.scales.Insert(scale);
			response.pivots.Insert(pivot);
		}
	}
	
	void GetPrefabEmat(IEntitySource prefab, PrefabImporterResponse response)
	{
		ResourceName ematPath, absPathEmat;
		string matName;
		array<string> sourceMat = new array<string>;
		prefab.Get("Materials", ematPath);
		if(ematPath != "")
		{
			//Long string (matName,relPath;....)
			ematPath.Split(";", sourceMat, false);
			for(int i = 0; i < sourceMat.Count(); i++)
			{
				//Material name
				matName = sourceMat[i].Substring(0,sourceMat[i].IndexOf(","));
				//Relative Path
				ematPath = sourceMat[i].Substring(sourceMat[i].IndexOf("{"), sourceMat[i].IndexOf(".emat") + 5 - sourceMat[i].IndexOf("{"));
				//Absolute Path
				Workbench.GetAbsolutePath(ematPath.GetPath(), absPathEmat);
				
				response.sourceMat.Insert(response.fbx.Count().ToString() + "|" + matName);
				response.ematPath.Insert(ematPath);
			}	
		}
	}
	
	
	
	void ReadAncestor(IEntitySource prefab, PrefabImporterResponse response, bool getMeshObject)
	{

		//Check if Ancestor of main prefab is null if yes then its a BasePrefab and we can return
		BaseContainer empty;
		BaseContainer ancestorMain = prefab.GetAncestor();
		if(ancestorMain == empty)
		{
			return;
		}
		
		ReadChild(prefab, response);
		ReadAncestor(ancestorMain, response, getMeshObject);
		return;
	}
	
	
	void ReadChild(IEntitySource prefab, PrefabImporterResponse response)
	{	
		for(int i = 0; i < prefab.GetNumChildren(); i++)
		{
			IEntitySource child = prefab.GetChild(i).ToEntitySource();
			if(child.GetClassName() == "GenericEntity" || child.GetClassName() == "Building" || child.GetClassName() == "GameEntity")
			{
				GetPrefabParams(child, response);
			}
		}
	}

	void ReadFirst(ResourceName path, PrefabImporterResponse response)
	{
		ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
		//Get MetaFile from AbsPath of prefab (.et)
		MetaFile meta = resourceManager.GetMetaFile(path);
		Resource resource = Resource.Load(meta.GetResourceID());
		BaseContainer prefab = resource.GetResource().ToEntitySource();
		BaseContainer ancestorPrefab = prefab.GetAncestor();

		//int scrEntity = GetArrayIndex(prefab, "SCR_PreviewEntityComponent");	
		//VehicleParams(prefab, scrEntity, response);
		
		GetPrefabParams(prefab, response);
		//ReadAncestor čte všechny děti co v sobě prefab má, proto když tohle volám jako druhé tak teprve zde jsou čteny děti toho main prefabu
		ReadChild(prefab, response);
		//ReadAncestor(prefab, response, true);		
		
	}

			
			
	int ClearJSON(PrefabImporterResponse response, int smth)
	{
		for(int i = 0; i < smth; i++)
		{
			response.fbx.RemoveOrdered(0);
			response.scales.RemoveOrdered(0);
			response.angles.RemoveOrdered(0);
			response.coords.RemoveOrdered(0);			
			response.pivots.RemoveOrdered(0);
			response.parents.RemoveOrdered(0);
		}
				
		//Odstraním vše co přebývá dokud nemám size na to to poslat
		while(response.GetSizeOf() > 60000)
		{
			response.fbx.RemoveOrdered(response.fbx.Count()-1);
			response.scales.RemoveOrdered(response.scales.Count()-1);
			response.angles.RemoveOrdered(response.angles.Count()-1);
			response.coords.RemoveOrdered(response.coords.Count()-1);			
			response.pivots.RemoveOrdered(response.pivots.Count()-1);
			response.parents.RemoveOrdered(response.parents.Count()-1);
		}
		int lenght = response.fbx.Count() + smth;		
				
		return lenght;
				
	}		
};

		


class PrefabImporter: NetApiHandler
{
	override JsonApiStruct GetRequest()
	{
		return new PrefabImporterRequest();
	}
	
	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		PrefabImporterRequest req = PrefabImporterRequest.Cast(request);
		PrefabImporterResponse response = new PrefabImporterResponse();
		PrefabImporterUtils utils = new PrefabImporterUtils();
		utils.ReadFirst(req.absPath, response);
		Print(response.fbx.Count());
		response.size = response.fbx.Count();
		response.stopped = utils.ClearJSON(response, req.start);
		return response;
	}
}