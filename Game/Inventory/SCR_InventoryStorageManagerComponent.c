[EntityEditorProps(category: "GameScripted/UI/Inventory", description: "[MOD] Inventory Slot UI class")]
class SCR_InventoryStorageManagerComponentClass: ScriptedInventoryStorageManagerComponentClass
{
};

enum EInventoryRetCode
{
	RETCODE_OK = 0,
	RETCODE_ITEM_TOO_BIG = 2,
	RETCODE_ITEM_TOO_HEAVY = 4,
	RETCODE_DEFAULT_STATE = 0xFFFF
};

enum ECallbackState
{
	DROP = 0,
	INSERT = 1,
	MOVE = 2,
	DELETE = 3,
	FINAL = 4
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
		return (SCR_ConsumableItemComponent.Cast(queriedComponents[0])).GetConsumableType() == EConsumableType.Bandage);
	}
};

class SCR_ApplicableMedicalItemPredicate : InventorySearchPredicate
{
	IEntity characterEntity;
	ECharacterHitZoneGroup hitZoneGroup;

	void SCR_ApplicableMedicalItemPredicate()
	{
		QueryComponentTypes.Insert(SCR_ConsumableItemComponent);
	}

	override protected bool IsMatch(BaseInventoryStorageComponent storage, IEntity item, array<GenericComponent> queriedComponents, array<BaseItemAttributeData> queriedAttributes)
	{
		EConsumableType type = SCR_ConsumableItemComponent.Cast(queriedComponents[0]).GetConsumableType();
		bool isMatch = (type == EConsumableType.Bandage)
			|| (type == EConsumableType.Health)
			|| (type == EConsumableType.Tourniquet)
			|| (type == EConsumableType.Saline)
			|| (type == EConsumableType.Morphine);

		if (!isMatch)
			return false;

		SCR_ConsumableItemComponent medicalItem = SCR_ConsumableItemComponent.Cast(item.FindComponent(SCR_ConsumableItemComponent));
		SCR_ConsumableEffectHealthItems effect = SCR_ConsumableEffectHealthItems.Cast(medicalItem.GetConsumableEffect());
		if (!effect)
			return false;

		return effect.CanApplyEffectToHZ(characterEntity, characterEntity, hitZoneGroup);
	}
};

// Searches for attachments of the defined atttachmentType
class SCR_CompatibleAttachmentPredicate: InventorySearchPredicate
{
	typename attachmentType;

	void SCR_CompatibleAttachmentPredicate()
	{
		QueryComponentTypes.Insert(InventoryItemComponent);
	}

