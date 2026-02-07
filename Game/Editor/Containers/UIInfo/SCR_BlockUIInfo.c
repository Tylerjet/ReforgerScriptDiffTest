[BaseContainerProps(), BaseContainerCustomTitleField("Name")]
class SCR_BlockUIInfo: SCR_UIInfo
{
	[Attribute()]
	protected ref array<ref SCR_SubBlockUIName> m_aDescriptionBlocks;
	
	override bool HasDescription()
	{
		return super.HasDescription() || !m_aDescriptionBlocks.IsEmpty();
	}
	override LocalizedString GetDescription()
	{
		LocalizedString description = super.GetDescription();
		if (m_aDescriptionBlocks)
		{
			for (int i, count = m_aDescriptionBlocks.Count(); i < count; i++)
			{
				if (m_aDescriptionBlocks[i].HasName())
				{
					if (m_aDescriptionBlocks[i].IsInline() || description.IsEmpty())
						description += /*" " +*/ m_aDescriptionBlocks[i].GetName();
					else
						description += "<br/>" + m_aDescriptionBlocks[i].GetName();
				}
			}
		}
		return description;
	}
};
[BaseContainerProps(), BaseContainerCustomTitleField("Name")]
class SCR_SubBlockUIName: SCR_UIName
{
	[Attribute()]
	protected bool m_bInline;
	
	bool IsInline()
	{
		return m_bInline;
	}
};
[BaseContainerProps(), BaseContainerCustomTitleField("Name")]
class SCR_BulletPointBlockUIName: SCR_SubBlockUIName
{	
	override LocalizedString GetName()
	{
		return "<image set='{73BAE6966DBC17CB}UI/Imagesets/Hint/Hint.imageset' name='Bullet' scale='1'/>" + super.GetName(); //--- ToDo: Don't hard-code
	}
};
[BaseContainerProps(), BaseContainerCustomTitleField("Name")]
class SCR_TipBlockUIName: SCR_SubBlockUIName
{
	override LocalizedString GetName()
	{
		return "<color rgba='255,255,255,160'><image set='{73BAE6966DBC17CB}UI/Imagesets/Hint/Hint.imageset' name='Tip' scale='1'/> <i>" + super.GetName() + "</i></color>"; //--- ToDo: Don't hard-code
	}
};
[BaseContainerProps(), BaseContainerCustomTitleField("Name")]
class SCR_DeviceBlockUIName: SCR_SubBlockUIName
{
	[Attribute("0", uiwidget: UIWidgets.ComboBox, enums: { ParamEnum("Any", "0"), ParamEnum("Mouse & Keyboard", "1"), ParamEnum("Gamepad", "2")})]
	protected int m_iDevice;
	
	override bool HasName()
	{
		bool isInputMatch = true;
		switch (m_iDevice)
		{
			case 1: isInputMatch = GetGame().GetInputManager().IsUsingMouseAndKeyboard(); break;
			case 2: isInputMatch = !GetGame().GetInputManager().IsUsingMouseAndKeyboard(); break;
		}
		return isInputMatch;
	}
};
[BaseContainerProps(), BaseContainerCustomTitleField("Name")]
class SCR_ActionBlockUIName: SCR_DeviceBlockUIName
{
	[Attribute()]
	protected string m_sActionName;
	
	protected const string m_sDelimiter = " - ";
	
	override LocalizedString GetName()
	{
		//--- ToDo: Turn color conversion to general function
		Color sRGBA = new Color(UIColors.CONTRAST_COLOR.R(), UIColors.CONTRAST_COLOR.G(), UIColors.CONTRAST_COLOR.B(), UIColors.CONTRAST_COLOR.A());
		
		//--- Convert to sRGBA format for rich text
		sRGBA.LinearToSRGB();
		
		//--- Convert to ints, no fractions allowed in rich text
		int colorR = sRGBA.R() * 255;
		int colorG = sRGBA.G() * 255;
		int colorB = sRGBA.B() * 255;
		int colorA = sRGBA.A() * 255;
		
		return string.Format("<color rgba='%2,%3,%4,%5'><action name='%1' scale='1.25'/></color>", m_sActionName, colorR, colorG, colorB, colorA) + m_sDelimiter + super.GetName();
	}
};
[BaseContainerProps(), BaseContainerCustomTitleField("Name")]
class SCR_KeyBlockUIName: SCR_DeviceBlockUIName
{
	[Attribute()]
	protected ref array<ref SCR_KeyBlockEntry> m_aKeys;
	
	protected const LocalizedString m_sHoldModifier = "#ENF-HoldModifier";
	protected const LocalizedString m_sDoubleModifier = "#ENF-DoubleModifier";
	protected const LocalizedString m_sComboModifier = "#ENF-ComboModifier";
	protected const LocalizedString m_sEllipsisModifier = "..."; //--- ToDo: Localize
	protected const LocalizedString m_sDelimiter = " - "; //--- ToDo: Localize
	
