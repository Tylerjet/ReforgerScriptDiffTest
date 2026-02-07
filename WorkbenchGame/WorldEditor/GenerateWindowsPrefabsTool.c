/** @defgroup GenerateWindowsPrefabsTool Windows Prefab Generator
Workbench plugin for generation of windows & glass prefabs
*/

/** @ingroup GenerateWindowsPrefabsTool
*/

/*!
Workbench plugin for generation of windows & glass prefabs.

Two separate buttons for glass and window frames generation
*/
[WorkbenchToolAttribute("Windows Prefabs Generator Tool", "Generate prefabs for destroyable windows.\n\Create Window - Triggers creation of window frames prefabs\nCreate Glass - Triggers creation of glass panels prefabs", "2", awesomeFontCode : 0xf2d2)]
class GenerateWindowsPrefabsTool : WorldEditorTool
{
	// General
	[Attribute("1", UIWidgets.ComboBox, "File system where new prefabs will be created", "",ParamEnumAddons.FromEnum(), "General")]
	int m_AddonToUse;
	
	// Glass generation
	[Attribute("{86834A0D5920F32F}Prefabs/Structures/Core/DestructibleGlass_Base.et", UIWidgets.ResourceNamePicker, "Choose a base class of your glass prefabs.", "et", NULL, "Glass")]
	private ResourceName m_BaseClassGlass;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Choose a path to save your glass prefabs.", "unregfolders", NULL, "Glass")]
	private ResourceName m_PathToSaveGlass;
	
	[Attribute("", UIWidgets.ResourceAssignArray, "Choose XOB files to process.", "xob", NULL, "Glass")]
	protected ref array<ResourceName> m_GlassVariants;
	
	[Attribute("1", UIWidgets.EditBox, "Number of damage variants","", NULL, "Glass")]
	private int m_GlassDmgCount;
	
	[Attribute("", UIWidgets.EditBox, "Model base path and base name to use for the shards (adds the index to the end automatically)","", NULL, "Glass")]
	private string m_sGlassName;
	
	[Attribute("0", UIWidgets.CheckBox, "Generate name from save path.","", NULL, "Glass")]
	private bool m_bGlassNameGenerate;
	
	[Attribute("0", UIWidgets.CheckBox, "Automatically detect number of damage variant of glass pieces.","", NULL, "Glass")]
	private bool m_bGlassAutoCount;
	
	[Attribute("1", UIWidgets.CheckBox, "Use Multi Phase destruction instead of Fractal destructrion.","", NULL, "Glass")]
	private bool m_bUseMultiPhaseDestruction;
	
