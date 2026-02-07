/** @defgroup GenerateWindowsPrefabsTool Windows Prefab Generator
Workbench plugin for generation of windows & glass prefabs
*/

/** @ingroup GenerateWindowsPrefabsTool
*/

/*!
Workbench plugin for generation of windows & glass prefabs.

Two separate buttons for glass and window frames generation
*/
[WorkbenchToolAttribute("Windows Prefabs Generator Tool", "Generate prefabs for destroyable windows.\nCreate Window - Triggers creation of window frames prefabs\nCreate Glass - Triggers creation of glass panels prefabs", "2", awesomeFontCode : 0xF2D2)]
class GenerateWindowsPrefabsTool : WorldEditorTool
{
	// General
	[Attribute("1", UIWidgets.ComboBox, "File system where new prefabs will be created", "", ParamEnumAddons.FromEnum(), "General")]
	protected int m_iAddonToUse;

	// Glass generation
	[Attribute("{86834A0D5920F32F}Prefabs/Structures/Core/DestructibleGlass_Base.et", UIWidgets.ResourceNamePicker, "Choose a base class of your glass prefabs.", "et", null, "Glass")]
	protected ResourceName m_sBaseClassGlass;

	[Attribute("", UIWidgets.ResourceNamePicker, "Choose a path to save your glass prefabs.", "unregfolders", null, "Glass")]
	protected ResourceName m_sPathToSaveGlass;

	[Attribute("", UIWidgets.ResourceAssignArray, "Choose XOB files to process.", "xob", null, "Glass")]
	protected ref array<ResourceName> m_aGlassVariants;

	[Attribute("1", UIWidgets.EditBox, "Number of damage variants", "", null, "Glass")]
	protected int m_GlassDmgCount;

	[Attribute("", UIWidgets.EditBox, "Model base path and base name to use for the shards (adds the index to the end automatically)", "", null, "Glass")]
	protected string m_sGlassName;

	[Attribute("0", UIWidgets.CheckBox, "Generate name from save path.", "", null, "Glass")]
	protected bool m_bGlassNameGenerate;

	[Attribute("0", UIWidgets.CheckBox, "Automatically detect number of damage variant of glass pieces.", "", null, "Glass")]
	protected bool m_bGlassAutoCount;

	[Attribute("1", UIWidgets.CheckBox, "Use Multi Phase destruction instead of Fractal destruction.", "", null, "Glass")]
	protected bool m_bUseMultiPhaseDestruction;

	// Window generation
	[Attribute("{3FF56BA19C8780DE}Prefabs/Structures/BuildingParts/Windows/Window_Base.et", UIWidgets.ResourceNamePicker, "Choose a base class of your window prefabs.", "et", null, "Window")]
	protected ResourceName m_sBaseClass;

	[Attribute("", UIWidgets.ResourceNamePicker, "Choose a path to save your window prefab.", "unregFolders", null, "Window")]
	protected ResourceName m_sPathToSave;

	[Attribute("", UIWidgets.ResourceNamePicker, "", "xob", null, "Window")]
	protected ref array<ResourceName> m_aFrameModels;

	[Attribute("", UIWidgets.ResourceNamePicker, "", "et", null, "Window")]
	protected ref array<ResourceName> m_aGlassPrefabs;

	[Attribute("", UIWidgets.ResourceNamePicker, "Pick a config with sockets & glass prefabs configuration.", "conf", null, "Window")]
	protected ResourceName m_sGlassConfig;

	protected WorldEditor worldEditor;
	protected ResourceManager resourceManager;

	protected IEntityComponentSource m_MeshObject;

	//------------------------------------------------------------------------------------------------
	// Trim GUID
	protected string TrimGUID(string inputString)
	{
		if (inputString.Contains("{") && inputString.Contains("}"))
		{
			"{"; // fix indent issue
			int guidBracket = inputString.IndexOf("}");
			inputString = inputString.Substring(guidBracket + 1, inputString.Length() - guidBracket - 1);
			return inputString;
		}
		return "";
	}

	//------------------------------------------------------------------------------------------------
	// Get path to destroyed model
	protected string GetDestroyedModel(string inputString, int variant)
	{
		string tempString = TrimGUID(inputString);
		if (variant > 0)
			tempString.Replace("Glass_01", "Glass_0" + variant);

		tempString = FilePath.StripFileName(tempString) + "dst/" + FilePath.StripPath(tempString);
		tempString.Replace(".xob", "_dst.xob");
		string absPathDst;
		Workbench.GetAbsolutePath(tempString, absPathDst);
		if (!absPathDst)
		{
			Print("Unable to find destroyed model for " + tempString + ". Base model will be used instead", LogLevel.WARNING);
			return inputString;
		}
		MetaFile meta = resourceManager.GetMetaFile(absPathDst);
		tempString = String(meta.GetResourceID());
		return tempString;
	}

