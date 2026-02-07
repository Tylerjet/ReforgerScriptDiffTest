[WorkbenchPluginAttribute("Material Import", "Material Import Helper", "", "", {"ResourceManager"})]
class MaterialImportPlugin: ResourceManagerPlugin
{
	[Attribute("true", UIWidgets.CheckBox)]
	bool Enabled;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Default PRB Cubemap", "edds")]
	ResourceName PBRCubeMap;
	
	ref map<string, ResourceName> m_Textures = new map<string, ResourceName>();
	
	bool SearchTexture(string absMaterialPath, GeneratedResources generatedResources, string propertyName, string postfix)
	{
		string absPathNoExt = absMaterialPath.Substring(0, absMaterialPath.Length() - 5); // removes '.emat'
		
		string path = absPathNoExt + "_" + postfix;
		string pathEDDS = absPathNoExt + "_" + postfix + ".edds";
		bool found = false;
		
		if (FileIO.FileExist(path + ".dds"))
		{
			path = path + ".dds";
			found = true;
		}
		else if (FileIO.FileExist(path + ".tga"))
		{
			path = path + ".tga";
			found = true;
		}
		else if (FileIO.FileExist(path + ".png"))
		{
			path = path + ".png";
			found = true;
		}
		
		if (found)
		{
			Print("MaterialImportPlugin: " + postfix + " texture found!", LogLevel.VERBOSE);
			
			ResourceName resourceID;
			if (generatedResources.RegisterResource(path, resourceID))
			{
				m_Textures.Insert(propertyName, resourceID);
				return true;
			}
			else
			{
				Print("MaterialImportPlugin: Can't open texture '" + pathEDDS + "'", LogLevel.ERROR);
			}
		}
		
		return false;
	}
	
	override string OnGetMaterialClassName(string absMaterialPath, GeneratedResources generatedResources)
	{
		string materialClassName = "MatPBRBasic";
		if (Enabled == false) return materialClassName;
		
		Print("MaterialImportPlugin: Searching textures for material '" + absMaterialPath + "'", LogLevel.VERBOSE);

		//find DummyVolume material name
		int dotIndex   = absMaterialPath.LastIndexOf("."); 
		int slashIndex = absMaterialPath.LastIndexOf("/") + 1; 
		if (dotIndex > 0 && slashIndex < dotIndex)
		{
			string name = absMaterialPath.Substring(slashIndex, dotIndex - slashIndex);
			name.ToLower();
			if (name == "dummyvolume")
			{
				return "MatDummy";
			}
		}
		

		bool isPbr = false;
		bool isPbrBasic = false; // is using RoughnessMap and MetalnessMap in one texture
		bool isSuper = false;
		
		isSuper |= SearchTexture(absMaterialPath, generatedResources, "ASMap", "AS");
		SearchTexture(absMaterialPath, generatedResources, "AlbedoMap", "CO");
		SearchTexture(absMaterialPath, generatedResources, "NormalMap", "NO");
		isPbr |= SearchTexture(absMaterialPath, generatedResources, "MetalnessMap", "MT");
		isPbr |= SearchTexture(absMaterialPath, generatedResources, "RoughnessMap", "RG");
		isPbrBasic |= SearchTexture(absMaterialPath, generatedResources, "ORMMap", "ORM");
		SearchTexture(absMaterialPath, generatedResources, "DetailMap", "CDT");
		SearchTexture(absMaterialPath, generatedResources, "MacroMap", "MC");
		SearchTexture(absMaterialPath, generatedResources, "EmissiveMap", "EM");
		SearchTexture(absMaterialPath, generatedResources, "AOMap", "AO");
		SearchTexture(absMaterialPath, generatedResources, "AmbientMetalnessMap", "AM");
		SearchTexture(absMaterialPath, generatedResources, "GlossinesSpecularMap", "GS");
		// SearchTexture(absMaterialPath, generatedResources, "EnvReflMap", "???"); 
		
		if (isPbr)
		{
			if (PBRCubeMap.Length()) m_Textures.Insert("EnvReflMap", PBRCubeMap);
			materialClassName = "MatPBR";
		} else if (isSuper)
		{
                 	materialClassName = "MatSuper";
		} else
		{
			//prioritize PbrBasic
			//if (isPbrBasic)
			if (PBRCubeMap.Length()) m_Textures.Insert("EnvReflMap", PBRCubeMap);
			materialClassName = "MatPBRBasic";
		}

		return materialClassName;
	}
	
	override void OnMaterialCreated(string absMaterialPath, BaseContainer materialSrc, GeneratedResources generatedResources)
	{
		foreach (string prop, ResourceName res: m_Textures)
		{
			materialSrc.Set(prop, res);
		}
		
		m_Textures.Clear();
	}
	
	override void Configure()
	{
		Workbench.ScriptDialog("Configure Material Import Plugin", "", this);
	}
	
	[ButtonAttribute("OK")]
	void OkButton() {}
}