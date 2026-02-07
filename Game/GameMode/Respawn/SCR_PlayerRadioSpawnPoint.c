[EntityEditorProps(category: "GameScripted/GameMode", description: "")]
class SCR_PlayerRadioSpawnPointClass: SCR_PlayerSpawnPointClass
{
};
class SCR_PlayerRadioSpawnPoint: SCR_PlayerSpawnPoint
{
	protected void OnItemAdded(IEntity item, BaseInventoryStorageComponent storageOwner)
	{
		BaseLoadoutClothComponent loadoutCloth = BaseLoadoutClothComponent.Cast(item.FindComponent(BaseLoadoutClothComponent));
		if (loadoutCloth && loadoutCloth.GetArea() == ELoadoutArea.ELA_Backpack && item.FindComponent(SCR_RadioComponent))
			super.ActivateSpawnPoint();
	}
	protected void OnItemRemoved(IEntity item, BaseInventoryStorageComponent storageOwner)
	{
		BaseLoadoutClothComponent loadoutCloth = BaseLoadoutClothComponent.Cast(item.FindComponent(BaseLoadoutClothComponent));
		if (loadoutCloth && loadoutCloth.GetArea() == ELoadoutArea.ELA_Backpack && item.FindComponent(SCR_RadioComponent))
			super.DeactivateSpawnPoint();
	}
	
	override protected void ActivateSpawnPoint()
	{
		//--- Track when the player picks up or drops radio backpack
		SCR_InventoryStorageManagerComponent inventoryManager = SCR_InventoryStorageManagerComponent.Cast(m_TargetPlayer.FindComponent(SCR_InventoryStorageManagerComponent));
		if (inventoryManager)
		{
			inventoryManager.m_OnItemAddedInvoker.Insert(OnItemAdded);
			inventoryManager.m_OnItemRemovedInvoker.Insert(OnItemRemoved);
		}
		
		//--- If the player currently has a radio backpack, activate the spawn point instantly
		BaseLoadoutManagerComponent loadoutManager = BaseLoadoutManagerComponent.Cast(m_TargetPlayer.FindComponent(BaseLoadoutManagerComponent));
		if (loadoutManager)
		{
			IEntity backpack = loadoutManager.GetClothByArea(ELoadoutArea.ELA_Backpack);
			if (backpack)
				OnItemAdded(backpack, null);
		}
	}
	override protected void DeactivateSpawnPoint()
	{
		if (m_TargetPlayer)
		{
			SCR_InventoryStorageManagerComponent inventoryManager = SCR_InventoryStorageManagerComponent.Cast(m_TargetPlayer.FindComponent(SCR_InventoryStorageManagerComponent));
			if (inventoryManager)
			{
				inventoryManager.m_OnItemAddedInvoker.Remove(OnItemAdded);
				inventoryManager.m_OnItemRemovedInvoker.Remove(OnItemRemoved);
			}
		}
		super.DeactivateSpawnPoint();
	}
};