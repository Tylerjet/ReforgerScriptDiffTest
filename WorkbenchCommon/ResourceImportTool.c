
const string MeshObjectCommon = "Common";

//----------------------------------------------------------------------------------------------
void ReportIssue(string resource, string issue)
{
	PrintFormat("@\"%1\" : %2", resource, issue);
}

//----------------------------------------------------------------------------------------------
bool FixMeshObjectMetaFile(MetaFile meta, string absFileName)
{
	bool anyAncestorModified = false;
	
	BaseContainerList configurations = meta.GetObjectArray("Configurations");
	if(!configurations)
		return false;
		
	for(int c = 0; c < configurations.Count(); c++)
	{
		BaseContainer cfg = configurations.Get(c);
		
		string cfgName = cfg.GetName();
		
		//ensure that object which we want set ancestor to does exist. Create new instance if it's missing
		bool IsSet = cfg.IsVariableSetDirectly(MeshObjectCommon);
		if( !IsSet )
		{
			Resource res = BaseContainerTools.CreateContainer("TXOCommonClass");
			BaseContainer cont = BaseContainer.Cast( res.GetResource() );
			cfg.SetObject(MeshObjectCommon, cont);
		}
		
		//get an object which must be inherited from base configs
		BaseContainer commonObj = cfg.GetObject(MeshObjectCommon);
		if(!commonObj)
		{
			ReportIssue(absFileName, "Cannot set configuraction ancestor (" + cfgName + ")" );
			continue;
		}
	
		ResourceName ancestor = "";

		switch(cfgName)
		{
			case "PC":
				ancestor = "{0877E7C4BB2B2C9A}configs/ResourceTypes/PC/MeshObjectCommon.conf";
				break;
			case "XBOX_ONE":
				ancestor = "{9D5B6207F7628CE2}configs/ResourceTypes/XBOX_ONE/MeshObjectCommon.conf";
				break;
			case "XBOX_SERIES":
				ancestor = "{DD9F02115C764647}configs/ResourceTypes/XBOX_SERIES/MeshObjectCommon.conf";
				break;
			case "PS4":
				ancestor = "{53EC476BC921D99A}configs/ResourceTypes/PS4/MeshObjectCommon.conf";
				break;
			case "HEADLESS":
				ancestor = "{3A5B3356978039E8}configs/ResourceTypes/HEADLESS/MeshObjectCommon.conf";
				break;
		}
		
		if(ancestor != "")
		{
			commonObj.SetAncestor(ancestor);						
			anyAncestorModified = true;
		}
	}
	
	return anyAncestorModified;
}

//----------------------------------------------------------------------------------------------
bool FixSoundMetaFile(MetaFile meta, string absFileName)
{
	BaseContainerList configurations = meta.GetObjectArray("Configurations");
	if(!configurations)
		return false;
		
	bool anyChangeInMetaFile = false;
	
	for(int c = 0; c < configurations.Count(); c++)
	{
		BaseContainer cfg = configurations.Get(c);
		
		string cfgName = cfg.GetName();
		ResourceName ancestor = "";
		
		switch(cfgName)
		{
			case "PC":
				ancestor = "{F72B05D02F3F135E}configs/ResourceTypes/PC/Sound.conf";
				break;
			case "XBOX_ONE":
				ancestor = "{A0776163143143AC}configs/ResourceTypes/XBOX_ONE/Sound.conf";
				break;
			case "XBOX_SERIES":
				ancestor = "{382981CA1FC1C8DB}configs/ResourceTypes/XBOX_SERIES/Sound.conf";
				break;
			case "PS4":
				ancestor = "{89EB939911B4C093}configs/ResourceTypes/PS4/Sound.conf";
				break;
			case "HEADLESS":
				ancestor = "{D63592CA950AF3A8}configs/ResourceTypes/HEADLESS/Sound.conf";
				break;
		}
		
		if(ancestor != "")
		{
			cfg.SetAncestor(ancestor);						
			anyChangeInMetaFile = true;
		}
	}
	
	return anyChangeInMetaFile;
}

