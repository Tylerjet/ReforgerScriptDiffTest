//Input keys from blender
class MaterialPreviewRequest : JsonApiStruct
{
	ResourceName name;
	ref array<string> newRelPaths = new array<string>;
	ref array<string> matNames = new array<string>;
	bool fromXob = true;

	void MaterialPreviewRequest()
	{
		RegV("name");
		RegV("newRelPaths");
		RegV("matNames");
		RegV("fromXob");
	}
}

//Output keys for blender
class MaterialPreviewGetMapping : JsonApiStruct
{
	string source_mat;
	string emat_rel_path;
	string mat_class;
	bool xobMissing;
	ref array<string> matMissing = new array<string>;

	void MaterialPreviewGetMapping()
	{
		RegV("source_mat");
		RegV("emat_rel_path");
		RegV("mat_class");
		RegV("xobMissing");
		RegV("matMissing");
	}
}


class MaterialPreviewUtils
{
	void ReadXOB(ResourceName xob, MaterialPreviewGetMapping mapping, MaterialPreviewRequest req)
	{
		string absPathEmat;
		ResourceName ematPath;
		EBTEmatUtils ematUtils = new EBTEmatUtils();
		map<string, ResourceName> materials = new map<string, ResourceName>();
		
		// get assigned materials
		bool meta = ematUtils.GetMaterials(xob, materials);
		if(!meta)
		{
			mapping.xobMissing = true;
			return;
		}
		
		
		for (int i = 0; i < materials.Count(); i++)
		{
			string matName = materials.GetKey(i);
			// if matName is in sent names return path
			if(req.matNames.Contains(matName))
			{
				ematPath = req.newRelPaths[req.matNames.Find(matName)];
			}
			// if not get its path from Xob Meta
			else
			{
				ematPath = materials.Get(matName);
			}
			
			// get absolute path to the emat
			Workbench.GetAbsolutePath(ematPath.GetPath(), absPathEmat);
			if(!absPathEmat)
			{
				mapping.matMissing.Insert(matName);
			}
			// set response values
			
			mapping.source_mat += matName + ";";
			mapping.emat_rel_path += ematPath + ";";
			mapping.mat_class += GetEmatClass(ematPath) + ";";
		}
		return;
	}

	// get class
	string GetEmatClass(ResourceName path)
	{
		Resource resource = Resource.Load(path);
		BaseContainer ematCont = resource.GetResource().ToBaseContainer();
		return(ematCont.GetClassName());
	}

}


class MaterialMapping : NetApiHandler
{
	override JsonApiStruct GetRequest()
	{
		return new MaterialPreviewRequest();
	}


	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		MaterialPreviewRequest req = MaterialPreviewRequest.Cast(request);
		MaterialPreviewGetMapping mapping = new MaterialPreviewGetMapping();
		MaterialPreviewUtils matutils = new MaterialPreviewUtils();
		
		// check if script is called from FBXExport or EmatExport
		ResourceName xob = req.name;
		if (req.fromXob)
		{
			matutils.ReadXOB(xob, mapping, req);
		}
		else
		{
			mapping.mat_class = matutils.GetEmatClass(req.name);
		}

		Print(mapping.source_mat);
		Print(mapping.mat_class);
		Print(mapping.emat_rel_path);
		return mapping;
	}
}
