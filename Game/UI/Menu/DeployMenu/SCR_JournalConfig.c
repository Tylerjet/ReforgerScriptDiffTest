enum SCR_EJournalEntryType
{
	Situation,
	Mission,
	Execution,
	Signal,
	Intel,
	Custom
};

[BaseContainerProps(configRoot: true)]
class SCR_JournalSetupConfig
{
	[Attribute(defvalue: "-1", desc: "Which entry should be shown when the journal is opened (0 to number of entries)", params: "-1 inf")]
	protected int m_iJournalEntryToShowOnStart;
	
	[Attribute("")]
	protected ref array<ref SCR_JournalConfig> m_aJournals;
		
	SCR_JournalConfig GetJournalConfig(FactionKey factionKey = FactionKey.Empty)
	{
		SCR_JournalConfig empty;

		foreach (SCR_JournalConfig config : m_aJournals)
		{
			if (config.GetFactionKey() == FactionKey.Empty)
				empty = config;

			if (config.GetFactionKey() == factionKey)
				return config;
		}

		return empty;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return the index of the journal entry which is configured to be shown
	int GetJournalEntryToBeShown()
	{
		return m_iJournalEntryToShowOnStart;
	}
};

[BaseContainerProps(configRoot: true)]
class SCR_JournalConfig
{
	[Attribute("")]
	FactionKey m_FactionKey;

	[Attribute("")]
	protected ref array<ref SCR_JournalEntry> m_aEntries;

	array<ref SCR_JournalEntry> GetEntries()
	{
		return m_aEntries;
	}

	FactionKey GetFactionKey()
	{
		return m_FactionKey;
	}
};

[BaseContainerProps()]
class SCR_JournalEntry
{
	[Attribute(defvalue: "-1", params: "-1 inf")]
	protected int m_iEntryID;
	
	[Attribute("0", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_EJournalEntryType))]
	protected SCR_EJournalEntryType m_eJournalEntryType;

	[Attribute("{1AB2FA99C5CD3919}UI/layouts/Menus/DeployMenu/JournalButton.layout")]
	protected ResourceName m_sEntryButtonLayout;

	[Attribute("Custom", desc: "Custom name when using SCR_EJournalEntryType.Custom")]
	protected string m_sCustomEntryName;

	[Attribute("")]
	protected string m_sEntryText;

	[Attribute("0")]
	protected bool m_bUseCustomLayout;

	[Attribute("")]
	protected ResourceName m_sEntryLayoutCustom;

	protected ResourceName m_sEntryLayoutDefault = "{9BA70BE88082F251}UI/layouts/Menus/DeployMenu/JournalEntryDefault.layout";
	
	protected string m_sEntryTextParam1;
	protected Widget m_Widget;
	
	//------------------------------------------------------------------------------------------------
	int GetEntryID()
	{
		return m_iEntryID;
	}

	//------------------------------------------------------------------------------------------------
	string GetEntryName()
	{
		switch (m_eJournalEntryType)
		{
			case SCR_EJournalEntryType.Situation:
			{
				return "#AR-RespawnMap_FRAGO_SituationTitle";
			}

			case SCR_EJournalEntryType.Mission:
			{
				return "#AR-RespawnMap_FRAGO_MissionTitle";
			}

			case SCR_EJournalEntryType.Execution:
			{
				return "#AR-RespawnMap_FRAGO_ExecutionTitle";
			}

			case SCR_EJournalEntryType.Signal:
			{
				return "#AR-RespawnMap_FRAGO_SignalTitle";
			}

			case SCR_EJournalEntryType.Intel:
			{
				return "#AR-RespawnMap_FRAGO_IntelTitle";
			}

			case SCR_EJournalEntryType.Custom:
			{
				return m_sCustomEntryName;
			}
		}

		return string.Empty;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetCustomEntryName()
	{
		return m_sCustomEntryName;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetEntryText(string text)
	{
		m_sEntryText = text;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetEntryText()
	{
		return m_sEntryText;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetEntryTextWithParam1()
	{
		return string.Format(m_sEntryText, m_sEntryTextParam1);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetEntryTextParam1(string text)
	{
		m_sEntryTextParam1 = text;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetEntryTextParam1()
	{
		return m_sEntryTextParam1;
	}
	
	//------------------------------------------------------------------------------------------------
	Widget GetWidget()
	{
		return m_Widget;
	}
	
	//------------------------------------------------------------------------------------------------
	Widget SetEntryLayoutTo(out notnull Widget target)
	{
		if (m_bUseCustomLayout)
		{
			if (!m_Widget)
				m_Widget = GetGame().GetWorkspace().CreateWidgets(m_sEntryLayoutCustom, target);
			
			TextWidget text = TextWidget.Cast(m_Widget.FindAnyWidget("Text"));
			if (text)
				text.SetTextFormat(m_sEntryText, m_sEntryTextParam1);
		}
		else
		{
			if (!m_Widget)
				m_Widget = GetGame().GetWorkspace().CreateWidgets(m_sEntryLayoutDefault, target);
			
			TextWidget text = TextWidget.Cast(m_Widget.FindAnyWidget("Text"));
			if (text)
				text.SetTextFormat(m_sEntryText, m_sEntryTextParam1);
		}
		
		return m_Widget;
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetEntryButtonLayout()
	{
		return m_sEntryButtonLayout;
	}

};