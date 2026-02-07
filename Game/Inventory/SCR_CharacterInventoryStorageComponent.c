[ComponentEditorProps(category: "GameScripted/Inventory", description: "Inventory 2.0", color: "0 0 255 255", icon: HYBRID_COMPONENT_ICON)]
class SCR_CharacterInventoryStorageComponentClass: CharacterInventoryStorageComponentClass
{
};

//------------------------------------------------------------------------------------------------
//! default scritped storage for the character

enum EItemType
{
	IT_NONE = 16385
};

class SCR_CharacterInventoryStorageComponent : CharacterInventoryStorageComponent
{
	//TODO: define this on loadout level. This is temporary and will be removed!
	[Attribute( "0", UIWidgets.EditBox, "How much weight the character can carry")]
	protected float m_fMaxWeight;
	
	//TODO: define this on loadout level. This is temporary and will be removed!
	[Attribute( "0", UIWidgets.EditBox, "How much volume the character can carry")]
	protected float m_fMaxVolume;
	
	#ifndef DISABLE_INVENTORY
	
	private BaseInventoryStorageComponent m_pLootStorage;
	protected ref array<IEntity>						m_aQuickSlots = { null, null, null, null, null, null, null, null, null, null };
	protected ref array<IEntity>						m_aWeaponQuickSlotsStorage = {}; //Is used to store first four quickslots of turrets.
	protected ref array< int >							m_aQuickSlotsHistory = new array< int >();	//here we'll be remembering the items stored
	// protected ref array<EntityComponentPrefabData>		m_aPrefabsData = { null, null, null, null, null, null, null, null, null, null }; // todo: figure out the intentions
	protected string				 					m_pLoadout;
	private const int									GADGET_OFFSET = 9999;	//EWeaponType && EGadgetType might be have the same number, offset it ( not nice (i agree) )
	/*
	protected ref array<ref array<int>>					m_aDefaultRiflemanQuickSlots = 	{	{ EWeaponType.WT_RIFLE, EWeaponType.WT_SNIPERRIFLE, EWeaponType.WT_MACHINEGUN },
																							{ EWeaponType.WT_HANDGUN, EWeaponType.WT_ROCKETLAUNCHER, EWeaponType.WT_GRENADELAUNCHER, ( EGadgetType.BINOCULARS ) + GADGET_OFFSET  },
																							{ ( EGadgetType.FLASHLIGHT ) + GADGET_OFFSET },
																							{ EWeaponType.WT_FRAGGRENADE, EWeaponType.WT_SMOKEGRENADE },
																							{ ( EGadgetType.MAP ) + GADGET_OFFSET },
																							{ ( EGadgetType.COMPASS ) + GADGET_OFFSET } 
																						};
	*/
	protected ref array<ref array<int>>					m_aDefaultQuickSlots = 			{	{ EWeaponType.WT_RIFLE, EWeaponType.WT_SNIPERRIFLE, EWeaponType.WT_MACHINEGUN },
																							{ EWeaponType.WT_RIFLE, EWeaponType.WT_ROCKETLAUNCHER, EWeaponType.WT_GRENADELAUNCHER, EWeaponType.WT_SNIPERRIFLE, EWeaponType.WT_MACHINEGUN },
																							{ EWeaponType.WT_HANDGUN },
																							{ EWeaponType.WT_FRAGGRENADE, EWeaponType.WT_SMOKEGRENADE },
																							{ EGadgetType.CONSUMABLE + GADGET_OFFSET },
																							{ EGadgetType.FLASHLIGHT + GADGET_OFFSET },
																							{ EGadgetType.BINOCULARS + GADGET_OFFSET },
																							{ EGadgetType.MAP + GADGET_OFFSET },
																							{ EGadgetType.COMPASS + GADGET_OFFSET },
																							{ EItemType.IT_NONE }
																						};

	protected ref array<BaseInventoryStorageComponent> m_aStoragesInStorageList = {};		//here we remember the opened storages in the Inventory menu ( in the Storages list area )
	protected SCR_CompartmentAccessComponent m_CompartmentAcessComp;
	
