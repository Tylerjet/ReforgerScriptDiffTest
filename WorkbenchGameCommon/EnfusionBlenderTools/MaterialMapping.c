//Input keys from blender
class MaterialPreviewRequest: JsonApiStruct
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
		if(!meta)
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
			if(!absPathEmat)
			{
				mapping.issue = "Emat file " + sourceMat[i].Substring(sourceMat[i].IndexOf("{"), sourceMat[i].IndexOf(".emat") + 5 - sourceMat[i].IndexOf("{")) + " couldn't be found!";
				return;	
			}
			mapping.source_mat += source + ";";
			mapping.emat_rel_path += ematPath + ";";
			mapping.mat_class += ReadEmat(ematPath) + ";";
		}		
		return;
	
	}
			
	string ReadEmat(ResourceName path)
	{
		BaseContainer ematCont;
				
		//Getting ResourceClassName
		Resource resource = Resource.Load(path);						
		ematCont = resource.GetResource().ToBaseContainer();		
		return(ematCont.GetClassName());
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
		if(req.fromXob)
		{
			matutils.ReadXOB(xob, mapping, req);		
		}
		else
		{
			mapping.mat_class = matutils.ReadEmat(req.name);
		}

		return mapping;
	}
}