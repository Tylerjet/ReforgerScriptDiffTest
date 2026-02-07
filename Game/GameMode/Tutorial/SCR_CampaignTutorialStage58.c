class SCR_CampaignTutorialStage58Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage58 : SCR_BaseCampaignTutorialStage
{
	protected SCR_CampaignMilitaryBaseComponent m_Base;
	
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_DRIVING_15");
		m_bCheckWaypoint = false;
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_SuppliesUnload", duration: -1, isSilent: true);
		m_TutorialComponent.SetWaypointTruckPosition(SCR_ETutorialSupplyTruckWaypointMode.BACK);
		
		m_Base = SCR_CampaignMilitaryBaseComponent.Cast(GetGame().GetWorld().FindEntityByName("TownBaseChotain").FindComponent(SCR_CampaignMilitaryBaseComponent));
		
		if (m_Base && m_Base.GetSupplies() < 450)
			m_Base.AddSupplies(450 - m_Base.GetSupplies());
		
		SCR_CompartmentAccessComponent accessComp = SCR_CompartmentAccessComponent.Cast(m_Player.FindComponent(SCR_CompartmentAccessComponent));
		
		if (accessComp)
			accessComp.GetOnCompartmentLeft().Remove(m_TutorialComponent.OnSupplyTruckLeft);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (m_Base)
			return (m_Base.GetSupplies() >= 950);
		else
			return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void Reset()
	{
		m_TutorialComponent.StageReset_SpawnTruck(m_WaypointEntity);
		m_TutorialComponent.GetSupplyTruckComponent().AddSupplies(1000);
	}
};