	// Window generation
	[Attribute("{3FF56BA19C8780DE}Prefabs/Structures/BuildingParts/Windows/Window_Base.et", UIWidgets.ResourceNamePicker, "Choose a base class of your window prefabs.", "et", NULL, "Window")]
	private ResourceName m_BaseClass;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Choose a path to save your window prefab.", "unregFolders", NULL, "Window")]
	private ResourceName m_PathToSave;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "", "xob", NULL, "Window")]
	protected ref array<ResourceName> m_frameModels;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "", "et", NULL, "Window")]
	protected ref array<ResourceName> m_glassPrefabs;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Pick a config with sockets & glass prefabs configuration.", "conf", NULL, "Window")]
	private ResourceName m_glassConfig;
	
	private ref SCR_WindowsGeneratorSockets m_aGlassPrefabsGroupConfig;
	private ref array<string> m_aSocketsList;
	
	private string tempName;
	private WorldEditor we;
	private ResourceManager rb;
		
	IEntityComponentSource m_meshObject;
	
	// Trim GUID
	private string TrimGUID(string inputString)
	{
		if (inputString.Contains("{") && inputString.Contains("}"))
		{
			int guidBracket = inputString.IndexOf("}");
			inputString = inputString.Substring(guidBracket+1, inputString.Length() - guidBracket - 1);
			return inputString;
		} 
		return "";
	}

	// Get path to destroyed model
	private string GetDestroyedModel(string inputString, int variant)
	{
		string tempString = TrimGUID(inputString);
		if(variant > 0) {tempString.Replace("Glass_01","Glass_0" + variant)};
		tempString = FilePath.StripFileName(tempString) + "dst/" + FilePath.StripPath(tempString);
		tempString.Replace(".xob","_dst.xob");
		private string absPathDst;
		Workbench.GetAbsolutePath(tempString, absPathDst);
		if (!absPathDst) 
		{
			Print("Unable to find destroyed model for " + tempString + ". Base model will be used instead", LogLevel.WARNING);
			return inputString;
		}
		MetaFile meta = rb.GetMetaFile(absPathDst);
		tempString = String(meta.GetResourceID());
		return tempString;
	}
	// Get damage mask string
	private string GetDamageMask(string inputString)
	{
		inputString = TrimGUID(inputString);
		inputString = FilePath.StripFileName(inputString) + "Dmg/" + FilePath.StripPath(inputString);
		inputString.Replace(".xob","_dmg_");
		return inputString;
	}
	// Check how many damage variants are present in data
	private string GetDamageVariants(string inputString)
	{
		private bool variantExist = true;
		private int numberOfVariants = 0;
		private string outputString; 
		while (true)
		{
			private string absPath;
			string inputStringTemp = inputString + "0" + numberOfVariants + ".xob";
			variantExist = Workbench.GetAbsolutePath(inputStringTemp, absPath);
			if(!variantExist) return outputString; 
			outputString = outputString + String(Workbench.GetResourceName(inputStringTemp)) + ",";
			numberOfVariants++;
		}
		return "";
	}
	private int GetGlassCount(string inputString)
	{
		private bool variantExist = true;
		private int numberOfVariants = 1;
		inputString = TrimGUID(inputString);
		inputString.Replace("Glass_01","Dmg/Glass_01");
		while (true)
		{
			private string absPath;
			string inputStringTemp = inputString;
			inputStringTemp.Replace(".xob","_dmg_00.xob");
			inputStringTemp.Replace("Glass_01","Glass_0" + numberOfVariants);
			Print(inputStringTemp);
			variantExist = Workbench.GetAbsolutePath(inputStringTemp, absPath);
			if(!variantExist) return numberOfVariants-1; 
			numberOfVariants++;
		}
		return 1;
	}
	
	
	
	[ButtonAttribute("Create Window")]
	private void Execute()
	{
		// Validate input parameters
		if (!m_frameModels) 
		{
			Print("Frame Models is empty. Please fill it with correct name", LogLevel.ERROR);
			return;
		}
		if (!m_PathToSave) 
		{
			Print("Path To Save is empty. Please select valid save location", LogLevel.ERROR);
			return;
		}
		if (!m_BaseClass) 
		{
			Print("Base Class is empty. Please select valid base prefab for your window", LogLevel.ERROR);
			return;
		}
		if (!m_glassPrefabs && !m_glassConfig) 
		{
			Print("Glass Prefabs and Glass Config is empty. Please select valid prefab for window glass", LogLevel.ERROR);
			return;
		}
		if (m_glassPrefabs && !m_glassConfig) 
		{
			if(m_frameModels.Count() != m_glassPrefabs.Count())
			{
				Print("Frame Models count doesn't correspond to m_glassPrefabs! Each single frame model needs one glass prefab.", LogLevel.ERROR);
				return;
			}
		}
	
		we = Workbench.GetModule(WorldEditor);
		rb = Workbench.GetModule(ResourceManager);
		
		// Get addon 
		string addon = SCR_AddonTool.GetAddonIndex(m_AddonToUse);
		addon = SCR_AddonTool.ToFileSystem(addon);
	
		// Get absolute path for CreateEntityTemplate
		private string m_sPathToSave = TrimGUID(String(m_PathToSave));
		private string absPath;
		Workbench.GetAbsolutePath(addon + m_sPathToSave, absPath);
		
		// Load config if its present
		if(m_glassConfig) {
			Resource holder = BaseContainerTools.LoadContainer(m_glassConfig);
			if (!holder)
				return;
			
			m_aGlassPrefabsGroupConfig = SCR_WindowsGeneratorSockets.Cast(BaseContainerTools.CreateInstanceFromContainer(holder.GetResource().ToBaseContainer()));
			m_aSocketsList = new array<string>();
			foreach (int currentIndex2, SCR_WindowsGeneratorSocketsPair currentElement2: m_aGlassPrefabsGroupConfig.m_ReplacementArray)
			{
				m_aSocketsList.Insert(currentElement2.m_SocketName);
			}
		}

		foreach (int currentIndex, string currentElement: m_frameModels)
		{
			// Load metafile and get resource GUID
			private string modelName = currentElement; 
			private string modelDstName = GetDestroyedModel(modelName,0);
		
			// Create entity with selected base class
			m_API.BeginEntityAction("Processing " + modelName);
			IEntity m_Entity = m_API.CreateEntity(m_BaseClass, "", m_API.GetCurrentEntityLayerId(), null, vector.Zero, vector.Zero);
			IEntitySource entSrc = m_API.EntityToSource(m_Entity);
		
		
			// Apply basic values
			auto containerPath = new array<ref ContainerIdPathEntry>();
		
			
			int count = entSrc.GetComponentCount();
			bool m_Found_MeshObject;
			for(int i = 0; i < count; i++) //workaround for nonfunctional FindComponent(Hierarchy)
			{
				IEntityComponentSource comp = entSrc.GetComponent(i);
	
				if(comp.GetClassName() == "MeshObject")
				{
					m_Found_MeshObject = true;
				}
			}
		
			// Try to find MeshObject component and if it doesn't exist, then create new one
			if(!m_Found_MeshObject) { m_meshObject = m_API.CreateComponent(entSrc,"MeshObject");}
			containerPath.Clear();
			containerPath.Insert(new ContainerIdPathEntry("MeshObject")); 
			m_API.SetVariableValue(entSrc, containerPath, "Object", modelName);
			m_Entity = m_API.SourceToEntity(entSrc);
		
			// Get Bone Names and extract socket names from it
			array<string> boneNames = new array<string>; 
			m_Entity.GetBoneNames(boneNames);
		
			int m_glassCount = boneNames.Count();
			if(m_glassCount > 0) 
			{
				// Remove scene root bone
				boneNames.Remove(boneNames.Find("Scene_Root"));
				m_glassCount--;
			}else{
				// Set it to 1 when m_glassPrefabs are used
				if(!m_glassConfig) {
					m_glassCount = 1;
				}else{
					Print("No sockets were detected even though model is using config!", LogLevel.ERROR);
				}
			}
			private string m_PivotName;
			private string m_GlassPrefabName;
			for(int i = 0; i < m_glassCount; i++) 
			{
				if(!m_glassConfig) {
					// Use prefabs list
					m_GlassPrefabName = m_glassPrefabs[currentIndex];
				}
				if(m_glassCount >= 1) {
					if(m_glassConfig) {
						// Use config file
						m_PivotName = boneNames[i];
						string searchString = m_PivotName;
						searchString.ToLower();
						searchString = searchString.Substring(0,searchString.LastIndexOf("_"));
						int indexFound = m_aSocketsList.Find(searchString);
						if(indexFound >= 0) 
						{
							m_GlassPrefabName = m_aGlassPrefabsGroupConfig.m_ReplacementArray[indexFound].m_GlassPrefab
						}else{
							PrintFormat("Unable to find prefab for coresponding socket %1. Full pivot name: %2",searchString,m_PivotName);
						}
					}else{
						// Use prefabs list
						m_PivotName = boneNames[i];
						m_GlassPrefabName = m_glassPrefabs[currentIndex];
					}
				}
				IEntity glassEntity = m_API.CreateEntity(m_GlassPrefabName, "", m_API.GetCurrentEntityLayerId(), entSrc, vector.Zero, vector.Zero);
				if(m_PivotName != "") 
				{
					IEntitySource glassEntSrc = m_API.EntityToSource(glassEntity);
					array<ref ContainerIdPathEntry> subcContainerPath = { ContainerIdPathEntry("Hierarchy",)};
					m_API.SetVariableValue(glassEntSrc, subcContainerPath, "PivotID", m_PivotName)
				}
			}
			// Fill slots with glass prefabs. If there are sockets, 
		
			// Save our modified entity to prefab
			private bool fileCreated;
			array<string> tempArray = new array<string>();
			modelName.Split("/",tempArray,false);
			tempName = tempArray[tempArray.Count()-1];
			tempName.Replace(".xob","");
			fileCreated = m_API.CreateEntityTemplate(entSrc,FilePath.Concat(absPath, tempName  + "_base.et"));
		
			if (fileCreated) 
			{
				Print(string.Format("@\"%1\"", FilePath.Concat(absPath, tempName  + "_base.et")));
			}else{
				Print("Script was unable to create new prefab at designated location", LogLevel.ERROR);
			}
		
			// Delete entiy and finish operations
			m_Entity = m_API.SourceToEntity(entSrc);
			m_API.DeleteEntity(m_Entity);
			m_API.EndEntityAction();	
	
		}
	}
	[ButtonAttribute("Create Glass")]
	private void ExecuteGlass()
	{
		// Validate input parameters
		if (m_sGlassName == "" && !m_bGlassNameGenerate) 
		{
			Print("Glass Name is empty. Please fill it with correct name", LogLevel.ERROR);
			return;
		}
		if (!m_PathToSaveGlass) 
		{
			Print("Path To SaveGlass is empty. Please select valid save location", LogLevel.ERROR);
			return;
		}
		if (!m_BaseClassGlass) 
		{
			Print("Base Class Glass is empty. Please select base prefab for your glass", LogLevel.ERROR);
			return;
		}
		if (m_GlassVariants.Count() == 0) 
		{
			Print("Glass Variants is empty. Please select at least one glass XOB variant", LogLevel.ERROR);
			return;
		}
		if(m_bGlassNameGenerate) m_sGlassName = FilePath.StripPath(m_PathToSaveGlass);
	
		we = Workbench.GetModule(WorldEditor);
		rb = Workbench.GetModule(ResourceManager);
	
		// Get addon 
		string addon = SCR_AddonTool.GetAddonIndex(m_AddonToUse);
		addon = SCR_AddonTool.ToFileSystem(addon);
	
		// Get absolute path for CreateEntityTemplate
		private string m_sPathToSave = TrimGUID(String(m_PathToSaveGlass));
		private string absPath;
		Workbench.GetAbsolutePath(addon + m_sPathToSave, absPath);
	
		if (!m_GlassVariants) 
		{
			Print("At least one glass variant is required in order to generate glass prefab!", LogLevel.ERROR);
			return;
		}
	
		// Automatically detect number of glass variants
		if(m_bGlassAutoCount)
		{
			m_GlassDmgCount = GetGlassCount(String(m_GlassVariants[0]));
		}
	
		// Create new entity
		m_API.BeginEntityAction("Processing " + m_sGlassName);
		IEntity m_Entity = m_API.CreateEntity(m_BaseClassGlass, "", m_API.GetCurrentEntityLayerId(), null, vector.Zero, vector.Zero);
		IEntitySource entSrc = m_API.EntityToSource(m_Entity);
		array<ref ContainerIdPathEntry> containerPath;
	
		// Add glass variants to entity with correct parameters
		// Use Multi Phase destruction
		if(m_bUseMultiPhaseDestruction)
		{
			// Modify SCR_DestructionMultiPhaseComponent first m_DamagePhases and replace m_PhaseModel with ResourceName fetched from SCR_DestructionFractalComponent
			array<ref ContainerIdPathEntry> subcContainerPath = { ContainerIdPathEntry("SCR_DestructionMultiPhaseComponent"), ContainerIdPathEntry("m_DamagePhases", 0) };
			m_API.SetVariableValue(entSrc, subcContainerPath, "m_PhaseModel",GetDestroyedModel(m_GlassVariants[0],0));
		}else
		{
		// Use Fractal destruction
			containerPath = {new ContainerIdPathEntry("SCR_DestructionFractalComponent")};
			foreach (int currentIndex, ResourceName currentElement: m_GlassVariants)
			{
				for(int i = m_GlassDmgCount; i > 0;i--)
				{
					// Modify damage mask
					string damageMask = GetDamageMask(currentElement);
					if(m_GlassDmgCount > 1)
					{
						string tempPath = FilePath.StripFileName(damageMask);
						string tempFile = FilePath.StripPath(damageMask);
						tempFile.Replace("Glass_01","Glass_0" + i);
						damageMask = tempPath + tempFile;
					};
				
					// Apply parameters to sub variant
					m_API.CreateObjectArrayVariableMember(entSrc, containerPath, "m_FractalVariants", "SCR_FractalVariation",currentIndex);
					array<ref ContainerIdPathEntry> subcContainerPath = { ContainerIdPathEntry("SCR_DestructionFractalComponent"), ContainerIdPathEntry("m_FractalVariants", currentIndex) };
					m_API.SetVariableValue(entSrc, subcContainerPath, "m_ModelNormal", String(currentElement));
					m_API.SetVariableValue(entSrc, subcContainerPath, "m_ModelDestroyed", GetDestroyedModel(currentElement,i));
					m_API.SetVariableValue(entSrc, subcContainerPath, "m_aModelFragments", GetDamageVariants(damageMask));
				};
			}
		}
		
		// Assign some model to MeshObject
		containerPath = {new ContainerIdPathEntry("MeshObject")};
		m_API.SetVariableValue(entSrc, containerPath, "Object", m_GlassVariants[0]);
	
		// Save our modified entity to prefab
		private bool fileCreated;
		fileCreated = m_API.CreateEntityTemplate(entSrc,FilePath.Concat(absPath, m_sGlassName  + ".et"));
		if (fileCreated) 
		{
			Print(string.Format("@\"%1\"", FilePath.Concat(absPath, m_sGlassName  + ".et")));
		}else{
			Print("Script was unable to create new prefab at designated location", LogLevel.ERROR);
		}
	
		m_Entity = m_API.SourceToEntity(entSrc);
		m_API.DeleteEntity(m_Entity);
		m_API.EndEntityAction();	
	}

	//------------------------------------------------------------------------------------------------
	void GenerateWindowsPrefabsTool(IEntitySource src, IEntity parent)
	{
	}

	//------------------------------------------------------------------------------------------------
	void ~GenerateWindowsPrefabsTool()
	{
	}
};

//------------------------------------------------------------------------------------------------
//! SCR_WindowsGeneratorSocketList config
[BaseContainerProps(configRoot: true)]
class SCR_WindowsGeneratorSockets
{
	[Attribute()]
	ref array<ref SCR_WindowsGeneratorSocketsPair> m_ReplacementArray;
};

[BaseContainerProps(configRoot: false), BaseContainerCustomTitleField("m_SocketName")]
class SCR_WindowsGeneratorSocketsPair
{
	[Attribute()] string m_SocketName;
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "", params: "et")]
	ResourceName m_GlassPrefab;
};