class MaterialPropertiesRequest : JsonApiStruct
{
	string path;

	void MaterialPropertiesRequest()
	{
		RegV("path");
	}
}

class MaterialPropertiesResponse : JsonApiStruct
{
	ref array<string> mat_props  = new array<string>();

	void MaterialPropertiesResponse()
	{
		RegV("mat_props");
	}
}

class MaterialPropertiesUtils
{
	void GetEmatProperties(ResourceName path, MaterialPropertiesResponse props)
	{
		BaseContainer ematCont;
		BaseContainer uvTransform;
		BaseContainer empty;
		string temp = ".";
		string textureType;
		TextureUtils tUtils = new TextureUtils();

		
		Resource resource = Resource.Load(path);


		ematCont = resource.GetResource().ToBaseContainer();


		// looping through material properties
		for (int i = 0; i < ematCont.GetNumVars(); i++)
		{
			// get MatUVTransforms
			if (ematCont.GetObject(ematCont.GetVarName(i)) != empty) //Uv Tiling
			{
				// formatting for EBT
				// 1 -> as True when the specified UV Tiling is there
				props.mat_props.Insert(ematCont.GetVarName(i));
				props.mat_props.Insert("1");
				
				uvTransform = ematCont.GetObject(ematCont.GetVarName(i));
				// getting individual transforms
				for (int j = 0; j < uvTransform.GetNumVars(); j++)
				{
					uvTransform.Get(uvTransform.GetVarName(j), temp);
					if (temp != "")
					{
						// again every output must be formated for EBT
						props.mat_props.Insert(ematCont.GetVarName(i)+"_"+uvTransform.GetVarName(j));
						props.mat_props.Insert(temp);
					}
				}
			}
			// getting other paramaters
			ematCont.Get(ematCont.GetVarName(i), temp);
			if (temp != "")
			{
				props.mat_props.Insert(ematCont.GetVarName(i));
				if (temp.Contains(".edds"))
				{
					// texture info
					array<string> texProps = tUtils.GetEdds(temp, false);
					// texture itself has different format for better interpretation in EBT
					if (texProps.Count() == 0)
					{
						// add invalid GUID if the texture couldn't be found
						props.mat_props.Insert("InvalidGUID|PBRBasic|" + EBTConfig.tosRGB + "|" + EBTConfig.rgHQCompression);
					}
					else
					{
						// format it for EBT
						string texPropsStr = "";
						for (int number = 0; number < texProps.Count(); number++)
						{
							texPropsStr += texProps[number] + "|";
						}
						props.mat_props.Insert(texPropsStr.Substring(0,texPropsStr.Length()));
					}
				}
				else if (temp.Contains(" "))
				{
					// vectors and other values that has space in it must be replaced with "|"
					temp.Replace(" ", "|");
					props.mat_props.Insert(temp);
				}
				else
				{
					// everything else can be interpreted as it is
					props.mat_props.Insert(temp);
				}
				temp = "";
			}

		}
		return;
	}

}

class MaterialProperties : NetApiHandler
{
	override JsonApiStruct GetRequest()
	{
		return new MaterialPropertiesRequest();
	}

	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		MaterialPropertiesRequest req = MaterialPropertiesRequest.Cast(request);
		MaterialPropertiesResponse props = new MaterialPropertiesResponse();
		MaterialPropertiesUtils utils = new MaterialPropertiesUtils();
		ResourceName path = req.path;

		utils.GetEmatProperties(path, props);
		return props;
	}
}