	//------------------------------------------------------------------------ USER METHODS ------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//TODO: define this on loadout level. This is temporary and will be removed!
	float GetMaxLoad() { return m_fMaxWeight; }
	
	//------------------------------------------------------------------------------------------------
	// !
	BaseInventoryStorageComponent GetWeaponStorage()
	{
		auto genericComponent = GetOwner().FindComponent( EquipedWeaponStorageComponent );
		if (!genericComponent)
			return null;
		return BaseInventoryStorageComponent.Cast( genericComponent );
	}
	
	//------------------------------------------------------------------------------------------------
	// !
	InventoryItemComponent GetItemFromLoadoutSlot( ELoadoutArea eSlot )
	{
		// Not all attached Entities to slots are storages.
		// For instance boots - even though attached to character storage slot by themselves represent item, not storage
		// However if entity attached to slot it is guaranteed to have InventoryItemComponent
		InventoryStorageSlot slot = GetSlotForArea(eSlot);
		if (!slot)
			return null;
		auto entity = slot.GetAttachedEntity();
		if (!entity)
			return null;
		auto genericComponent = entity.FindComponent( InventoryItemComponent );
		if (!genericComponent)
			return null;
		return InventoryItemComponent.Cast( genericComponent );
	}
	//------------------------------------------------------------------------------------------------
	// !
	BaseInventoryStorageComponent GetStorageFromLoadoutSlot( ELoadoutArea eSlot )
	{
		return BaseInventoryStorageComponent.Cast( GetItemFromLoadoutSlot(eSlot) );
	}
	
	//------------------------------------------------------------------------------------------------
	// 
	protected bool HasStorageComponent( IEntity pEntity )
	{
		return GetStorageComponentFromEntity( pEntity ) != null;	
	}
	
	
	//------------------------------------------------------------------------------------------------
	// ! returns all topmost storages
	void GetStorages( out notnull array<SCR_UniversalInventoryStorageComponent> storagesInInventory )
	{
		array<IEntity> pEntities = new array<IEntity>();
		int iNrOfStorages = GetAll( pEntities );
		
		foreach( IEntity pEntity: pEntities )
		{
			SCR_UniversalInventoryStorageComponent pUniComp = GetStorageComponentFromEntity( pEntity );
			if( pUniComp )
				storagesInInventory.Insert( pUniComp );
		}
	}
		

	//------------------------------------------------------------------------------------------------
	// ! get the item inventory component 
	SCR_UniversalInventoryStorageComponent GetStorageComponentFromEntity( IEntity pEntity )
	{
		if ( pEntity == null )
			return null;
		
		return SCR_UniversalInventoryStorageComponent.Cast(pEntity.FindComponent( SCR_UniversalInventoryStorageComponent ));	
	}

	//------------------------------------------------------------------------------------------------
	// !
	void SetLootStorage( IEntity pOwner )
	{
		if( !pOwner )
		{
			m_pLootStorage = null;
			return;
		}
		m_pLootStorage = BaseInventoryStorageComponent.Cast( pOwner.FindComponent( BaseInventoryStorageComponent ) );
	}

	BaseInventoryStorageComponent GetLootStorage()
	{
		return m_pLootStorage;
	}
	
	//------------------------------------------------------------------------------------------------
	// ! returns the visibility state of the adequate UI container - if the storage was previously shown in the inventory
	bool GetIsStorageShown( notnull BaseInventoryStorageComponent pStorage )
	{
		return m_aStoragesInStorageList.Find( pStorage ) != -1;
	}
	//------------------------------------------------------------------------------------------------
	// ! 
	void SetStorageAsShown( notnull BaseInventoryStorageComponent pStorage )
	{
		if ( !GetIsStorageShown( pStorage ) )
		{
			m_aStoragesInStorageList.Insert( pStorage );
		}
	}
	//------------------------------------------------------------------------------------------------
	// !
	void SetStorageAsHidden( notnull BaseInventoryStorageComponent pStorage )
	{
		m_aStoragesInStorageList.RemoveItem( pStorage );
	}
	
