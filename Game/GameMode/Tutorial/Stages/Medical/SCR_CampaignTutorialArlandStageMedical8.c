[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandStageMedical8Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandStageMedical8: SCR_BaseCampaignTutorialArlandStage
{
	protected SCR_InspectCasualtyWidget m_CasualtyInspectDisplay;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		array<BaseInfoDisplay> infoDisplays = {};
		GetGame().GetPlayerController().GetHUDManagerComponent().GetInfoDisplays(infoDisplays);
		foreach (BaseInfoDisplay baseInfoDisplays : infoDisplays)
		{
			m_CasualtyInspectDisplay = SCR_InspectCasualtyWidget.Cast(baseInfoDisplays);
			if (m_CasualtyInspectDisplay)
				break;
		}
		
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		PlaySoundSystem("FirstAid_CheckHealth", true);
		HintOnVoiceOver();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{		
		if (!m_CasualtyInspectDisplay)
			return false;

		return m_CasualtyInspectDisplay.IsActive();
	}
};