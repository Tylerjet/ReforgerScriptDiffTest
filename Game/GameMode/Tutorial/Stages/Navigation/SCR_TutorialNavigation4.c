[EntityEditorProps(insertable: false)]
class SCR_TutorialNavigation4Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TutorialNavigation4 : SCR_BaseCampaignTutorialArlandStage
{
	Widget m_wHighlight;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{	
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		PlaySoundSystem("Navigation_MapTools");
		
		Widget toolMenu = GetGame().GetWorkspace().FindAnyWidget("ToolMenu");
		
		if (toolMenu)
			m_wHighlight = SCR_WidgetHighlightUIComponent.CreateHighlight(toolMenu, "{D574871D2C37B255}UI/layouts/Common/WidgetHighlight.layout");
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return !m_TutorialComponent.GetVoiceSystem().IsPlaying();
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_TutorialNavigation4()
	{
		delete m_wHighlight;
	}
};