//----------------------------------------------------------------------------------------------
bool FixParticleEffectMetaFile(MetaFile meta, string absFileName)
{
	BaseContainerList configurations = meta.GetObjectArray("Configurations");
	if(!configurations)
		return false;
		
	bool anyChangeInMetaFile = false;
	
	for(int c = 0; c < configurations.Count(); c++)
	{
		BaseContainer cfg = configurations.Get(c);
		
		string cfgName = cfg.GetName();
		ResourceName ancestor = "";
		
		switch(cfgName)
		{
			case "PC":
				ancestor = "{4B4500E061CCD624}configs/ResourceTypes/PC/ParticleEffect.conf";
				break;
			case "XBOX_ONE":
				ancestor = "{73FE8562A95EBA36}configs/ResourceTypes/XBOX_ONE/ParticleEffect.conf";
				break;
			case "XBOX_ONE":
				ancestor = "{DC1673718B31CF7E}configs/ResourceTypes/XBOX_SERIES/ParticleEffect.conf";
				break;
			case "PS4":
				ancestor = "{B94602D6C3160C98}configs/ResourceTypes/PS4/ParticleEffect.conf";
				break;
			case "HEADLESS":
				ancestor = "{EED8908A9A55ED1C}configs/ResourceTypes/HEADLESS/ParticleEffect.conf";
				break;
		}
		
		if(ancestor != "")
		{
			cfg.SetAncestor(ancestor);						
			anyChangeInMetaFile = true;
		}
	}
	
	return anyChangeInMetaFile;
}

//--------------------------------------------------------------------
bool IsMeshObject(string className)
{
	return	className == "FBXResourceClass" || className == "TXOResourceClass";
}

//--------------------------------------------------------------------
bool IsSound(string className)
{
	 return className == "WAVResourceClass";
}

//--------------------------------------------------------------------
bool IsParticleEffect(string className)
{
	 return className == "PTCResourceClass";
}


[WorkbenchPluginAttribute("Model Import", "Model Import Helper", "", "", {"ResourceManager"})]
class ResourceImportPlugin: ResourceManagerPlugin
{
	[Attribute("true", UIWidgets.CheckBox)]
	bool Enabled;

	//--------------------------------------------------------------------
	void ResourceImportPlugin()
	{
	}

	//--------------------------------------------------------------------
	override string OnGetMaterialGenerateDir(string absModelPath)
	{
		return absModelPath + "/Data";
	}
	
	//--------------------------------------------------------------------
	override void OnRegisterResource(string absFileName, BaseContainer metaFile)
	{
		BaseContainer conf = metaFile.GetObjectArray("Configurations")[0];
		if (!Enabled)
			return;

		string className = conf.GetClassName();

		if( IsMeshObject(className) )
			FixMeshObjectMetaFile(metaFile, absFileName);
		else if( IsSound(className) )
			FixSoundMetaFile(metaFile, absFileName);
		else if( IsParticleEffect(className) )
			FixParticleEffectMetaFile(metaFile, absFileName);
	}

	override void Configure()
	{
		Workbench.ScriptDialog("Configure Texture Import Plugin", "", this);
	}

	[ButtonAttribute("OK")]
	void OkButton() {}
}




[WorkbenchPluginAttribute("Batch resource processor", "Perform simple checks and fixes on resource files", "", "", {"ResourceManager"},"",0xf15b)]
class BatchMeshObjectProcessorPlugin: WorkbenchPlugin
{
	[Attribute(uiwidget: UIWidgets.EditBox, desc: "Types (file extensions) to process")]
	ref array<string> FileTypes;

	[Attribute("true", UIWidgets.CheckBox, "Change configuration inheritance etc")]
	bool FixMetaFile;
	
	[Attribute("true", UIWidgets.CheckBox, "Resaves metafile even if it's without changes")]
	bool ForceResaveMetaFile;
	
	[Attribute("true", UIWidgets.CheckBox, "Report missing meta-file")]
	bool ReportMissingMetaFile;
	
	[Attribute("true", UIWidgets.CheckBox, "Report missing configurations")]
	bool ReportMissingConfigurations;
	
	[Attribute("true", UIWidgets.CheckBox, "Report custom property values")]
	bool ReportCustomPropertyValues;
	
	[Attribute("false", UIWidgets.CheckBox, "Fix custom property values (sets all PC custom changes to all other platforms)")]
	bool FixCustomPropertyValues;
	
	ref array<string> m_Resources = new array<string>;
	
	//----------------------------------------------------------------------------------------------
	void BatchMeshObjectProcessorPlugin()
	{
	}

	//----------------------------------------------------------------------------------------------
	[ButtonAttribute("Run", true)]
	bool OK()
	{
		return true;
	}

