
class ParamEnum: Managed
{
	string m_Key;
	string m_Value;
	string m_Desc;

	void ParamEnum(string key, string value, string desc = "")
	{
		m_Key = key;
		m_Value = value;
		m_Desc = desc;
	}
}

class ParamEnumArray: array<ref ParamEnum>
{
	static ParamEnumArray FromEnum(typename e)
	{
		ParamEnumArray params = new ParamEnumArray();
		int cnt = e.GetVariableCount();
		int val;

		for (int i = 0; i < cnt; i++)
		{
			if (e.GetVariableType(i) == int && e.GetVariableValue(NULL, i, val))
			{
				params.Insert(new ParamEnum(e.GetVariableName(i), val.ToString()));
			}
		}

		return params;
	}
}

// -------------------------------------------------------------------------
class UIWidgets
{
	//! Widget is set automatically by script variable type.
	static const string	Auto = "auto";

	//! Property is hidden when None is used. Same as UIWidgets.None.
	static const string	Hidden = "";

	//! Property is hidden when None is used.
	static const string	None = "";

	/*!
	Color wheel picker. m_strParams has no usage. Type must be enf::DVT_COLOR
	or enf::DVT_VECTOR3.
	*/
	static const string	ColorPicker = "colorPicker";

	/*!
	Registered resource picker.

	m_strParams contains file extensions separated by whitespace. For folders
	there are reserved virtual extensions "folders" and "unregFolders" that
	can be mixed with other file extensions. "unregFolders" allows to choose
	also unregistered folders which is good when property will not be saved to
	game data (scripted tools for example).

	You can also specify param that will filter config files of specific
	class. Use "class=MyClassName" param for that.

	Type must be enf::DVT_RESOURCENAME.
	*/
	static const string	ResourceNamePicker = "resourcePickerSimple";

	/*!
	Registered resource picker with thumbnail support.

	m_strParams contains file extensions separated by whitespace. For folders
	there are reserved virtual extensions "folders" and "unregFolders" that
	can be mixed with other file extensions. "unregFolders" allows to choose
	also unregistered folders what is good when property will not be saved to
	game data (scripted tools for example)

	You can also specify param that will filter config files of specific
	class. Use "class=MyClassName" param for that.

	Type must be enf::DVT_RESOURCENAME
	*/
	static const string	ResourcePickerThumbnail = "resourcePickerThumbnail";

	/*!
	File name picker. m_strParams contains file extensions separated by
	whitespace. For folders there are reserved virtual extensions "folders".

	Returned path is exact path (in format "$filesystemName:path"). For
	absolute path use FileNameFormat=absolute e.g.
	"emat FileNameFormat=absolute".

	Only for editor stuff. All game paths must use ResourceName +
	ResourcePicker.

	Type must be enf::DVT_STRING.
	*/
	static const string	FileNamePicker = "fileNamePicker";

	/*!
	Array of name-ResourceName pairs. Uses resource picker. m_strParams
	contains file extensions separated by whitespace

	Type must be enf::DVT_OBJECTARRAY with objects of type enf::BaseContainer
	of MaterialAssignClass class.
	*/
	static const string	ResourceAssignArray = "resourceAssignArray";

	/*!
	Date picker. m_strParams has no usage.
	Type must be enf::DVT_INTEGER. Format is provided by enf::DateTimeAsInt.
	*/
	static const string	Date = "date";

	/*!
	Graph, represented either by enf::DVT_VECTOR2ARRAY or
	enf::DVT_VECTOR4ARRAY, where the first value is fraction, and rest is
	value. m_strParams has no usage.
	*/
	static const string	Graph = "graph";

	//! Font picker. m_strParams has no usage. Type must be enf::DVT_STRING.
	static const string	Font = "font";

	/*!
	SpinBox. m_strParams may contain min and max value separated by space.
	Type must be enf::DVT_INTEGER or enf::DVT_SCALAR.
	*/
	static const string	SpinBox = "spinbox";

	/*!
	Combobox. m_strParams has usage for numeric values, where there may be min
	and max value separated by space. Enums may contain selectable values.
	Type must be enf::DVT_STRING, or enf::DVT_INTEGER or enf::DVT_SCALAR.
	*/
	static const string	ComboBox = "combobox";

	static const string	EditComboBox = "editcombobox";

	static const string	SearchComboBox = "searchcombobox";

	/*!
	Editbox for picking up IDs from localization table. Type must be
	enf::DVT_STRING.
	*/
	static const string	LocaleEditBox = "localeeditbox";

	/*!
	Editbox. m_strParams has usage for numeric values, where there may be min
	and max value separated by space. Type must be enf::DVT_STRING, or
	enf::DVT_INTEGER or enf::DVT_SCALAR.
	*/
	static const string	EditBox = "editbox";

	//! Checkbox. m_strParams has no usage. Type must be enf::DVT_BOOLEAN.
	static const string	CheckBox = "checkbox";

	/*!
	Slider. m_strParams may contain min, max and step values separated by
	space. Type must be enf::DVT_INTEGER of enf::DVT_SCALAR.
	*/
	static const string	Slider = "slider";

