class SCR_TourniquetStorageComponentClass : SCR_EquipmentStorageComponentClass
{
};

class SCR_TourniquetStorageComponent : SCR_EquipmentStorageComponent
{
	//------------------------------------------------------------------------------------------------
	void AddTourniquetToSlot(ECharacterHitZoneGroup eHitZoneGroup, IEntity tourniquet)
	{
		SCR_TourniquetStorageComponent tourniquetStorageComp = SCR_TourniquetStorageComponent.Cast(GetOwner().FindComponent(SCR_TourniquetStorageComponent));
		if (!tourniquetStorageComp)
			return;		
		
		SCR_InventoryStorageManagerComponent storageMan = SCR_InventoryStorageManagerComponent.Cast(GetOwner().FindComponent(SCR_InventoryStorageManagerComponent));
		if (!storageMan)
			return;
	
		if (!tourniquet)
			return;
		
		SCR_TourniquetStorageSlot tqTargetSlot;
		for (int i, count = tourniquetStorageComp.GetSlotsCount(); i < count; i++)
		{
			tqTargetSlot = SCR_TourniquetStorageSlot.Cast(tourniquetStorageComp.GetSlot(i));
			if (!tqTargetSlot)
				continue;
				
			
			if (tqTargetSlot.GetAssociatedHZGroup() != eHitZoneGroup)
				continue;
			
			if (tqTargetSlot.GetItem(i))
			{
				Debug.Error("TourniquetSlot already contained some item");
				return;
			}

			break;
		}
		
		SCR_CharacterInventoryStorageComponent charInventoryStorageComp = SCR_CharacterInventoryStorageComponent.Cast(GetOwner().FindComponent(SCR_CharacterInventoryStorageComponent));
		if (!charInventoryStorageComp)
			return;

		storageMan.TryMoveItemToStorage(tourniquet, tourniquetStorageComp, tqTargetSlot.GetID());
	}
	
	//------------------------------------------------------------------------------------------------
	bool RemoveTourniquetFromSlot(ECharacterHitZoneGroup eHitZoneGroup)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(GetOwner());
		if (!character)
			return false;
		
		SCR_CharacterDamageManagerComponent damageMgr = SCR_CharacterDamageManagerComponent.Cast(character.GetDamageManager());
		if (!damageMgr)
			return false;
		
		SCR_TourniquetStorageComponent tourniquetStorageComp = SCR_TourniquetStorageComponent.Cast(GetOwner().FindComponent(SCR_TourniquetStorageComponent));
		if (!tourniquetStorageComp)
			return false;

		SCR_InventoryStorageManagerComponent storageMan = SCR_InventoryStorageManagerComponent.Cast(GetOwner().FindComponent(SCR_InventoryStorageManagerComponent));
		if (!storageMan)
			return false;
				
		IEntity targetTourniquet;
		SCR_TourniquetStorageSlot tqTargetSlot;
		for (int i, count = tourniquetStorageComp.GetSlotsCount(); i < count; i++)
		{
			tqTargetSlot = SCR_TourniquetStorageSlot.Cast(tourniquetStorageComp.GetSlot(i));
			if (!tqTargetSlot)
				continue;

			if (tqTargetSlot.GetAssociatedHZGroup() != eHitZoneGroup)
				continue;
			
			targetTourniquet = tqTargetSlot.GetItem(i);
			break;
		}

		return storageMan.TryMoveItemToStorage(targetTourniquet, storageMan.FindStorageForItem(targetTourniquet, EStoragePurpose.PURPOSE_DEPOSIT));		
	}
	
	override void OnAddedToSlot(IEntity item, int slotID)
	{
		ChimeraCharacter char = ChimeraCharacter.Cast(GetOwner());
		if (!char)
			return;

		SCR_TourniquetStorageSlot tqSlot = SCR_TourniquetStorageSlot.Cast(GetSlot(slotID));
		if (!tqSlot)
			return;

		SCR_CharacterDamageManagerComponent damageMgr = SCR_CharacterDamageManagerComponent.Cast(char.GetDamageManager());
		if (!damageMgr)
			return;

		damageMgr.SetTourniquettedGroup(tqSlot.GetAssociatedHZGroup(), true);		
	}
	
	override void OnRemovedFromSlot(IEntity item, int slotID)
	{
		ChimeraCharacter char = ChimeraCharacter.Cast(GetOwner());
		if (!char)
			return;
		
		SCR_TourniquetStorageSlot tqSlot = SCR_TourniquetStorageSlot.Cast(GetSlot(slotID));
		if (!tqSlot)
			return;

		SCR_CharacterDamageManagerComponent damageMgr = SCR_CharacterDamageManagerComponent.Cast(char.GetDamageManager());
		if (!damageMgr)
			return;

		damageMgr.SetTourniquettedGroup(tqSlot.GetAssociatedHZGroup(), false);
	}
};