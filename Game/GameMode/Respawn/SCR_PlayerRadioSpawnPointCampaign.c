[EntityEditorProps(category: "GameScripted/GameMode", description: "")]
class SCR_PlayerRadioSpawnPointCampaignClass: SCR_PlayerRadioSpawnPointClass
{
};
class SCR_PlayerRadioSpawnPointCampaign: SCR_PlayerRadioSpawnPoint
{
	//------------------------------------------------------------------------------------------------
	Faction GetCachedFaction()
	{
		return m_CachedFaction;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void ActivateSpawnPoint()
	{
		//--- Track when the player picks up or drops radio backpack
		SCR_InventoryStorageManagerComponent inventoryManager = SCR_InventoryStorageManagerComponent.Cast(m_TargetPlayer.FindComponent(SCR_InventoryStorageManagerComponent));
		if (inventoryManager)
		{
			inventoryManager.m_OnItemAddedInvoker.Insert(OnItemAdded);
			inventoryManager.m_OnItemRemovedInvoker.Insert(OnItemRemoved);
		}
		
		//--- Check current count of active radios
		SCR_ChimeraCharacter player = SCR_ChimeraCharacter.Cast(m_TargetPlayer);
		
		if (!player)
			return;
		
		Faction faction = player.GetFaction();
		
		if (!faction)
			return;
		
		SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
		
		if (!campaign)
			return;
		
		int left = campaign.GetMaxRespawnRadios() - campaign.GetActiveRespawnRadiosCount(faction.GetFactionKey());
		
		if (left <= 0)
			return;
		
		//--- If the player currently has a radio backpack, activate the spawn point instantly
		BaseLoadoutManagerComponent loadoutManager = BaseLoadoutManagerComponent.Cast(m_TargetPlayer.FindComponent(BaseLoadoutManagerComponent));
		if (loadoutManager)
		{
			IEntity backpack = loadoutManager.GetClothByArea(ELoadoutArea.ELA_Backpack);
			if (backpack)
				OnItemAdded(backpack, null);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void ActivateSpawnPointPublic()
	{
		ActivateSpawnPoint();
	}
	
	//------------------------------------------------------------------------------------------------
	void DeactivateSpawnPointPublic()
	{
		super.DeactivateSpawnPoint();
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_PlayerRadioSpawnPointCampaign()
	{
		// If disconnecting player currently holds a respawn radio, handle it
		if (!Replication.IsServer())
			return;
		
		if (GetFactionKey().IsEmpty())
			return;
		
		SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
		
		if (!campaign)
			return;
		
		campaign.RemoveActiveRespawnRadio(GetFactionKey());
	}
};