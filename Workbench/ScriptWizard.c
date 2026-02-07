//------------------------------------------------------------------------------------------------
enum WizardScriptType
{
	Object = 0,
	Entity = 1,
	Component = 2,
};

//------------------------------------------------------------------------------------------------
[WorkbenchPluginAttribute("Script Wizard", "Wizard for generating script files the easy way!", "CTRL+N", "", {"ScriptEditor"},"",0xf1c9)]
class ScriptWizard : WorkbenchPlugin
{
	// Generic options
	protected const string SPLITTER = "//------------------------------------------------------------------------------------------------";
	protected const string TAB = "\t";
	protected const string SCRIPT_SCR_PREFIX = "SCR_";
	protected const string PATH_ENTITIES = "scripts/Game/entities/";
	protected const string PATH_COMPONENTS = "scripts/Game/components/";
	protected const string PATH_OBJECTS = "scripts/Game/";
	protected const string PATH_SCRIPTS_ROOT = "scripts/Game/";
	protected const string DEFAULT_PARENT_ENTITY = "GenericEntity";
	protected const string DEFAULT_PARENT_COMPONENT = "ScriptComponent";
	protected const string DEFAULT_CATEGORY = "GameScripted/ScriptWizard";
	protected const string DEFAULT_DESCRIPTION = "ScriptWizard generated script file.";

	// Documentation
	protected const string DOXY_BEGIN = "/*!";
	protected const string DOXY_END = "*/";

	// Init, Frame
	protected const string FUNCTION_FRAME = "override void EOnFrame(IEntity owner, float timeSlice)";
	protected const string FUNCTION_INIT = "override void EOnInit(IEntity owner)";
	protected const string FUNCTION_POSTINIT = "override void OnPostInit(IEntity owner)";

	// Constructors
	protected const string FUNCTION_OBJECT_CONSTRUCTOR = "void %1()";
	protected const string FUNCTION_COMPONENT_CONSTRUCTOR = "void %1(IEntityComponentSource src, IEntity ent, IEntity parent)";
	protected const string FUNCTION_ENTITY_CONSTRUCTOR = "void %1(IEntitySource src, IEntity parent)";

	// Entity EventMask/Flags
	protected const string FUNCTION_ENTITY_EVENTMASK = "SetEventMask(EntityEvent.INIT | EntityEvent.FRAME);";
	protected const string FUNCTION_ENTITY_EVENTFLAGS = "SetFlags(EntityFlags.ACTIVE, true);";

	// Component EventMask/Flags
	protected const string FUNCTION_COMPONENT_EVENTMASK = "SetEventMask(owner, EntityEvent.INIT | EntityEvent.FRAME);";
	protected const string FUNCTION_COMPONENT_EVENTFLAGS = "owner.SetFlags(EntityFlags.ACTIVE, true);";

	// Destructor
	protected const string FUNCTION_DESTRUCTOR = "void ~%1()";

	// Script file extension
	protected const string SCRIPT_EXTENSION = ".c";

	// Line 0, 1...
	protected const string SCRIPT_ATTRIBUTE = "[EntityEditorProps(category: \"%1\", description: \"%2\")]";
	protected const string SCRIPT_CLASSCLASS = "class %1Class : %2Class";
	protected const string SCRIPT_CLASSCLASS_NOINH = "class %1Class";
	protected const string SCRIPT_CLASSINH = "class %1 : %2";
	protected const string SCRIPT_CLASS_NOINH = "class %1";

	// TODO: WizardScriptType instead of ParamEnum
	[Attribute("0", UIWidgets.ComboBox, "Type of script. Object = empty class, Entity inherits from GenericEntity by default, Component inherits from ScriptComponent by default.", "", ParamEnumArray.FromEnum(WizardScriptType))]
	protected int ScriptType;

	[Attribute("MyClass", UIWidgets.EditBox, "Name of class to create (No extension, no prefixes! Example: CommonEntity, SuperComponent, MyFancyThing, ...)" )]
	protected string ScriptName;

	[Attribute("", UIWidgets.EditBox, "Parent class of this script. If empty, automatically selects by specified class type. (Valid examples: GenericEntity, ScriptComponent, ...)" )]
	protected string ScriptParent;

	[Attribute("", UIWidgets.EditBox, "If specified, script file will be created in scripts/Game/<ScriptPath/OfYourDesire/>. Leave empty for default placement.", "")]
	protected string ScriptPath;

