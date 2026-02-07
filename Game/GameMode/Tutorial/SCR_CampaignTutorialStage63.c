class SCR_CampaignTutorialStage63Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage63 : SCR_BaseCampaignTutorialStage
{
	protected bool m_bIsBunkerBuilt;
	
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_CONFLICT_BUILD_BUNKER");
		m_fWaypointHeightOffset = 1.3;
		m_bCheckWaypoint = false;
		m_fConditionCheckPeriod = 1;
		
		GetGame().GetCallqueue().Remove(OverrideGMHint);
		GetGame().GetCallqueue().CallLater(OverrideGMHint, 250, true);
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_BuildingBunker_GM", duration: -1);
		
		// Find local player controller
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return;
		
		// Find campaign network component to send RPC to server
		SCR_CampaignNetworkComponent campaignNetworkComponent = SCR_CampaignNetworkComponent.Cast(playerController.FindComponent(SCR_CampaignNetworkComponent));
		if (!campaignNetworkComponent)
			return;
		
		campaignNetworkComponent.CheatRank();
		campaignNetworkComponent.CheatRank();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{			
		if (m_bIsBunkerBuilt)
		{
			if (m_TutorialComponent.GetSupplyTruckComponent() && m_TutorialComponent.GetSupplyTruckComponent().GetSupplies() >= 200)
				m_TutorialComponent.FinishStage(this, SCR_ECampaignTutorialStage.CONFLICT_BOARD_TRUCK);
			else
				return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnStructureBuilt(SCR_CampaignBase base, IEntity structure)
	{
		if (base != SCR_CampaignBase.Cast(GetGame().GetWorld().FindEntityByName("TownBaseChotain")))
			return;
		
		SCR_CampaignBunkerComponent comp = SCR_CampaignBunkerComponent.Cast(structure.FindComponent(SCR_CampaignBunkerComponent));
		
		if (!comp)
			return;
		
		m_bIsBunkerBuilt = true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void Reset()
	{
		m_TutorialComponent.StageReset_SpawnTruck(GetGame().GetWorld().FindEntityByName("SupplyTruckBunkerPos"));
		m_TutorialComponent.StageReset_PrepareChotain();
	}
	
	//------------------------------------------------------------------------------------------------
	void OverrideGMHint()
	{
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_BuildingBunker_GM", duration: -1, isSilent: true);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_CampaignTutorialStage63()
	{
		GetGame().GetCallqueue().Remove(OverrideGMHint);
	}
};