	//----------------------------------------------------------------------------------------------
	[ButtonAttribute("Cancel")]
	bool Cancel()
	{
		return false;
	}

	//----------------------------------------------------------------------------------------------
	void Find(ResourceName resName, string filePath)
	{
		m_Resources.Insert(filePath);
	}
	
	//----------------------------------------------------------------------------------------------
	override void Run()
	{
		if (!Workbench.ScriptDialog("Batch resource processor", "", this))
			return;

		Workbench.SearchResources(Find, FileTypes);
		m_Resources.Sort();

		ResourceManager rb = Workbench.GetModule(ResourceManager);
		WBProgressDialog progress = new WBProgressDialog("Processing...", rb);
		
		Print("Batch resource processor - BEGIN");

		float count = m_Resources.Count();

		foreach (int resourceIdx, string resource : m_Resources)
		{
			progress.SetProgress(resourceIdx / count);

			MetaFile meta = rb.GetMetaFile(resource);
			if (!meta)
			{
				if (ReportMissingMetaFile)
				{
					ReportIssue(resource, "meta-file is missing");
				}
				continue;
			}

			BaseContainerList configurations = meta.GetObjectArray("Configurations");
			if (!configurations)
			{
				if (ReportMissingConfigurations)
				{
					ReportIssue(resource, "meta-file is missing 'Configurations' property");
				}
				meta.Release();
				continue;
			}

			bool anyChangeInMetaFile = ForceResaveMetaFile;
			
			if (ReportCustomPropertyValues || FixCustomPropertyValues)
			{
				int nPlatforms = configurations.Count();
				
				ResourceName pcConfigName = configurations[0].GetResourceName();
				array<string> pcConfigVarsChanged = {};
				set<string> reportVarName = new set<string>;
				//---
				
				for (int iPlatform = 0; iPlatform < nPlatforms; iPlatform++)
				{
					BaseContainer platformConf = configurations.Get(iPlatform);
					BaseContainer platformConfAncestor = platformConf.GetAncestor();
					
					if (iPlatform)
					{
						if (platformConfAncestor && platformConfAncestor.GetResourceName() == pcConfigName)
							continue;
						
						if (!FixCustomPropertyValues && platformConf.GetName() == "HEADLESS")
							continue;
						
						if (FixCustomPropertyValues)
						{
							foreach (string varName: pcConfigVarsChanged)
							{
								//CopyVariable(configurations[0], platformConf, varName);
								string val;
								configurations[0].Get(varName, val);
								platformConf.Set(varName, val);
								anyChangeInMetaFile = true;
							}
						}
						else
						{
							foreach (string varName: pcConfigVarsChanged)
							{
								if (!platformConf.IsVariableSetDirectly(varName))	
								{
									reportVarName.Insert(varName);
									break;
								}
							}
						}
					}
					else
					{
						int nVars = platformConf.GetNumVars();
						for (int iVar = 0; iVar < nVars; iVar++)
						{
							string varName = platformConf.GetVarName(iVar);
							if (platformConf.IsVariableSetDirectly(varName))	
							{
								pcConfigVarsChanged.Insert(varName);
							}
						}
					}
					
					if (pcConfigVarsChanged.IsEmpty())
						break;
				}
				
				if (FixCustomPropertyValues)
				{
					foreach (string varName: pcConfigVarsChanged)
						ReportIssue(resource, "Custom property applied to all platforms: " + varName);
				}
				else
				{
					foreach (string varName: reportVarName)
						ReportIssue(resource, "Custom property only for PC: " + varName);
				}
			}
			
			if(FixMetaFile)
			{
				BaseContainer conf = configurations.Get(0);
				string className = conf.GetClassName();
				if( IsMeshObject(className) )
				{
					if( FixMeshObjectMetaFile(meta, resource) )
						anyChangeInMetaFile = true;
				}
				else if( IsSound(className) )
				{
					if( FixSoundMetaFile(meta, resource) )
						anyChangeInMetaFile = true;
				}
				else if( IsParticleEffect(className) )
				{
					if( FixParticleEffectMetaFile(meta, resource) )
						anyChangeInMetaFile = true;
				}
			}

			if(anyChangeInMetaFile )
			{
//				Print(resource);
				meta.Save();
			}
			meta.Release();
		}
		
		Print("Batch resource processor - END");
	}
}