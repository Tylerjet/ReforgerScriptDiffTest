/**
	\brief Attribute to manually set a static title.
	@code
	[BaseContainerProps(), SCR_BaseContainerHandMadeTitleField("My Super Title")]
	class TestConfigClass
	{
	}
	@endcode
*/
class SCR_BaseContainerStaticTitleField : BaseContainerCustomTitle
{
	private string m_sCustomTitle;
	
	void SCR_BaseContainerStaticTitleField(string customTitle = "")
	{
		customTitle.Trim();
		m_sCustomTitle = customTitle;
	}
	
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		// if (!m_sCustomTitle)
		// {
		// 	return false;
		// }
		
		title = m_sCustomTitle;
		return true;
	}
};

/**
	\brief Attribute for setting any string property as custom title.
	@code
	[BaseContainerProps(), SCR_BaseContainerCustomTitleField("m_sDisplayName", "Title is %1")]
	class TestConfigClass
	{
		[Attribute()]
		private string m_sDisplayName;
	}
	@endcode
*/
class SCR_BaseContainerCustomTitleField : BaseContainerCustomTitle
{
	protected string m_sPropertyName;
	protected string m_sFormat;
	
	void SCR_BaseContainerCustomTitleField(string propertyName, string format = "%1")
	{
		m_sPropertyName = propertyName;
		m_sFormat = format;
	}

	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		if (!source.Get(m_sPropertyName, title))
		{
			return false;
		}

		title = string.Format(m_sFormat, title);
		return true;
	}
};

/**
	\brief Attribute to manually set a LOCALIZED (translated) title.
	@code
	[BaseContainerProps(), SCR_BaseContainerLocalizedTitleField("m_sTitle")]
	class TestConfigClass
	{
	}
	@endcode
*/
class SCR_BaseContainerLocalizedTitleField : BaseContainerCustomTitle
{
	protected string m_sPropertyName;
	protected string m_sFormat;
	
	void SCR_BaseContainerLocalizedTitleField(string propertyName, string format = "%1")
	{
		m_sPropertyName = propertyName;
		m_sFormat = format;
	}

	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		if (!source.Get(m_sPropertyName, title))
		{
			return false;
		}

		title = string.Format(m_sFormat, WidgetManager.Translate(title));
		return true;
	}
};

/**
	\brief Attribute for setting any enum property as custom title.
	@code
	[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditorMode, "m_Mode")]
	class TestConfigClass
	{
		[Attribute()]
		private EEditorMode m_Mode;
	}
	@endcode
*/
class SCR_BaseContainerCustomTitleEnum : BaseContainerCustomTitle
{
	private typename m_EnumType;
	private string m_PropertyName;
	private string m_sFormat;
	
	void SCR_BaseContainerCustomTitleEnum(typename enumType, string propertyName, string format = "%1")
	{
		m_EnumType = enumType;
		m_PropertyName = propertyName;
		m_sFormat = format;
	}
	
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		int enumValue;
		if (!source.Get(m_PropertyName, enumValue))
		{
			return false;
		}
		
		title = string.Format(m_sFormat, typename.EnumToString(m_EnumType, enumValue));
		return true;
	}
};
/**
	\brief Attribute for setting any flags enum property as custom title.
	@code
	[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditorMode, "m_Modes")]
	class TestConfigClass
	{
		[Attribute()]
		private EEditorMode m_Modes;
	}
	@endcode
*/
class SCR_BaseContainerCustomTitleFlags : BaseContainerCustomTitle
{
	private typename m_EnumType;
	private string m_PropertyName;
	private string m_sFormat;
	
	void SCR_BaseContainerCustomTitleFlags(typename enumType, string propertyName, string format = "%1")
	{
		m_EnumType = enumType;
		m_PropertyName = propertyName;
		m_sFormat = format;
	}
	
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		int enumValue;
		if (!source.Get(m_PropertyName, enumValue))
		{
			return false;
		}
		
		array<int> values = {};
		for (int i = 0, count = SCR_Enum.BitToIntArray(enumValue, values); i < count; i++)
		{
			if (i > 0)
			{
				title += " | ";
			}
			
			title += typename.EnumToString(m_EnumType, values[i]);
		}
		title = string.Format(m_sFormat, title);
		return true;
	}
};
/**
	\brief Attribute for setting any ResourceName path property as custom title.
	@code
	[BaseContainerProps(), SCR_BaseContainerCustomTitleResourceName("m_Path")]
	class TestConfigClass
	{
		[Attribute()]
		private ResourceName m_Path;
	}
	@endcode
*/
class SCR_BaseContainerCustomTitleResourceName : BaseContainerCustomTitle
{
	private string m_sPropertyName;
	private bool m_bFileNameOnly;
	private string m_sFormat;
	
	void SCR_BaseContainerCustomTitleResourceName(string propertyName, bool fileNameOnly = false, string format = "%1")
	{
		m_sPropertyName = propertyName;
		m_bFileNameOnly = fileNameOnly;
		m_sFormat = format;
	}
	
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		ResourceName path;
		if (!source.Get(m_sPropertyName, path))
		{
			return false;
		}
		title = path.GetPath();
		if (m_bFileNameOnly) title = string.Format(m_sFormat, FilePath.StripPath(title));
		return true;
	}
};

/**
	\brief Attribute for setting any object classname property as custom title.
	@code
	[BaseContainerProps(), SCR_BaseContainerCustomTitleResourceName("m_Variable")]
	class TestConfigClass
	{
		[Attribute()]
		private SCR_SomeClass m_Variable;
	}
	@endcode
*/
class SCR_BaseContainerCustomTitleObject : BaseContainerCustomTitle
{
	private string m_sPropertyName;
	private string m_sFormat;
	
	void SCR_BaseContainerCustomTitleObject(string propertyName, string format = "%1")
	{
		m_sPropertyName = propertyName;
		m_sFormat = format;
	}
	
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		BaseContainer object = source.GetObject(m_sPropertyName);
		if (!object)
			return false;
		
		title = string.Format(m_sFormat, object.GetClassName());
		return true;
	}
};

/**
	\brief Attribute for setting a custom string as title
	@code
	[BaseContainerProps(), BaseContainerCustomStringTitleField("Title")]
	class TestConfigClass
	{

	}
	@endcode
*/
class BaseContainerCustomStringTitleField : BaseContainerCustomTitle
{
	string m_Title;
	
	void BaseContainerCustomStringTitleField(string title)
	{
		m_Title = title;
	}
	
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		title = m_Title;
		
		if (!m_Title.IsEmpty())		
			return true;
		else
			return false;
	}
};

/**
	\brief Attribute for setting UIInfo's name property as custom title.
	@code
	[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
	class TestConfigClass
	{
		[Attribute()]
		private SCR_UIInfo m_Info;
	}
	@endcode
*/
class SCR_BaseContainerCustomTitleUIInfo : BaseContainerCustomTitle
{
	private string m_sPropertyName;
	private string m_sFormat;
	
	void SCR_BaseContainerCustomTitleUIInfo(string propertyName, string format = "%1")
	{
		m_sPropertyName = propertyName;
		m_sFormat = format;
	}
	
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		BaseContainer info = source.GetObject(m_sPropertyName);
		if (!info || !info.Get("Name", title))
		{
			return false;
		}

		title = string.Format(m_sFormat, title);
		return true;
	}
};