[EntityEditorProps(category: "GameScripted/UI/Inventory", description: "[MOD] Inventory Slot UI class")]
class SCR_InventoryStorageManagerComponentClass: InventoryStorageManagerComponentClass
{
};

enum EInventoryRetCode
{
	RETCODE_OK = 0,
	RETCODE_ITEM_TOO_BIG = 2,
	RETCODE_ITEM_TOO_HEAVY = 4,
	RETCODE_DEFAULT_STATE = 0xFFFF
};

class SCR_HoldableItemPredicate: InventorySearchPredicate
{
    ECommonItemType wanted;
	void SCR_HoldableItemPredicate()
	{
        QueryAttributeTypes.Insert(SCR_ItemOfInterestAttribute);
	}
	
	override protected bool IsMatch(BaseInventoryStorageComponent storage, IEntity item, array<GenericComponent> queriedComponents, array<BaseItemAttributeData> queriedAttributes)
	{
        SCR_ItemOfInterestAttribute optionalAttribute = SCR_ItemOfInterestAttribute.Cast(queriedAttributes[0]);
		return optionalAttribute.GetInterestType() == wanted;
	}
};

class SCR_BandagePredicate: InventorySearchPredicate
{
	void SCR_BandagePredicate()
	{
		QueryComponentTypes.Insert(SCR_ConsumableItemComponent);
	}

	override protected bool IsMatch(BaseInventoryStorageComponent storage, IEntity item, array<GenericComponent> queriedComponents, array<BaseItemAttributeData> queriedAttributes)
	{		
		return (SCR_ConsumableItemComponent.Cast(queriedComponents[0])).GetConsumableType() == EConsumableType.Bandage;
	}
};

// Searches for magazines with certain mag well
class SCR_MagazinePredicate: InventorySearchPredicate
{
	typename magWellType;
	
	void SCR_MagazinePredicate()
	{
		QueryComponentTypes.Insert(BaseMagazineComponent);
	}
	
	override protected bool IsMatch(BaseInventoryStorageComponent storage, IEntity item, array<GenericComponent> queriedComponents, array<BaseItemAttributeData> queriedAttributes)
	{
		BaseMagazineComponent iMag = BaseMagazineComponent.Cast(queriedComponents[0]);
		
		if (!iMag)
			return false;
		
		BaseMagazineWell iMagWell = iMag.GetMagazineWell();
		
		if (!iMagWell)
			return false;
		
		return (iMagWell.IsInherited(magWellType)); // Check if magwells match
	}
};

// Searches for items with same prefab name
class SCR_PrefabNamePredicate: InventorySearchPredicate
{
	string prefabName;
	
	override protected bool IsMatch(BaseInventoryStorageComponent storage, IEntity item, array<GenericComponent> queriedComponents, array<BaseItemAttributeData> queriedAttributes)
	{
		EntityPrefabData pd = item.GetPrefabData();
		return pd.GetPrefabName() == this.prefabName;
	}
};

// Searches for items with same prefab data (prefer this to prefab name if you have prefab data already)
class SCR_PrefabDataPredicate: InventorySearchPredicate
{
	EntityPrefabData prefabData;
	
	override protected bool IsMatch(BaseInventoryStorageComponent storage, IEntity item, array<GenericComponent> queriedComponents, array<BaseItemAttributeData> queriedAttributes)
	{
		return item.GetPrefabData() == this.prefabData;
	}
};

class SCR_InventoryStorageManagerComponent : InventoryStorageManagerComponent
{
	private SCR_CharacterInventoryStorageComponent				m_pStorage;
	private CharacterControllerComponent						m_pCharacterController;
	private	ref SCR_BandagePredicate 							m_bandagePredicate = new SCR_BandagePredicate();
	protected EInventoryRetCode									m_ERetCode;
	protected int												m_iHealthEquipment	=	0;
	protected bool 												m_bIsInventoryLocked = false;
	protected ref SCR_WeaponSwitchingBaseUI						m_pWeaponSwitchingUI;
	private bool												m_bWasRaised;
	private IEntity 											m_pStorageToOpen;
	
