class EBTConfig
{	
	// EMAT
	static const string conf = "Configurations";
	static const string materialAssigns = "MaterialAssigns";
	static const string UVTransform = "MatUVTransform";
	// EDDS
	static const string conversion = "Conversion";
	static const string colorSpace = "ColorSpace";
	static const string mips = "ContainsMips";
	static const string rgHQCompression = "RedGreenHQCompression";
	static const string colorHQCompression = "ColorHQCompression";
	static const string tosRGB = "TosRGB";
	static const string toLinear = "ToLinear";
	static const string raw = "Raw";
	static const string sRGB = "sRGB";
	//PREFAB
	static const string coords = "coords";
	static const string angle = "angle";
	static const string scale = "scale";
	static const string pivot = "PivotID";
	static const string hierarchy = "Hierarchy";
	static const string components = "components";
	static const string meshObject = "MeshObject";
	static const string object = "Object";
	static const ref array<string> supportedTypes = {"GenericEntity", "Building", "GameEntity"};
	static const ref array<string> angles = {"angleX","angleY","angleZ"};
}


class EBTEmatUtils
{
	bool GetMaterials(string xobPath, out map<string,ResourceName> AssignedMats)
	{
		// Get xob meta file
		ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
		MetaFile meta = resourceManager.GetMetaFile(xobPath);
		// return false if meta is missing
		if(!meta)
		{
			return false;
		}
		// Get assigned materials from Meta file
		BaseContainerList configurations = meta.GetObjectArray(EBTConfig.conf);
		BaseContainer cfg = configurations.Get(0);
		string materialAssigns;
		array<string> pairs = new array<string>;
		cfg.Get(EBTConfig.materialAssigns, materialAssigns);
		
		// format them into map and pair correct name - path
		materialAssigns.Split(";", pairs, true);
		foreach (string pair : pairs)
		{
			array<string> keyValue = new array<string>;
			pair.Split(",", keyValue, true);
			AssignedMats.Set(keyValue[0], keyValue[1]);
		}
		return true;
	}
	
}
