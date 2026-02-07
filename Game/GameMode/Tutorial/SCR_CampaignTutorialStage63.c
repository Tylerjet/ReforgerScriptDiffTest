class SCR_CampaignTutorialStage63Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage63 : SCR_BaseCampaignTutorialStage
{
	protected int m_iOriginalSpawnpointsCount;
	
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_CONFLICT_BUILD_BUNKER");
		m_fWaypointHeightOffset = 1.3;
		m_bCheckWaypoint = false;
		m_fConditionCheckPeriod = 1;
		
		GetGame().GetCallqueue().Remove(OverrideGMHint);
		GetGame().GetCallqueue().CallLater(OverrideGMHint, 250, true);
		
		if (GetGame().GetInputManager().IsUsingMouseAndKeyboard())
			SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_BuildingBunker_GM_Arland" + CreateString("#AR-KeybindSeparator_ManualCameraRotate", "CharacterSprint"), duration: -1);
		else
			SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_BuildingBunker_GM_Arland" + CreateString("#AR-KeybindSeparator_ManualCameraRotate", "EditorTransformRotateYaw"), duration: -1);
		
		PlayerController playerController = GetGame().GetPlayerController();
		
		if (!playerController)
			return;
		
		SCR_PlayerXPHandlerComponent comp = SCR_PlayerXPHandlerComponent.Cast(playerController.FindComponent(SCR_PlayerXPHandlerComponent));
		
		if (!comp)
			return;
		
		comp.CheatRank();
		comp.CheatRank();
		
		SCR_CampaignMilitaryBaseComponent base = SCR_CampaignMilitaryBaseComponent.Cast(GetGame().GetWorld().FindEntityByName("TownBaseChotain").FindComponent(SCR_CampaignMilitaryBaseComponent));
		m_iOriginalSpawnpointsCount = base.GetSpawnPoint().GetChildSpawnPoints().Count();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{	
		SCR_CampaignMilitaryBaseComponent base = SCR_CampaignMilitaryBaseComponent.Cast(GetGame().GetWorld().FindEntityByName("TownBaseChotain").FindComponent(SCR_CampaignMilitaryBaseComponent));
		array<SCR_Position> spawnpoints = base.GetSpawnPoint().GetChildSpawnPoints();
		
		if (base.GetSpawnPoint().GetChildSpawnPoints().Count() > m_iOriginalSpawnpointsCount)
		{
			if (m_TutorialComponent.GetSupplyTruckComponent() && m_TutorialComponent.GetSupplyTruckComponent().GetSupplies() >= 200)
				m_TutorialComponent.FinishStage(this, SCR_ECampaignTutorialStage.CONFLICT_BOARD_TRUCK);
			else
				return true;
		}
		
		return false;
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
		if (GetGame().GetInputManager().IsUsingMouseAndKeyboard())
			SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_BuildingBunker_GM_Arland" + CreateString("#AR-KeybindSeparator_ManualCameraRotate", "CharacterSprint"), duration: -1);
		else
			SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_BuildingBunker_GM_Arland" + CreateString("#AR-KeybindSeparator_ManualCameraRotate", "EditorTransformRotateYaw"), duration: -1);
		GetGame().GetCallqueue().Remove(OverrideGMHint);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnInputDeviceChanged(bool switchedToKeyboard)
	{
		if (switchedToKeyboard)
			SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_BuildingBunker_GM_Arland" + CreateString("#AR-KeybindSeparator_ManualCameraRotate", "CharacterSprint"), duration: -1);
		else
			SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_BuildingBunker_GM_Arland" + CreateString("#AR-KeybindSeparator_ManualCameraRotate", "EditorTransformRotateYaw"), duration: -1);
	}
	
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_CampaignTutorialStage63()
	{
		GetGame().GetCallqueue().Remove(OverrideGMHint);
	}
};