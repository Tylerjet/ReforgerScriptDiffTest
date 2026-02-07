[ComponentEditorProps(category: "GameScripted/GameMode/Components", description: "Briefing screen shown in respawn menu.")]
class SCR_RespawnBriefingComponentClass : SCR_BaseGameModeComponentClass
{
};

class SCR_RespawnBriefingComponent : SCR_BaseGameModeComponent
{
	[Attribute("")]
	protected ResourceName m_sJournalConfigPath;

	[Attribute()]
	protected ref SCR_UIInfo m_Info;
	
	[Attribute("{324E923535DCACF8}UI/Textures/DeployMenu/Briefing/conflict_HintBanner_1_UI.edds", desc: "background shown when briefing has no hints")]
	protected ResourceName m_SimpleBriefingBackground;
	
	[Attribute()]
	protected ref array<ref SCR_UIInfo> m_aGameModeHints;

	[Attribute()]
	protected ref array<ref SCR_BriefingVictoryCondition> m_aWinConditions;

	protected ref ScriptInvoker m_OnBriefingChanged = new ScriptInvoker(); 
	
	protected bool m_bWasShown = false;
	
	/*!
	\return Local instance of the briefing component.
	*/
	static SCR_RespawnBriefingComponent GetInstance()
	{
		if (GetGame().GetGameMode())
			return SCR_RespawnBriefingComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_RespawnBriefingComponent));
		else
			return null;
	}
	
	/*!
	\return Briefing UI info
	*/
	SCR_UIInfo GetInfo()
	{
		return m_Info;
	}
	
	/*!
	\return Simple briefing background image
	*/
	ResourceName GetSimpleBriefingBackground()
	{
		return m_SimpleBriefingBackground;
	}

	ResourceName GetJournalConfigPath()
	{
		return m_sJournalConfigPath;
	}

	//------------------------------------------------------------------------------------------------
	int GetGameModeHints(out array<ref SCR_UIInfo> hints)
	{
		hints = m_aGameModeHints;

		return m_aGameModeHints.Count();
	}

	//------------------------------------------------------------------------------------------------
	int GetWinConditions(out array<ref SCR_BriefingVictoryCondition> conditions)
	{
		conditions = m_aWinConditions;

		return m_aWinConditions.Count();
	}

	/*!
	\return Event called when info changes, so that respawn menu can be updated (to be override by inherited classes)
	*/
	ScriptInvoker GetOnBriefingChanged()
	{
		return m_OnBriefingChanged;
	}

	//------------------------------------------------------------------------------------------------	
	void SetBriefingShown(bool shown = true)
	{
		m_bWasShown = shown;
	}

	//------------------------------------------------------------------------------------------------
	bool GetWasBriefingShown()
	{
		return m_bWasShown;
	}
};

[BaseContainerProps()]
class SCR_BriefingVictoryCondition
{
	[Attribute("1", UIWidgets.ComboBox, "Type of victory condition", "", ParamEnumArray.FromEnum(ETaskIconType))]
	protected ETaskIconType victoryCondition;
	
	[Attribute()]
	protected string name;
	
	[Attribute()]
	protected string description;
	
	//------------------------------------------------------------------------------------------------
	string GetName()
	{
		return name;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetDescription()
	{
		return description;
	}
	
	//------------------------------------------------------------------------------------------------
	ETaskIconType GetConditionType()
	{
		return victoryCondition;
	}
};