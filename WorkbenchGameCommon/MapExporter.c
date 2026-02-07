/*
TODO:
-Set default class flags on root/base prefabs (currently setting only when IsVariableSetDirectly is true)
-Sometimes the exporter just halts and seemingly does nothing for long periods of time before continuing, this should be debugged
	-the above might require extensive logging, so logging to a special log file might be beneficial
*/

[BaseContainerProps(configRoot: true)]
class SCR_MapExporterConfig
{
    [Attribute(defvalue: "c:/Reforger/Data/Arland", desc: "Folder of the map to export")]
    string m_sExportMapDir;

    [Attribute(defvalue: "Assets/_SharedData", desc: "Relative paths to shared data to be copied over")]
    ref array<string> m_aCopyPathsMixed;

    [Attribute(defvalue: "edds;emat;gamemat", desc: "Shared data extensions, only these will be copied over")]
    string m_sCopyPathsMixedExtensions;

    [Attribute(defvalue: "", desc: "Paths to copy .xob files from")]
    ref TStringArray m_aModelPaths;

    [Attribute(defvalue: "Tree::StaticModelEntity", desc: "Classname->Classname conversion mapping during the export process")]
    ref TStringArray m_aClassMappings;

    [Attribute(defvalue: "RoadGeneratorEntity", desc: "Entities/Prefabs inheriting classes in this list are not converted, even if they match any other criteria")]
    ref TStringArray m_aEntityClassWhitelist;

    [Attribute(defvalue: "SCR_", desc: "Entities/Prefabs inheriting classes containing keywords from this list are converted to their corresponding classes")]
    ref TStringArray m_aEntityClassBlacklistKeywords;

    [Attribute(defvalue: "MeshObject", desc: "Components inheriting these classes in this list are not converted, even if they match any other criteria")]
    ref TStringArray m_aComponentClassWhitelist;

    [Attribute(defvalue: "", desc: "Suffix added to the resources that were converted during the export process, typically prefabs")]
    string m_sConvertedAssetsSuffix;

    [Attribute(defvalue: "true", desc: "After export is finished, automatically delete temporary files created during the export")]
    bool m_bDeleteTempFiles;

    [Attribute(defvalue: "", desc: "Resources with these GUIDs will not be copied over")]
    ref TResourceNameArray m_aResourceExcludeGUIDs;

    [Attribute(defvalue: "", desc: "When cloning prefabs, ignore the ones coming from PrefabLibrary and alter prefab inheritance accordingly")]
    bool m_bSkipPrefabLibrary;
	
	[Attribute(defvalue: "false", desc: "Export source files along with the resource files. (will export FBX, TIF etc.. which are not used at run time)")]
    bool m_bExportSourceFiles;

   bool IsValid()
{
    // Check if m_ModelPaths is null or has no elements
    if (m_aModelPaths == null || m_aModelPaths.Count() == 0)
        return false;

    // Check if m_CopyPathsMixed is null or has no elements
    if (m_aCopyPathsMixed == null || m_aCopyPathsMixed.Count() == 0)
        return false;

    // Check if m_CopyPathsMixedExtensions is empty
    if (m_sCopyPathsMixedExtensions.IsEmpty())
        return false;

    // Check if m_ClassMappings is null or has no elements
    if (m_aClassMappings == null || m_aClassMappings.Count() == 0)
        return false;

    // Check if m_EntityClassWhitelist is null or has no elements
    if (m_aEntityClassWhitelist == null || m_aEntityClassWhitelist.Count() == 0)
        return false;

    // Check if m_EntityClassBlacklistKeywords is null or has no elements
    if (m_aEntityClassBlacklistKeywords == null || m_aEntityClassBlacklistKeywords.Count() == 0)
        return false;

    // Check if m_ComponentClassWhitelist is null or has no elements
    if (m_aComponentClassWhitelist == null || m_aComponentClassWhitelist.Count() == 0)
        return false;

    // Check if m_aResourceExcludeGUIDs is null or has no elements
    if (m_aResourceExcludeGUIDs == null || m_aResourceExcludeGUIDs.Count() == 0)
        return false;
	
    // All checks passed
    return true;
}

}


#ifdef WORKBENCH
enum ExporterAssetType
{
	PREFAB,
	XOB,
	MAT,
	TXT,
	MISC,
}

[WorkbenchPluginAttribute(name: "Map Exporter", wbModules: { "WorldEditor" }, shortcut: "", awesomeFontCode: 0xF338)] // 0xF338 = ↨
class SCR_MapExporterPlugin : WorkbenchPlugin
{
	const string SUFFIX_FALLBACK = "__DefExport";
	[Attribute(defvalue: "", desc: "'MapExporterConfig' type config", params: "conf")]
	ResourceName m_ConfigResource;
	
	[Attribute(defvalue: "c:/Work/MapExport/", desc: "Destination folder for the export")]
	string m_ExportDestination;
	
	[Attribute(defvalue: "true", desc: "Mostly for debug purposes, enables/disables copying of files identified during export, for normal export, leave enabled")]
	bool m_AllowResourceCopy;
	
	ref SCR_MapExporterConfig m_Config;
	WorldEditor m_WorldEditor;
	ref map<string, string>							m_mEntityClassBlacklist;
	ref map<ResourceName,ResourceName> 				m_mCreatedPrefabs;
	ref TStringArray 								m_aCreatedFiles;
	ref set<BaseContainer>							m_sDiscoveredPrefabs;
	ref map<ResourceName,bool> 						m_mPrefabsNeedingReplace;//when prefab is inside this container, that means it's been processed, 'true' value means it needs to be replaced, 'false' it does not
	
	ref set<ResourceName>							m_sPrefabsToMigrate;
	ref set<ResourceName>							m_sModelsToMigrate;
	ref set<ResourceName>							m_sMaterialsToMigrate;
	ref set<ResourceName>							m_sTexturesToMigrate;
	ref set<ResourceName>							m_sMiscToMigrate;
	ref set<ResourceName>							m_sResourceExcludeGUIDs;//for faster lookup

	WorldEditorAPI m_WEapi;
	ResourceManager resourceManager;
	bool m_PerformExport;
	string m_sConvertedAssetsSuffix;
	bool m_bUsingAssetSuffixFallback;//for faster checking if fallback is being used
	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Run Export", true)]
	protected bool ButtonExport()
	{
		if(!IsExportTargetFolderEmpty())
		{
			Print(string.Format("Export target folder'%1' not empty, clear before export", m_ExportDestination), LogLevel.ERROR);
			return true;
		}
		m_PerformExport = true;
		Print(m_PerformExport);
		return false;
	}
	//-----------------------------------------------------------------------------------------------
	protected bool Init()
	{
		m_Config = GetConfig(m_ConfigResource);
		if (!m_Config || !m_Config.IsValid())
		{
			Print("Config load failed", LogLevel.ERROR);
			return false;
		}
		m_sConvertedAssetsSuffix = m_Config.m_sConvertedAssetsSuffix;
		if (m_sConvertedAssetsSuffix == string.Empty)
		{
			m_sConvertedAssetsSuffix = SUFFIX_FALLBACK;//since we are duplicating resource files during the map export, we need them to have different filename regardless of whether the user wants it or not, however, if they don't, we remove this default suffix at the end of the export process
			m_bUsingAssetSuffixFallback = true;
		}
		
		m_WorldEditor = Workbench.GetModule(WorldEditor);
		m_WEapi = m_WorldEditor.GetApi();
		resourceManager = Workbench.GetModule(ResourceManager);
		
		m_mEntityClassBlacklist 		= new map<string, string>();
		m_sResourceExcludeGUIDs			= new set<ResourceName>	();
		
		foreach(string turnInto:m_Config.m_aClassMappings)
		{
			TStringArray output = {};
			turnInto.Split("::",output, true);
			m_mEntityClassBlacklist.Insert(output[0], output[1]);
		}
		
		foreach(ResourceName resName: m_Config.m_aResourceExcludeGUIDs)
		{
			m_sResourceExcludeGUIDs.Insert(resName);
		}
		


		return true;
	}
	