	/*!
	Array of bit flags represented by checkboxes. m_strParams has no usage.
	Enums must contain bit definitions. Type must be enf::DVT_INTEGER.
	*/
	static const string	Flags = "flags";

	/*!
	Generic ".." button without default functionality. It's for custom GUI
	implementation only. Type must be enf::DVT_STRING or enf::DVT_INTEGER or
	enf::DVT_SCALAR.
	*/
	static const string	Button = "button";

	/*!
	Small script preview and button which opens ScriptEditor to edit script
	property. Type must be enf::DVT_STRING.
	*/
	static const string	Script = "script";

	/*!
	Edit box with a generic ".." button which is without default
	functionality. It's for custom GUI implementation only. Type must be
	enf::DVT_STRING or enf::DVT_INTEGER or enf::DVT_SCALAR.
	*/
	static const string	EditBoxWithButton = "editboxWithButton";

	/*!
	Edit box with a generic ".." button which is used to open separate window
	with text editor. Type must be enf::DVT_STRING.
	*/
	static const string	EditBoxMultiline = "editboxMultiline";

	//! Dedicated to edit LOD factors. Type must be enf::DVT_SCALAR_ARRAY.
	static const string	LODFactorsEdit = "lodFactorsEdit";

	/*!
	Single object or an array of objects (depends on variable definition).
	Type must be enf::DVT_OBJECT or enf::DVT_OBJECTARRAY with objects. Use
	"noDetails" parameter with it to remove extra information from GUI
	(ClassName + goto Prefab button).
	*/
	static const string	Object = "object";

	//! 3-element vector type in X/Y/Z form. Type must be enf::DVT_VECTOR3.
	static const string Coords = "coords";

	//! 2-element vector type in min-max form. Type must be enf::DVT_VECTOR2.
	static const string Range = "range";

	/*!
	Array of CallbackClass objects. Type must be enf::DVT_OBJECTARRAY with
	objects inherited from Callback class.
	*/
	static const string	Callback = "callback";

	/*!
	Single object or an array of objects (depends on variable definition).
	Type must be enf::DVT_OBJECT or enf::DVT_OBJECTARRAY with objects. When
	type is enf::DVT_OBJECTARRAY, there is no GUI of the array variable
	visible, but GUI of the array members will be placed on the top level of
	hierarchy. All ancestors and descendant objects (depends by context) will
	be editable.
	*/
	static const string	TopLevelObject = "topLevelObject";

	/*!
	Editor for splines which opens a dialog where the curve can be edited.
	Type must be CurvePoints or CurvePointsMulti. rangescale in attribute may
	specify "xScale yScale xOffset yOffset".
	*/
	static const string GraphDialog = "graphDialog";

	/*!
	Editor which displays min and max bounds, also allows using the
	BoundingVolumeEditor. Type must be enf::DVT_OBJECT.
	*/
	static const string BoundingVolumeEditor = "boundingVolume";
}



enum NamingConvention
{
	NC_CAN_HAVE_NAME,
	NC_MUST_HAVE_NAME,
	NC_MUST_HAVE_GUID
}

/*!
\defgroup Attributes Attributes
\{
*/

class Attribute
{
	string m_DefValue;
	string m_UiWidget; //!< use values from UIWidgets
	/*!
	Properties for UI in editor, depends on property type and used UI Widget.
	- for numeric types use format:"MIN_VALUE MAX_VALUE STEP" eg. "1 100 0.5", for min/max values can be used also inf / -inf as infinite
	- for file dialog use format "ext1;ext2"
	*/
	string m_Params;
	string m_Desc;
	string m_Category;
	int m_Precision;

	/*!
	Only ints and floats are currently supported. Array can be defined this
	way:
	\code
		{ ParamEnum("Choice 1", "1"), ParamEnum("Choicen 2", "2") }
	\endcode
	*/
	ref ParamEnumArray m_Enums;

	void Attribute(string defvalue = "", string uiwidget = "auto"/*use UIWidgets*/, string desc = "", string params = "", ParamEnumArray enums = NULL, string category = "", int precision = 3)
	{
		m_DefValue = defvalue;
		m_UiWidget = uiwidget;
		m_Params = params;
		m_Desc = desc;
		m_Category = category;
		m_Enums = enums;
		m_Precision = precision;
	}
}

//! Default icon for all components written in script that don't inherit ScriptComponent
const string HYBRID_COMPONENT_ICON = ":/Workbench/WorldEditor/Entity/componentHybrid.png";

class CommonEditorProps
{
	string m_Category; //<! Typically a "folder/item" path for placing the type into a tree-view, eg. "StaticEntities/Walls".
	string m_Description; //<! Typically some description of type usage.

	string m_Icon; //<! Typically a path to icon file for better visualization in a tree-view.
	bool m_Visible; //<! Is the entity visualizer visible when not selected.
	bool m_Insertable; //<! Is the type available for insertion from a tree-view
	bool m_ConfigRoot; //<! Whether the type can be used as a root object of a config file (.conf).

