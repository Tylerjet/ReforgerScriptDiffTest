//Input keys from blender
class MaterialPreviewRequest: JsonApiStruct
{
	ResourceName name;
	ref array<string> newRelPaths = new array<string>;
	ref array<string> matNames = new array<string>; 
	
	void MaterialPreviewRequest()
	{
		RegV("name");
		RegV("newRelPaths");
		RegV("matNames");
	}
};

//Output keys for blender
class MaterialPreviewGetMapping : JsonApiStruct
{
	string source_mat;
	string emat_rel_path;
	string mat_class;
	string issue;
	
	void MaterialPreviewGetMapping()
	{
		RegV("source_mat");
		RegV("emat_rel_path");
		RegV("mat_class");
		RegV("issue");
	}
};




class MaterialPreviewUtils
{
	

	
	void ReadXOB(ResourceName xob, MaterialPreviewGetMapping mapping, MaterialPreviewRequest req)
	{
		string absolutePath;
		string absPathEmat;
		ResourceName ematPath;
		string sourceMatText;
		string source;
		array<string<string>> sourceMat = new array<string<string>>;
		
		//Getting abs path from rel and finding meta file

		ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
		MetaFile meta = resourceManager.GetMetaFile(xob);
		if(meta == false)
		{
			mapping.issue = "Missing XOB";
			return;
		}
		BaseContainerList configurations = meta.GetObjectArray("Configurations");
		BaseContainer cfg = configurations.Get(0);

		cfg.Get("MaterialAssigns", sourceMatText);
		
		sourceMatText.Split(";", sourceMat, false);
		for(int i = 0; i < sourceMat.Count(); i++)
		{
			//Material name
			source = sourceMat[i].Substring(0,sourceMat[i].IndexOf(","));
			if(req.matNames.Contains(source))
			{
				ematPath = req.newRelPaths[req.matNames.Find(source)];
			}
			//Extracting rel path
			else
			{
				ematPath = sourceMat[i].Substring(sourceMat[i].IndexOf("{"), sourceMat[i].IndexOf(".emat") + 5 - sourceMat[i].IndexOf("{"));
			}
			Workbench.GetAbsolutePath(ematPath.GetPath(), absPathEmat);
			
			mapping.source_mat += source + ";";
			mapping.emat_rel_path += ematPath + ";";
			ReadEmat(ematPath, mapping);
		}		
		return;
	
	}
			
	void ReadEmat(ResourceName path, MaterialPreviewGetMapping mapping)
	{
		BaseContainer ematCont;
				
		//Getting ResourceClassName
		Resource resource = Resource.Load(path);						
		ematCont = resource.GetResource().ToBaseContainer();		
		mapping.mat_class += ematCont.GetClassName() + ";";

		return;
	}
	
};


class MaterialMapping: NetApiHandler
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
		//Input to variable
		ResourceName xob = req.name;		
		matutils.ReadXOB(xob, mapping, req);

		return mapping;
	}
}