	[Attribute("$ArmaReforger:", UIWidgets.EditBox, "File system in which new script will be created.", "")]
	protected string ScriptAddon;

	[Attribute("1", UIWidgets.CheckBox, "Should the \'_SCR\' prefix be used? If true, it's automatically added to the name.", "")]
	protected bool UseSCRPrefix;

	[Attribute("1", UIWidgets.CheckBox, "Should constructor, destructor, EOnInit and EOnFrame be automatically generated? Not valid for ScriptType of value Object.", "")]
	protected bool GenerateFunctions;

	[Attribute("0", UIWidgets.CheckBox, "Should this class be set as active by default? Only works when GenerateFunctions is set to true.", "")]
	protected bool GenerateActiveFlag;

	protected ScriptEditor scriptEditor;


	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		scriptEditor = Workbench.GetModule(ScriptEditor);
		if (Workbench.ScriptDialog("Script Creation Wizard", "Please specify the type and properties of the script you want to create.\nYou can hover your mouse over each property to see its tooltip.\n\nDo not forget to apply value changes with ENTER.", this) && Workbench.OpenModule(ScriptEditor))
		{
			string name = ScriptName;
			name.Replace(" ", "");

			// Add SCR_ before name if enabled
			if (UseSCRPrefix)
				name = SCRIPT_SCR_PREFIX + name;

			string path = string.Empty;
			switch (ScriptType)
			{
				case WizardScriptType.Entity:
					path = PATH_ENTITIES;
					if (ScriptParent == string.Empty)
						ScriptParent = DEFAULT_PARENT_ENTITY;

					break;

				case WizardScriptType.Component:
					path = PATH_COMPONENTS;
					if (ScriptParent == string.Empty)
						ScriptParent = DEFAULT_PARENT_COMPONENT;
					break;

				case WizardScriptType.Object:
					if (path == string.Empty)
						path = PATH_OBJECTS;
					break;
			}

			if (ScriptPath != string.Empty)
			{
				path = PATH_SCRIPTS_ROOT + ScriptPath;
				array<string> folders = new array<string>();
				ScriptPath.Split("/", folders, true);

				int foldersCount = folders.Count();
				string currentFolderPath = ScriptAddon + PATH_SCRIPTS_ROOT;
				for (int i = 0; i < foldersCount; i++)
				{
					currentFolderPath += folders[i];

					if (!FileIO.FileExist(currentFolderPath))
						FileIO.MakeDirectory(currentFolderPath);

					if (!currentFolderPath.EndsWith("/"))
						currentFolderPath += "/";
				}

			}

			if (!path.EndsWith("/"))
				path += "/";

			path = string.Format("%1%2%3%4", ScriptAddon, path, name, SCRIPT_EXTENSION);

			// Already exists
			if (FileIO.FileExist(path))
			{
				Workbench.Dialog("Script Creation ERROR!", "A file with specified name already exists in target directory.");
				ResetValues();
				return;
			}
			else
			{
				if (WriteScriptTemplate(path, name, ScriptParent, DEFAULT_CATEGORY, DEFAULT_DESCRIPTION, ScriptType) == -1)
				{
					Workbench.Dialog("Script Creation ERROR!", "FileHandle error: File at specified path couldn't be created.");
					return;
				}
				scriptEditor.SetOpenedResource(path);
				ResetValues();
			}


		}
	}

	//------------------------------------------------------------------------------------------------
	int WriteScriptTemplate(string path, string className, string classParent, string classCategory, string classDescription, WizardScriptType type)
	{
		FileHandle file = FileIO.OpenFile(path, FileMode.WRITE);

		if (!file)
			return -1;

		// Only entites and components need the ClassClass
		if (type != WizardScriptType.Object)
		{
			// [EditorAttribute("box"....
			string attribute = string.Format(SCRIPT_ATTRIBUTE, classCategory, classDescription);
			file.WriteLine(attribute);

			// class MyClass : MyClassClass
			string classClassLine;
			if (!classParent.IsEmpty())
				classClassLine = string.Format(SCRIPT_CLASSCLASS, className, classParent);
			else
				classClassLine = string.Format(SCRIPT_CLASSCLASS_NOINH, className);

			file.WriteLine(classClassLine);
			// {}
			file.WriteLine("{");
			file.WriteLine(TAB + "// prefab properties here");
			file.WriteLine("};");

			// Empty line
			file.WriteLine("");
		}

		// Splitter
		file.WriteLine(SPLITTER);

		// Brief description
		file.WriteLine(DOXY_BEGIN);
		file.WriteLine("\tClass generated via ScriptWizard.");
		file.WriteLine(DOXY_END);

		// SCR_MyEntity : GenericEntity
		string classInheritance = "";
		if (!classParent.IsEmpty())
			classInheritance = string.Format(SCRIPT_CLASSINH, className, classParent);
		else
			classInheritance = string.Format(SCRIPT_CLASS_NOINH, className);
		file.WriteLine(classInheritance);

		// {}
		file.WriteLine("{");

		// Constructor, destructor, init, frame
		if (GenerateFunctions)
		{
			// empty line
			file.WriteLine("");

			if (type != WizardScriptType.Object)
			{
				file.WriteLine(TAB + SPLITTER);
				// frame function
				string frame = TAB + FUNCTION_FRAME;
				file.WriteLine(frame);

				// {}
				file.WriteLine(TAB + "{");
				file.WriteLine(TAB + "}");

				// postInit function in components
				if (ScriptType == WizardScriptType.Component)
				{
					// empty line
					file.WriteLine("");
					file.WriteLine(TAB + SPLITTER);

					// method header
					string postInit = TAB + FUNCTION_POSTINIT;
					file.WriteLine(postInit);
					//{
					file.WriteLine(TAB + "{");

					// SetEventMask and SetFlags
					file.WriteLine(TAB + TAB + FUNCTION_COMPONENT_EVENTMASK);
					if (GenerateActiveFlag)
						file.WriteLine(TAB + TAB + FUNCTION_COMPONENT_EVENTFLAGS);

					//}
					file.WriteLine(TAB + "}");
				}

				// empty line
				file.WriteLine("");
				file.WriteLine(TAB + SPLITTER);
				// init function
				string init = TAB + FUNCTION_INIT;
				file.WriteLine(init);

				// {}
				file.WriteLine(TAB + "{");
				file.WriteLine(TAB + "}");

			}

			switch (ScriptType)
			{
				// Entity constructor
				case WizardScriptType.Entity:

					// empty line
					file.WriteLine("");
					file.WriteLine(TAB + SPLITTER);
					// constructor
					string constructor = string.Format(TAB + FUNCTION_ENTITY_CONSTRUCTOR, className);
					file.WriteLine(constructor);

					// {}
					file.WriteLine(TAB + "{");

					// SetEventMask and SetFlags
					file.WriteLine(TAB + TAB + FUNCTION_ENTITY_EVENTMASK);
					if (GenerateActiveFlag)
						file.WriteLine(TAB + TAB + FUNCTION_ENTITY_EVENTFLAGS);

					file.WriteLine(TAB + "}");

					break;

				// Component constructor
				case WizardScriptType.Component:

					// empty line
					file.WriteLine("");
					file.WriteLine(TAB + SPLITTER);
					// constructor
					string constructor = string.Format(TAB + FUNCTION_COMPONENT_CONSTRUCTOR, className);
					file.WriteLine(constructor);

					// {}
					file.WriteLine(TAB + "{");
					file.WriteLine(TAB + "}");

					break;

				case WizardScriptType.Object:

					// empty line
					file.WriteLine(TAB + SPLITTER);
					// constructor
					string constructor = string.Format(TAB + FUNCTION_OBJECT_CONSTRUCTOR, className);
					file.WriteLine(constructor);

					// {}
					file.WriteLine(TAB + "{");
					file.WriteLine(TAB + "}");

					break;

				default:
					break;
			}

			// empty line
			file.WriteLine("");
			file.WriteLine(TAB + SPLITTER);
			// destructor
			string destructor = string.Format(TAB + FUNCTION_DESTRUCTOR, className);
			file.WriteLine(destructor);

			// {}
			file.WriteLine(TAB + "{");
			file.WriteLine(TAB + "}");
		}

		// Empty line
		file.WriteLine("");

		file.WriteLine("};");

		file.Close();

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	protected void ResetValues()
	{
		ScriptType = 0;
		ScriptName = "MyClass";
		ScriptParent = string.Empty;
		ScriptPath = "";
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("OK")]
	void OkButton()
	{
	}
};
