class SCR_CampaignTutorialStage84Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage84 : SCR_BaseCampaignTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_RadioPickup", duration: -1);
		GetGame().GetCallqueue().CallLater(DelayedPopup, 10000, false, "#AR-Tutorial_Popup_Title-UC", "#AR-Tutorial_Popup_Radios", 10, "", "", "", "");
		SCR_InventoryStorageManagerComponent comp = SCR_InventoryStorageManagerComponent.Cast(m_Player.FindComponent(SCR_InventoryStorageManagerComponent));
		
		if (comp)
		{
			comp.m_OnItemAddedInvoker.Remove(m_TutorialComponent.CheckRadioPickup);
			comp.m_OnItemAddedInvoker.Insert(m_TutorialComponent.CheckRadioPickup);
		}
		
		SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
		
		if (!campaign)
			return;
		
		while (campaign.GetActiveRespawnRadiosCount(campaign.FACTION_BLUFOR) > 0)
			campaign.RemoveActiveRespawnRadio(campaign.FACTION_BLUFOR);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return (m_TutorialComponent.GetPlayerRadio() != null);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Reset()
	{
		m_TutorialComponent.StageReset_CaptureLaruns();
		m_TutorialComponent.StageReset_HandleRespawnRadios();
	}
};