	void CommonEditorProps(string category = "", string description = "", string color = "255 0 0 255", bool visible = true, bool insertable = true, bool configRoot = false, string icon = "")
	{
		m_Category = category;
		m_Description = description;
		m_Visible = visible;
		m_Insertable = insertable;
		m_ConfigRoot = configRoot;
		m_Icon = icon;
	}
}

class ComponentEditorProps: CommonEditorProps
{
}

class EntityEditorProps: CommonEditorProps
{
	vector m_SizeMin; //!< Minimum dimensions of an entity visualizer shape
	vector m_SizeMax; //!< Maximum dimensions of an entity visualizer shape
	string m_Style; //!< Can be: "none", "box", "sphere", "cylinder", "capsule", "pyramid", "diamond". Anything else is custom. Empty is none.
	string m_Color; //<! Color of an entity visualizer shape outline
	string m_Color2; //<! Color of an entity visualizer shape
	bool m_DynamicBox; //<! Is the entity visualizer using custom dimensions (provided by _WB_GetBoundBox)

	void EntityEditorProps(string category = "", string description = "", string color = "255 0 0 255", bool visible = true, bool insertable = true, bool configRoot = false, string icon = "", string style = "box", vector sizeMin = "-0.25 -0.25 -0.25", vector sizeMax = "0.25 0.25 0.25", string color2 = "0 0 0 0", bool dynamicBox = false)
	{
		m_Style = style;
		m_SizeMin = sizeMin;
		m_SizeMax = sizeMax;
		m_Color = color;
		m_Color2 = color2;
		m_DynamicBox = dynamicBox;
	}
}

// -------------------------------------------------------------------------
/*!
An Attribute for BaseContainer.

\code
	[BaseContainerProps()]
	class MyObject
	{
		[Attribute("true", UIWidgets.CheckBox)]
		bool Prop1;

		[Attribute("7", UIWidgets.EditBox)]
		int Prop2;

		[Attribute("hello", UIWidgets.EditBox)]
		string Prop3;
	}

	[BaseContainerProps()]
	class MyObjectSpecial: MyObject
	{
		[Attribute("1.2", UIWidgets.EditBox)]
		float Prop4;

		[Attribute("", UIWidgets.Object)]
		ref MyObject Prop5;
	}

	// ...
	class TestCube : GenericEntity
	{
		[Attribute("", UIWidgets.Object)]
		ref MyObject m_singleObject;

		[Attribute("", UIWidgets.Object)]
		ref array<ref MyObject> m_arrayOfObjects;
	};
\endcode
*/
class BaseContainerProps: CommonEditorProps
{
	NamingConvention m_NamingConvention;

	void BaseContainerProps(string category = "", string description = "", string color = "255 0 0 255", bool visible = true, bool insertable = true, bool configRoot = false, string icon = "", NamingConvention namingConvention = NamingConvention.NC_MUST_HAVE_GUID)
	{
		m_NamingConvention = namingConvention;
	}
}

/*!
Base class for attribute for setting custom title in property grid
(Workbench). Works only on classes defined by BaseContainerProps attribute.
Inherit this attribute to set your own custom title.
\code
	class MyCustomTitle: BaseContainerCustomTitle
	{
		override bool _WB_GetCustomTitle(BaseContainer source, out string title)
		{
			title = "My Title";
			return true;
		}
	}

	[BaseContainerProps(), MyCustomTitle()]
	class TestConfigClass
	{
		[Attribute()]
		string m_ID;
	};
\endcode
*/
class BaseContainerCustomTitle
{
	bool _WB_GetCustomTitle(BaseContainer source, out string title);
}

/*!
Attribute for setting any string property as custom title.
\code
	[BaseContainerProps(), BaseContainerCustomTitleField("m_ID")]
	class TestConfigClass
	{
		[Attribute()]
		string m_ID;
	};
\endcode
*/
class BaseContainerCustomTitleField: BaseContainerCustomTitle
{
	string m_PropertyName;

	void BaseContainerCustomTitleField(string propertyName)
	{
		m_PropertyName = propertyName;
	}

	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		return source.Get(m_PropertyName, title);
	}
}

/*!
Attribute for callback system. Method with this attribute will be listed in callback list.
\code
	class MyManager
	{
		[CallbackMethod()]
		void DoSomething(string p1, int p2);

		[CallbackMethod()]
		void DoSomethingElse(bool p1);
	}

	[CallbackContext()]
	MyManager GetMyManager();

	// editor will offer "GetMyManager.DoSomething" and "GetMyManager.DoSomethingElse" in callback properties
\endcode
*/
class CallbackMethod
{
}

/*!
Attribute for callback system. Method - getter with this attribute is returning instance(context) to callbacks on.
\code
	class MyManager
	{
		[CallbackMethod()]
		void DoSomething(string p1, int p2);

		[CallbackMethod()]
		void DoSomethingElse(bool p1);
	}

	[CallbackContext()]
	MyManager GetMyManager();

	// editor will offer "GetMyManager.DoSomething" and "GetMyManager.DoSomethingElse" in callback properties
\endcode
*/
class CallbackContext
{
}

/*!
\}
*/
