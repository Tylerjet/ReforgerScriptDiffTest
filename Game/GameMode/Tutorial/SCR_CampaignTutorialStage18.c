class SCR_CampaignTutorialStage18Class: SCR_BaseCampaignTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialStage18 : SCR_BaseCampaignTutorialStage
{
	protected bool m_bHasM16Mag;
	
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_MAGAZINE_PICK");
		m_fWaypointHeightOffset = 0;
		m_bCheckWaypoint = false;
		SCR_InventoryStorageManagerComponent storageManComp = SCR_InventoryStorageManagerComponent.Cast(m_Player.FindComponent(SCR_InventoryStorageManagerComponent));
		
		if (storageManComp)	
		{
			storageManComp.m_OnItemAddedInvoker.Remove(OnItemAdded);
			storageManComp.m_OnItemAddedInvoker.Insert(OnItemAdded);
		}
		
		IEntity ammobox = GetGame().GetWorld().FindEntityByName("Ammobox");
		
		if (ammobox)
			m_WaypointEntity.SetOrigin(ammobox.GetOrigin());
		
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_MagazinePickup" + CreateString("#AR-Keybind_GadgetToggle","CharacterAction"), duration: -1);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_bHasM16Mag;
	}
	
	//------------------------------------------------------------------------------------------------
	override void Reset()
	{
		m_TutorialComponent.StageReset_RifleRespawn();
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool OnItemAdded(IEntity item)
	{
		MagazineComponent magComp = MagazineComponent.Cast(item.FindComponent(MagazineComponent));
		
		if (!magComp)
			return false;
		 
		BaseMagazineWell baseMagwell;
		baseMagwell = magComp.GetMagazineWell();
		
		if (!baseMagwell)
			return false;
		
		MagazineWellStanag556 m16mag = MagazineWellStanag556.Cast(baseMagwell);
		
		if (!m16mag)	
			return false;
		
		SCR_InventoryStorageManagerComponent storageManComp = SCR_InventoryStorageManagerComponent.Cast(m_Player.FindComponent(SCR_InventoryStorageManagerComponent));
		
		if (storageManComp)	
			storageManComp.m_OnItemAddedInvoker.Remove(OnItemAdded);
		
		m_bHasM16Mag = true;
		return true;
	}
};