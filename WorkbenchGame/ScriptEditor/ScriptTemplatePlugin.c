enum EScriptTemplateType
{
	None,
	Entity,
	Component,
	WidgetComponent,
	ScriptInvoker,
	WorkbenchPlugin,
	WorldEditorTool,
};

[WorkbenchPluginAttribute("Fill from Template", "", "Ctrl+T", "", {"ScriptEditor"},"", 0xf1c9)]
class ScriptTemplatePlugin: WorkbenchPlugin
{
	[Attribute("0", UIWidgets.ComboBox, "Template type.", "", ParamEnumArray.FromEnum(EScriptTemplateType))]
	protected EScriptTemplateType m_ClassType;

	[Attribute("", UIWidgets.EditBox, "Full classname. When empty, file name will be used.")]
	protected string m_sClassName;

	[Attribute("", UIWidgets.EditBox, "Parent class name. When empty, default for given script type will be used.")]
	protected string m_sParentName;

	//--- Let's not expose this, could be overwhelming for the user
	//[Attribute("{51BB186E949A991B}Configs/Workbench/ScriptTemplatePlugin/ScriptTemplateConfig.conf", uiwidget: UIWidgets.ResourceNamePicker, params: "conf")]
	protected ResourceName m_Config = "{51BB186E949A991B}Configs/Workbench/ScriptTemplatePlugin/ScriptTemplateConfig.conf";

	override void Run()
	{
		ScriptEditor scriptEditor = Workbench.GetModule(ScriptEditor);
		string className;
		if (!scriptEditor.GetCurrentFile(className))
		{
			Print("Cannot insert script template, no file is opened!", LogLevel.WARNING);
			return;
		}

		if (Workbench.ScriptDialog("Fill from Template", "Insert a template of given class type to selected line in currently opened file.", this) && Workbench.OpenModule(ScriptEditor))
		{
			//--- Load config
			Resource configResource = Resource.Load(m_Config);
			if (!configResource.IsValid())
			{
				Debug.Error2(Type().ToString(), string.Format("Cannot load config '%1'!", m_Config));
				return;
			}

			ScriptTemplateConfig config = ScriptTemplateConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(configResource.GetResource().ToBaseContainer()));
			if (!config)
			{
				Debug.Error2(Type().ToString(), string.Format("Cannot load config '%1'!", m_Config));
				return;
			}

			//--- Get template
			string fullPath, defaultParentName;
			foreach (ScriptTemplateConfigEntry entry: config.m_Entries)
			{
				if (entry.m_Type == m_ClassType)
				{
					fullPath = entry.m_TemplateFile.GetPath();
					defaultParentName = entry.m_sDefaultParentName;
					break;
				}
			}

			//--- Get script class
			if (m_sClassName.IsEmpty())
			{
				className = FilePath.StripPath(className);
				className = FilePath.StripExtension(className);
			}
			else
			{
				className = m_sClassName;
			}

			//--- Get parent class
			string parentName;
			if (m_sParentName.IsEmpty())
			{
				parentName = defaultParentName;
			}
			else
			{
				parentName = m_sParentName;
			}
			if (parentName)
				parentName = " : " + parentName;

			//--- Open template file
			FileHandle file = FileIO.OpenFile(fullPath, FileMode.READ);
			if (!file)
			{
				Print(string.Format("Cannot open file '%1'!", fullPath), LogLevel.ERROR);
				return;
			}

			//--- Copy template into the script
			string line;
			int lineNumber = scriptEditor.GetCurrentLine();
			while (file.FGets(line ) > 0)
			{
				scriptEditor.InsertLine(string.Format(line, className, parentName), lineNumber);
				lineNumber++;
			}
			file.CloseFile();
		}
	}
	
	[ButtonAttribute("Fill", true)]
	bool ButtonConfirm()
	{
		return true;
	}
	
	[ButtonAttribute("Cancel")]
	bool ButtonCancel()
	{
		return false;
	}
};

[BaseContainerProps(configRoot: true)]
class ScriptTemplateConfig
{
	[Attribute()]
	ref array<ref ScriptTemplateConfigEntry> m_Entries;
};

[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EScriptTemplateType, "m_Type")]
class ScriptTemplateConfigEntry
{
	[Attribute(defvalue: "", uiwidget: UIWidgets.ComboBox, desc: "", enums: ParamEnumArray.FromEnum(EScriptTemplateType))]
	EScriptTemplateType m_Type;

	[Attribute("", uiwidget: UIWidgets.ResourceNamePicker, params: "txt")]
	ResourceName m_TemplateFile;

	[Attribute()]
	string m_sDefaultParentName;
};
