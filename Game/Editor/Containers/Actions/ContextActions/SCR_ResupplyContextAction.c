[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_ResupplyContextAction : SCR_SelectedEntitiesContextAction
{
	
	[Attribute("4", desc: "Amount of magazines the action will ensure are in the inventory (fill to this amount)")]
	private int m_MagazineQuantityInInventory;
	
	override bool CanBeShown(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
	{
		if (selectedEntity == null || selectedEntity.GetEntityType() != EEditableEntityType.CHARACTER)
		{
			return false;
		}
		
		return true;
	}
	
	override bool CanBePerformed(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
	{
		return IsRefillAvailable(selectedEntity);
	}
	
	override void Perform(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition)
	{
		World world = GetGame().GetWorld();
		
		ChimeraCharacter character = ChimeraCharacter.Cast(selectedEntity.GetOwner());
		if (!character) return;
		
		BaseWeaponManagerComponent weaponsManager = BaseWeaponManagerComponent.Cast(character.FindComponent(BaseWeaponManagerComponent));
		SCR_InventoryStorageManagerComponent inventoryManager = SCR_InventoryStorageManagerComponent.Cast(character.FindComponent(SCR_InventoryStorageManagerComponent));
		
		array<IEntity> weaponList = {};
		weaponsManager.GetWeaponsList(weaponList);
		
		array<RplId> resupplyItemIds = {};
		
		foreach	(IEntity weapon : weaponList)
		{
			BaseWeaponComponent comp = BaseWeaponComponent.Cast(weapon.FindComponent(BaseWeaponComponent));
			string weaponSlotType = comp.GetWeaponSlotType();
			
			// Only refill primary and secondary weapons
			if (!(weaponSlotType == "primary" || weaponSlotType == "secondary")) continue;
			
			MuzzleComponent muzzleComponent = MuzzleComponent.Cast(comp.GetCurrentMuzzle());
			if (!muzzleComponent) continue;
			
			Resource magazinePrefabResource = Resource.Load(muzzleComponent.GetDefaultMagazinePrefab().GetResourceName());
			if (magazinePrefabResource == null || !magazinePrefabResource.IsValid()) continue;
			
			int resupplyCount = m_MagazineQuantityInInventory - inventoryManager.GetMagazineCountByWeapon(comp);
			if (resupplyCount <= 0) continue;
			
			for (int i = 0; i < resupplyCount ; i++)
			{
				IEntity newMagazineEntity = GetGame().SpawnEntityPrefab(magazinePrefabResource, world);
				RplComponent rplComponent = RplComponent.Cast(newMagazineEntity.FindComponent(RplComponent));
				resupplyItemIds.Insert(rplComponent.Id());
			}
		}
		
		inventoryManager.ResupplyItems(resupplyItemIds);
	}
	
	protected bool IsRefillAvailable(SCR_EditableEntityComponent selectedEntity)
	{
		bool refillAvailable = false;
		ChimeraCharacter character = ChimeraCharacter.Cast(selectedEntity.GetOwner());
		
		if (!character)
			return false;
		
		BaseWeaponManagerComponent weaponsManager = BaseWeaponManagerComponent.Cast(character.FindComponent(BaseWeaponManagerComponent));
		SCR_InventoryStorageManagerComponent inventoryManager = SCR_InventoryStorageManagerComponent.Cast(character.FindComponent(SCR_InventoryStorageManagerComponent));
		
		array<IEntity> weaponList = {};
		weaponsManager.GetWeaponsList(weaponList);
		
		foreach	(IEntity weapon : weaponList)
		{
			BaseWeaponComponent comp = BaseWeaponComponent.Cast(weapon.FindComponent(BaseWeaponComponent));
			string weaponSlotType = comp.GetWeaponSlotType();
			
			// Only refill primary and secondary weapons
			if (!(weaponSlotType == "primary" || weaponSlotType == "secondary")) continue;
			
			if (m_MagazineQuantityInInventory - inventoryManager.GetMagazineCountByWeapon(comp) > 0)
			{
				refillAvailable = true;
				break;
			}
		}
		return refillAvailable;
	}
};