	//------------------------------------------------------------------------------------------------
	// Get damage mask string
	protected string GetDamageMask(string inputString)
	{
		inputString = TrimGUID(inputString);
		inputString = FilePath.StripFileName(inputString) + "Dmg/" + FilePath.StripPath(inputString);
		inputString.Replace(".xob", "_dmg_");
		return inputString;
	}

	//------------------------------------------------------------------------------------------------
	// Check how many damage variants are present in data
	protected string GetDamageVariants(string inputString)
	{
		bool variantExist = true;
		int numberOfVariants = 0;
		string outputString;
		while (true)
		{
			string absPath;
			string inputStringTemp = inputString + "0" + numberOfVariants + ".xob";
			variantExist = Workbench.GetAbsolutePath(inputStringTemp, absPath);
			if (!variantExist)
				return outputString;

			outputString = outputString + String(Workbench.GetResourceName(inputStringTemp)) + ",";
			numberOfVariants++;
		}
		return "";
	}

	//------------------------------------------------------------------------------------------------
	protected int GetGlassCount(string inputString)
	{
		bool variantExist = true;
		int numberOfVariants = 1;
		inputString = TrimGUID(inputString);
		inputString.Replace("Glass_01", "Dmg/Glass_01");
		while (true)
		{
			string absPath;
			string inputStringTemp = inputString;
			inputStringTemp.Replace(".xob", "_dmg_00.xob");
			inputStringTemp.Replace("Glass_01", "Glass_0" + numberOfVariants);
			Print(inputStringTemp);
			variantExist = Workbench.GetAbsolutePath(inputStringTemp, absPath);
			if (!variantExist)
				return numberOfVariants - 1;
			numberOfVariants++;
		}
		return 1;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Create Window")]
	protected void Execute()
	{
		// Validate input parameters
		if (!m_aFrameModels)
		{
			Print("Frame Models is empty. Please fill it with correct name", LogLevel.ERROR);
			return;
		}

		if (!m_sPathToSave)
		{
			Print("Path To Save is empty. Please select valid save location", LogLevel.ERROR);
			return;
		}

		if (!m_sBaseClass)
		{
			Print("Base Class is empty. Please select valid base prefab for your window", LogLevel.ERROR);
			return;
		}

		if (!m_aGlassPrefabs && !m_sGlassConfig)
		{
			Print("Glass Prefabs and Glass Config is empty. Please select valid prefab for window glass", LogLevel.ERROR);
			return;
		}

		if (m_aGlassPrefabs && !m_sGlassConfig)
		{
			if (m_aFrameModels.Count() != m_aGlassPrefabs.Count())
			{
				Print("Frame Models count doesn't correspond to m_aGlassPrefabs! Each single frame model needs one glass prefab.", LogLevel.ERROR);
				return;
			}
		}

		worldEditor = Workbench.GetModule(WorldEditor);
		resourceManager = Workbench.GetModule(ResourceManager);

		// Get addon
		string addon = SCR_AddonTool.GetAddonIndex(m_iAddonToUse);
		addon = SCR_AddonTool.ToFileSystem(addon);

		// Get absolute path for CreateEntityTemplate
		string pathToSave = TrimGUID(String(m_sPathToSave));
		string absPath;
		Workbench.GetAbsolutePath(addon + pathToSave, absPath);

		array<string> socketsList;
		SCR_WindowsGeneratorSockets windowsGeneratorSockets;
		// Load config if its present
		if (m_sGlassConfig)
		{
			Resource holder = BaseContainerTools.LoadContainer(m_sGlassConfig);
			if (!holder)
				return;

			windowsGeneratorSockets = SCR_WindowsGeneratorSockets.Cast(BaseContainerTools.CreateInstanceFromContainer(holder.GetResource().ToBaseContainer()));
			socketsList = {};
			foreach (int currentIndex2, SCR_WindowsGeneratorSocketsPair currentElement2 : windowsGeneratorSockets.m_aReplacementArray)
			{
				socketsList.Insert(currentElement2.m_sSocketName);
			}
		}

		foreach (int currentIndex, string currentElement : m_aFrameModels)
		{
			// Load metafile and get resource GUID
			string modelName = currentElement;
			string modelDstName = GetDestroyedModel(modelName, 0);

			// Create entity with selected base class
			m_API.BeginEntityAction("Processing " + modelName);
			IEntity entity = m_API.CreateEntity(m_sBaseClass, "", m_API.GetCurrentEntityLayerId(), null, vector.Zero, vector.Zero);
			IEntitySource entSrc = m_API.EntityToSource(entity);

			// Apply basic values
			array<ref ContainerIdPathEntry> containerPath = {};

			int count = entSrc.GetComponentCount();
			bool meshObjectFound;
			for (int i = 0; i < count; i++) //workaround for nonfunctional FindComponent(Hierarchy)
			{
				if (entSrc.GetComponent(i).GetClassName() == "MeshObject")
				{
					meshObjectFound = true;
					break;
				}
			}

			// Try to find MeshObject component and if it doesn't exist, then create new one
			if (!meshObjectFound)
				m_MeshObject = m_API.CreateComponent(entSrc, "MeshObject");

			containerPath.Clear();
			containerPath.Insert(new ContainerIdPathEntry("MeshObject"));
			m_API.SetVariableValue(entSrc, containerPath, "Object", modelName);
			entity = m_API.SourceToEntity(entSrc);

			// Get Bone Names and extract socket names from it
			array<string> boneNames = {};
			entity.GetBoneNames(boneNames);

			int glassCount = boneNames.Count();
			if (glassCount > 0)
			{
				// Remove scene root bone
				boneNames.Remove(boneNames.Find("Scene_Root"));
				glassCount--;
			}
			else
			{
				// Set it to 1 when m_aGlassPrefabs are used
				if (!m_sGlassConfig)
					glassCount = 1;
				else
					Print("No sockets were detected even though model is using config!", LogLevel.ERROR);
			}

			string pivotName;
			string glassPrefabName;
			string tempName;
			for (int i = 0; i < glassCount; i++)
			{
				// Use prefabs list
				if (!m_sGlassConfig)
					glassPrefabName = m_aGlassPrefabs[currentIndex];

				if (glassCount >= 1)
				{
					if (m_sGlassConfig)
					{
						// Use config file
						pivotName = boneNames[i];
						string searchString = pivotName;
						searchString.ToLower();
						searchString = searchString.Substring(0, searchString.LastIndexOf("_"));
						int indexFound = socketsList.Find(searchString);
						if (indexFound >= 0)
							glassPrefabName = windowsGeneratorSockets.m_aReplacementArray[indexFound].m_sGlassPrefab;
						else
							PrintFormat("Unable to find prefab for coresponding socket %1. Full pivot name: %2", searchString, pivotName);
					}
					else
					{
						// Use prefabs list
						pivotName = boneNames[i];
						glassPrefabName = m_aGlassPrefabs[currentIndex];
					}
				}

				IEntity glassEntity = m_API.CreateEntity(glassPrefabName, "", m_API.GetCurrentEntityLayerId(), entSrc, vector.Zero, vector.Zero);
				if (!pivotName.IsEmpty())
				{
					IEntitySource glassEntSrc = m_API.EntityToSource(glassEntity);
					array<ref ContainerIdPathEntry> subcContainerPath = { ContainerIdPathEntry("Hierarchy") };
					m_API.SetVariableValue(glassEntSrc, subcContainerPath, "PivotID", pivotName)
				}
			}
			// Fill slots with glass prefabs. If there are sockets,

			// Save our modified entity to prefab
			bool fileCreated;
			array<string> tempArray = {};
			modelName.Split("/", tempArray, false);
			tempName = tempArray[tempArray.Count() - 1];
			tempName.Replace(".xob", "");
			fileCreated = m_API.CreateEntityTemplate(entSrc, FilePath.Concat(absPath, tempName + "_base.et"));

			if (fileCreated)
				Print(string.Format("@\"%1\"", FilePath.Concat(absPath, tempName + "_base.et")));
			else
				Print("Script was unable to create new prefab at designated location", LogLevel.ERROR);

			// Delete entiy and finish operations
			entity = m_API.SourceToEntity(entSrc);
			m_API.DeleteEntity(entity);
			m_API.EndEntityAction();
		}
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Create Glass")]
	protected void ExecuteGlass()
	{
		// Validate input parameters
		if (m_sGlassName.IsEmpty() && !m_bGlassNameGenerate)
		{
			Print("Glass Name is empty. Please fill it with correct name", LogLevel.ERROR);
			return;
		}

		if (!m_sPathToSaveGlass)
		{
			Print("Path To SaveGlass is empty. Please select valid save location", LogLevel.ERROR);
			return;
		}

		if (!m_sBaseClassGlass)
		{
			Print("Base Class Glass is empty. Please select base prefab for your glass", LogLevel.ERROR);
			return;
		}

		if (m_aGlassVariants.IsEmpty())
		{
			Print("Glass Variants is empty. Please select at least one glass XOB variant", LogLevel.ERROR);
			return;
		}

		if (m_bGlassNameGenerate)
			m_sGlassName = FilePath.StripPath(m_sPathToSaveGlass);

		worldEditor = Workbench.GetModule(WorldEditor);
		resourceManager = Workbench.GetModule(ResourceManager);

		// Get addon
		string addon = SCR_AddonTool.GetAddonIndex(m_iAddonToUse);
		addon = SCR_AddonTool.ToFileSystem(addon);

		// Get absolute path for CreateEntityTemplate
		string pathToSave = TrimGUID(String(m_sPathToSaveGlass));
		string absPath;
		Workbench.GetAbsolutePath(addon + pathToSave, absPath);

		if (!m_aGlassVariants)
		{
			Print("At least one glass variant is required in order to generate glass prefab!", LogLevel.ERROR);
			return;
		}

		// Automatically detect number of glass variants
		if (m_bGlassAutoCount)
			m_GlassDmgCount = GetGlassCount(String(m_aGlassVariants[0]));

		// Create new entity
		m_API.BeginEntityAction("Processing " + m_sGlassName);
		IEntity m_Entity = m_API.CreateEntity(m_sBaseClassGlass, "", m_API.GetCurrentEntityLayerId(), null, vector.Zero, vector.Zero);
		IEntitySource entSrc = m_API.EntityToSource(m_Entity);
		array<ref ContainerIdPathEntry> containerPath;

		// Add glass variants to entity with correct parameters
		// Use Multi Phase destruction
		if (m_bUseMultiPhaseDestruction)
		{
			// Modify SCR_DestructionMultiPhaseComponent first m_DamagePhases and replace m_PhaseModel with ResourceName fetched from SCR_DestructionFractalComponent
			array<ref ContainerIdPathEntry> subcContainerPath = { ContainerIdPathEntry("SCR_DestructionMultiPhaseComponent"), ContainerIdPathEntry("m_DamagePhases", 0) };
			m_API.SetVariableValue(entSrc, subcContainerPath, "m_PhaseModel", GetDestroyedModel(m_aGlassVariants[0], 0));
		}
		else
		{
		// Use Fractal destruction
			containerPath = { new ContainerIdPathEntry("SCR_DestructionFractalComponent") };
			foreach (int currentIndex, ResourceName currentElement : m_aGlassVariants)
			{
				for (int i = m_GlassDmgCount; i > 0;i--)
				{
					// Modify damage mask
					string damageMask = GetDamageMask(currentElement);
					if (m_GlassDmgCount > 1)
					{
						string tempPath = FilePath.StripFileName(damageMask);
						string tempFile = FilePath.StripPath(damageMask);
						tempFile.Replace("Glass_01", "Glass_0" + i);
						damageMask = tempPath + tempFile;
					};

					// Apply parameters to sub variant
					m_API.CreateObjectArrayVariableMember(entSrc, containerPath, "m_FractalVariants", "SCR_FractalVariation", currentIndex);
					array<ref ContainerIdPathEntry> subcContainerPath = { ContainerIdPathEntry("SCR_DestructionFractalComponent"), ContainerIdPathEntry("m_FractalVariants", currentIndex) };
					m_API.SetVariableValue(entSrc, subcContainerPath, "m_ModelNormal", String(currentElement));
					m_API.SetVariableValue(entSrc, subcContainerPath, "m_ModelDestroyed", GetDestroyedModel(currentElement, i));
					m_API.SetVariableValue(entSrc, subcContainerPath, "m_aModelFragments", GetDamageVariants(damageMask));
				};
			}
		}

		// Assign some model to MeshObject
		containerPath = { new ContainerIdPathEntry("MeshObject") };
		m_API.SetVariableValue(entSrc, containerPath, "Object", m_aGlassVariants[0]);

		// Save our modified entity to prefab
		bool fileCreated;
		fileCreated = m_API.CreateEntityTemplate(entSrc, FilePath.Concat(absPath, m_sGlassName + ".et"));
		if (fileCreated)
			Print(string.Format("@\"%1\"", FilePath.Concat(absPath, m_sGlassName + ".et")));
		else
			Print("Script was unable to create new prefab at designated location", LogLevel.ERROR);

		m_Entity = m_API.SourceToEntity(entSrc);
		m_API.DeleteEntity(m_Entity);
		m_API.EndEntityAction();
	}
};

//------------------------------------------------------------------------------------------------
//! SCR_WindowsGeneratorSocketList config
[BaseContainerProps(configRoot: true)]
class SCR_WindowsGeneratorSockets
{
	[Attribute()]
	ref array<ref SCR_WindowsGeneratorSocketsPair> m_aReplacementArray;
};

[BaseContainerProps(configRoot: false), BaseContainerCustomTitleField("m_sSocketName")]
class SCR_WindowsGeneratorSocketsPair
{
	[Attribute()]
	string m_sSocketName;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "", params: "et")]
	ResourceName m_sGlassPrefab;
};
