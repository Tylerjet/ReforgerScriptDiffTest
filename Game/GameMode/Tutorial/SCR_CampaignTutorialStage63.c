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
		m_fWaypointHeightOffset = 0;
		m_bCheckWaypoint = false;
		/*m_fConditionCheckPeriod = 1;
		
		GetGame().GetCallqueue().Remove(OverrideGMHint);
		GetGame().GetCallqueue().CallLater(OverrideGMHint, 250, true);
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_BuildingBunker_GM", duration: -1);*/
		
		string hintString = "#AR-Tutorial_Hint_BuildingBunker <h1 align='center' scale='4'><color rgba='34,196,244,255'><image set='{F7E8D4834A3AFF2F}UI/Imagesets/Conflict/conflict-icons-bw.imageset' name='Slot_Fortification'/></color></h1>";
		
		GetGame().GetCallqueue().CallLater(DelayedPopup, 20000, false, "#AR-Tutorial_Popup_Title-UC", "#AR-Tutorial_Popup_BlockedSlots", 12, "", "", "", "");
		SCR_HintManagerComponent.ShowCustomHint(hintString + CreateString("#AR-ControlsHint_BuildingCompositionRotate","BuildingPreviewRotationUp","BuildingPreviewRotationDown"), duration: -1);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{			
		/*if (m_bIsBunkerBuilt)
		{
			if (m_TutorialComponent.GetSupplyTruckComponent() && m_TutorialComponent.GetSupplyTruckComponent().GetSupplies() >= 200)
				m_TutorialComponent.FinishStage(this, SCR_ECampaignTutorialStage.CONFLICT_BOARD_TRUCK);
			else
				return true;
		}*/
		
		SCR_SiteSlotEntity bunker_slot = SCR_SiteSlotEntity.Cast(GetGame().GetWorld().FindEntityByName("buildingSlot"));	
						
		if (bunker_slot && bunker_slot.IsOccupied())
		{
			m_TutorialComponent.FinishStage(this, SCR_ECampaignTutorialStage.CONFLICT_BUILDING_QUIT);
		}
		else
		{
			SCR_CampaignBuildingComponent buildComp = SCR_CampaignBuildingComponent.Cast(m_Player.FindComponent(SCR_CampaignBuildingComponent));
			
			if (buildComp && !buildComp.IsBuilding())
				m_TutorialComponent.FinishStage(this, SCR_ECampaignTutorialStage.CONFLICT_TURN_ON_BUILDING);
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	/*override void OnStructureBuilt(SCR_CampaignBase base, IEntity structure)
	{
		if (base != SCR_CampaignBase.Cast(GetGame().GetWorld().FindEntityByName("TownBaseChotain")))
			return;
		
		SCR_CampaignBunkerComponent comp = SCR_CampaignBunkerComponent.Cast(structure.FindComponent(SCR_CampaignBunkerComponent));
		
		if (!comp)
			return;
		
		m_bIsBunkerBuilt = true;
	}*/
	
	//------------------------------------------------------------------------------------------------
	override void Reset()
	{
		m_TutorialComponent.StageReset_SpawnTruck(GetGame().GetWorld().FindEntityByName("SupplyTruckBunkerPos"));
		m_TutorialComponent.StageReset_PrepareChotain();
	}
	
	//------------------------------------------------------------------------------------------------
	/*void OverrideGMHint()
	{
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_BuildingBunker_GM", duration: -1, isSilent: true);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_CampaignTutorialStage63()
	{
		GetGame().GetCallqueue().Remove(OverrideGMHint);
	}*/
};