	override protected bool IsMatch(BaseInventoryStorageComponent storage, IEntity item, array<GenericComponent> queriedComponents, array<BaseItemAttributeData> queriedAttributes)
	{
		InventoryItemComponent itemComp = InventoryItemComponent.Cast(queriedComponents[0]);

		if (!itemComp)
			return false;

		ItemAttributeCollection itemAttributes = itemComp.GetAttributes();

		if (!itemAttributes)
			return false;

		WeaponAttachmentAttributes itemAttribute = WeaponAttachmentAttributes.Cast(itemAttributes.FindAttribute(WeaponAttachmentAttributes));

		if (!itemAttribute)
			return false;

		BaseAttachmentType itemAttachmentType = itemAttribute.GetAttachmentType();

		if (!itemAttachmentType)
			return false;

		typename itemAttachmentTypename = itemAttachmentType.Type();

		if (!itemAttachmentTypename)
			return false;

		return (itemAttachmentTypename.IsInherited(attachmentType)); // Check if attachment types match
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

class DropAndMoveOperationCallback: ScriptedInventoryOperationCallback
{
	InventoryItemComponent m_ItemBefore;
	InventoryItemComponent m_ItemAfter;
	InventoryStorageSlot m_TargetSlot;
	SCR_InventoryStorageManagerComponent m_Manager;
	SCR_InvCallBack m_FinalCB;
	bool m_bIstakenFromArsenal;
	bool m_bDeleteItemIfEmpty;
	ref array<IEntity> m_aItemsToMove = {};
	ECallbackState m_ECurrentState = 0; // 0 - drop, 1 - insert, 2 - move, 3 - delete, 4 - final
	
	//------------------------------------------------------------------------------------------------
	override protected void OnComplete()
	{
		switch (m_ECurrentState)
		{
			case ECallbackState.DROP:
			{
				OnDropComplete();
			}
			break;
			case ECallbackState.INSERT:
			{
				OnInsertComplete();
			}
			break;
			case ECallbackState.MOVE:
			{
				OnMoveComplete();
			}
			break;
			case ECallbackState.DELETE:
			{
				OnDeleteComplete();
			}
			break;
			case ECallbackState.FINAL:
			{
				OnFinalState();
			}
			break;
		}
	} 

	//------------------------------------------------------------------------------------------------
	protected void OnDropComplete()
	{
		m_ECurrentState += 1;
		m_Manager.TryMoveItemToStorage(m_ItemAfter.GetOwner(), m_TargetSlot.GetStorage(), m_TargetSlot.GetID(), this);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnInsertComplete()
	{
		if (!m_bIstakenFromArsenal)
		{
			m_ECurrentState = ECallbackState.FINAL;
			OnFinalState();
			return;
		} 
		
		m_ECurrentState++;
		//BaseInventoryStorageComponent itemIsStorage = BaseInventoryStorageComponent.Cast(m_ItemBefore);
		//m_Manager.GetAllItems(m_aItemsToMove, itemIsStorage);
		OnMoveComplete();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnMoveComplete()
	{
		/* Temporarily disabled for performance reasons. Once it gets fixed the transfer of items can be reenabled.
		if (m_aItemsToMove.Count() == 0)
		{
			m_ECurrentState++;
			OnDeleteComplete();
			return;
		}
		IEntity item = m_aItemsToMove[m_aItemsToMove.Count() - 1];
		m_aItemsToMove.Resize(m_aItemsToMove.Count() - 1);
		BaseInventoryStorageComponent storage = m_Manager.FindStorageForItem(item, EStoragePurpose.PURPOSE_ANY);
		if (!m_Manager.TryMoveItemToStorage(item, storage, -1, this))
		{
			OnMoveComplete();
		}
		*/
		
		
		if (m_bDeleteItemIfEmpty)
		{
			m_ECurrentState++;
			OnDeleteComplete();
		}
		else
		{		
			m_ECurrentState = ECallbackState.FINAL;
			OnFinalState();
		}
			
		return;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnDeleteComplete()
	{			
		m_aItemsToMove.Clear();
		BaseInventoryStorageComponent itemIsStorage = BaseInventoryStorageComponent.Cast(m_ItemBefore);
		m_Manager.GetAllItems(m_aItemsToMove, itemIsStorage);
			
		if (m_aItemsToMove.IsEmpty())
		{
			m_Manager.AskServerToDeleteEntity(m_ItemBefore.GetOwner());
		}
		
		OnFinalState();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnFinalState()	
	{
		if (m_FinalCB)
			m_FinalCB.InternalComplete();
	}
};
class SCR_ResupplyMagazinesCallback: ScriptedInventoryOperationCallback
{
	protected SCR_InventoryStorageManagerComponent m_Manager;
	protected ref map<ResourceName, int> m_MagazinesToSpawn = new map<ResourceName, int>();
	
	void Insert(ResourceName prefab, int count)
	{
		int currentCount;
		m_MagazinesToSpawn.Find(prefab, currentCount);
		m_MagazinesToSpawn.Insert(prefab, currentCount + count);
	}
	void Start()
	{
		OnComplete();
	}
	
	override protected void OnComplete()
	{
		if (!m_Manager)
			return;
		
		if (!m_MagazinesToSpawn.IsEmpty())
		{
			//--- Next item to process
			ResourceName prefab = m_MagazinesToSpawn.GetKey(0);
			int count = m_MagazinesToSpawn.GetElement(0);
			count--;
			
			if (count == 0)
				m_MagazinesToSpawn.Remove(prefab);
			else
				m_MagazinesToSpawn.Set(prefab, count);
			
			m_Manager.TrySpawnPrefabToStorage(prefab, cb: this);
		}
		else
		{
			//--- All processed
			OnFailed();
		}
	}
	override protected void OnFailed()
	{
		//--- Delete itself (after a delay - can't delete itself at this frame)
		GetGame().GetCallqueue().CallLater(m_Manager.EndResupplyMagazines, 1, false);
	}
	void SCR_ResupplyMagazinesCallback(SCR_InventoryStorageManagerComponent manager)
	{
		m_Manager = manager;
	}
};

class SCR_InventoryStorageManagerComponent : ScriptedInventoryStorageManagerComponent
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
	protected ref SCR_ResupplyMagazinesCallback						m_ResupplyMagazineCallback;
	
	ref ScriptInvoker<IEntity, BaseInventoryStorageComponent>	m_OnItemAddedInvoker		= new ref ScriptInvoker<IEntity, BaseInventoryStorageComponent>();
	ref ScriptInvoker<IEntity, BaseInventoryStorageComponent>	m_OnItemRemovedInvoker		= new ref ScriptInvoker<IEntity, BaseInventoryStorageComponent>();

	ref ScriptInvoker<bool> 									m_OnInventoryOpenInvoker	= new ref ScriptInvoker<bool>();
	ref ScriptInvoker<bool> 									m_OnQuickBarOpenInvoker		= new ref ScriptInvoker<bool>();
	
	protected EventHandlerManagerComponent m_pEventHandlerManager;

	// Callback when item is added (will be performed locally after server completed the Insert/Move operation)
	override protected void OnItemAdded(BaseInventoryStorageComponent storageOwner, IEntity item)
	{		
		super.OnItemAdded(storageOwner, item);
		
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
		super.OnItemRemoved(storageOwner, item);
		
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
	override protected bool ShouldForbidRemoveByInstigator(InventoryStorageManagerComponent instigatorManager, BaseInventoryStorageComponent fromStorage, IEntity item)
	{
		if (m_pCharacterController && !m_pCharacterController.IsUnconscious())
			return true;
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void PlayItemSound(IEntity entity, string soundEvent)
	{	
		if (!entity)
			return;

		RplComponent rplComp = RplComponent.Cast(entity.FindComponent(RplComponent));
		if (rplComp)
			Rpc(RpcAsk_PlaySound, rplComp.Id(), soundEvent);
		else
		{
			SoundComponent soundComp = SoundComponent.Cast(entity.FindComponent(SoundComponent));
			if (!soundComp)
				return;
			
			soundComp.SoundEvent(soundEvent);
		}
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
		
		SoundComponent soundComp = SoundComponent.Cast(entity.FindComponent(SoundComponent));
		if (!soundComp)
			return;
		
		soundComp.SoundEvent(soundAction);
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
				
		foreach ( BaseInventoryStorageComponent storage: storages )
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
			string soundEvent = SCR_SoundEvent.SOUND_EQUIP;
			//TryInsertItem( pItem, EStoragePurpose.PURPOSE_DEPOSIT);	// works for the owned storages ( not for the vicinity storages )
			if ( !TryInsertItem( pItem, EStoragePurpose.PURPOSE_WEAPON_PROXY, cb ) )
			{
				if ( !TryInsertItem( pItem, EStoragePurpose.PURPOSE_DEPOSIT, cb ) )
				{
					if ( !TryMoveItemToStorage( pItem, FindStorageForItem( pItem, EStoragePurpose.PURPOSE_ANY ), -1, cb ) )
						canInsert = TryMoveItemToStorage( pItem, m_pStorage, -1, cb ); 				// clothes from storage in vicinity
					else
						soundEvent = SCR_SoundEvent.SOUND_PICK_UP;	// play pick up sound for everything else
				
					SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_INV_HOTKEY_CONFIRM);
				}
				else
					soundEvent = SCR_SoundEvent.SOUND_PICK_UP;
			}
			
			if (canInsert)
				PlayItemSound(pItem, soundEvent);
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
				else
					SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_INV_DROP_ERROR);
			}
			if ( FindStorageForInsert( pItem, pStorageTo, EStoragePurpose.PURPOSE_ANY ) )
				pStorageTo = FindStorageForInsert( pItem, pStorageTo, EStoragePurpose.PURPOSE_ANY );
			
			if ( !pStorageFrom )
			{
				TryInsertItemInStorage( pItem, pStorageTo, -1, cb );	// if we move item from ground to opened storage
			}
			else
				canInsert = TryMoveItemToStorage( pItem, pStorageTo, -1, cb );		// if we move item between storages
		}
		
		if (!canInsert)
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_INV_DROP_ERROR);
		
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
			if (!TrySwapItems( pOwnerEntity, m_pStorage.GetWeaponStorage(), cb ))
				SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_INV_DROP_ERROR);
			else
				SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_INV_CONTAINER_DIFR_DROP);
			
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
		
		if (!EquipAny( m_pStorage.GetWeaponStorage(), pOwnerEntity, prefered, cb ))
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_INV_DROP_ERROR);
		