	//------------------------------------------------------------------------------------------------
	// !
	int GetItemType( IEntity pItem )
	{
		int iItemType = -1;
		//Weapons:
		BaseWeaponComponent weaponComponent = BaseWeaponComponent.Cast( pItem.FindComponent( BaseWeaponComponent ) );
		if ( weaponComponent )
		{
			iItemType = weaponComponent.GetWeaponType();
		}
		else
		{
			//Gadgets:
			SCR_GadgetComponent gadgetComponent = SCR_GadgetComponent.Cast( pItem.FindComponent( SCR_GadgetComponent ) );
			if ( gadgetComponent )
			{
				iItemType = gadgetComponent.GetType() + GADGET_OFFSET;
			}
		}
				
		return iItemType;
	}
	
	//------------------------------------------------------------------------------------------------
	// !
	bool ItemBelongsToSlot( int iItemType, int iSlotIndex )
	{
		return m_aDefaultQuickSlots[ iSlotIndex ].Find( iItemType ) != -1;
	}
	
	//------------------------------------------------------------------------------------------------
	// !
	void StoreItemToQuickSlot( notnull IEntity pItem, int iSlotIndex = -1, bool isForced = false )
	{
		int iItemType = GetItemType( pItem );
		if ( iSlotIndex == -1 ) //we don't know what slot we put the item into. Check first if we remember the type of the item
		{	
			for ( int iLoop = 0; iLoop < m_aQuickSlots.Count(); iLoop ++ )	//go through the all quick slots
			{
				if ( m_aQuickSlots[ iLoop ] ) //do it only for the empty slots
					continue;
				if ( iItemType == m_aQuickSlotsHistory[ iLoop ] ) // there was aready something in the slot with this index
				{
					iSlotIndex = iLoop;
					break;
				}	
				else
				{
					//there was nothing before, put the item into slot defined by the template ( m_aDefaultQuickSlots )
					if ( ItemBelongsToSlot( iItemType, iLoop ) )
					{
						iSlotIndex = iLoop;
						break;
					}
				}					
			}
		}
				
		if ( iSlotIndex == - 1 )	//any suitable slot not found, do not insert into quick slot
			return;
		
		if (!isForced)
		{
			ChimeraCharacter character = ChimeraCharacter.Cast(GetOwner());
			if (!character)
				return;
			
			CompartmentAccessComponent compAccessComponent = character.GetCompartmentAccessComponent();
			if (!compAccessComponent)
				return;
			
			TurretCompartmentSlot turretCompartment = TurretCompartmentSlot.Cast(compAccessComponent.GetCompartment());
			if (turretCompartment)
			{
				if (iSlotIndex < 4)
					return;
				
				array<IEntity> turretWeapons = {};
				GetTurretWeaponsList(turretWeapons, turretCompartment);
				if (turretWeapons.Contains(pItem))
					return;
			}
		}
		
		if ( pItem == m_aQuickSlots.Get( iSlotIndex ) )	//if there's already the item in the slot, remove it
		{
			m_aQuickSlots.Set( iSlotIndex, null );
		}
		else
		{
			int iOldIndex = RemoveItemFromQuickSlot( pItem );
			if ( 0 <= iOldIndex && iOldIndex < m_aQuickSlotsHistory.Count() )
				m_aQuickSlotsHistory[ iOldIndex ] = 0;	//in case the item is already in slot and we shift the item into a different slot
			m_aQuickSlots.Set( iSlotIndex, pItem );
			
			m_aQuickSlotsHistory[ iSlotIndex ] = iItemType; // remember it
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// !
	int RemoveItemFromQuickSlot( IEntity pItem )
	{
		int iIndex = m_aQuickSlots.Find( pItem );
		if ( iIndex != -1 )
			m_aQuickSlots.Set( iIndex, null );
		
		return iIndex;
	}
	
	//------------------------------------------------------------------------------------------------
	// !
	void RemoveItemFromQuickSlotAtIndex(int iIndex)
	{
		if (iIndex >= 0 && m_aQuickSlots.Get(iIndex))
			m_aQuickSlots.Set(iIndex, null);
	}
	
	//------------------------------------------------------------------------------------------------
	// !
	array<IEntity> GetQuickSlotItems() 
	{ 
		return m_aQuickSlots; 
	}
	
	
	//------------------------------------------------------------------------------------------------
	// ! 
	protected override void OnAddedToSlot(IEntity item, int slotID)
	{
		// Loadout manager is taking care of this since there are some items that shouldn't be visible when attached to slot, some have different meshes for different states.
		// Consider glasses in first person view - they deffinitely should be disabled
		// it is slightly more complex then this
			
		
		InventoryItemComponent itemComponent = InventoryItemComponent.Cast( item .FindComponent( InventoryItemComponent ) );
		if ( itemComponent == null ) 
			return;
		
		SCR_UniversalInventoryStorageComponent storageComponent = GetStorageComponentFromEntity( item );
		if( !storageComponent )
			return;
		
		auto attr = SCR_ItemAttributeCollection.Cast ( storageComponent.GetAttributes() );
		
		if( !attr )
			return;
		UIInfo UIinfoItem = attr.GetUIInfo();
		if( !UIinfoItem )
			return;
		#ifdef DEBUG_INVENTORY20
			PrintFormat( "INV: item %1 was added. It's weight is: %2, and total weight of item/storage is: %3", UIinfoItem.GetName(), attr.GetWeight(), storageComponent.GetTotalWeight() );
		#endif
		
	}
	
	//------------------------------------------------------------------------------------------------
	// !
	void HandleOnItemAddedToInventory( IEntity item, BaseInventoryStorageComponent storageOwner )
	{
		StoreItemToQuickSlot( item );
	}
	
	//------------------------------------------------------------------------------------------------
	// !
	void HandleOnItemRemovedFromInventory( IEntity item, BaseInventoryStorageComponent storageOwner )
	{
		RemoveItemFromQuickSlot( item );
	}
	
	protected void GetPlayersWeapons( inout array<IEntity> outWeapons )
	{
		BaseWeaponManagerComponent pWeaponManager = BaseWeaponManagerComponent.Cast( GetOwner().FindComponent( BaseWeaponManagerComponent ) );
		if ( !pWeaponManager )
			return;
		pWeaponManager.GetWeaponsList( outWeapons );
	}
	
	//------------------------------------------------------------------------------------------------
	// !
	void InitAsPlayer(IEntity pOwner, bool pControlled)
	{
		auto pInventoryManager = SCR_InventoryStorageManagerComponent.Cast( GetOwner().FindComponent( SCR_InventoryStorageManagerComponent ) );
		if ( !pInventoryManager )
			return;
			
		if( pControlled )
		{
			int i = m_aDefaultQuickSlots.Count();	
			if ( m_aQuickSlotsHistory.Count() <  i )
			{
				for ( int m = 0; m < i; m++ )
					m_aQuickSlotsHistory.Insert( 0 );	//default is none	
			}
			
			//Insert weapons to quick slots as first, because they can't be equipped into hands directly from different storage than EquippedWeaponStorage
			//TODO: optimalize ( hotfix for ability to select grenades from quickslot )
			
			array<IEntity> outWeapons = {};
			GetPlayersWeapons( outWeapons );
			foreach ( IEntity weapon : outWeapons )
			{
				StoreItemToQuickSlot( weapon );
			}
		
			
			array<IEntity> outItems = {};
			pInventoryManager.GetItems( outItems );
			foreach ( auto pItem : outItems )
			{
				BaseWeaponComponent pWeaponSlotComp = BaseWeaponComponent.Cast( pItem.FindComponent( BaseWeaponComponent ) );
				if ( pWeaponSlotComp )
					continue;
				StoreItemToQuickSlot( pItem );
			}
			
			
			
			pInventoryManager.m_OnItemAddedInvoker.Insert( HandleOnItemAddedToInventory );
			pInventoryManager.m_OnItemRemovedInvoker.Insert( HandleOnItemRemovedFromInventory );
			
			ChimeraCharacter character = ChimeraCharacter.Cast(pOwner);
			if (!character)
				return;

			SCR_CharacterControllerComponent charController = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
			if (charController)
				charController.m_OnItemUseCompleteInvoker.Insert(OnItemUsed);
			
			m_CompartmentAcessComp = SCR_CompartmentAccessComponent.Cast(character.GetCompartmentAccessComponent());
			if (!m_CompartmentAcessComp)
				return;
			
			if (m_CompartmentAcessComp)
			{
				m_CompartmentAcessComp.GetOnCompartmentEntered().Insert(OnCompartmentEntered);
				m_CompartmentAcessComp.GetOnCompartmentLeft().Insert(OnCompartmentLeft);
			}
		}
		else
		{
			pInventoryManager.m_OnItemAddedInvoker.Remove( HandleOnItemAddedToInventory );
			pInventoryManager.m_OnItemRemovedInvoker.Remove( HandleOnItemRemovedFromInventory );
			
			if (m_CompartmentAcessComp)
			{
				m_CompartmentAcessComp.GetOnCompartmentEntered().Remove(OnCompartmentEntered);
				m_CompartmentAcessComp.GetOnCompartmentLeft().Remove(OnCompartmentLeft);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	// Called when consumable item is used by the player
	protected void OnItemUsed(IEntity item)
	{
		SCR_ConsumableItemComponent consumable = SCR_ConsumableItemComponent.Cast(item.FindComponent(SCR_ConsumableItemComponent));
		if (consumable && consumable.GetConsumableType() == EConsumableType.Bandage)
		{
			InventoryStorageManagerComponent invMan = InventoryStorageManagerComponent.Cast(GetOwner().FindComponent(InventoryStorageManagerComponent));
			if (!invMan)
				return;

			array<IEntity> bandages = {};
			SCR_BandagePredicate predicate = new SCR_BandagePredicate();
			invMan.FindItems(bandages, predicate);
			if (bandages.IsEmpty())
				return;

			RemoveItemFromQuickSlot(item);
			foreach (IEntity nextBandage : bandages)
			{
				if (nextBandage != item)
				{
					StoreItemToQuickSlot(nextBandage);
					break;
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! SCR_CompartmentAccessComponent event
	protected void OnCompartmentEntered(IEntity targetEntity, BaseCompartmentManagerComponent manager, int mgrID, int slotID, bool move)
	{
		BaseCompartmentSlot compartment = manager.FindCompartment(slotID);
		
		if (m_CompartmentAcessComp && m_CompartmentAcessComp.IsGettingIn())
		{
			m_aWeaponQuickSlotsStorage.Clear();
			for (int i = 0; i < 4; i++)
			{
				m_aWeaponQuickSlotsStorage.Insert(m_aQuickSlots[i]);
			}
		}
		
		if (!TurretCompartmentSlot.Cast(compartment) && !m_aWeaponQuickSlotsStorage.IsEmpty())
		{
			RemoveItemsFromWeaponQuickSlots();
			
			IEntity quickSlotEntity;
			SCR_InventoryStorageManagerComponent pInventoryManager = SCR_InventoryStorageManagerComponent.Cast(GetOwner().FindComponent( SCR_InventoryStorageManagerComponent));
			
			for (int i = 0; i < 4; i++)
			{
				quickSlotEntity = m_aWeaponQuickSlotsStorage[i];
				
				if (!quickSlotEntity)
					continue;
				
				if (!pInventoryManager.Contains(quickSlotEntity))
					continue;
				
				StoreItemToQuickSlot(quickSlotEntity, i, true);
			}
			
			SCR_WeaponSwitchingBaseUI.RefreshQuickSlots();
			
			return;
		}

		array<IEntity> temporaryWeaponsList = {};
		int result = GetTurretWeaponsList(temporaryWeaponsList, compartment);
		
		if (result > 0)
		{
			RemoveItemsFromWeaponQuickSlots();
			
			foreach (IEntity weapon: temporaryWeaponsList)
			{
				if (!weapon)
					continue;
				
				StoreItemToQuickSlot(weapon, -1, true);
			}
		}
		
		SCR_WeaponSwitchingBaseUI.RefreshQuickSlots();
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_CompartmentAccessComponent event
	protected void OnCompartmentLeft(IEntity targetEntity, BaseCompartmentManagerComponent manager, int mgrID, int slotID, bool move)
	{
		BaseCompartmentSlot compartment = manager.FindCompartment(slotID);
		
		if (!TurretCompartmentSlot.Cast(compartment))
		{
			m_aWeaponQuickSlotsStorage.Clear();
			for (int i = 0; i < 4; i++)
				m_aWeaponQuickSlotsStorage.Insert(m_aQuickSlots[i]);
		}
		
		if (m_CompartmentAcessComp && m_CompartmentAcessComp.IsGettingOut())
		{
			RemoveItemsFromWeaponQuickSlots();
			
			IEntity quickSlotEntity;
			SCR_InventoryStorageManagerComponent pInventoryManager = SCR_InventoryStorageManagerComponent.Cast(GetOwner().FindComponent( SCR_InventoryStorageManagerComponent));
			
			for (int i = 0; i < 4; i++)
			{
				quickSlotEntity = m_aWeaponQuickSlotsStorage[i];
				
				if (!quickSlotEntity)
					continue;
				
				if (!pInventoryManager.Contains(quickSlotEntity))
					continue;
				
				StoreItemToQuickSlot(quickSlotEntity, i, true);
			}
			
			m_aWeaponQuickSlotsStorage.Clear();
		}
		
		SCR_WeaponSwitchingBaseUI.RefreshQuickSlots();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void RemoveItemsFromWeaponQuickSlots()
	{
		for (int i = 0; i < 4; i++)
			RemoveItemFromQuickSlotAtIndex(i);
	}
	
	//------------------------------------------------------------------------------------------------
	protected int GetTurretWeaponsList(out array<IEntity> turretWeaponsList, BaseCompartmentSlot compartment)
	{
		TurretCompartmentSlot turretCompartment = TurretCompartmentSlot.Cast(compartment);
		if (!turretCompartment)
			return -1;
		
		TurretControllerComponent turretController = TurretControllerComponent.Cast(turretCompartment.GetController());
		if (!turretController)
			return -1;
		
		BaseWeaponManagerComponent weaponManager = turretController.GetWeaponManager();
		if (!weaponManager)
			return -1;
		
		return weaponManager.GetWeaponsList(turretWeaponsList);
	}
	
	//------------------------------------------------------------------------ COMMON METHODS ----------------------------------------------------------------------
	
	#else
	float GetMaxLoad();
	BaseInventoryStorageComponent GetWeaponStorage();
	InventoryItemComponent GetItemFromLoadoutSlot( ELoadoutArea eSlot );
	BaseInventoryStorageComponent GetStorageFromLoadoutSlot( ELoadoutArea eSlot );
	protected bool HasStorageComponent( IEntity pEntity );
	void GetStorages( out notnull array<SCR_UniversalInventoryStorageComponent> storagesInInventory );
	SCR_UniversalInventoryStorageComponent GetStorageComponentFromEntity( IEntity pEntity );
	void SetLootStorage( IEntity pOwner );
	BaseInventoryStorageComponent GetLootStorage();
	//override bool CanStoreItem(IEntity item, int slotID);
	//override bool CanRemoveItem(IEntity item);
	//protected override void OnRemovedFromSlot(IEntity item, int slotID);
	//override InventoryStorageSlot GetEmptySlotForItem( IEntity item );
	//protected override void OnAddedToSlot(IEntity item, int slotID);
	#endif
};