	override LocalizedString GetName()
	{
		//--- ToDo: Turn color conversion to general function
		Color sRGBA = new Color(UIColors.CONTRAST_COLOR.R(), UIColors.CONTRAST_COLOR.G(), UIColors.CONTRAST_COLOR.B(), UIColors.CONTRAST_COLOR.A());
		
		//--- Convert to sRGBA format for rich text
		sRGBA.LinearToSRGB();
		
		//--- Convert to ints, no fractions allowed in rich text
		int colorR = sRGBA.R() * 255;
		int colorG = sRGBA.G() * 255;
		int colorB = sRGBA.B() * 255;
		int colorA = sRGBA.A() * 255;
		
		LocalizedString keys;
		foreach (int i, SCR_KeyBlockEntry entry: m_aKeys)
		{
			if (i > 0)
			{
				switch (entry.m_iDelimiter)
				{
					case 0: keys += m_sComboModifier; break;
					case 1: keys += m_sEllipsisModifier; break;
				}
			}
			
			switch (entry.m_iModifier)
			{
				case 1: keys += m_sHoldModifier; break;
				case 2: keys += m_sDoubleModifier; break;
			}
			keys += string.Format("<key name='%1' scale='1.25'/>", entry.m_sKey);
		}
		
		return string.Format("<color rgba='%2,%3,%4,%5'>%1</color>", keys, colorR, colorG, colorB, colorA) + m_sDelimiter + super.GetName();
	}
};
[BaseContainerProps(), BaseContainerCustomTitleField("m_sKey")]
class SCR_KeyBlockEntry
{
	[Attribute()]
	string m_sKey;
	
	[Attribute("0", uiwidget: UIWidgets.ComboBox, enums: { ParamEnum(" + ", "0"), ParamEnum("...", "1")})]
	int m_iDelimiter;
	
	[Attribute("0", uiwidget: UIWidgets.ComboBox, enums: { ParamEnum("None", "0"), ParamEnum("Hold", "1"), ParamEnum("Double", "2")})]
	int m_iModifier;
};

[BaseContainerProps(), BaseContainerCustomTitleField("Name")]
class SCR_SimpleTagBlockUIName: SCR_SubBlockUIName
{
	[Attribute("0", uiwidget: UIWidgets.Flags, enums: { ParamEnum("<p>", "1"), ParamEnum("<b>", "2"), ParamEnum("<i>", "4"), ParamEnum("<h1>", "8"), ParamEnum("<h2>", "16") })]
	protected int m_iTags;
	
	override LocalizedString GetName()
	{
		LocalizedString text;
		
		if (m_iTags & 1) text += "<p>";
		if (m_iTags & 2) text += "<b>";
		if (m_iTags & 4) text += "<i>";
		if (m_iTags & 8) text += "<h1>";
		if (m_iTags & 16) text += "<h2>";
		
		text += super.GetName();
		
		if (m_iTags & 1) text += "</p>";
		if (m_iTags & 2) text += "</b>";
		if (m_iTags & 4) text += "</i>";
		if (m_iTags & 8) text += "</h1>";
		if (m_iTags & 16) text += "</h2>";
		
		return text;
	}
	override bool IsInline()
	{
		return super.IsInline() || m_iTags & 1; //--- <p> is always inline, we don't want extra <br/> there
	}
};
/*
[BaseContainerProps(), BaseContainerCustomTitleField("Name")]
class SCR_TagBlockUIName: SCR_SubBlockUIName
{
	[Attribute()]
	protected ref array<ref SCR_BaseTagBlockEntry> m_aTags;
	
	override LocalizedString GetName()
	{
		LocalizedString text;
		int tagCount = m_aTags.Count();
		for (int i; i < tagCount; i++)
		{
			text += m_aTags[i].GetStart();
		}
		
		text += super.GetName();
		
		for (int i; i < tagCount; i++)
		{
			text += m_aTags[i].GetEnd();
		}
		return text;
	}
};

[BaseContainerProps()]
class SCR_BaseTagBlockEntry
{
	string GetStart();
	string GetEnd();
};
[BaseContainerProps()]
class SCR_CustomTagBlockEntry: SCR_BaseTagBlockEntry
{
	[Attribute()]
	protected string m_sStart;
	
	[Attribute()]
	protected string m_sEnd;
	
	override string GetStart()	{ return m_sStart; }
	override string GetEnd()	{ return m_sEnd; }
};
[BaseContainerProps()]
class SCR_BoldTagBlockEntry: SCR_BaseTagBlockEntry
{
	override string GetStart()	{ return "<b>"; }
	override string GetEnd()	{ return "</b>"; }
};
[BaseContainerProps()]
class SCR_ItalicTagBlockEntry: SCR_BaseTagBlockEntry
{
	override string GetStart()	{ return "<i>"; }
	override string GetEnd()	{ return "</i>"; }
};
[BaseContainerProps()]
class SCR_HeaderTagBlockEntry: SCR_BaseTagBlockEntry
{
	override string GetStart()	{ return "<h1>"; }
	override string GetEnd()	{ return "</h1>"; }
};
[BaseContainerProps()]
class SCR_SubHeaderTagBlockEntry: SCR_BaseTagBlockEntry
{
	override string GetStart()	{ return "<h2>"; }
	override string GetEnd()	{ return "</h2>"; }
};
*/