	protected void InitVars()
	{
		m_bUsingAssetSuffixFallback		= false;
		m_PerformExport					= false;
		m_sDiscoveredPrefabs			= new set<BaseContainer>;
		m_mCreatedPrefabs 				= new map<ResourceName, ResourceName>;
		m_aCreatedFiles					= {};
		m_mPrefabsNeedingReplace 		= new map<ResourceName,bool>;
		m_sPrefabsToMigrate				= new set<ResourceName>;
		m_sModelsToMigrate				= new set<ResourceName>;
		m_sMaterialsToMigrate			= new set<ResourceName>;
		m_sTexturesToMigrate			= new set<ResourceName>;
		m_sMiscToMigrate				= new set<ResourceName>;
	}
	//-----------------------------------------------------------------------------------------------
	override void Run()
	{
		InitVars();

		bool reload = true;
		while(reload)
		{
			reload = Workbench.ScriptDialog("Map Exporter", "This tool coverts the assets used on a map as well as the map \n entities based on the predefined rules, many of which can be adjusted in the config file.\nThe resolt is a folder containing the converted assets, as well as map data.\nIt's meant to strip the map of any gameplay specific features,\n leaving only the bare minimum to aproach a visual parity with the original map.\n\nAfter loading the map in the project the export is meant for, it's recommended to run the Re-save plugin which cleans the data by\n getting rid of some properties of uknown types,\nreducing the number of errors displayed when loading the map.\n", this);
		}
		
		if (!Init())//we need access to the config, which gets set in the ScriptDialog
		{
			Print("Init failed", LogLevel.ERROR);
			return;
		}

		if(m_PerformExport)
			PerformExport();
	}
	//-----------------------------------------------------------------------------------------------
	void PerformExport()
	{
		
		if (m_WEapi)
		{
			ref array<IEntitySource> allEntities = {};
			
			int selectedEntCount = m_WEapi.GetSelectedEntitiesCount();
			//int selectedEntCount = 0;
			if(selectedEntCount > 0)
			{
				for (int i = 0; i < selectedEntCount; i++)
				{
					IEntitySource entitySource = m_WEapi.GetSelectedEntity(i);
					if(!entitySource.GetParent())
						allEntities.Insert(entitySource);	
				}
			}
			else
			{
				int countAll = m_WEapi.GetEditorEntityCount();
			
				Print("GetEditorEntityCount():"+countAll);
				for (int i = 0; i < countAll; i++)
				{
					IEntitySource entitySource = m_WEapi.GetEditorEntity(i);
					if(!entitySource.GetParent())
						allEntities.Insert(entitySource);	
					
				}
			}

			Print("walk entities array count: "+allEntities.Count());
			
			m_WEapi.BeginEntityAction("ENT_CLONE");
			WBProgressDialog progress = new WBProgressDialog("Exporting map...", m_WorldEditor);
			foreach(int x, IEntitySource entSource:allEntities)
			{
				if(x % 100 == 0)
				{
					Print("Entities processed: " + x + "/"+allEntities.Count());
					progress.SetProgress((x / allEntities.Count()) * 0.9);
				}
				
				IEntitySource parent = entSource.GetParent();
				if(!parent)
				{
					ProcessEntityRecur(entSource,null,false);
				}
			}
			OnBeforeResourceExport();
			m_WEapi.EndEntityAction("ENT_CLONE");
			
			if(m_AllowResourceCopy)
				PerformResourceExport(progress);
		}
	}
	//-----------------------------------------------------------------------------------------------
	void OnBeforeResourceExport()
	{
		/*
		ResourceName prefabResName = m_mCreatedPrefabs.Get("{A43A100E3C377DB2}Prefabs//Structures//Core//Building_Base.et");
		if(prefabResName)
		{
			Print("Step1");
			Resource resource = BaseContainerTools.LoadContainer(prefabResName);
			BaseResourceObject bro = resource.GetResource();
	
			IEntitySource source = bro.ToEntitySource();
			if(source)
			{
				m_WEapi.CreateComponent(source, "Occluder");			
			}
		}*/ 
	}
	//-----------------------------------------------------------------------------------------------
	override void Configure()
	{
		Workbench.ScriptDialog("Configure Map Exporter plugin", "", this);
	}
	//-----------------------------------------------------------------------------------------------
	SCR_MapExporterConfig GetConfig(ResourceName configPath)
	{
		Resource configResource = Resource.Load(configPath);
		if (!configResource.IsValid())
		{
			Print(string.Format("Cannot load config '%1'!", configPath), LogLevel.ERROR);
			return null;
		}

		BaseResourceObject configContainer = configResource.GetResource();
		if (!configContainer)
			return null;

		BaseContainer configBase = configContainer.ToBaseContainer();
		if (!configBase)
			return null;

		if (configBase.GetClassName() != "SCR_MapExporterConfig")
		{
			Print(string.Format("Config '%1' is of type '%2', must be 'SCR_MapExporterConfig'!", configPath, configBase.GetClassName()), LogLevel.ERROR);
			return null;
		}

		return SCR_MapExporterConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(configBase));
	}
	
	
	//-----------------------------------------------------------------------------------------------
	bool AddAssetToMigrate(ExporterAssetType type, ResourceName resource, ResourceName resSource = string.Empty)
	{
	    if (!resource || !resource.GetPath())
	    {
	        return false;
	    }
	
	    bool registered = false;
	
	    switch (type)
	    {
	        case ExporterAssetType.PREFAB:
	            registered = m_sPrefabsToMigrate.Insert(resource);
	            break;
	        case ExporterAssetType.XOB:
	            registered = m_sModelsToMigrate.Insert(resource);
	            break;
	        case ExporterAssetType.MAT:
	            registered = m_sMaterialsToMigrate.Insert(resource);
	            break;
	        case ExporterAssetType.TXT:
	            registered = m_sTexturesToMigrate.Insert(resource);
	            break;
	        case ExporterAssetType.MISC:
	            registered = m_sMiscToMigrate.Insert(resource);
	            break;
	        default:
	            return false;
	    }
	
	    return registered;
	}

	//-----------------------------------------------------------------------------------------------
	//TODO: this may be possible to cache 
	//some entities may reference assets in specific properties, for instance the world entity references water and sky materials, we handle these here
	void HandleSpecificEntityDependencies(notnull BaseContainer entitySource)
	{
		if(entitySource.GetClassName() == "GenericWorldEntity")
		{
			ResourceName res;
			entitySource.Get("SkyPreset", res);
			RegisterMaterial(res);
			entitySource.Get("CloudsPreset", res);
			RegisterMaterial(res);
			entitySource.Get("OceanMaterial", res);
			RegisterMaterial(res);
			entitySource.Get("OceanSimulation", res);
			RegisterMaterial(res);
		
			array<ResourceName> stars = new array<ResourceName>();
			entitySource.Get("PlanetPreset", stars);
			foreach(ResourceName star:stars)
				RegisterMaterial(star);
		}
		else if(entitySource.GetClassName() == "DecalEntity")
		{
			ResourceName res;
			entitySource.Get("Mat", res);
			RegisterMaterial(res);
		}
		else if(entitySource.GetClassName() == "RoadEntity")
		{
			ResourceName res;
			entitySource.Get("Material", res);
			RegisterMaterial(res);
		}
	}
	//-----------------------------------------------------------------------------------------------
	//some entities placed on the map might have incompatible data
	void HandleSpecificEntityData(notnull BaseContainer entitySource)
	{
		if(entitySource.IsVariableSetDirectly("GeneratorType"))
		{
			m_WEapi.CreateObjectVariableMember(entitySource, {}, "GeneratorType", "GeneratorType");
		}

	}
	//-----------------------------------------------------------------------------------------------
	void PurgeCurveData(IEntitySource curve)
	{
		BaseContainerList points = curve.GetObjectArray("Points");
		for(int i = 0, icount = points.Count(); i < icount; i++)
		{
			BaseContainer point = points.Get(i);
			BaseContainerList data = point.GetObjectArray("Data");
			
			for(int j = data.Count() - 1; j >=0 ; j--)
			{
				BaseContainer dataItem = data.Get(j);
				//Print("found classname:"+dataItem.GetClassName());
				if (dataItem.GetClassName() != "SplinePointData")
				{
					
					auto containerPath = new array<ref ContainerIdPathEntry>();
					auto entry1 = new ContainerIdPathEntry("Points", i); // Take the first point
					containerPath.Insert(entry1);
			
					m_WEapi.RemoveObjectArrayVariableMember(curve, containerPath, "Data", j);
				}
			}
		}
	}

	//-----------------------------------------------------------------------------------------------
	bool IsEntityClassBlacklisted(IEntitySource entSource)
	{
		string entClassName = entSource.GetClassName();
		
		if(m_Config.m_aEntityClassWhitelist.Contains(entClassName))//first check whitelist, as some checks later on can be quite broad
			return false;
		if(m_mEntityClassBlacklist.Contains(entClassName))
			return true;
		
		foreach(string keyword:m_Config.m_aEntityClassBlacklistKeywords)
			if(entClassName.Contains(keyword))
				return true;
	
		return false;
	}
	//-----------------------------------------------------------------------------------------------
	string GetAlternateClassName(string currentName)
	{
		if(m_mEntityClassBlacklist.Contains(currentName))
			return m_mEntityClassBlacklist.Get(currentName);
		return "GenericEntity";
	}
	//-----------------------------------------------------------------------------------------------
	bool IsComponentClassBlacklisted(string className)
	{
		return !m_Config.m_aComponentClassWhitelist.Contains(className);
	}

	//-----------------------------------------------------------------------------------------------
	bool IsAnyComponentIncompatible(notnull IEntitySource entSource)
	{
		int componentsCount = entSource.GetComponentCount();
		for (int i = 0; i < componentsCount; i++)
		{
			IEntityComponentSource componentSource = entSource.GetComponent(i);
			if(IsComponentClassBlacklisted(componentSource.GetClassName()))
				return true;

		}
		return false;
	}
	//-----------------------------------------------------------------------------------------------
	bool DoesEntityNeedReplacing(IEntitySource entSource)
	{
		if(IsEntityClassBlacklisted(entSource))
			return true;
		
		if(IsAnyComponentIncompatible(entSource))
			return true;
		
		IEntitySource prefab = entSource.GetAncestor();
		if(prefab)
			return DoesPrefabNeedReplacingFast(prefab);
		return false;
	}
	//-----------------------------------------------------------------------------------------------
	
	//cached version, use this as it's faster
	bool DoesPrefabNeedReplacingFast(BaseContainer prefab)
	{
		ResourceName resource = prefab.GetResourceName();
		if(m_mPrefabsNeedingReplace.Contains(resource))
		{
			//Print("USING_CACHE " + resource.GetPath());
			return m_mPrefabsNeedingReplace.Get(resource);
		}
		else
		{
			//Print("NOT USING_CACHE " + resource.GetPath());
			bool result = DoesPrefabNeedReplacing(prefab);
			m_mPrefabsNeedingReplace.Insert(resource, result);
			return result;
		}
	}
	//-----------------------------------------------------------------------------------------------
	//uncached version, only use from inside the cached version
	bool DoesPrefabNeedReplacing(BaseContainer prefab)
	{
		//first check the class, that's the cheapest disqualifier
		if(IsEntityClassBlacklisted(prefab))
		{
			return true;
		}
		
		// check components
		if(IsAnyComponentIncompatible(prefab))
			return true;
		
		//now check immediate children
		int childCount = prefab.GetNumChildren();
		
		for(int i = 0; i < childCount; i++)
		{
			BaseContainer child = prefab.GetChild(i);
			if(DoesEntityNeedReplacing(child))
			{
				return true;
			}
		}
		
		RunPrefabDiscovery(prefab);//there may be a more optimal position for this call
		return false;
	}
	
	void RegisterXob(BaseContainer meshObjectComp)
	{
			ResourceName resourceName;
			meshObjectComp.Get("Object", resourceName);
			
			if(resourceName)
			{
				//Print("MarkEntityXob found model:" + resourceName);
				AddAssetToMigrate(ExporterAssetType.XOB, resourceName);
			}
			
			DiscoverMeshObjectMaterialOverrides(meshObjectComp, resourceName);
	}
	//-----------------------------------------------------------------------------------------------
	//takes in an entity or prefab source, saves the xob and checks for any material overrides
	//TODO:ideally should only do anything if the xob or the materials are overriden on the specific container, if it's at all possible check that
	void RegisterPrefabXob(IEntitySource entSource)
	{
		//Print("MarkEntityXob Called on:" + entSource.GetResourceName());
		BaseContainer container = FindComponentSource(entSource, "MeshObject");

		//-------------------------- MeshObject ---------------------------
		if (container)
		{
			BaseContainer ancestor = entSource.GetAncestor();
		
			if(ancestor)
			{
				BaseContainer componentSourceAnc = FindComponentSource(ancestor, "MeshObject");
				if(container == componentSourceAnc)
					return;//no override
			}

			RegisterXob(container);
		}
	}
	//-----------------------------------------------------------------------------------------------
	void DiscoverMeshObjectMaterialOverrides(BaseContainer meshContainer, ResourceName resourceName)
	{
		BaseContainerList materials = meshContainer.GetObjectArray("Materials");
		//Print("For " + entSource.GetResourceName() +" found " + materials.Count() + " materials");
		//Print("found model "+resourceName);
		if (materials)
		{
			for (int i = 0; i < materials.Count(); i++)
			{
				BaseContainer materialData = materials.Get(i);
				ResourceName matTgt;
				materialData.Get("AssignedMaterial", matTgt);
				if(matTgt)
				{
					//Print("Found material:" + matTgt);
					RegisterMaterial(matTgt);
				}
				
			}
		}
	}
	//-----------------------------------------------------------------------------------------------
	void GetMaterialsFromXobs()
	{
		foreach(ResourceName res:m_sModelsToMigrate)
		{
			MetaFile meta = resourceManager.GetMetaFile(res.GetPath());
			if(!meta)
			{
				return;
			}
			BaseContainerList configurations = meta.GetObjectArray("Configurations");
			BaseContainer cfg = configurations.Get(0);
	
			string materialAssigns;
			array<string> pairs = new array<string>;
			cfg.Get("MaterialAssigns", materialAssigns);
			materialAssigns.Split(";", pairs, true);
			
			foreach (string pair : pairs)
			{
				array<string> keyValue = new array<string>;
			   	pair.Split(",", keyValue, true);
				ResourceName resName = keyValue[1];
				RegisterMaterial(resName);
			}
		}
	}
	//-----------------------------------------------------------------------------------------------
	void GetTexturesFromMaterials()
	{
		//foreach(ResourceName res:m_sMaterialsToMigrate)
		for(int j = 0; j < m_sMaterialsToMigrate.Count();j++)
		{
			ResourceName resName = m_sMaterialsToMigrate.Get(j);
			Resource resource = BaseContainerTools.LoadContainer(resName);
			
			if(!resource || !resource.IsValid())
				return;
			
			BaseResourceObject bro = resource.GetResource();

			BaseContainer container = bro.ToBaseContainer();
			if(!container)
				return;
			int varCount = container.GetNumVars();
			
			for(int i = 0; i < varCount; i++)
			{
				if(container.GetDataVarType(i) == DataVarType.TEXTURE)
				{
					string varName = container.GetVarName(i);
					ResourceName texture;
					container.Get(varName,texture);
					//Print(texture);
					
					array<string> textures = new array<string>;
					texture.Split(" ", textures, true);
					if(textures.Count() > 1)
					{
						foreach(string txt:textures)
						{
							if(txt.Get(0) == "{")
							{
								AddAssetToMigrate(ExporterAssetType.TXT,txt, resName);
								//Print("ARRAY:"+txt);
							}
						}
					}
					else
					{
						AddAssetToMigrate(ExporterAssetType.TXT,texture, resName);
					}
					//Print("texture:" + texture);
				}
			}
		}
	}
	//-----------------------------------------------------------------------------------------------
	void RegisterMaterial(ResourceName resName)
	{
		if(m_sMaterialsToMigrate.Contains(resName))
			return;
		AddAssetToMigrate(ExporterAssetType.MAT,resName);
				
		Resource res = BaseContainerTools.LoadContainer(resName);
		if(!res)
			return;
		BaseResourceObject bro = res.GetResource();
		if(!bro)
			return;
		BaseContainer containerMaterial = bro.ToBaseContainer();
		if(!containerMaterial)
			return;
		BaseContainer anc = containerMaterial.GetAncestor();
		if(anc)
			RegisterMaterial(containerMaterial.GetAncestor().GetResourceName());
		
		int varCount = containerMaterial.GetNumVars();
		
		for(int i = 0; i < varCount; i++)
		{
			if(containerMaterial.GetDataVarType(i) == DataVarType.RESOURCE_NAME)
			{
				string varName = containerMaterial.GetVarName(i);
				ResourceName material;
				containerMaterial.Get(varName,material);
				if(!material.GetPath() || !material.GetPath().Contains(".emat"))
					continue;
				//Print("doing_it");//TODO:check if it's actually logged EVER
				RegisterMaterial(material);
				
			}
		}
	}
	//-----------------------------------------------------------------------------------------------
	//this method tries to identify(discover) prefabs that have not been discovered yet
	void RunPrefabDiscovery(IEntitySource prefab)
	{
		RegisterPrefabXob(prefab);//this is for prefab instance, inside another prefab, which can have model override
		
		if(m_sDiscoveredPrefabs.Contains(prefab))
			return;
		ResourceName resName = prefab.GetResourceName();
		if(resName && resName.GetPath())
		{
			m_sDiscoveredPrefabs.Insert(prefab);
		}
		
		int childCount = prefab.GetNumChildren();
		for(int i = 0; i < childCount; i++)
		{
			IEntitySource child = prefab.GetChild(i);
			RunPrefabDiscovery(child);
		}
		
		IEntitySource ancestor = prefab.GetAncestor();
		if(ancestor)
			RunPrefabDiscovery(ancestor);
		
	}
	//-----------------------------------------------------------------------------------------------		
	
	void ProcessEntityRecur(IEntitySource entSource, IEntitySource parent, bool parentToBeDeleted)
	{
		IEntitySource newEntitySrc = entSource;
		HandleSpecificEntityDependencies(entSource);
		bool isPrefabChild = IsPartOfPrefab(entSource);
		bool toBeDeleted;
		RegisterPrefabXob(entSource);//this is here because an entity placed in the world can have either xob override and/or material override
		if(!isPrefabChild)
		{
			IEntitySource ancestor = entSource.GetAncestor();
			
			
			if(DoesEntityNeedReplacing(entSource))
			{
				newEntitySrc = CloneEntityFull(entSource, parent);
				if(!parentToBeDeleted)
					toBeDeleted = true;
			}
			else 
			{
				HandleSpecificEntityData(entSource);
				typename classType = newEntitySrc.GetClassName().ToType();
				if(classType.IsInherited(ShapeEntity))
					PurgeCurveData(newEntitySrc);
				if (entSource.GetParent() != parent)
				{
					IEntity entParent = m_WEapi.SourceToEntity(parent);
					IEntity entChild = m_WEapi.SourceToEntity(entSource);
					if(!entParent || !entChild || !m_WEapi.ParentEntity(parent ,entSource , false))
					{
						Print("REPARENT ERROR START",LogLevel.ERROR);
						Print("parent:" + entParent);
						Print("child:" + entChild);
						Print("REPARENT_LAYER:"+entSource.GetLayerID());
						Print("REPARENT_ENT_CLASS:" +entSource.GetClassName());
						Print("REPARENT_CURRENT_PARENT_CLASS:" +entSource.GetParent().GetClassName());
						Print("REPARENT_NEW_PARENT_CLASS:" +parent.GetClassName());
						if(entSource.GetAncestor())
							Print("REPARENT_ANC_CLASS:" +entSource.GetAncestor().GetClassName());					
						Print("REPARENT ERROR END");
					}
				}
			}
		}

		int childCount = entSource.GetNumChildren();
		array<IEntitySource> children = new array<IEntitySource>();
		for(int x = 0; x < childCount; x++)
		{
			IEntitySource childSource = entSource.GetChild(x);
			children.Insert(childSource);
		}
		
		foreach(IEntitySource child:children)
		{
			ProcessEntityRecur(child, newEntitySrc, toBeDeleted);
		}

		if(toBeDeleted)
		{
			
			//Print("deleting:" + m_WEapi.SourceToEntity(entSource).ClassName());
			string name = entSource.GetName();
			m_WEapi.DeleteEntity(entSource);
			if(name)
			{
				newEntitySrc.SetName(name);
			}
			
		}
	}
	
	
	//-----------------------------------------------------------------------------------------------
	IEntitySource CloneEntityFull(IEntitySource entSource, IEntitySource parent)
	{
		string className = entSource.GetClassName();
		
		if(IsEntityClassBlacklisted(entSource))
		{
			className = GetAlternateClassName(className);
		}
		
		
		BaseContainer prefab = entSource.GetAncestor();
		if(prefab)
		{
			if(DoesPrefabNeedReplacingFast(prefab))
			{
				ResourceName newPrefab = ClonePrefabFast(prefab);
			
				if(newPrefab)
				{
					//string fileName = FilePath.StripPath(newPrefab.GetPath());
					className = newPrefab.GetPath();
				}
				else
					Print("Something went wrong with prefab replace:" + prefab.GetResourceName(), LogLevel.ERROR);
			}
		}

		IEntitySource newEntSrc = CloneSingleEntity(className, entSource, parent, entSource.GetLayerID());
		
		CopyComponents(entSource, newEntSrc);
		return newEntSrc;
	}
	//-----------------------------------------------------------------------------------------------
	protected ResourceName ClonePrefabFast(BaseContainer prefab)
	{
		ResourceName newPrefab;
		ResourceName prefabResName = prefab.GetResourceName();
		
		if(!m_mCreatedPrefabs.Contains(prefabResName))
		{
			newPrefab = ClonePrefabSlow(prefab);
		}
		else
		{
			newPrefab = m_mCreatedPrefabs.Get(prefabResName);
		}
		return newPrefab;
	}
	//-----------------------------------------------------------------------------------------------
	bool IsFromPrefabLibrary(ResourceName prefab)
	{
		return prefab.GetPath().Contains("PrefabLibrary");
	}
	
	//-----------------------------------------------------------------------------------------------
	protected ResourceName ClonePrefabSlow(notnull BaseContainer originalPrefab)
	{
		ResourceName ancestorPrefabName;
		ResourceName originalPrefabName = originalPrefab.GetResourceName();
		BaseContainer ancestorPrefab = originalPrefab.GetAncestor();//prefab container this prefab container is inheriting from
		
		if(ancestorPrefab)
		{
			if(DoesPrefabNeedReplacingFast(ancestorPrefab))
			{
				ancestorPrefabName = ClonePrefabFast(ancestorPrefab);
			}
			else
			{
				ancestorPrefabName = ancestorPrefab.GetResourceName();
			}
		}
		
		//this skips over the prefabs from PrefabLibrary in the inheritence of the newly created prefabs
		//...by returning the ancestor prefab as the cloned prefab, instead of the prefab from the PrefabLibrary
		if(m_Config.m_bSkipPrefabLibrary && IsFromPrefabLibrary(originalPrefabName) && ancestorPrefabName)
		{
			return ancestorPrefabName;
		}
			
		string prefabPath = originalPrefab.GetResourceName().GetPath();
		string prefabPathNoFileName = FilePath.StripFileName(prefabPath);
		string prefabFileName = FilePath.StripPath(prefabPath);
		string prefabFileNameNoExt = FilePath.StripExtension(prefabFileName);
		
		string newPrefabFileName = prefabFileNameNoExt + m_sConvertedAssetsSuffix+".et";
		string newPrefabPath = prefabPathNoFileName + newPrefabFileName;
		string newPrefabPathAbs;
		Workbench.GetAbsolutePath(newPrefabPath, newPrefabPathAbs, false);
		IEntitySource entSrc;
		if(newPrefabPathAbs)
		{
			Print("new prefab alternative not found for "+originalPrefabName+", lets create it");
			
			entSrc = DeepPrefabEntityCloneRecur(originalPrefab,null,ancestorPrefabName);
			m_WEapi.CreateEntityTemplate(entSrc,newPrefabPathAbs);//<------------ CREATE PREFAB
			resourceManager.WaitForFile("$ArmaReforger:"+newPrefabPath, 10000);
			ResourceName newPrefabName = Workbench.GetResourceName(newPrefabPath);//variable re-use
			
			AddAssetToMigrate(ExporterAssetType.PREFAB, newPrefabName);
			RegisterNewPrefab(originalPrefabName,newPrefabName);
			
			Print("new prefab ready for:"+originalPrefabName+", new resource name:" + newPrefabName);
			
			// Delete ent.
			if(entSrc)
				m_WEapi.DeleteEntity(entSrc);
			
			return newPrefabName;
		}
		return string.Empty;
	}
	
	//-----------------------------------------------------------------------------------------------
	IEntitySource DeepPrefabEntityCloneRecur(IEntitySource prefab, IEntitySource parent, ResourceName ancestor)
	{
		IEntitySource newEntSrc;
		if(!IsPartOfPrefab(prefab))
		{
			string className = prefab.GetClassName();
			
			if(IsEntityClassBlacklisted(prefab))
			{
				className = GetAlternateClassName(className);
			}
				
			BaseContainer anc = GetPrefab(prefab);//okno_old_prefab
			if(anc)
			{
				if(DoesPrefabNeedReplacingFast(anc))
				{
					ResourceName newPrefab = ClonePrefabFast(anc);//okno_new_prefab
			
					if(newPrefab)
						className = newPrefab;
					else
						Print("Something went wrong with prefab replace:" + anc.GetResourceName(), LogLevel.ERROR);
				}
				else
				{
					className = GetPrefabName(prefab);
				}
			}
			
			
			newEntSrc = CloneSingleEntity(className, prefab, parent);//okno_new_prefab_instance
			if(ancestor && ancestor.GetPath())
			{
				//Print("setting ancestor:" +  ancestor);
				newEntSrc.SetAncestor(ancestor);
				
			}
			CopyComponents(prefab, newEntSrc);

		}

		int childCount = prefab.GetNumChildren();
		//Print(prefab.GetName() + "|" + childCount);
		
		for(int i = 0; i < childCount;i++)
		{
			IEntitySource child = prefab.GetChild(i);
			DeepPrefabEntityCloneRecur(child,newEntSrc,string.Empty);
		}
		return newEntSrc;
	}
	
	
	//-----------------------------------------------------------------------------------------------
	void RegisterNewPrefab(ResourceName resNameOriginal, ResourceName resNameNew)
	{
		m_mCreatedPrefabs.Insert(resNameOriginal, resNameNew);
		m_aCreatedFiles.Insert(resNameNew.GetPath());
	}
	
	//-----------------------------------------------------------------------------------------------
	bool IsPartOfPrefab(IEntitySource entSource)
	{
		BaseContainer thisEntPrefabRoot;
		if(entSource.GetAncestor() && entSource.GetAncestor().GetParent())
			thisEntPrefabRoot = entSource.GetAncestor().GetParent();
		
		IEntitySource parentSource = entSource.GetParent();
		
		BaseContainer parentPrefab;
		if(parentSource && parentSource.GetAncestor())
			parentPrefab = parentSource.GetAncestor();

		return (thisEntPrefabRoot && parentPrefab && parentPrefab == thisEntPrefabRoot);
	}
	
	static BaseContainer GetPrefab(BaseContainer source)
	{
		BaseContainer container = source.GetAncestor();
		BaseContainer prefab;
		while(container)
		{
			if(container.GetResourceName() != ResourceName.Empty)
			{
				prefab = container;
				break;
			}
			container = container.GetAncestor();
		}
		return prefab;
	}
	
	static ResourceName GetPrefabName(BaseContainer source)
	{
		BaseContainer container = source.GetAncestor();
		ResourceName prefabRes;
		while(container)
		{
			if(container.GetResourceName() != ResourceName.Empty)
			{
				prefabRes = container.GetResourceName();
				break;
			}
			container = container.GetAncestor();
		}
		return prefabRes;
	}
	
	
	
	
	//-----------------------------------------------------------------------------------------------
	void CopyComponentProperties(BaseContainer sourceComponent, BaseContainer targetComponent, BaseContainer targetEntity, array<ref ContainerIdPathEntry> path)
	{
		string varName;
		for (int v = 0, varCount = sourceComponent.GetNumVars(); v < varCount; v++)
		{
			//Print("sourceComponent:" + sourceComponent);
			varName = sourceComponent.GetVarName(v);
			//Print("processing varName:" + varName);
			if (!sourceComponent.IsVariableSetDirectly(varName))
				continue;
			//Print("varName set:" + varName);
			switch (sourceComponent.GetDataVarType(v))
			{
				case DataVarType.SCALAR_ARRAY:
				{
					break;
				}
				
				case DataVarType.RESOURCE_NAME:
				{
					ResourceName valueSrc, valueTrg;
					sourceComponent.Get(varName, valueSrc);
					targetComponent.Get(varName, valueTrg);
					if(valueSrc != valueTrg)
						m_WEapi.SetVariableValue(targetEntity, path, varName, valueSrc);
					//Print("varName setting as STRING:" + varName +"|" + valueSrc);
					
					break;
				}

				case DataVarType.STRING:
				{
					string valueSrc, valueTrg;
					sourceComponent.Get(varName, valueSrc);
					targetComponent.Get(varName, valueTrg);
					if(valueSrc != valueTrg)
						m_WEapi.SetVariableValue(targetEntity, path, varName, valueSrc);
					//Print("varName setting as STRING:" + varName +"|" + valueSrc);
					
					break;
				}
				case DataVarType.INTEGER:
				{
					int valueSrc, valueTrg;
					sourceComponent.Get(varName, valueSrc);
					targetComponent.Get(varName, valueTrg);
					//Print("varName setting as INTEGER:" + varName +"|" + valueSrc.ToString());
					if(valueSrc != valueTrg)
						m_WEapi.SetVariableValue(targetEntity, path, varName, valueSrc.ToString());
					break;
				}
				case DataVarType.SCALAR:
				{
					float valueSrc, valueTrg;
					sourceComponent.Get(varName, valueSrc);
					targetComponent.Get(varName, valueTrg);
					//Print("varName setting as SCALAR:" + varName +"|" + valueSrc.ToString());
					if(valueSrc != valueTrg)
						m_WEapi.SetVariableValue(targetEntity, path, varName, valueSrc.ToString());
					break;
				}
				case DataVarType.VECTOR3:
				{
					vector valueSrc, valueTrg;
					sourceComponent.Get(varName, valueSrc);
					targetComponent.Get(varName, valueTrg);
					//Print("varName setting as VECTOR3:" + varName +"|" + valueSrc.ToString());
					if(valueSrc != valueTrg)
						m_WEapi.SetVariableValue(targetEntity, path, varName, valueSrc.ToString(false));
					break;
				}
				case DataVarType.BOOLEAN:
				{
					int valueSrc, valueTrg;
					sourceComponent.Get(varName, valueSrc);
					targetComponent.Get(varName, valueTrg);
					
					if(valueSrc != valueTrg)
					{
						//Print("varName setting as BOOLEAN:" + varName +"|" + valueSrc.ToString() + "| current value:" +valueTrg.ToString());
						m_WEapi.SetVariableValue(targetEntity, path, varName, valueSrc.ToString());
					}
					break;
				}
				case DataVarType.FLAGS:
				{
					int valueSrc, valueTrg;
					sourceComponent.Get(varName, valueSrc);
					targetComponent.Get(varName, valueTrg);
					//Print("varName setting as FLAGS:" + varName +"|" + valueSrc.ToString());
					if(valueSrc != valueTrg)
						m_WEapi.SetVariableValue(targetEntity, path, varName, valueSrc.ToString());
					break;
				}
				default:
				{
					//Print(string.Format("Cannot copy variable '%1' from template, it has unsupported type %2!", varName, typename.EnumToString(DataVarType, sourceComponent.GetDataVarType(v))), LogLevel.ERROR);
				}
			}
		}
	}
	//-----------------------------------------------------------------------------------------------
	void CopyComponents(IEntitySource source, IEntitySource target)
	{
	    array<string> componentsToCopy = {"MeshObject", "Hierarchy", "RigidBody"};
	    foreach (string componentName : componentsToCopy)
	    {
	        CopyComponent(componentName, source, target);
	    }
	}
	
	//-----------------------------------------------------------------------------------------------
	void CopyComponent(string componentName, IEntitySource source, IEntitySource target)
	{
	    BaseContainer componentSource = FindComponentSource(source, componentName);
		BaseContainer ancestor = source.GetAncestor();
		
		if(ancestor)
		{
			BaseContainer componentSourceAnc = FindComponentSource(ancestor, componentName);
			if(componentSource == componentSourceAnc)
				return;//no override
		}
	    
	    if (componentSource)
	    {
	        array<ref ContainerIdPathEntry> containerPath = {new ContainerIdPathEntry(componentName)};
	        BaseContainer componentTarget = FindComponentSource(target, componentName);
	
	        if (!componentTarget)
	        {
	            componentTarget = m_WEapi.CreateComponent(target, componentName);
	        }
	
	        CopyComponentProperties(componentSource, componentTarget, target, containerPath);
			CopyComponentPropertiesSpecial(componentName, componentSource, componentTarget, target, containerPath);
	    }
	}
	//-----------------------------------------------------------------------------------------------
	void CopyComponentPropertiesSpecial(string componentName, BaseContainer sourceComponent, BaseContainer targetComponent, BaseContainer targetEntity, array<ref ContainerIdPathEntry> path)
	{
		
		if(componentName == "MeshObject")
		{
			RegisterXob(sourceComponent);
			if(!sourceComponent.IsVariableSetDirectly("Materials"))
				return;
			
			BaseContainerList materialsSource = sourceComponent.GetObjectArray("Materials");
			BaseContainerList materialsTarget = targetComponent.GetObjectArray("Materials");
			//Print("-------------------------------------------------------------");
			//Print("Material overrides:");
			if (materialsSource)
			{
				for (int i = 0; i < materialsSource.Count(); i++)
				{
					BaseContainer materialDataSource = materialsSource.Get(i);
		
					ResourceName matSrc, nameSrc;
					materialDataSource.Get("AssignedMaterial", matSrc);
					materialDataSource.Get("SourceMaterial", nameSrc);
					
					m_WEapi.CreateObjectArrayVariableMember(targetEntity, path, "Materials", "MaterialAssignClass", i);
					
					array<ref ContainerIdPathEntry> containerPath = {};
					foreach(ContainerIdPathEntry entry: path)
						containerPath.Insert(entry);
					containerPath.Insert(new ContainerIdPathEntry("Materials", i)); // Take the first point
					
					m_WEapi.SetVariableValue(targetEntity, containerPath, "SourceMaterial", nameSrc);
					m_WEapi.SetVariableValue(targetEntity, containerPath, "AssignedMaterial", matSrc);
				}
				//Print("-------------------------------------------------------------");
			}
		}
	}
	//-----------------------------------------------------------------------------------------------
	void ExportFiles(set<ResourceName> resourceNames)
	{
		foreach(ResourceName name:resourceNames)
		{
			if(m_sResourceExcludeGUIDs.Contains(name))
				continue;//skip resource when excluded
			string filePath = name.GetPath();
			ExportFile(filePath);
		}
	}
	//-----------------------------------------------------------------------------------------------
	void ExportFile(string filePath)
	{
		string fileName = FilePath.StripPath(filePath);
		string relativePath = FilePath.StripFileName(filePath);
		
		if(m_bUsingAssetSuffixFallback)
		{
			fileName.Replace(SUFFIX_FALLBACK, "");
		}

		string fileNameMeta = fileName +".meta";
		string newPathDir = m_ExportDestination  + relativePath;
		FileIO.MakeDirectory(newPathDir);
		string source1 = filePath;
		string source2 = filePath +".meta";

		string destination1 = newPathDir + fileName;//resource file
		string destination2 = newPathDir + fileNameMeta;//resource file's meta

		if(FileIO.FileExists(source1))
			FileIO.CopyFile(source1,destination1);
		
		MetaFile meta = resourceManager.GetMetaFile(source1);
		if(m_Config.m_bExportSourceFiles && meta)
		{
			string sourceFilePath = meta.GetSourceFilePath();
			string fsName = FilePath.FileSystemNameFromFileName(sourceFilePath);
			string fsComplete = "$"+fsName+":";
			sourceFilePath.Replace(fsComplete,"");
			string sourceFilename = FilePath.StripPath(sourceFilePath);
			if(!sourceFilename.IsEmpty())
			{
				string destination3 = m_ExportDestination+sourceFilePath;
				if(FileIO.FileExists(sourceFilePath))
					FileIO.CopyFile(sourceFilePath,destination3);
			}

		}
		if(FileIO.FileExists(source2))
			FileIO.CopyFile(source2,destination2);
	}
	//-----------------------------------------------------------------------------------------------
	void ExportMapData()
	{
		TStringArray files = {};
		FileIO.FindFiles(files.Insert,m_Config.m_sExportMapDir,string.Empty);
		foreach(string filePath:files)
			ExportFile(filePath);
	}
	//-----------------------------------------------------------------------------------------------
	void ExportSharedData()
	{
		set<ResourceName> foundResources = new set<ResourceName>();
	
		array<string> extensions = {};
		m_Config.m_sCopyPathsMixedExtensions.Split(";",extensions, true);
		
		foreach(string path: m_Config.m_aCopyPathsMixed)
		{
			SearchResourcesFilter filter = new SearchResourcesFilter();
			filter.rootPath = "$ArmaReforger:" + path;
			filter.fileExtensions = extensions;
			
			ResourceDatabase.SearchResources(filter, foundResources.Insert);
		}
		
		foreach(string path: m_Config.m_aModelPaths)
		{
			SearchResourcesFilter filter = new SearchResourcesFilter();
			filter.rootPath = "$ArmaReforger:" + path;
			filter.fileExtensions = {"xob"};
			
			ResourceDatabase.SearchResources(filter, foundResources.Insert);
		}
		
		ExportFiles(foundResources);
	}

	//-----------------------------------------------------------------------------------------------
	IEntitySource CloneSingleEntity(string className, IEntitySource oldEntSrc,  IEntitySource parent, int layerID = 0)
	{
		float angleX, angleY, angleZ;
		
		oldEntSrc.Get("angleX", angleX);
		oldEntSrc.Get("angleY", angleY);
		oldEntSrc.Get("angleZ", angleZ);
		
		vector rot = Vector(angleX, angleY, angleZ);
		
		IEntitySource newEntSrc = m_WEapi.CreateEntityExt(className, string.Empty, layerID, parent, "0 0 0", rot, 0);
		if(newEntSrc)
			CopyEntityProperties(oldEntSrc, newEntSrc);
		else
		{
			//cloning failed TODO: add print
		}

		return newEntSrc;
	}
	//-----------------------------------------------------------------------------------------------
	void CopyEntityProperties( IEntitySource oldEntSrc,  IEntitySource newEntSrc)
	{
		if(oldEntSrc.IsVariableSetDirectly("Flags"))
		{
			EntityFlags flags;
			oldEntSrc.Get("Flags", flags);
			m_WEapi.SetVariableValue(newEntSrc, {}, "Flags", flags.ToString()); // Set Name to given value
		}
		if(oldEntSrc.IsVariableSetDirectly("scale"))
		{
			float scale;
			oldEntSrc.Get("scale", scale);
			m_WEapi.SetVariableValue(newEntSrc, {}, "scale", scale.ToString()); // Set Name to given value
		}
		if(oldEntSrc.IsVariableSetDirectly("coords"))
		{
			vector coords;
			oldEntSrc.Get("coords", coords);
			m_WEapi.SetVariableValue(newEntSrc, {}, "coords", coords.ToString(false)); // Set Name to given value
		}
		if(oldEntSrc.IsVariableSetDirectly("placement"))
		{
			string placement;
			oldEntSrc.Get("placement", placement);
			m_WEapi.SetVariableValue(newEntSrc, {}, "placement", placement); // Set Name to given value
		}
		
		CopyEntityPropertiesSpecial(oldEntSrc, newEntSrc);
	}
	//-----------------------------------------------------------------------------------------------
	void CopyContainerProperties(IEntitySource oldEntSrc,  IEntitySource newEntSrc)
	{
		int varCount = oldEntSrc.GetNumVars();
		
		for(int i = 0; i < varCount; i++)
		{
			string varName = oldEntSrc.GetVarName(i);
	
			if (!oldEntSrc.IsVariableSetDirectly(varName))
				continue;
			switch (oldEntSrc.GetDataVarType(i))
			{
				case DataVarType.FLAGS:
				{
					EntityFlags value;
					oldEntSrc.Get(varName, value);
					m_WEapi.SetVariableValue(newEntSrc, {}, varName, value.ToString()); // Set Name to given value
					break;
				}
				case DataVarType.VECTOR3:
				{
					vector value;
					oldEntSrc.Get(varName, value);
					m_WEapi.SetVariableValue(newEntSrc, {}, varName, value.ToString(false)); // Set Name to given value
					break;
				}
				case DataVarType.RESOURCE_NAME:
				{
					ResourceName value;
					oldEntSrc.Get(varName, value);
					m_WEapi.SetVariableValue(newEntSrc, {}, varName, value); // Set Name to given value
					break;
				}
				case DataVarType.INTEGER:
				{
					int value;
					oldEntSrc.Get(varName, value);
					m_WEapi.SetVariableValue(newEntSrc, {}, varName, value.ToString()); // Set Name to given value
					break;
				}
				case DataVarType.SCALAR:
				{
					float value;
					oldEntSrc.Get(varName, value);
					m_WEapi.SetVariableValue(newEntSrc, {}, varName, value.ToString()); // Set Name to given value
					break;
				}
				case DataVarType.BOOLEAN:
				{
					int value;
					oldEntSrc.Get(varName, value);
					m_WEapi.SetVariableValue(newEntSrc, {}, varName, value.ToString()); // Set Name to given value
					break;
				}
			}
		}
	}
	//-----------------------------------------------------------------------------------------------
	void CopyEntityPropertiesSpecial( IEntitySource oldEntSrc,  IEntitySource newEntSrc)
	{
		//TODO: hide behind bool ? this is not optimal as it's getting called on every entity, when there is realistically only 1 entity in the whole world matching this condition
		if(oldEntSrc.GetClassName() == "TimeAndWeatherManagerEntity" && newEntSrc.GetClassName() == "BaseWeatherManagerEntity")
		{
			CopyContainerProperties(oldEntSrc, newEntSrc);
			FixWeatherConfig("WeatherStateMachine", oldEntSrc,newEntSrc );
			
			ResourceName confResource;
			oldEntSrc.Get("WeatherParameters", confResource);
			AddAssetToMigrate(ExporterAssetType.MISC,confResource);
		}
		else if(newEntSrc.GetClassName() == "TreeEntity")
		{
			int value;
			if (oldEntSrc.IsVariableSetDirectly("SoundType"))
			{
				oldEntSrc.Get("SoundType", value);
				m_WEapi.SetVariableValue(newEntSrc, {}, "soundType", value.ToString());
			}
			if (oldEntSrc.IsVariableSetDirectly("m_iFoliageHeight"))
			{
				oldEntSrc.Get("m_iFoliageHeight", value);
				m_WEapi.SetVariableValue(newEntSrc, {}, "foliageHeight", value.ToString());
			}
		}
	}
	//-----------------------------------------------------------------------------------------------
	void FixWeatherConfig(string varName,  IEntitySource oldEntSrc,  IEntitySource newEntSrc)
	{
		if (!oldEntSrc.IsVariableSetDirectly(varName))
			return;
		
		ResourceName confResource;
		oldEntSrc.Get(varName, confResource);
		if(confResource.IsEmpty())
			return;
		string newFileName = FilePath.StripExtension(confResource.GetPath()) + m_sConvertedAssetsSuffix+".conf";

		FileIO.CopyFile(confResource.GetPath(), newFileName);

		string absPath;
		bool success =	Workbench.GetAbsolutePath(newFileName, absPath, true);

		MetaFile meta = resourceManager.RegisterResourceFile(absPath);
		meta.Save();

		resourceManager.WaitForFile("$ArmaReforger:"+newFileName, 3000);
		m_WEapi.SetVariableValue(newEntSrc, {}, varName, meta.GetResourceID());
		AddAssetToMigrate(ExporterAssetType.MISC,meta.GetResourceID());
		m_aCreatedFiles.Insert("$ArmaReforger:"+newFileName);
	
		TStringArray lines = {};
		FileHandle textFileR = FileIO.OpenFile("$ArmaReforger:"+newFileName, FileMode.READ);
		if (textFileR)
		{
			string line;
			while(textFileR.ReadLine(line) >= 0)
			{
				if(line.Contains("SCR_WeatherState"))
					line.Replace("SCR_WeatherState", "WeatherState");
				if(line.Contains("#AR-Weather_"))
					line.Replace("#AR-Weather_", "");
				lines.Insert(line);
			}
			textFileR.Close();
		}
		FileHandle textFileW = FileIO.OpenFile("$ArmaReforger:"+newFileName, FileMode.WRITE);
		if (textFileW)
		{
			foreach(string line:lines)
				textFileW.WriteLine(line);
			textFileW.Close();
		}
	}
	//-----------------------------------------------------------------------------------------------
	// this iterates through all prefabs seen on the map
	void ProcessDiscoveredPrefabs()
	{
		foreach(BaseContainer prefab:m_sDiscoveredPrefabs)
		{
			RegisterPrefabXob(prefab);
			ResourceName resName = prefab.GetResourceName();
			if(!m_mCreatedPrefabs.Contains(resName))//exclude prefabs that are being replaced by a new prefab
			{
				AddAssetToMigrate(ExporterAssetType.PREFAB,resName);
			}
		}
	}
	//-----------------------------------------------------------------------------------------------
	void PerformResourceExport(WBProgressDialog dialog)
	{
		ProcessDiscoveredPrefabs();
		GetMaterialsFromXobs();
		GetTexturesFromMaterials();

		ExportFiles(m_sPrefabsToMigrate);
		dialog.SetProgress(0.91);
		ExportFiles(m_sModelsToMigrate);
		dialog.SetProgress(0.92);
		ExportFiles(m_sMaterialsToMigrate);
		dialog.SetProgress(0.93);
		ExportFiles(m_sTexturesToMigrate);
		dialog.SetProgress(0.94);
		ExportFiles(m_sMiscToMigrate);
		dialog.SetProgress(0.95);
		
		Print("---------------------------Models------------------------------");
		foreach(ResourceName name:m_sModelsToMigrate)
			Print(name);

		Print("---------------------------Materials------------------------------");
		foreach(ResourceName name:m_sMaterialsToMigrate)
			Print(name);
		
		Print("---------------------------Textures------------------------------");
		foreach(ResourceName name:m_sTexturesToMigrate)
			Print(name);
		
		Print("-------------------------Prefabs --------------------------------");
		foreach(ResourceName name:m_sPrefabsToMigrate)
			Print(name);
		
		Print("-------------------------Misc --------------------------------");
		foreach(ResourceName name:m_sMiscToMigrate)
			Print(name);

		
		ExportSharedData();
		dialog.SetProgress(0.98);
		m_WorldEditor.Save();//Save the world
		ExportMapData();
		dialog.SetProgress(0.99);
		if(m_Config.m_bDeleteTempFiles)
			ClearExportSourceData();
	}
	//-----------------------------------------------------------------------------------------------
	void ClearExportSourceData()
	{
		foreach(ResourceName resName:m_mCreatedPrefabs)
		{
			DeleteFilePlus(resName.GetPath());
		}
		foreach(string filePath:m_aCreatedFiles)
		{
			DeleteFilePlus(filePath);
		}
	} 
	//-----------------------------------------------------------------------------------------------
	void DeleteFilePlus(string path)
	{
		string pathMeta = path +".meta";
		
		FileIO.DeleteFile(path);
		FileIO.DeleteFile(pathMeta);
	}
	//-----------------------------------------------------------------------------------------------
	bool IsExportTargetFolderEmpty()
	{
		TStringArray filesAndDirs = {};
		FileIO.FindFiles(filesAndDirs.Insert,m_ExportDestination, string.Empty);
		return filesAndDirs.Count() == 0;
	}
	//--------------------------------------------------------
	static IEntityComponentSource FindComponentSource(IEntitySource prefabEntity, string componentClassName)
	{
		if (!prefabEntity)
			return null;

		IEntityComponentSource componentSource;
		for (int i, componentsCount = prefabEntity.GetComponentCount(); i < componentsCount; i++)
		{
			componentSource = prefabEntity.GetComponent(i);
			if (componentSource.GetClassName() == componentClassName)
				return componentSource;
		}

		return null;
	}
}
#endif