		if (cb && cb.m_pStorageFrom != cb.m_pStorageTo)
				SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_INV_CONTAINER_DIFR_DROP);
			else
				SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_INV_CONTAINER_SAME_DROP);
		
		// Play sound
		PlayItemSound(pOwnerEntity, SCR_SoundEvent.SOUND_EQUIP);
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
		
		EquipAny(storage, pOwnerEntity, -1, cb );
		
		// Play sound
		PlayItemSound(pOwnerEntity, SCR_SoundEvent.SOUND_EQUIP);
	}
	
	//------------------------------------------------------------------------------------------------
	//! try equip the item into the slot (gadget)
	void EquipGadget( IEntity pOwnerEntity, SCR_InvCallBack cb = null )
	{
		//(kamil) the gadget slots are now present directly on individual clothing items - will have to revise logic here if swapping is wanted
		BaseInventoryStorageComponent storageComp = FindStorageForItem(pOwnerEntity, EStoragePurpose.PURPOSE_EQUIPMENT_ATTACHMENT);
		if (storageComp)
		{
			EquipAny( storageComp, pOwnerEntity, -1, cb );
			
			// Play sound
			PlayItemSound(pOwnerEntity, SCR_SoundEvent.SOUND_EQUIP);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! try equip the item into the slot (cloth)
	void EquipCloth( IEntity pOwnerEntity )
	{
		// m_pStorage because character storage is inherited from SCR_EquipedLoadoutStorageComponent
		EquipAny(m_pStorage, pOwnerEntity);
		
		// Play sound
		PlayItemSound(pOwnerEntity, SCR_SoundEvent.SOUND_EQUIP);
	}
	
	//------------------------------------------------------------------------------------------------
	//! try equip the item into the storage at provided slot
	bool EquipAny(BaseInventoryStorageComponent storage, IEntity item, int prefered = -1, SCR_InvCallBack cb = null)
	{
		if (!storage || !item)
			return false;
		InventoryItemComponent itemComp = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
		if (!itemComp)
			return false;
		
		InventoryStorageSlot m_TargetSlot = storage.FindSuitableSlotForItem(item);
		// Storage doesn't have suitable slot for item (therefore any future opearation would fail)
		if (!m_TargetSlot)
			return false;

		if (prefered > 0 && prefered < storage.GetSlotsCount())
		{
			m_TargetSlot = storage.GetSlot(prefered);
		}
		else
		{
			prefered = m_TargetSlot.GetID();
		}
		

		InventoryStorageSlot sourceSlot = itemComp.GetParentSlot();
		// Item is on the ground as it does not belong to any storage (eg is not in the slot)
		if (!sourceSlot || !sourceSlot.GetStorage())
		{
			// we are picking up item from ground
			// if target slot is not empty return the result of replace operation
			if (m_TargetSlot.GetAttachedEntity())
				return TryReplaceItem(item, storage, prefered, cb);
			// we are picking up item from ground into empty slot, simply return result of the insert operation
			return TryInsertItemInStorage(item, storage, prefered, cb);
		}

		// our target slot is empty and we moving item from another storage
		// simply return the result of move operation
		if (!m_TargetSlot.GetAttachedEntity())
			return TryMoveItemToStorage(item, storage, prefered, cb);

		BaseInventoryStorageComponent sourceStorage = sourceSlot.GetStorage();
		bool istakenFromArsenal = sourceStorage.GetOwner().FindComponent(SCR_ArsenalComponent);
		bool isTakenFromBody = false;
		
		ChimeraCharacter lootedBodyCharacter = ChimeraCharacter.Cast(sourceStorage.GetOwner());
		DamageManagerComponent lootedBodyDamageManager;
		
		if (lootedBodyCharacter)
			lootedBodyDamageManager = lootedBodyCharacter.GetDamageManager();
		
		if (lootedBodyDamageManager)
			isTakenFromBody = lootedBodyDamageManager.GetState() == EDamageState.DESTROYED;
		
		bool performDropOfOriginalItem = isTakenFromBody || istakenFromArsenal || sourceStorage.GetOwner().FindComponent(SCR_CampaignArmoryStorageComponent) ;
		
		// If we don't want to drop item
		if (performDropOfOriginalItem)
		{
			// if we want to drop originally equipped item
			// here sequence would be as follows:
			// 1 - drop original item
			// 2 - insert item to target storage
			// 3 - try move as many items as possible from dropped item back to inventory
			// 4 - delete dropped item
	
			// At first - let's validate if this is even possible
			if (!CanSwapItemStorages(item, m_TargetSlot.GetAttachedEntity()))
				return false;
	
			DropAndMoveOperationCallback chainedCallback = new DropAndMoveOperationCallback();
			chainedCallback.m_Manager = this;
			chainedCallback.m_ItemAfter = itemComp;
			chainedCallback.m_ItemBefore = InventoryItemComponent.Cast(m_TargetSlot.GetAttachedEntity().FindComponent(InventoryItemComponent));
			chainedCallback.m_TargetSlot = m_TargetSlot;
			chainedCallback.m_FinalCB = cb;
			chainedCallback.m_bDeleteItemIfEmpty = true;
			chainedCallback.m_bIstakenFromArsenal = istakenFromArsenal;
	
			return TryRemoveItemFromStorage(m_TargetSlot.GetAttachedEntity(), m_TargetSlot.GetStorage(), chainedCallback);
		}
		
		// we return the result of swap opoeration were item from slotA will be transfered to slotB and item from slotB to slotA
		return TrySwapItemStorages(item, m_TargetSlot.GetAttachedEntity(), cb);
	} 
	
	//------------------------------------------------------------------------------------------------
	//! try to drop the original item and replace it by itemToReplace at the slot specified by slotID
	bool TryReplaceAndDropItemAtSlot(BaseInventoryStorageComponent storage, IEntity item, int slotID, SCR_InvCallBack cb = null, bool isTakenFromArsenal = false, bool deleteOriginalItemIfEmpty = false)
	{
		if (!storage || !item)
			return false;
		InventoryItemComponent itemComp = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
		if (!itemComp)
			return false;
		
		InventoryStorageSlot m_TargetSlot = storage.GetSlot(slotID);
		// Storage doesn't have suitable slot for item (therefore any future opearation would fail)
		if (!m_TargetSlot)
			return false;		

		InventoryStorageSlot sourceSlot = itemComp.GetParentSlot();
		// Item is on the ground as it does not belong to any storage (eg is not in the slot)
		if (!sourceSlot || !sourceSlot.GetStorage())
		{
			// we are picking up item from ground
			// if target slot is not empty return the result of replace operation
			if (m_TargetSlot.GetAttachedEntity())
				return TryReplaceItem(item, storage, slotID, cb);
			// we are picking up item from ground into empty slot, simply return result of the insert operation
			return TryInsertItemInStorage(item, storage, slotID, cb);
		}

		// our target slot is empty and we moving item from another storage
		// simply return the result of move operation
		if (!m_TargetSlot.GetAttachedEntity())
			return TryMoveItemToStorage(item, storage, slotID, cb);


		// if we want to drop originally equipped item
		// here sequence would be as follows:
		// 1 - drop original item
		// 2 - insert item to target storage
		// 3 - try move as many items as possible from dropped item back to inventory
		// 4 - delete dropped item

		// At first - let's validate if this is even possible
		if (!CanSwapItemStorages(item, m_TargetSlot.GetAttachedEntity()))
			return false;

		DropAndMoveOperationCallback chainedCallback = new DropAndMoveOperationCallback();
		chainedCallback.m_Manager = this;
		chainedCallback.m_ItemAfter = itemComp;
		chainedCallback.m_ItemBefore = InventoryItemComponent.Cast(m_TargetSlot.GetAttachedEntity().FindComponent(InventoryItemComponent));
		chainedCallback.m_TargetSlot = m_TargetSlot;
		chainedCallback.m_bDeleteItemIfEmpty = deleteOriginalItemIfEmpty; 
		chainedCallback.m_bIstakenFromArsenal = isTakenFromArsenal;
		chainedCallback.m_FinalCB = cb;
		
		return TryRemoveItemFromStorage(m_TargetSlot.GetAttachedEntity(), m_TargetSlot.GetStorage(), chainedCallback);
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
	
	//------------------------------------------------------------------------------------------------
	/*!
	Resupply all weapons so they have given number of magazines.
	/param maxMagazineCount Desired number of magazines
	*/
	void ResupplyMagazines(int maxMagazineCount = 4)
	{
		Rpc(ResupplyMagazinesOwner, maxMagazineCount);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void ResupplyMagazinesOwner(int maxMagazineCount)
	{
		BaseWeaponManagerComponent weaponsManager = BaseWeaponManagerComponent.Cast(GetOwner().FindComponent(BaseWeaponManagerComponent));
		
		array<IEntity> weaponList = {};
		weaponsManager.GetWeaponsList(weaponList);
		
		if (!m_ResupplyMagazineCallback)
			m_ResupplyMagazineCallback = new SCR_ResupplyMagazinesCallback(this);
		
		foreach (IEntity weapon: weaponList)
		{
			BaseWeaponComponent comp = BaseWeaponComponent.Cast(weapon.FindComponent(BaseWeaponComponent));
			string weaponSlotType = comp.GetWeaponSlotType();
			
			// Only refill primary and secondary weapons
			if (!(weaponSlotType == "primary" || weaponSlotType == "secondary"))
				continue;
			
			int resupplyCount = maxMagazineCount - GetMagazineCountByWeapon(comp);
			if (resupplyCount <= 0)
				continue;
			
			MuzzleComponent muzzleComponent = MuzzleComponent.Cast(comp.GetCurrentMuzzle());
			if (!muzzleComponent)
				continue;
			
			ResourceName magazinePrefab = muzzleComponent.GetDefaultMagazinePrefab().GetResourceName();
			m_ResupplyMagazineCallback.Insert(magazinePrefab, resupplyCount);
		}
		
		m_ResupplyMagazineCallback.Start();
	}
	void EndResupplyMagazines()
	{
		delete m_ResupplyMagazineCallback;
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
		m_pCharacterController.SetGadgetRaisedModeWanted(false);
		
		// Pin grenade
		if(m_pCharacterController.GetInputContext() && m_pCharacterController.GetInputContext().GetThrow())
			m_pCharacterController.SetThrow(false, true);
				
		// Inspection or lowered weapon stance
		m_bWasRaised = m_pCharacterController.IsWeaponRaised();
		m_pCharacterController.SetWeaponRaised(false);
	}

	//------------------------------------------------------------------------------------------------
	void CloseInventory()
	{
		auto menuManager = GetGame().GetMenuManager();
		auto inventoryMenu = menuManager.FindMenuByPreset(ChimeraMenuPreset.Inventory20Menu);
		if (inventoryMenu)
			menuManager.CloseMenu(inventoryMenu);
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
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_ServerToDeleteEntity(RplId targetRplId)
	{
	    RplComponent rplComp = RplComponent.Cast(Replication.FindItem(targetRplId));
        if (!rplComp)
            return;
        IEntity entity = rplComp.GetEntity();
        if (!entity)
            return;
      	RplComponent.DeleteRplEntity(entity, false);	
	}
	
	
	//------------------------------------------------------------------------------------------------
	void AskServerToDeleteEntity(IEntity ent)
	{
		RplComponent rplComp = RplComponent.Cast(ent.FindComponent(RplComponent));
		
		if (!rplComp)
			return;
			
		RplId rplId = rplComp.Id();
		Rpc(RpcAsk_ServerToDeleteEntity, rplId);
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
	void ~SCR_InventoryStorageManagerComponent()
	{
		m_ERetCode = EInventoryRetCode.RETCODE_DEFAULT_STATE;
		if (m_pEventHandlerManager)
			m_pEventHandlerManager.RemoveScriptHandler("OnConsciousnessChanged", this, OnConsciousnessChanged);
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
	
	//------------------------------------------------------------------------------------------------
	int GetAllItems(inout array<IEntity> items, BaseInventoryStorageComponent storage)
	{
		if (!storage || !items)
			return 0;
			
		int count = 0;
		
		if (!ClothNodeStorageComponent.Cast(storage))
		{
			count = storage.GetAll(items);
		}
		array<BaseInventoryStorageComponent> itemToReplaceAttachedStorages = {};
		storage.GetOwnedStorages(itemToReplaceAttachedStorages, 1, false);
		foreach (BaseInventoryStorageComponent attachedStorage : itemToReplaceAttachedStorages)
		{			
			if (ClothNodeStorageComponent.Cast(attachedStorage))
			{
				attachedStorage.GetOwnedStorages(itemToReplaceAttachedStorages, 1, false);
			}
			else
			{
				count += attachedStorage.GetAll(items);
			}
		}
			
		return count;
	}
	
	//------------------------------------------------------------------------------------------------	
	//! Get available prefabs of weapons of specified types
	void GetWeaponPrefabsOfType(notnull array<IEntity> weapons, EWeaponType weaponType, notnull out array<EntityPrefabData> prefabs)
	{
		BaseWeaponComponent weapon;
		EntityPrefabData prefabData;
		foreach (IEntity item: weapons)
		{
			weapon = BaseWeaponComponent.Cast(item.FindComponent(BaseWeaponComponent));
			if (weapon.GetWeaponType() != weaponType)
				continue;
			
			// Ignore currently selected items
			prefabData = item.GetPrefabData();
			if (prefabData && !prefabs.Contains(prefabData))
				prefabs.Insert(prefabData);
		}
	}
	
	//------------------------------------------------------------------------------------------------	
	//! Find next weapon of specified types, excluding currentItem, but including it in sorting
	IEntity FindNextWeaponOfType(EWeaponType weaponType, IEntity currentItem = null)
	{
		array<EntityPrefabData> prefabs = {};
		
		// Currently selected weapon may be outside of inventory and it has to be considered for sorting
		EntityPrefabData currentPrefab;
		BaseWeaponComponent currentWeapon;
		if (currentItem)
		{
			currentPrefab = currentItem.GetPrefabData();
			currentWeapon = BaseWeaponComponent.Cast(currentItem.FindComponent(BaseWeaponComponent));
			
			if (currentPrefab && currentWeapon.GetWeaponType() == weaponType)
				prefabs.Insert(currentPrefab);
		}
		
		// Collect all the matching prefabs
		array<IEntity> items = {};
		FindItemsWithComponents(items, {BaseWeaponComponent});
		GetWeaponPrefabsOfType(items, weaponType, prefabs);
		
		// No valid prefabs
		if (prefabs.IsEmpty())
			return null;
		
		// TODO: better sorting, perhaps by name
		prefabs.Sort();
		
		/*
		foreach (EntityPrefabData prefab: prefabs)
			Print(prefab.GetPrefabName(), LogLevel.WARNING);
		*/
		
		// Select next prefab
		int nextPrefabID = (prefabs.Find(currentPrefab) + 1) % prefabs.Count();
		EntityPrefabData nextPrefab = prefabs[nextPrefabID];
		
		// Return nothing if prefab is unchanged
		if (nextPrefab == currentPrefab)
			return null;
		
		// Select the weapon that matches the type to be selected
		foreach (IEntity item: items)
		{
			if (item && item.GetPrefabData() == nextPrefab)
				return item;
		}
		
		return null;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnConsciousnessChanged(bool conscious)
	{
		if (!conscious)
		{
			CloseInventory();

			ChimeraCharacter character = ChimeraCharacter.Cast(GetOwner());
			if (!character)
				return;

			AIControlComponent aiControl = AIControlComponent.Cast(GetOwner().FindComponent(AIControlComponent));
			if (!aiControl || !aiControl.IsAIActivated())
				return;

			CharacterControllerComponent charCtrl = character.GetCharacterController();
			if (!charCtrl)
				return;

			IEntity currentWeapon;
			BaseWeaponManagerComponent wpnMan = BaseWeaponManagerComponent.Cast(character.FindComponent(BaseWeaponManagerComponent));
			if (wpnMan && wpnMan.GetCurrentWeapon())
				currentWeapon = wpnMan.GetCurrentWeapon().GetOwner();

			if (currentWeapon)
				charCtrl.TryEquipRightHandItem(currentWeapon, EEquipItemType.EEquipTypeSlinged, true);
		}
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
		m_pEventHandlerManager = EventHandlerManagerComponent.Cast(ent.FindComponent(EventHandlerManagerComponent));
		if (m_pEventHandlerManager)
		{
			m_pEventHandlerManager.RegisterScriptHandler("OnConsciousnessChanged", this, OnConsciousnessChanged);
		}
		#endif
	}
};