	ref ScriptInvoker<IEntity, BaseInventoryStorageComponent>	m_OnItemAddedInvoker		= new ref ScriptInvoker<IEntity, BaseInventoryStorage>();
	ref ScriptInvoker<IEntity, BaseInventoryStorageComponent>	m_OnItemRemovedInvoker		= new ref ScriptInvoker<IEntity, BaseInventoryStorage>();

	ref ScriptInvoker<bool> 									m_OnInventoryOpenInvoker	= new ref ScriptInvoker<bool>();
	ref ScriptInvoker<bool> 									m_OnQuickBarOpenInvoker		= new ref ScriptInvoker<bool>();
	
	// Callback when item is added (will be performed locally after server completed the Insert/Move operation)
	override protected void OnItemAdded(BaseInventoryStorageComponent storageOwner, IEntity item)
	{		
		auto consumable = SCR_ConsumableItemComponent.Cast(item.FindComponent(SCR_ConsumableItemComponent));
		if ( consumable && consumable.GetConsumableType() == EConsumableType.Bandage )
			m_iHealthEquipment++;	//store count of the health components
				
		if ( m_OnItemAddedInvoker )
			m_OnItemAddedInvoker.Invoke( item, storageOwner );
		
		// Withdraw item from gc collection
		GarbageManager garbageManager = GetGame().GetGarbageManager();
		if (garbageManager)
		{
			if (item.FindComponent(InventoryItemComponent))
				garbageManager.Withdraw(item);
		}
	}
	
