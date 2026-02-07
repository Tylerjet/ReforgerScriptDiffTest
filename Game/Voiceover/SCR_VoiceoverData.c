//------------------------------------------------------------------------------------------------
[BaseContainerProps(configRoot: true)]
class SCR_VoiceoverData
{
	[Attribute()]
	protected ref array<ref SCR_VoiceoverLineStandalone> m_aLines;

	[Attribute()]
	protected ref array<ref SCR_VoiceoverSequence> m_aSequences;

	//------------------------------------------------------------------------------------------------
	SCR_VoiceoverLineStandalone GetLineByName(string name)
	{
		foreach (SCR_VoiceoverLineStandalone line : m_aLines)
		{
			if (line.GetName() == name)
				return line;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	SCR_VoiceoverSequence GetSequenceByName(string name)
	{
		foreach (SCR_VoiceoverSequence sequence : m_aSequences)
		{
			if (sequence.GetName() == name)
				return sequence;
		}

		return null;
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_BaseContainerCustomTitleField("m_sName")]
class SCR_VoiceoverSequence
{
	[Attribute("")]
	protected string m_sName;

	[Attribute()]
	protected ref array<ref SCR_VoiceoverLine> m_aLines;

	//------------------------------------------------------------------------------------------------
	string GetName()
	{
		return m_sName;
	}

	//------------------------------------------------------------------------------------------------
	void GetLines(out notnull array<ref SCR_VoiceoverLine> lines)
	{
		if (m_aLines)
			lines = m_aLines;
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_BaseContainerCustomTitleField("m_sSoundEventName")]
class SCR_VoiceoverLine
{
	[Attribute(SCR_EVoiceoverActor.ACTOR_1.ToString(), uiwidget: UIWidgets.ComboBox, desc: "Type", enums: ParamEnumArray.FromEnum(SCR_EVoiceoverActor))]
	protected SCR_EVoiceoverActor m_eActor;

	[Attribute("", desc: "Sound event name as defined in the appropriate .acp file in the actor's SCR_CommunicationSoundComponent.")]
	protected string m_sSoundEventName;

	[Attribute("")]
	protected string m_sSubtitleText;

	//------------------------------------------------------------------------------------------------
	SCR_EVoiceoverActor GetActor()
	{
		return m_eActor;
	}

	//------------------------------------------------------------------------------------------------
	string GetSoundEventName()
	{
		return m_sSoundEventName;
	}

	//------------------------------------------------------------------------------------------------
	string GetSubtitleText()
	{
		return m_sSubtitleText;
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_BaseContainerCustomTitleField("m_sName")]
class SCR_VoiceoverLineStandalone : SCR_VoiceoverLine
{
	[Attribute("")]
	protected string m_sName;

	//------------------------------------------------------------------------------------------------
	string GetName()
	{
		return m_sName;
	}
}

enum SCR_EVoiceoverActor
{
	ACTOR_1,
	ACTOR_2,
	ACTOR_3,
	ACTOR_4,
	ACTOR_5
}