	// Callback when item is removed (will be performed locally after server completed the Remove/Move operation)
	override protected void OnItemRemoved(BaseInventoryStorageComponent storageOwner, IEntity item)
	{
		auto consumable = SCR_ConsumableItemComponent.Cast(item.FindComponent(SCR_ConsumableItemComponent));
		if ( consumable && consumable.GetConsumableType() == EConsumableType.Bandage )
			m_iHealthEquipment--;	//store count of the health components
		
		if ( m_OnItemRemovedInvoker )
			m_OnItemRemovedInvoker.Invoke( item, storageOwner );
		
		// Insert item into gc collection
		GarbageManager garbageManager = GetGame().GetGarbageManager();
		if (garbageManager)
		{
			if (item.FindComponent(InventoryItemComponent))
				garbageManager.Insert(item);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void PlayItemSound(RplId itemRplId, string soundEvent)
	{
		Rpc(RpcAsk_PlaySound, itemRplId, soundEvent);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_PlaySound(RplId targetRplId, string soundAction)
	{
		Rpc(RpcDo_PlaySound, targetRplId, soundAction);
		RpcDo_PlaySound(targetRplId, soundAction);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_PlaySound(RplId targetRplId, string soundAction)
	{
		RplComponent rplComp = RplComponent.Cast(Replication.FindItem(targetRplId));
		if (!rplComp)
			return;
		
		IEntity entity = rplComp.GetEntity();
		if (!entity)
			return;
		
		SimpleSoundComponent soundComp = SimpleSoundComponent.Cast(entity.FindComponent(SimpleSoundComponent));
		if (!soundComp)
			return;
		
		vector transformation[4];
		entity.GetTransform(transformation);
		soundComp.SetTransformation(transformation);
		
		soundComp.PlayStr(soundAction);
	}
	
#ifndef DISABLE_INVENTORY

	//------------------------------------------------------------------------------------------------
	SCR_CharacterInventoryStorageComponent GetCharacterStorage() { return m_pStorage; }
	
	//------------------------------------------------------------------------------------------------
	// ! TODO: make this method as native ( cannot override the proto native CanMoveItemToStorage )
	bool CanInsertItemInActualStorage(IEntity item, BaseInventoryStorageComponent storage, int slotID = -1)
	{
		if (!IsAnimationReady() || IsInventoryLocked())
			return false;

		array<BaseInventoryStorageComponent> pStorages = new array<BaseInventoryStorageComponent>();
		storage.GetOwnedStorages( pStorages, 1, false );
		pStorages.Insert( storage );

		foreach ( BaseInventoryStorageComponent pStorage : pStorages )
		{	
			bool bCanInsert = CanInsertItemInStorage( item, pStorage, -1 ); //split becouse of debug purposes
			bool bCanMove = CanMoveItemToStorage( item, pStorage, -1 );
			if ( bCanInsert || bCanMove )
				return true;
		}
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	// ! The return code informs about the state of the operation ( i.e. cannot insert item, since it is too large )
	// ! it clear the flag
	void SetReturnCode( EInventoryRetCode ERetCode ) 
	{ 
		m_ERetCode &= ~ERetCode; 
	}
	
	//------------------------------------------------------------------------------------------------
	// ! The return code informs about the state of the operation ( i.e. cannot insert item, since it is too large )
	// ! it clear the flag
	void SetReturnCodeDefault() 
	{ 
		m_ERetCode = EInventoryRetCode.RETCODE_DEFAULT_STATE;
	}
	
	//------------------------------------------------------------------------------------------------
	// ! 
	EInventoryRetCode GetReturnCode() { return m_ERetCode; }
	
	//------------------------------------------------------------------------------------------------
	// !
	float GetTotalWeightOfAllStorages()
	{
		ref array<BaseInventoryStorageComponent> storages = new array<BaseInventoryStorageComponent>();
		float fTotalWeight = 0.0;

		//TODO: actually not a very good way how to get storages, but using the GetStorages() method causes the weight being doubled. We need to get just the "parent" storages
		storages.Insert( m_pStorage.GetWeaponStorage() );
		storages.Insert( m_pStorage );
				
		foreach( BaseInventoryStorageComponent storage: storages )
		{
			fTotalWeight += storage.GetTotalWeight();
		}
		return fTotalWeight;
	}
	
	//------------------------------------------------------------------------------------------------
	// !
	void SetLootStorage( IEntity pOwner )
	{
		if( m_pStorage )
			m_pStorage.SetLootStorage( pOwner );
	}
	
	//------------------------------------------------------------------------------------------------
	//! try insert the item into the storage (not slot)
	void InsertItem( IEntity pItem, BaseInventoryStorageComponent pStorageTo = null, BaseInventoryStorageComponent pStorageFrom = null, SCR_InvCallBack cb = null  )
	{
		if (!pItem || !IsAnimationReady() || IsInventoryLocked())
			return;
		
		SetInventoryLocked(true);

		bool canInsert = true;
		if ( !pStorageTo ) // no storage selected, put it into best fitting storage
		{
			//TryInsertItem( pItem, EStoragePurpose.PURPOSE_DEPOSIT);	// works for the owned storages ( not for the vicinity storages )
			if ( !TryInsertItem( pItem, EStoragePurpose.PURPOSE_WEAPON_PROXY, cb ) )
				if ( !TryInsertItem( pItem, EStoragePurpose.PURPOSE_DEPOSIT, cb ) )
					if ( !TryMoveItemToStorage( pItem, FindStorageForItem( pItem, EStoragePurpose.PURPOSE_ANY ), -1, cb ) )
						canInsert = TryMoveItemToStorage( pItem, m_pStorage, -1, cb ); 				// clothes from storage in vicinity
		}
		else
		{
			if ( pStorageTo == m_pStorage )
			{
				if ( TryReplaceItem( pStorageTo, pItem, 0, cb ) )
				{
					SetInventoryLocked(false);
					return;
				}
			}
			if ( FindStorageForInsert( pItem, pStorageTo, EStoragePurpose.PURPOSE_ANY ) )
				pStorageTo = FindStorageForInsert( pItem, pStorageTo, EStoragePurpose.PURPOSE_ANY );
			
			if ( !pStorageFrom )
			{
				TryInsertItemInStorage( pItem, pStorageTo, -1, cb );	// if we move item from ground to opened storage
			}
			else
				TryMoveItemToStorage( pItem, pStorageTo, -1, cb );		// if we move item between storages
		}

		if (m_pCharacterController && canInsert)
		{
			m_pCharacterController.TryPlayItemGesture(EItemGesture.EItemGesturePickUp);
		}

		SetInventoryLocked(false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Locks the inventory for the duration of the item removal process
	bool TryRemoveItemFromInventory(IEntity pItem, BaseInventoryStorageComponent storage = null, InventoryOperationCallback cb = null)
	{
		if (!CanMoveItem(pItem))
			return false;

		SetInventoryLocked(true);
		bool result = TryRemoveItemFromStorage(pItem, storage, cb);
		SetInventoryLocked(false);

		return result;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Checks whether it is possible to move the item
	bool CanMoveItem(IEntity item)
	{
		if (!item || !IsAnimationReady() || IsInventoryLocked())
			return false;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! 
	/*
	override bool TryInsertItemInStorageScr( IEntity pItem, BaseInventoryStorageComponent pStorageTo, int slotID = -1, InventoryOperationCallback cb = null )
	{
		
		if ( !pStorageTo )
			return;
		
		array<BaseInventoryStorageComponent> pStorages = new array<BaseInventoryStorageComponent>();
		pStorage.GetOwnedStorages( pStorages, 1, false );	// get all the storages, the storage has attached to it
		pStorages.Insert( pStorage );						// and put there also the storage
		foreach ( BaseInventoryStorageComponent tmpStorage : pStorages )
		{
			if ( MoveOperation( itemComponent, tmpStorage ) )
			{
				bRet = true;
				break;
			}
		}			
	}
	*/
	
	
	//------------------------------------------------------------------------------------------------
	//!
	bool TrySwapItems( IEntity pOwnerEntity, BaseInventoryStorageComponent pStorageTo, SCR_InvCallBack cb = null )
	{
		if ( !pStorageTo )
			return false;
						
		InventoryStorageSlot slot =  pStorageTo.FindSuitableSlotForItem( pOwnerEntity );
		if ( !slot )
			return false;
		if ( slot.GetAttachedEntity() )
			return TrySwapItemStorages( pOwnerEntity, slot.GetAttachedEntity(), cb );
		else
			return TryMoveItemToStorage( pOwnerEntity, pStorageTo, slot.GetID(), cb );

		return false;				
	}
	
	//------------------------------------------------------------------------------------------------
	//! try equip the item into the slot (weapon)
	void EquipWeapon( IEntity pOwnerEntity, SCR_InvCallBack cb = null, bool bFromVicinity = true )
	{
		if ( !bFromVicinity )
		{
			TrySwapItems( pOwnerEntity, m_pStorage.GetWeaponStorage(), cb );
			return;
		}
		
		IEntity user = GetOwner();
		if (!user)
			return;
		
		BaseWeaponManagerComponent weaponManager = BaseWeaponManagerComponent.Cast(user.FindComponent(BaseWeaponManagerComponent));
		if (!weaponManager)
			return;
	
		auto slot = weaponManager.GetCurrentSlot();
		int prefered = 0;
		if ( slot )
			prefered = slot.GetWeaponSlotIndex();
		
		if (EquipAny( m_pStorage.GetWeaponStorage(), pOwnerEntity, prefered, cb ))
			{
				// Play sound
				RplComponent RplComp = RplComponent.Cast(pOwnerEntity.FindComponent(RplComponent));
				if (RplComp)
					Rpc(RpcAsk_PlaySound, RplComp.Id(), "SOUND_EQUIP");
				else
				{
					SimpleSoundComponent simpleSoundComp = SimpleSoundComponent.Cast(pOwnerEntity.FindComponent(SimpleSoundComponent));
					if (simpleSoundComp)
					{
						vector mat[4];
						pOwnerEntity.GetWorldTransform(mat);
				
						simpleSoundComp.SetTransformation(mat);
						simpleSoundComp.PlayStr("SOUND_EQUIP");
					}		
		}
			}
	}

	//------------------------------------------------------------------------------------------------
	//! try equip the item into the slot (weapon)
	void EquipWeaponAttachment( IEntity pOwnerEntity, IEntity pUserEntity, SCR_InvCallBack cb = null )
	{
		auto weaponManager = BaseWeaponManagerComponent.Cast(pUserEntity.FindComponent(BaseWeaponManagerComponent));
		if (!weaponManager)
			return;
		auto slot = weaponManager.GetCurrentSlot();
		if (!slot)
			return;
		auto weaponEntity = slot.GetWeaponEntity();
		if (!weaponEntity)
			return;
		auto storage = BaseInventoryStorageComponent.Cast(weaponEntity.FindComponent(BaseInventoryStorageComponent));
		if (!storage)
			return;
		if (EquipAny(storage, pOwnerEntity, -1, cb ))
		{
			// Play sound
			RplComponent RplComp = RplComponent.Cast(pOwnerEntity.FindComponent(RplComponent));
			if (RplComp)
				Rpc(RpcAsk_PlaySound, RplComp.Id(), "SOUND_EQUIP");
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! try equip the item into the slot (gadget)
	void EquipGadget( IEntity pOwnerEntity, SCR_InvCallBack cb = null )
	{
		//(kamil) the gadget slots are now present directly on individual clothing items - will have to revise logic here if swapping is wanted
		BaseInventoryStorageComponent storageComp = FindStorageForItem(pOwnerEntity, EStoragePurpose.PURPOSE_EQUIPMENT_ATTACHMENT);
		if (storageComp)
			if (EquipAny( storageComp, pOwnerEntity, 0, cb ))
			{
				// Play sound
				RplComponent RplComp = RplComponent.Cast(pOwnerEntity.FindComponent(RplComponent));
				if (RplComp)
					Rpc(RpcAsk_PlaySound, RplComp.Id(), "SOUND_EQUIP");
				else
				{
					SimpleSoundComponent simpleSoundComp = SimpleSoundComponent.Cast(pOwnerEntity.FindComponent(SimpleSoundComponent));
					if (simpleSoundComp)
					{
						vector mat[4];
						pOwnerEntity.GetWorldTransform(mat);
						
						simpleSoundComp.SetTransformation(mat);
						simpleSoundComp.PlayStr("SOUND_EQUIP");
					}
				}
			}
	}
	
	//------------------------------------------------------------------------------------------------
	//! try equip the item into the slot (cloth)
	void EquipCloth( IEntity pOwnerEntity )
	{
		// m_pStorage because character storage is inherited from SCR_EquipedLoadoutStorageComponent
		if (EquipAny(m_pStorage, pOwnerEntity))
			{
				// Play sound
				RplComponent RplComp = RplComponent.Cast(pOwnerEntity.FindComponent(RplComponent));
				if (RplComp)
					Rpc(RpcAsk_PlaySound, RplComp.Id(), "SOUND_EQUIP");
				else
				{
					SimpleSoundComponent simpleSoundComp = SimpleSoundComponent.Cast(pOwnerEntity.FindComponent(SimpleSoundComponent));
					if (simpleSoundComp)
					{
						vector mat[4];
						pOwnerEntity.GetWorldTransform(mat);
						
						simpleSoundComp.SetTransformation(mat);
						simpleSoundComp.PlayStr("SOUND_EQUIP");
					}
				}
			}
	}
	//------------------------------------------------------------------------------------------------
	//! try equip the item into the storage at provided slot
	bool EquipAny(BaseInventoryStorageComponent storage, IEntity item, int prefered = 0, SCR_InvCallBack cb = null )
	{
		if (!storage)
			return false;
		
		bool canEquip = false;
		// If there are available slots at storage - equip there
		if ( CanInsertItemInStorage( item, storage, -1) )
		{
			// There are empty suitable slots at storage
			canEquip = TryInsertItemInStorage(item, storage, -1, cb);
		}
		else
		{
			canEquip = TryMoveItemToStorage(item, storage, -1, cb);
		}

		canEquip = TryReplaceItem(storage, item, prefered, cb);
		
		array<BaseInventoryStorageComponent> aOwnedStorages = {};
		storage.GetOwnedStorages( aOwnedStorages, 0, false );
		foreach ( BaseInventoryStorageComponent pStorage : aOwnedStorages )
		{
			if ( !pStorage )
				continue;
			canEquip = EquipAny( pStorage, item, 0, cb );
		}

		if (m_pCharacterController && canEquip)
			m_pCharacterController.TryPlayItemGesture(EItemGesture.EItemGesturePickUp);

		return canEquip;
	} 
	
	//------------------------------------------------------------------------------------------------
	//! try replace at prefered slot
	bool TryReplaceItem( BaseInventoryStorageComponent storage, IEntity item, int prefered, SCR_InvCallBack cb )
	{
		int slotCount = storage.GetSlotsCount();
		
		for ( int i = 0; i < slotCount; i++ )
		{
			int j = ( i + prefered ) % slotCount;
			if ( CanReplaceItem( item, storage, j ) )
			{
				if ( TryReplaceItem( item, storage, j, cb ) )
					return true;
			}
		}
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! try equip the item into the slot (weapon)
	void EquipItem( EquipedWeaponStorageComponent weaponStorage, IEntity weapon )
	{
		// There are empty suitable slots at weapon storage
		if ( CanInsertItemInStorage( weapon, weaponStorage, -1) )
		{
			TryInsertItemInStorage( weapon, weaponStorage, -1 );
			return;
		}
		// Otherwise try replace weapon at suitable slot
		BaseWeaponManagerComponent weaponManager = BaseWeaponManagerComponent.Cast( GetOwner().FindComponent(BaseWeaponManagerComponent) );
		if (!weaponManager)
			return;
		auto slot = weaponManager.GetCurrentSlot();
		int slotCount = weaponStorage.GetSlotsCount();
		int prefered = 0;
		if ( slot )
		{
			prefered = slot.GetWeaponSlotIndex();
		}
		for ( int i = 0; i < slotCount; i++ )
		{
			int j = ( i + prefered ) % slotCount;
			if ( CanReplaceItem( weapon, weaponStorage, j ) )
			{
				TryReplaceItem( weapon, weaponStorage, j );
				return;
			}
		}
	}
	
	void ResupplyItems(array<RplId> resupplyItemIds)
	{
		Rpc(ResupplyItemsOwner, resupplyItemIds);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void ResupplyItemsOwner(array<RplId> resupplyItemIds)
	{
		for (int i=0; i<resupplyItemIds.Count();i++)
		{
			RplComponent itemRplComponent = RplComponent.Cast(Replication.FindItem(resupplyItemIds[i]));
			if (!itemRplComponent) continue;
			
			IEntity itemEntity = itemRplComponent.GetEntity();
			if (!itemEntity) continue;
			
			BaseInventoryStorageComponent weaponStorage = FindStorageForItem(itemEntity, EStoragePurpose.PURPOSE_WEAPON_PROXY);
			if (!TryInsertItemInStorage(itemEntity, weaponStorage) && !TryInsertItem(itemEntity))
			{
				RplComponent.DeleteRplEntity(itemEntity, false);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! try to move item from owner's storage to vicinity. It might be moved to ground or to a storage in the vicinity
	void MoveItemToVicinity( IEntity pItem, BaseInventoryStorageComponent pStorageTo = null ) 
	{
	}
	
	
	//------------------------------------------------------------------------------------------------
	void OpenInventory()
	{
		auto menuManager = GetGame().GetMenuManager();
		auto menu = ChimeraMenuPreset.Inventory20Menu;
		
		auto inventoryMenu = menuManager.FindMenuByPreset( menu );
		if (inventoryMenu)
			return;
		
		menuManager.OpenMenu( menu );
		m_OnInventoryOpenInvoker.Invoke(true);
		
		if (!m_pCharacterController)
			return;
		
		// Quit ADS
		m_pCharacterController.SetWeaponADS(false);
		
		// Pin grenade
		if(m_pCharacterController.GetInputContext() && m_pCharacterController.GetInputContext().GetThrow())
			m_pCharacterController.SetThrow(false, true);
				
		// Inspection or lowered weapon stance
		m_bWasRaised = m_pCharacterController.IsWeaponRaised();
		m_pCharacterController.SetWeaponRaised(false);
	}
	
	//------------------------------------------------------------------------------------------------
	IEntity GetStorageToOpen()
	{
		IEntity result = m_pStorageToOpen;
		m_pStorageToOpen = null;

		return result;
	}

	//------------------------------------------------------------------------------------------------
	void SetStorageToOpen(IEntity storage)
	{
		m_pStorageToOpen = storage;
	}

	//------------------------------------------------------------------------------------------------
	void Action_OpenInventory()
	{
		CompartmentAccessComponent cac = m_pCharacterController.GetCharacter().GetCompartmentAccessComponent();
		if (cac && cac.IsInCompartment())
		{
			SetStorageToOpen(cac.GetCompartment().GetOwner());
		}

		OpenInventory();
	}
	
	//------------------------------------------------------------------------------------------------
	void OnInventoryMenuClosed()
	{
		m_OnInventoryOpenInvoker.Invoke(false);
		
		// Revert inspection or lowered weapon stance
		if (m_pCharacterController)
			m_pCharacterController.SetWeaponRaised(m_bWasRaised);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnStorageAdded(BaseInventoryStorageComponent storage) 
	{ 
		return;
 	}
	
	
	//------------------------------------------------------------------------------------------------
	//! Even after physics update
	//! \param owner The owner entity
	//! \param frameNumber Time passed since last frame
	/*override void EOnPostFrame(IEntity owner, float timeSlice)
	{
		m_OnPostFrameInvoker.Invoke(timeSlice);
	}*/
	
	//------------------------------------------------------------------------------------------------
	void DebugListAllItemsInInventory()
	{
		array<IEntity> items = new array<IEntity>();
		GetItems(items);
		PrintFormat( "INV: %1", "no item" );
		foreach (IEntity item : items)
		{
			InventoryItemComponent pInvComp = InventoryItemComponent.Cast(  item .FindComponent( InventoryItemComponent ) );
			if( pInvComp )
			{
				ItemAttributeCollection attribs = pInvComp.GetAttributes();
				if( !attribs )
					break;
				string sName = attribs.GetUIInfo().GetName();
				PrintFormat( "INV: %1", sName );

			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Enable/disable post frame event
	void EnablePostFrame(bool enable)
	{
		if (enable)
			SetEventMask(GetOwner(), EntityEvent.POSTFRAME);
		else
			ClearEventMask(GetOwner(), EntityEvent.POSTFRAME);
	}
	
	//------------------------------------------------------------------------------------------------
	// !
	int GetHealthComponentCount() { return m_iHealthEquipment; }
	
	//------------------------------------------------------------------------------------------------	
	//!
	IEntity GetBandageItem()
	{	
		return FindItem(m_bandagePredicate, EStoragePurpose.PURPOSE_DEPOSIT);
	}
	
	//------------------------------------------------------------------------------------------------	
	//!
	bool GetIsWeaponEquipped( IEntity pEnt )
	{
		BaseInventoryStorageComponent pWeaponStorage = m_pStorage.GetWeaponStorage();
		if ( !pWeaponStorage )
			return false;

		return pWeaponStorage.Contains( pEnt );		
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_InventoryStorageManagerComponent()
	{
		m_ERetCode = EInventoryRetCode.RETCODE_DEFAULT_STATE;
	}

	//------------------------------------------------------------------------------------------------
	bool IsAnimationReady()
	{
		if (m_pCharacterController)
			return m_pCharacterController.CanPlayItemGesture() || m_pCharacterController.IsPlayingItemGesture();
		return true;
	}

	//------------------------------------------------------------------------------------------------
	bool IsInventoryLocked()
	{
		return m_bIsInventoryLocked;	
	}
	
	//------------------------------------------------------------------------------------------------
	void SetInventoryLocked(bool isLocked)
	{
		m_bIsInventoryLocked = isLocked;
	}

#else
	void SetReturnCode( EInventoryRetCode ERetCode ) ;
	void SetReturnCodeDefault() ;
	EInventoryRetCode GetReturnCode() { return m_ERetCode; };
	float GetTotalWeightOfAllStorages();
	void SetLootStorage( IEntity pOwner );
	void InsertItem( IEntity pItem );
	void EquipWeapon( IEntity pOwnerEntity, SCR_InvCallBack cb = null, bool bFromVicinity = true );
	void EquipGadget( IEntity pOwnerEntity );
	void EquipCloth( IEntity pOwnerEntity );
	void EquipAny(BaseInventoryStorageComponent storage, IEntity item, int prefered = 0);
	void EquipItem( EquipedWeaponStorageComponent weaponStorage, IEntity weapon );
	void OpenInventory();
	void Action_OpenInventory();
	override void OnStorageAdded(BaseInventoryStorageComponent storage);
	void EnablePostFrame(bool enable);
	int GetHealthComponentCount();
	IEntity GetBandageItem();
	
#endif	
	
	
	void SCR_InventoryStorageManagerComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		#ifndef DISABLE_INVENTORY
		
		//ChimeraCharacter pChimeraChar = ChimeraCharacter.Cast( ent );
		//pChimeraChar.s_OnCharacterCreated.Insert( DebugListAllItemsInInventory );
		
		m_pStorage = SCR_CharacterInventoryStorageComponent.Cast( ent.FindComponent( CharacterInventoryStorageComponent ) );
		m_pCharacterController = CharacterControllerComponent.Cast(ent.FindComponent(CharacterControllerComponent));
		#endif
	}
};