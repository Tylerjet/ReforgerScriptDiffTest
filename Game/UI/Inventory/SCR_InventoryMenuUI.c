enum EMenuAction
{
	ACTION_SELECT,
	ACTION_DESELECT,
	ACTION_UNFOLD,
	ACTION_BACK,
	ACTION_MOVEBETWEEN,
	ACTION_DRAGGED,
	ACTION_DROPPED,
	ACTION_MOVEINSIDE
};

enum EStateMenuStorage
{
	STATE_INIT = 0,
	STATE_IDLE,
	STATE_OPENED,
};

enum EStateMenuItem
{
	STATE_INIT,
	STATE_IDLE,
	STATE_MOVING_ITEM_STARTED,
};

enum EDropContainer
{
	ISINSIDE,
	ISNOTINSIDE,
	NOCONTAINER,
};

//------------------------------------------------------------------------------------------------
class SCR_InvCallBack : ScriptedInventoryOperationCallback
{
	SCR_InventoryStorageBaseUI m_pStorageFrom;
	SCR_InventoryStorageBaseUI m_pStorageTo;
	SCR_InventoryStorageBaseUI m_pStorageToFocus;
	IEntity m_pItem;
	SCR_InventoryMenuUI m_pMenu;
	string m_sItemToFocus;
	bool m_bShouldEquip;

	//------------------------------------------------------------------------------------------------
	protected override void OnFailed()
	{
		if (m_pMenu && m_pItem && m_bShouldEquip)
		{
			m_pMenu.GetCharacterController().TryEquipRightHandItem(m_pItem, EEquipItemType.EEquipTypeWeapon, false);
			m_bShouldEquip = false;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnComplete()
	{
		if (m_pMenu && m_pItem && m_bShouldEquip)
		{
			m_pMenu.GetCharacterController().TryEquipRightHandItem(m_pItem, EEquipItemType.EEquipTypeWeapon, false);
			m_bShouldEquip = false;
		}

		if ( m_pStorageFrom )
		{
			if ( m_pMenu )
			{
				if ( m_pStorageFrom == m_pMenu.GetStorageList() )
				{
					m_pMenu.ShowStoragesList();
					m_pMenu.ShowAllStoragesInList();

				}
				else
				{
					m_pStorageFrom.Refresh();
				}
			}
		}
		else
		{
			if ( m_pMenu )
			{
				m_pMenu.ShowStoragesList();
				m_pMenu.ShowAllStoragesInList();
			}
		}
		if ( m_pStorageTo )
		{
			if ( m_pMenu )
			{
				if ( m_pStorageTo == m_pMenu.GetStorageList() )
				{
					m_pMenu.ShowStoragesList();
					m_pMenu.ShowAllStoragesInList();
				}
				else
				{
					m_pStorageTo.Refresh();
				}
			}
		}
		if ( m_pMenu )
		{ 
			if ( m_pStorageFrom != m_pMenu.GetLootStorage() && m_pStorageTo != m_pMenu.GetLootStorage() )
				m_pMenu.RefreshLootUIListener();
			
			SCR_InventoryStorageBaseUI pStorage = m_pMenu.GetActualStorageInCharacterStorageUI();
			if ( pStorage )
			{
				if ( pStorage.Type() == SCR_InventoryStorageWeaponsUI ) 
					pStorage = m_pMenu.GetStorageList();
				//pStorage.Refresh();
			}
						
			m_pMenu.NavigationBarUpdate();
		}
		if (m_pStorageToFocus && m_pMenu.IsUsingGamepad())
		{
			int itemId = m_pStorageToFocus.GetSlotIdForItem(m_sItemToFocus);
			if (m_pMenu.FocusOnSlotInStorage(m_pStorageToFocus, itemId, false))
			{
				m_pMenu.FocusOnSlotInStorage(m_pStorageToFocus, itemId);
			}
			else
			{
				m_pMenu.FocusOnSlotInStorage(m_pStorageTo);
			}
		}
	}
	
};

//------------------------------------------------------------------------------------------------
//! UI Script
//! Inventory Menu UI Layout
class SCR_InventoryMenuUI : ChimeraMenuBase
{	
	
	#ifndef DISABLE_INVENTORY
	static protected ResourceName							m_ItemPreviewManagerPrefab = "{9F18C476AB860F3B}Prefabs/World/Game/ItemPreviewManager.et";
	protected SCR_InventoryStorageManagerComponent			m_InventoryManager 		= null;
	protected SCR_CharacterInventoryStorageComponent		m_StorageManager 		= null;
	protected GenericEntity									m_Player 				= null;
	protected Widget										m_wContainer;
	protected Widget										m_wStorageList, m_wGadgetStorage, m_wLootStorage;
	protected Widget										m_widget;
	
	protected SCR_InventoryInspectionUI						m_InspectionScreen				= null;
	
	protected SCR_InventorySlotUI							m_pFocusedSlotUI				= null;
	protected SCR_InventorySlotUI							m_pSelectedSlotUI				= null;
	protected SCR_InventorySlotUI							m_pPrevSelectedSlotUI			= null;

	protected SCR_InventorySlotStorageUI					m_pFocusedSlotStorageUI			= null;
	protected SCR_InventorySlotStorageUI					m_pSelectedSlotStorageUI		= null;
	protected SCR_InventorySlotStorageUI					m_pSelectedSlotStoragePrevUI	= null;

	protected SCR_InventoryStorageBaseUI					m_pStorageBaseUI				= null;
	protected SCR_InventoryStoragesListUI					m_pStorageListUI				= null;
	protected SCR_InventoryStorageGadgetsUI					m_pGadgetsBaseUI				= null;
	protected SCR_InventoryStorageBaseUI					m_pStorageLootUI				= null;
	protected SCR_InventoryStorageWeaponsUI					m_pWeaponStorage				= null;
	protected EquipedWeaponStorageComponent					m_pWeaponStorageComp			= null;

	protected SCR_InventoryItemInfoUI						m_pItemInfo						= null;
	protected SCR_InventoryStorageBaseUI					m_pActiveStorageUI				= null;
	protected SCR_InventoryStorageBaseUI					m_pActiveHoveredStorageUI		= null;
	protected SCR_InventoryStorageBaseUI					m_pPrevActiveStorageUI			= null;
	protected ref array<SCR_InventorySlotUI>				m_aShownStorages				= new ref array<SCR_InventorySlotUI>();	//used for storing pointers on storages which are displayed on screen ( for the refreshing purposes )
	protected GridLayoutWidget								m_wStoragesContainer;
	protected const int										STORAGE_AREA_COLUMNS			= 2;
	protected int											m_iStorageListCounter			= 0;
	//protected static bool									m_bColdStart = true;			// uncomment to enable the expand / collapse feature of storages

	protected ProgressBarWidget							m_wProgressBarWeight;
	protected ProgressBarWidget							m_wProgressBarWeightItem;

	const string 										BACKPACK_STORAGE_LAYOUT = "{06E9285D68D190EF}UI/layouts/Menus/Inventory/InventoryContainerGrid.layout";			
	const string										WEAPON_STORAGE_LAYOUT	= "{7A2BC3A6F1CFF354}UI/layouts/Menus/Inventory/InventoryWeapons.layout";
	const string 										GADGETS_STORAGE_LAYOUT 	= "{265189B87ED5CD10}UI/layouts/Menus/Inventory/InventoryGadgetsPanel.layout";															
	const string 										STORAGES_LIST_LAYOUT 	= "{FC579324F5E4B3A3}UI/layouts/Menus/Inventory/InventoryCharacterGrid.layout";															
	const string 										ITEM_INFO			 	= "{AE8B7B0A97BB0BA8}UI/layouts/Menus/Inventory/InventoryItemInfo.layout";
	const string										ACTION_LAYOUT 			= "{81BB7785A3987196}UI/layouts/Menus/Inventory/InventoryActionNew.layout";

	protected EStateMenuStorage							m_EStateMenuStorage = EStateMenuStorage.STATE_IDLE; // is this useful for anything?
	protected EStateMenuItem							m_EStateMenuItem = EStateMenuItem.STATE_IDLE;
	protected string 									m_sFSMStatesNames[10]={"init", "idle", "item selected", "storage selected", "STATE_STORAGE_OPENED", "storage unfolded", "move started", "move finished", "closing", "STATE_UNKNOWN" };
	protected SCR_NavigationBarUI						m_pNavigationBar			= null;
	protected SCR_NavigationButtonComponent				m_CloseButton;

	//variables dedicated to move an item from storage to storage
	protected IEntity 									m_pItem;
	protected BaseInventoryStorageComponent				m_pDisplayedStorage, m_pLastCurrentNavStorage;
	protected BaseInventoryStorageComponent				m_pStorageFrom, m_pStorageTo;	//last known storages from the last move operation
//	protected SCR_InventorySlotUI	 					m_pStorageUIFrom;
	protected SCR_InventorySlotStorageUI				m_pStorageUITo;	//last known storagesUI from the last move operation
	protected SCR_CharacterVicinityComponent 			m_pVicinity;
	ItemPreviewWidget 									m_wPlayerRender, m_wPlayerRenderSmall;
	PreviewRenderAttributes 							m_PlayerRenderAttributes;
	protected ButtonWidget								m_wButtonShowAll;
	protected SCR_InventoryCharacterWidgetHelper 		m_pCharacterWidgetHelper;
	protected ItemPreviewManagerEntity 					m_pPreviewManager;
	protected bool										m_bDraggingEnabled;
	protected FrameWidget								m_wDragDropContainer;
	protected SCR_SlotUIComponent 						m_pDragDropFrameSlotUI;
	protected RenderTargetWidget						m_pDragDropPreviewImage;
	protected ref array<SCR_InventoryStorageBaseUI>  	m_aStorages = {};
	protected TextWidget								m_wTotalWeightText;
	protected bool										m_bLocked = false;	//helper variable

	//Item/Weapon Switching
	protected SCR_InventoryStorageQuickSlotsUI			m_pQuickSlotStorage;
	protected Widget									m_wQuickSlotStorage;
	protected ref SCR_InvCallBack						m_pCallBack = new SCR_InvCallBack();

	protected float 									m_fX, m_fY;	//debug;
	protected bool										m_bShowIt = true;
	protected int										m_iMouseX, m_iMouseY
	const int											DRAG_THRESHOLD 			= 5;

	//other character's information
	protected SCR_CharacterControllerComponent			m_CharController;
	protected Widget									m_wCharFeatureBleeding;
	protected Widget									m_wCharFeatureWounded;
	//protected ProgressBarWidget						m_wInfoStamina;	// Preparation for showing the stamina level in inventory
	protected ref array<HitZone> 						m_aBleedingHitZones = {};

	protected bool 										m_bWasJustTraversing;
	protected bool 										m_bStorageSwitchMode;
	protected SCR_InventorySlotUI 						m_pItemToAssign;

	//------------------------------------------------------------------------------------------------
	// !
	protected void InitializeCharacterInformation()
	{
		m_wCharFeatureBleeding = Widget.Cast( m_widget.FindAnyWidget( "infoBleeding" ) );
		m_wCharFeatureWounded = Widget.Cast( m_widget.FindAnyWidget( "infoWounded" ) );


		// Preparation for showing the stamina level in inventory
		m_CharController = SCR_CharacterControllerComponent.Cast( m_Player.FindComponent( SCR_CharacterControllerComponent ) );
		// m_wInfoStamina = ProgressBarWidget.Cast( m_widget.FindAnyWidget( "m_wInfoStamina" ) );


		OnDamageStateChanged();

		/*
		TODO: enable it once the performance issue with the invokers is solved
		m_CharDamageManager.GetOnDamageOverTimeAdded().Insert( OnDamageStateChanged );
		*/
	}

	//------------------------------------------------------------------------------------------------
	// !
	void OnDamageStateChanged()
	{

		SCR_CharacterDamageManagerComponent	m_CharDamageManager = SCR_CharacterDamageManagerComponent.Cast( m_Player.FindComponent( SCR_CharacterDamageManagerComponent ) );

		if ( m_wCharFeatureBleeding && m_CharDamageManager )
		{
			m_aBleedingHitZones.Clear();
			m_CharDamageManager.GetBleedingHitZones( m_aBleedingHitZones );
			if ( !m_aBleedingHitZones.IsEmpty() )
				m_wCharFeatureBleeding.SetVisible( true );
			else
				m_wCharFeatureBleeding.SetVisible( false );
		}
		
		if ( m_wCharFeatureWounded )
		{
			if ( m_CharDamageManager.GetHealthScaled() < 1 )
				m_wCharFeatureWounded.SetVisible( true );
			else 
				m_wCharFeatureWounded.SetVisible( false );
		}
		
		/*
		Preparation for showing the stamina level in inventory
		if ( m_wInfoStamina )
			m_wInfoStamina.SetCurrent( m_CharController.GetStamina() );
		*/
	}

	//------------------------------------------------------------------------------------------------
	// !
	void RegisterUIStorage( SCR_InventorySlotUI pStorageUI )
	{
		if ( m_aShownStorages.Find( pStorageUI ) > -1 )
			return;

		if ( SCR_InventorySlotStorageUI.Cast( pStorageUI ) || SCR_InventorySlotWeaponUI.Cast( pStorageUI ) )
			m_aShownStorages.Insert( pStorageUI );
	}

	//------------------------------------------------------------------------------------------------
	// !
	void UnregisterUIStorage( SCR_InventorySlotUI pStorageUI )
	{
		if ( pStorageUI )
			m_aShownStorages.RemoveItem( pStorageUI );
	}

	//------------------------------------------------------------------------------------------------
	void SetActiveStorage( SCR_InventoryStorageBaseUI storageUI )
	{
		m_pPrevActiveStorageUI = m_pActiveStorageUI;
		m_pActiveStorageUI = storageUI;
		if( m_pActiveStorageUI )
		{
			m_pFocusedSlotUI = m_pActiveStorageUI.GetFocusedSlot();
		}
	}

	//------------------------------------------------------------------------------------------------
	//---Get total weight from all storages (weapon, gadget, deposit)
	protected float GetTotalWeight()
	{
		if (!m_InventoryManager)
			return 0;
		
		return m_InventoryManager.GetTotalWeightOfAllStorages();
	}

	//------------------------------------------------------------------------------------------------
	SCR_InventoryStorageManagerComponent GetInventoryStorageManager()
	{
		return m_InventoryManager;
	}

	//------------------------------------------------------------------------------------------------
	//! Actualy opened UI storage in character's storage
	SCR_InventoryStorageBaseUI GetActualStorageInCharacterStorageUI() { return m_pStorageBaseUI; }
	SCR_InventoryStorageBaseUI GetStorageList() { return m_pStorageListUI; }
	SCR_InventoryStorageBaseUI GetLootStorage() { return m_pStorageLootUI; }

	//------------------------------------------------------------------------------------------------
	//! Returns whether interaction with inventory items is possible
	bool GetCanInteract()
	{
		if (GetInspectionScreen())
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void RefreshPlayerWidget()
	{
		if (!m_pPreviewManager)
			return;
		if (m_wPlayerRender)
			m_pPreviewManager.SetPreviewItem(m_wPlayerRender, m_Player, m_PlayerRenderAttributes);
		if (m_wPlayerRenderSmall)
			m_pPreviewManager.SetPreviewItem(m_wPlayerRenderSmall, m_Player, m_PlayerRenderAttributes);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuUpdate(float tDelta)
	{
		super.OnMenuUpdate(tDelta);

		if (m_InspectionScreen)
		{
			m_InspectionScreen.UpdateView(tDelta);
			return;
		}

		if (m_pCharacterWidgetHelper && m_pPreviewManager && m_PlayerRenderAttributes && m_pCharacterWidgetHelper.Update(tDelta, m_PlayerRenderAttributes))
		{
			m_pPreviewManager.SetPreviewItem(m_wPlayerRender, m_Player, m_PlayerRenderAttributes);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		if( !Init() )
		{
			Action_CloseInventory();
			return;
		}
		//TODO: decide what storage should be opened as default on the start

		//ShowStorage( ESlotID.SLOT_BACKPACK );
		//ShowStorage( ESlotID.SLOT_WEAPONS_STORAGE );
		//ShowGadgetStorage();
		ShowStoragesList();
		ShowAllStoragesInList();
		ShowVicinity();
		m_pPreviewManager = GetGame().GetItemPreviewManager();
		if (m_pPreviewManager)
		{
			m_wPlayerRender = ItemPreviewWidget.Cast( m_widget.FindAnyWidget( "playerRender" ) );
			auto collection = m_StorageManager.GetAttributes();
			if (collection)
				m_PlayerRenderAttributes = PreviewRenderAttributes.Cast(collection.FindAttribute(PreviewRenderAttributes));
			SizeLayoutWidget wPlayerRenderSmallRoot = SizeLayoutWidget.Cast( m_widget.FindAnyWidget( "playerRenderSmallRoot" ) );
			if ( wPlayerRenderSmallRoot )
				m_wPlayerRenderSmall = ItemPreviewWidget.Cast( wPlayerRenderSmallRoot.FindAnyWidget( "item" ) );
			RefreshPlayerWidget();
			m_pCharacterWidgetHelper = SCR_InventoryCharacterWidgetHelper(m_wPlayerRender, GetGame().GetWorkspace() );
		}

		Widget wrap = m_widget.FindAnyWidget( "WrapLayoutShow" );
		m_wButtonShowAll = ButtonWidget.Cast( wrap.FindAnyWidget( "ItemButton" ) );
		if( m_wButtonShowAll )
		{
			m_wButtonShowAll.AddHandler( new SCR_InventoryButton( EInventoryButton.BUTTON_SHOW_DEFAULT, this ) );
		}

		if( m_pNavigationBar )
		{
			m_pNavigationBar.m_OnAction.Insert(OnAction);

			m_CloseButton = SCR_NavigationButtonComponent.GetNavigationButtonComponent("Back", m_widget);
			if (m_CloseButton)
				m_CloseButton.m_OnActivated.Insert(Action_TryCloseInventory);
		}
		GetGame().GetInputManager().AddActionListener( "Inventory_Drag", EActionTrigger.DOWN, Action_DragDown );
		GetGame().GetInputManager().AddActionListener( "Inventory", EActionTrigger.DOWN, Action_CloseInventory );

		GetGame().OnInputDeviceUserChangedInvoker().Insert(OnInputDeviceChanged);

		SCR_UISoundEntity.SoundEvent("SOUND_INV_OPEN");

		if (IsUsingGamepad())
			SetStorageSwitchMode(true);

		ResetHighlightsOnAvailableStorages();
		SetOpenStorage();
		UpdateTotalWeightText();
	}

	//------------------------------------------------------------------------------------------------
	protected void SetStorageSwitchMode(bool enabled)
	{
		m_bStorageSwitchMode = enabled;
		foreach (SCR_InventoryStorageBaseUI storage : m_aStorages)
		{
			if (storage)
				storage.ActivateStorageButton(m_bStorageSwitchMode);
		}

		m_pStorageLootUI.ActivateStorageButton(m_bStorageSwitchMode);
		m_pStorageListUI.ActivateStorageButton(m_bStorageSwitchMode);
		m_pQuickSlotStorage.ActivateStorageButton(m_bStorageSwitchMode);

		if (m_bStorageSwitchMode)
		{
			Widget btnToFocus = m_pStorageLootUI.GetButtonWidget();
			if (m_pActiveStorageUI)
				btnToFocus = m_pActiveStorageUI.GetButtonWidget();
			GetGame().GetWorkspace().SetFocusedWidget(btnToFocus);
		}
		else
		{
			if (m_pActiveStorageUI)
			{
				m_pActiveStorageUI.ShowContainerBorder(false);
				FocusOnSlotInStorage(m_pActiveStorageUI);
			}
		}

		NavigationBarUpdate();
	}

	//------------------------------------------------------------------------------------------------
	protected void ToggleStorageSwitchMode()
	{
		SetStorageSwitchMode(!m_bStorageSwitchMode);
	}

	//------------------------------------------------------------------------------------------------
	bool FocusOnSlotInStorage(SCR_InventoryStorageBaseUI storage, int id = 0, bool focus = true)
	{
		if (!storage || id < 0)
			return false;

		array<SCR_InventorySlotUI> slots = {};
		storage.GetSlots(slots);

		if (slots.IsEmpty() || !slots[id])
			return false;

		if (focus)
		{
			Widget focusedWidget = slots[id].GetButtonWidget();
			GetGame().GetWorkspace().SetFocusedWidget(slots[id].GetButtonWidget());
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! returns true if the init procedure was success
	protected bool Init()
	{
		m_widget = GetRootWidget();

		if ( m_widget == null )
			return false;

		//Get player
		PlayerController playerController = GetGame().GetPlayerController();
		if ( playerController != null )
		{
			m_Player = GenericEntity.Cast(playerController.GetControlledEntity());
			if ( m_Player != null )
			{
				if (!GetGame().GetItemPreviewManager())
				{
					Resource rsc = Resource.Load(m_ItemPreviewManagerPrefab);
					if (rsc.IsValid())
						GetGame().SpawnEntityPrefabLocal(rsc);
				}

				SCR_CharacterControllerComponent charController = SCR_CharacterControllerComponent.Cast(m_Player.FindComponent(SCR_CharacterControllerComponent));
				if (charController)
				{
					charController.m_OnPlayerDeath.Insert(Action_CloseInventory);
					if (charController.IsDead())
					{
						return false;
					}
				}

				//Inventory Manager
				m_InventoryManager = SCR_InventoryStorageManagerComponent.Cast( m_Player.FindComponent( SCR_InventoryStorageManagerComponent ) );
				m_StorageManager = SCR_CharacterInventoryStorageComponent.Cast( m_Player.FindComponent( SCR_CharacterInventoryStorageComponent ) );
				if( !m_StorageManager )
					return false;
				m_pWeaponStorageComp = EquipedWeaponStorageComponent.Cast(m_StorageManager.GetWeaponStorage());
				if( !m_pWeaponStorageComp )
					return false;
				if ( m_InventoryManager )
				{
					m_InventoryManager.m_OnItemAddedInvoker.Insert(OnItemAddedListener);
					m_InventoryManager.m_OnItemRemovedInvoker.Insert(OnItemRemovedListener);
				}

				m_wProgressBarWeight = ProgressBarWidget.Cast( m_widget.FindAnyWidget( "ProgressBarWeight" ) );
				if( m_wProgressBarWeight )
				{
					m_wProgressBarWeight.SetMin( 0.0 );
					m_wProgressBarWeight.SetMax( m_StorageManager.GetMaxLoad() );
					m_wProgressBarWeight.SetCurrent( m_InventoryManager.GetTotalWeightOfAllStorages() );
				};

				m_wProgressBarWeightItem = ProgressBarWidget.Cast( m_widget.FindAnyWidget( "ProgressBarWeightItem" ) );
				if( m_wProgressBarWeightItem )
				{
					m_wProgressBarWeightItem.SetMin( 0.0 );
					m_wProgressBarWeightItem.SetMax( m_StorageManager.GetMaxLoad() );
					m_wProgressBarWeightItem.SetCurrent( 0.0 );
				};

				Widget wNaviBar = m_widget.FindAnyWidget( "Footer" );
				if( wNaviBar )
					m_pNavigationBar = SCR_NavigationBarUI.Cast( wNaviBar.FindHandler( SCR_NavigationBarUI ) );
				m_pVicinity = SCR_CharacterVicinityComponent.Cast(m_Player.FindComponent(SCR_CharacterVicinityComponent));
				if (m_pVicinity)
				{
					m_pVicinity.OnVicinityUpdateInvoker.Insert(RefreshLootUIListener);
				}

				m_wStoragesContainer = GridLayoutWidget.Cast( m_widget.FindAnyWidget( "StorageGrid" ) );
				m_wTotalWeightText = TextWidget.Cast( m_widget.FindAnyWidget("TotalWeightText") );

				InitializeCharacterInformation();
			}
			else
			{
				return false;
			}

		}
		else
		{
			return false;
		}


		ShowQuickSlotStorage();

		int quickSlotCount = m_StorageManager.GetQuickSlotItems().Count();
		for (int slotId = 1; slotId <= quickSlotCount; ++slotId)
		{
			GetGame().GetInputManager().AddActionListener("InventoryQuickSlot" + slotId, EActionTrigger.PRESSED, Action_SelectQuickSlot);
		}
		
		m_InventoryManager.m_OnItemAddedInvoker.Insert( OnItemAdded );
		m_InventoryManager.m_OnItemRemovedInvoker.Insert( OnItemRemoved );
		
		m_wDragDropContainer = FrameWidget.Cast( m_widget.FindAnyWidget( "DragDropContainer" ) );
		if ( m_wDragDropContainer )
		{
			m_pDragDropFrameSlotUI = SCR_SlotUIComponent.Cast( m_wDragDropContainer.FindHandler( SCR_SlotUIComponent ) );
			m_pDragDropPreviewImage = RenderTargetWidget.Cast( m_wDragDropContainer.FindAnyWidget( "item" ) );
		}
		return true;
	}

	//------------------------------------------------------------------------------------------------
	void OnAction( SCR_NavigationButtonComponent comp, string action, SCR_InventoryStorageBaseUI pParentStorage = null, int traverseStorageIndex = -1 )
	{
		switch (action)
		{
			case "Inventory":
			{
				Action_CloseInventory();
			}
			break;

			case "Inventory_UnassignFromQuickSlot":
			{
				Action_QuickSlotUnassign();
			} break;

			case "Inventory_AssignToQuickSlot":
			{
				Action_QuickSlotAssign();
			} break;

			case "Inventory_Drop":
			{
				Action_MoveBetween();
			} break;

			case "Inventory_Equip":
			{
				Action_EquipItem();
			} break;

			case "Inventory_Select":
			{
				if (m_bStorageSwitchMode && IsUsingGamepad())
				{
					SetStorageSwitchMode(false);
					return;
				}
				if (m_pFocusedSlotUI && !m_pFocusedSlotUI.IsSlotSelected())
					Action_SelectItem();
				else
					Action_DeselectItem();
			} break;

			case "Inventory_Deselect":
			{
				Action_DeselectItem();
			} break;

			case "Inventory_OpenStorage":
			{
				Action_UnfoldItem();
			} break;

			case "Inventory_Back":
			{
				Action_StepBack( pParentStorage, traverseStorageIndex );
			} break;

			case "Inventory_StepBack":
			{
				Action_StepBack( pParentStorage, traverseStorageIndex );
			} break;

			case "Inventory_Inspect":
			{
				Action_Inspect();
			} break;

			case "Inventory_Use":
			{
				Action_UseItem();
			} break;

			case "Inventory_Move":
			{
				if (m_bStorageSwitchMode)
				{
					Action_Drop();
					return;
				}
				Action_MoveItemToStorage();
			} break;

			case "Inventory_Swap":
			{
				if (m_bStorageSwitchMode)
				{
					ToggleStorageSwitchMode();
					FocusOnSlotInStorage(m_pActiveStorageUI);
					return;
				}
				Action_SwapItems(m_pSelectedSlotUI, m_pFocusedSlotUI);
			} break;
		}
	}

	//------------------------------------------------------------------------------------------------
	// ! Default view of the character's inventory
	void ShowDefault()
	{
		ShowEquipedWeaponStorage();
		if ( !m_pSelectedSlotStorageUI )
		{
			SimpleFSM( EMenuAction.ACTION_BACK );
			return;
		}
		m_pSelectedSlotStorageUI.SetSelected( false );

		if ( m_pSelectedSlotUI )
		{
			m_pSelectedSlotUI.SetSelected( false );
			m_pSelectedSlotUI = null;
		}
		if ( m_pSelectedSlotStorageUI )
			m_pSelectedSlotStorageUI = null;

		FilterOutStorages( false );
	}

	//------------------------------------------------------------------------------------------------
	protected int CloseOpenedStorage()
	{
		if ( !m_wContainer )
			return -1;	//any storage opened
		return CloseStorage( m_wContainer );
	}

	//------------------------------------------------------------------------------------------------
	protected int CloseStorage( SCR_InventoryStorageBaseUI pStorageUI )
	{
		Widget w = pStorageUI.GetRootWidget();
		if ( !w )
			return -1;
		return CloseStorage( w );
	}

	//------------------------------------------------------------------------------------------------
	protected int CloseStorage( notnull Widget w )
	{
		auto storageUIHandler = SCR_InventoryStorageBaseUI.Cast( w.FindHandler( SCR_InventoryStorageBaseUI ) );
		if ( !storageUIHandler )
			return -1;	//some storage opened, but it does not have any handler ( wrong )

		int iLastShownPage = storageUIHandler.GetLastShownPage();
		m_pLastCurrentNavStorage = storageUIHandler.GetCurrentNavigationStorage();
		w.RemoveHandler( storageUIHandler );	//remove the handler from the widget
		w.RemoveFromHierarchy();
		return iLastShownPage;
	}

	//------------------------------------------------------------------------------------------------
	// ! Hides/Unhides ( not closes ) the storage
	protected void ToggleStorageContainerVisibility( notnull SCR_InventorySlotUI pSlot )
	{
		BaseInventoryStorageComponent pStorage = pSlot.GetAsStorage();
		if ( !pStorage )
			return;
		SCR_InventoryStorageBaseUI pStorageUI = GetStorageUIByBaseStorageComponent( pStorage );
		if ( !pStorageUI )
			return;
		pStorageUI.ToggleShow();
	}

	//------------------------------------------------------------------------------------------------
	// ! Shows the content of the 1st available storage
	void ShowStorage()
	{
		if( !m_StorageManager )
			return;

		array<SCR_UniversalInventoryStorageComponent> pStorages = new array<SCR_UniversalInventoryStorageComponent>();
		m_StorageManager.GetStorages( pStorages );

		if( pStorages.Count() == 0 )
			return;
		SCR_UniversalInventoryStorageComponent pStorage = pStorages.Get( 0 );
		ShowStorage( pStorage );
	}

	//------------------------------------------------------------------------------------------------
	//!
	void ShowStorage( BaseInventoryStorageComponent storage, ELoadoutArea area = ELoadoutArea.ELA_None )
	{
		if ( !m_wStoragesContainer )
			return;
		m_wContainer =  GetGame().GetWorkspace().CreateWidgets( BACKPACK_STORAGE_LAYOUT, m_wStoragesContainer );
		int iRow = Math.Floor( m_iStorageListCounter / STORAGE_AREA_COLUMNS );
		int iCol = m_iStorageListCounter % STORAGE_AREA_COLUMNS;
		m_iStorageListCounter++;

		m_wStoragesContainer.SetRowFillWeight( iRow, 0 );
		m_wStoragesContainer.SetColumnFillWeight( iCol, 0 );

		GridSlot.SetColumn( m_wContainer, iCol );
		GridSlot.SetRow( m_wContainer, iRow );
		GridSlot.SetColumnSpan( m_wContainer, 1 );
		GridSlot.SetRowSpan( m_wContainer, 1 );

		if ( !m_wContainer )
			return;
		if ( storage.Type() == ClothNodeStorageComponent )
		{
			m_wContainer.AddHandler( new SCR_InventoryStorageLBSUI( storage, area, this, 0, null) );
			m_pStorageBaseUI = SCR_InventoryStorageBaseUI.Cast( m_wContainer.FindHandler( SCR_InventoryStorageLBSUI ) );
		}
		else if ( storage.Type() == EquipedWeaponStorageComponent )
		{
			m_wContainer.AddHandler( new SCR_InventoryStorageWeaponsUI( m_StorageManager.GetWeaponStorage(), area, this ) );
			m_pStorageBaseUI = SCR_InventoryStorageWeaponsUI.Cast( m_wContainer.FindHandler( SCR_InventoryStorageWeaponsUI ) );
		}
		else
		{
			m_wContainer.AddHandler( new SCR_InventoryStorageBackpackUI( storage, area, this, 0, null ) );
			m_pStorageBaseUI = SCR_InventoryStorageBaseUI.Cast( m_wContainer.FindHandler( SCR_InventoryStorageBaseUI ) );
		}

		m_aStorages.Insert( m_pStorageBaseUI );
	}

	//------------------------------------------------------------------------------------------------
	void ShowVicinity()
	{
		if (!m_pVicinity)
		{
			Print("No vicnity component on character!", LogLevel.DEBUG);
			return;
		}

		if ( m_wLootStorage )
		{
			m_wLootStorage.RemoveHandler( m_wLootStorage.FindHandler( SCR_InventoryStorageLootUI ) );	//remove the handler from the widget
			m_wLootStorage.RemoveFromHierarchy();
		}

		Widget parent = m_widget.FindAnyWidget( "StorageLootSlot" );
		m_wLootStorage =  GetGame().GetWorkspace().CreateWidgets( BACKPACK_STORAGE_LAYOUT, parent );
		if ( !m_wLootStorage )
			return;

		m_wLootStorage.AddHandler( new SCR_InventoryStorageLootUI( null, ESlotID.SLOT_LOOT_STORAGE, this, 0, null, m_Player ) );
		m_pStorageLootUI = SCR_InventoryStorageBaseUI.Cast( m_wLootStorage.FindHandler( SCR_InventoryStorageLootUI ) );
	}

	//------------------------------------------------------------------------------------------------
	void RefreshLootUIListener()
	{
		if (m_pVicinity && m_pStorageLootUI)
		{
			m_pStorageLootUI.Refresh();
		}
	}

	//------------------------------------------------------------------------------------------------
	void ShowStoragesList()
	{
		if( m_wStorageList )
		{
			m_wStorageList.RemoveHandler( m_wStorageList.FindHandler( SCR_InventoryStoragesListUI ) );	//remove the handler from the widget
			m_wStorageList.RemoveFromHierarchy();
		}

		Widget parent = m_widget.FindAnyWidget( "StoragesListSlot" );
		m_wStorageList =  GetGame().GetWorkspace().CreateWidgets( STORAGES_LIST_LAYOUT, parent );

		if( !m_wStorageList )
			return;

		m_wStorageList.AddHandler( new SCR_InventoryStoragesListUI( m_StorageManager, ELoadoutArea.ELA_None, this ) );
		m_pStorageListUI = SCR_InventoryStoragesListUI.Cast( m_wStorageList.FindHandler( SCR_InventoryStoragesListUI ) );
	}

	//------------------------------------------------------------------------------------------------
	void ShowAllStoragesInList()
	{
		m_iStorageListCounter = 0;
		array<SCR_InventorySlotUI> aSlotsUI = {};
		m_pStorageListUI.GetSlots( aSlotsUI );
		if ( !m_aStorages.IsEmpty() )
		{
			foreach ( SCR_InventoryStorageBaseUI pStorage : m_aStorages )
			{
				if ( !pStorage )
					continue;
				CloseStorage( pStorage );
			}
		}
		SortSlotsByLoadoutArea( aSlotsUI );
		m_aStorages.Resize( aSlotsUI.Count() );


		ShowStorage(m_pWeaponStorageComp);

		foreach ( SCR_InventorySlotUI pSlot : aSlotsUI )
		{
			if ( !pSlot )
				continue;

			BaseInventoryStorageComponent pStorage = pSlot.GetAsStorage();
			if ( pStorage )
			{
				ShowStorage( pStorage, pSlot.GetLoadoutArea() );

				/* Enable to have the expand / collapse feature
				if ( m_StorageManager.GetIsStorageShown( pStorage ) )
					m_pStorageBaseUI.Show( true );
				else
					m_pStorageBaseUI.Show( false );
				*/
			}
		}
	}

	protected SCR_InventoryStorageBaseUI GetStorageUIFromVicinity(BaseInventoryStorageComponent storage)
	{
		array<SCR_InventorySlotUI> slots = {};
		m_pStorageLootUI.GetSlots(slots);
		foreach (SCR_InventorySlotUI slot : slots)
		{
			if (slot && slot.GetStorageComponent() == storage)
				return slot.GetStorageUI();
		}

		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	protected SCR_InventoryStorageBaseUI GetStorageUIByBaseStorageComponent( BaseInventoryStorageComponent pStorage )
	{
		foreach ( SCR_InventoryStorageBaseUI pStorageUI : m_aStorages )
			if ( pStorageUI && pStorageUI.GetStorage() == pStorage )
				return pStorageUI;

		return null;
	}

	//------------------------------------------------------------------------------------------------
	protected void SortSlotsByLoadoutArea( out array<SCR_InventorySlotUI> aSlots )
	{
		array<SCR_InventorySlotUI> tmpSlots = {};
		tmpSlots.Copy( aSlots );
		aSlots.Clear();

		foreach ( SCR_InventorySlotUI pSlotUI : tmpSlots )
		{
			if ( !SCR_InventorySlotStorageUI.Cast( pSlotUI ) )
				continue;
			int iArea = pSlotUI.GetLoadoutArea();
			if ( iArea == -1 )
				continue;
			if ( iArea > aSlots.Count() - 1 )
				aSlots.Resize( iArea + 1 );
			aSlots.Set( iArea, pSlotUI );
		}


	}

	//------------------------------------------------------------------------------------------------
	protected void ShowGadgetStorage()
	{
		if( m_wGadgetStorage && m_wContainer )
		{
			m_wContainer.RemoveHandler( m_wGadgetStorage.FindHandler( SCR_InventoryStorageGadgetsUI ) );	//remove the handler from the widget
			m_wGadgetStorage.RemoveFromHierarchy();
		}

		Widget parent = m_widget.FindAnyWidget( "GadgetsGridSlot" );
		m_wGadgetStorage =  GetGame().GetWorkspace().CreateWidgets( GADGETS_STORAGE_LAYOUT, parent );

		if( !m_wGadgetStorage )
			return;

		//m_wGadgetStorage.AddHandler( new SCR_InventoryStorageGadgetsUI( m_StorageManager.GetGadgetsStorage(), ELoadoutArea.ELA_None, this ) );
		m_pGadgetsBaseUI = SCR_InventoryStorageGadgetsUI.Cast( m_wGadgetStorage.FindHandler( SCR_InventoryStorageGadgetsUI ) );
	}

	//------------------------------------------------------------------------------------------------
	protected void ShowEquipedWeaponStorage()
	{
		//CloseOpenedStorage();
		Widget parent = m_widget.FindAnyWidget( "StorageGridSlot" );
		m_wContainer =  GetGame().GetWorkspace().CreateWidgets( BACKPACK_STORAGE_LAYOUT, parent );
		if ( !m_wContainer )
			return;

		m_wContainer.AddHandler( new SCR_InventoryStorageWeaponsUI( m_StorageManager.GetWeaponStorage(), ELoadoutArea.ELA_None, this ) );
		m_pStorageBaseUI = SCR_InventoryStorageWeaponsUI.Cast( m_wContainer.FindHandler( SCR_InventoryStorageWeaponsUI ) );
		m_pWeaponStorage = SCR_InventoryStorageWeaponsUI.Cast( m_pStorageBaseUI );
		m_pWeaponStorageComp = EquipedWeaponStorageComponent.Cast( m_pWeaponStorage.GetStorage() );
		m_pDisplayedStorage = m_pWeaponStorageComp;
	}

	//------------------------------------------------------------------------------------------------
	void ShowItemInfo( string sName = "", string sDescr = "", float sWeight = 0.0 )
	{
		if ( !m_pItemInfo )
		{
			//Widget parent = m_widget.FindAnyWidget( "SoldierInfo" );
			Widget infoWidget = GetGame().GetWorkspace().CreateWidgets( ITEM_INFO, null );
			if ( !infoWidget )
				return;

			infoWidget.AddHandler( new SCR_InventoryItemInfoUI() );
			m_pItemInfo = SCR_InventoryItemInfoUI.Cast( infoWidget.FindHandler( SCR_InventoryItemInfoUI ) );
		}

		if( !m_pItemInfo )
			return;

		Widget w = WidgetManager.GetWidgetUnderCursor();
		if (!w)
		{
			w = m_pFocusedSlotUI.GetButtonWidget();
		}

		m_pItemInfo.Show( 0.6, w, IsUsingGamepad() );
		m_pItemInfo.SetName( sName );
		m_pItemInfo.SetDescription( sDescr );
		m_pItemInfo.SetWeight( sWeight );
		int iMouseX, iMouseY;

		float x, y;
		w.GetScreenPos(x, y);

		float width, height;
		w.GetScreenSize(width, height);

		float screenSizeX, screenSizeY;
		GetGame().GetWorkspace().GetScreenSize(screenSizeX, screenSizeY);

		float infoWidth, infoHeight;
		m_pItemInfo.GetInfoWidget().GetScreenSize(infoWidth, infoHeight);

		iMouseX = x;
		iMouseY = y + height;
		if (x + infoWidth > screenSizeX)
			iMouseX = screenSizeX - infoWidth - width / 2; // offset info if it would go outside of the screen

		m_pItemInfo.Move( GetGame().GetWorkspace().DPIUnscale( iMouseX ), GetGame().GetWorkspace().DPIUnscale( iMouseY ) );
	}

	//------------------------------------------------------------------------------------------------
	void HideItemInfo()
	{
		if ( !m_pItemInfo )
			return;
		m_pItemInfo.Hide();
	}

	//------------------------------------------------------------------------------------------------
	void SetSlotFocused( SCR_InventorySlotUI pFocusedSlot, SCR_InventoryStorageBaseUI pFromStorageUI, bool bFocused )
	{
		if( bFocused )
		{
			InputManager pInputManager = GetGame().GetInputManager();
			if ( !( pInputManager && pInputManager.IsUsingMouseAndKeyboard() ) )
			{
				if ( m_pActiveStorageUI != pFromStorageUI )
				{
					if ( m_pActiveStorageUI )
					{
						SCR_InventorySlotUI pLastFocusedSlot = pFromStorageUI.GetLastFocusedSlot();
						pFromStorageUI.SetSlotFocused( pLastFocusedSlot );
						if ( pLastFocusedSlot )
						{
							m_bLocked = true;
							GetGame().GetWorkspace().SetFocusedWidget( pLastFocusedSlot.GetButtonWidget(), true );
							m_bLocked = false;
						}
					}

					m_pActiveStorageUI = pFromStorageUI;
				}
			}

			m_pFocusedSlotUI = pFocusedSlot;
			SetFocusedSlotEffects();
		}
		else
		{
			if ( !m_bLocked )
			{
				if ( m_pActiveStorageUI )
					m_pActiveStorageUI.SetLastFocusedSlot( pFocusedSlot	 );

				HideItemInfo();
				m_pFocusedSlotUI = null;
				NavigationBarUpdate();
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void SetFocusedSlotEffects()
	{
		if( !m_pFocusedSlotUI )
		{
			if( m_wProgressBarWeightItem )
				m_wProgressBarWeightItem.SetCurrent( 0.0 );
			return;
		}

		//show info about the item
		InventoryItemComponent invItemComp = m_pFocusedSlotUI.GetInventoryItemComponent();
		if ( !invItemComp )
			return;
		auto attribs = SCR_ItemAttributeCollection.Cast( invItemComp.GetAttributes() );

		if ( !attribs )
			return;
		UIInfo itemInfo = attribs.GetUIInfo();
		if ( !itemInfo )
			HideItemInfo();
		else
			ShowItemInfo( itemInfo.GetName(), itemInfo.GetDescription(), invItemComp.GetTotalWeight() );
	
		//show the weight on the progressbar
		//TODO: overlap or add on the end, depending on if the item is already in the storage or is going to be added
		if( m_wProgressBarWeightItem )
		{
			float weight = invItemComp.GetTotalWeight();
			m_wProgressBarWeightItem.SetCurrent( weight );
		};

		NavigationBarUpdate();
	}

	//------------------------------------------------------------------------------------------------
	//!
	protected EDropContainer IsFocusedItemInsideDropContainer()
	{
		if ( !m_pActiveHoveredStorageUI )
			return EDropContainer.NOCONTAINER;
		if ( m_pActiveHoveredStorageUI.GetFocusedSlot() == m_pFocusedSlotUI )
			return EDropContainer.ISINSIDE;

		return EDropContainer.ISNOTINSIDE;
	}

	//------------------------------------------------------------------------------------------------
	protected bool CanSwapItems(SCR_InventorySlotUI slot1, SCR_InventorySlotUI slot2)
	{
		if (!slot1 || !slot2)
			return false;

		return m_InventoryManager.CanSwapItemStorages(
			slot1.GetInventoryItemComponent().GetOwner(),
			slot2.GetInventoryItemComponent().GetOwner()
		);
	}

	//------------------------------------------------------------------------------------------------
	protected bool CanMoveItem(SCR_InventorySlotUI slot1, SCR_InventorySlotUI slot2)
	{
		if (!slot1 || !slot2)
			return false;

		return m_InventoryManager.CanMoveItemToStorage(
			slot1.GetInventoryItemComponent().GetOwner(),
			slot2.GetStorageComponent()
		);
	}

	//------------------------------------------------------------------------------------------------
	protected bool CanMoveItem(SCR_InventorySlotUI slot1, SCR_InventoryStorageBaseUI slot2)
	{
		if (!slot1 || !slot2)
			return false;

		return m_InventoryManager.CanMoveItemToStorage(
			slot1.GetInventoryItemComponent().GetOwner(),
			slot2.GetStorage()
		);
	}

	//------------------------------------------------------------------------------------------------
	void NavigationBarUpdate()
	{
		if (!m_pNavigationBar)
			return;

		if (IsUsingGamepad())
		{
			NavigationBarUpdateGamepad();
			return;
		}

		m_pNavigationBar.SetAllButtonEnabled( false );
		m_pNavigationBar.SetButtonEnabled( "ButtonClose", true );

		if ( !m_pFocusedSlotUI )
			return;
		if ( m_pFocusedSlotUI.GetStorageUI() == m_pQuickSlotStorage )
			return;

		m_pNavigationBar.SetButtonEnabled( "ButtonSelect", true );
		m_pNavigationBar.SetButtonEnabled( "ButtonDrop", true );

		if ( m_pFocusedSlotUI.GetStorageUI() == m_pStorageLootUI )
			m_pNavigationBar.SetButtonActionName( "ButtonDrop", "Inventory_Pickup" );
		else
			m_pNavigationBar.SetButtonActionName( "ButtonDrop", "Inventory_Drop" );

		m_pNavigationBar.SetButtonEnabled( "ButtonStepBack", true );
		m_pNavigationBar.SetButtonEnabled( "ButtonInspect", true );

		HandleSlottedItemFunction();
	}

	//------------------------------------------------------------------------------------------------
	void HandleSlottedItemFunction()
	{
		string sAction = "Inventory_Select";

		switch ( m_pFocusedSlotUI.GetSlotedItemFunction() )
		{

			case ESlotFunction.TYPE_GADGET:
			{
				// m_pNavigationBar.SetButtonEnabled( "ButtonEquip", true );
			} break;

			case ESlotFunction.TYPE_WEAPON:
			{
				m_pNavigationBar.SetButtonEnabled( "ButtonEquip", true );
				IEntity item = m_pFocusedSlotUI.GetInventoryItemComponent().GetOwner();
				if (item)
				{
					WeaponComponent weaponComp = WeaponComponent.Cast(item.FindComponent(WeaponComponent));
					if (weaponComp &&
						weaponComp.GetWeaponType() != EWeaponType.WT_FRAGGRENADE &&
						weaponComp.GetWeaponType() != EWeaponType.WT_SMOKEGRENADE)
					{
						m_pNavigationBar.SetButtonEnabled( "ButtonOpenStorage", true );		// look into it
					}
				}
			} break;

			case ESlotFunction.TYPE_MAGAZINE:
			{
				// TODO: show the Reload action
				//m_pNavigationBar.SetButtonEnabled( "ButtonUse", true );
			} break;

			case ESlotFunction.TYPE_CONSUMABLE:
			{
				// TODO: show the Consume action
				m_pNavigationBar.SetButtonEnabled( "ButtonUse", true );
			} break;

			case ESlotFunction.TYPE_STORAGE:
			{
				if( m_EStateMenuItem == EStateMenuItem.STATE_MOVING_ITEM_STARTED && m_pFocusedSlotUI != m_pSelectedSlotUI )
				{
					sAction = "Inventory_Move";
					//m_pNavigationBar.SetButtonEnabled( "ButtonSelect", false );
					//m_pNavigationBar.SetButtonEnabled( "ButtonMove", true );
				}
				// Enable in case the storage is not "togglable" - can be only shown and only opening another storage will close it
				/*else if ( m_EStateMenuStorage == EStateMenuStorage.STATE_OPENED && m_pFocusedSlotUI == m_pSelectedSlotUI && m_pFocusedSlotUI.Type() != SCR_InventorySlotStorageEmbeddedUI)
				{
					m_pNavigationBar.SetButtonEnabled( "ButtonSelect", false );
				}*/
				else if ( m_pFocusedSlotUI.Type() == SCR_InventorySlotStorageEmbeddedUI )
				{
					m_pNavigationBar.SetButtonEnabled( "ButtonOpenStorage", true );
				}
			} break;

			case ESlotFunction.TYPE_HEALTH:
			{
				// TODO: show the Heal action
				m_pNavigationBar.SetButtonEnabled( "ButtonUse", true );
			} break;
		}

		HandleSelectButtonState( sAction );
	}

	//------------------------------------------------------------------------------------------------
	void NavigationBarUpdateGamepad()
	{
		m_pNavigationBar.SetAllButtonEnabled(false);
		m_pNavigationBar.SetButtonEnabled("ButtonSelect", true);

		if (m_bStorageSwitchMode)
		{
			m_CloseButton.SetLabel("#AR-Inventory_Close");
			bool shouldShowMove = (m_pSelectedSlotUI != null);
			if (m_pActiveStorageUI)
				shouldShowMove &= m_pActiveStorageUI.IsStorageHighlighted();
			m_pNavigationBar.SetButtonEnabled("ButtonMove", shouldShowMove);
			m_pNavigationBar.SetButtonEnabled("ButtonSelect", !m_pSelectedSlotUI);
		}
		else
		{
			m_pNavigationBar.SetButtonEnabled("ButtonMove", m_pSelectedSlotUI != null);
			m_pNavigationBar.SetButtonEnabled("ButtonSwap", m_pSelectedSlotUI != null);
		}

		if (!m_bStorageSwitchMode &&
			m_pActiveStorageUI != m_pStorageLootUI &&
			m_pActiveStorageUI != m_pStorageListUI)
		{
			m_pNavigationBar.SetButtonEnabled("ButtonQuickSlotAssign", true);
		}

		if (m_pActiveStorageUI == m_pQuickSlotStorage)
		{
			bool itmToAssign = m_pItemToAssign != null;
			m_pNavigationBar.SetAllButtonEnabled(false);
			m_pNavigationBar.SetButtonEnabled("ButtonClose", true);
			if (m_bStorageSwitchMode)
				m_pNavigationBar.SetButtonEnabled("ButtonSelect", true);

			m_pNavigationBar.SetButtonEnabled("ButtonQuickSlotAssign", itmToAssign);
			m_pNavigationBar.SetButtonEnabled("ButtonQuickSlotUnassign",
				!itmToAssign &&
				m_pFocusedSlotUI != null
			);
		}

		if (!m_pFocusedSlotUI)
			return;
		
		if (m_CloseButton)
			m_CloseButton.SetLabel("#AR-Menu_Back");
		m_pNavigationBar.SetButtonEnabled("ButtonDrop",
			m_pSelectedSlotUI == null &&
			m_pActiveStorageUI != m_pQuickSlotStorage
		);

		if (m_pFocusedSlotUI.GetStorageUI() == m_pStorageLootUI)
			m_pNavigationBar.SetButtonActionName("ButtonDrop", "Inventory_Pickup");
		else
			m_pNavigationBar.SetButtonActionName("ButtonDrop", "Inventory_Drop");

		if (m_pActiveStorageUI != m_pQuickSlotStorage)
			HandleSlottedItemFunction();
	}

	//------------------------------------------------------------------------------------------------
	protected void HandleSelectButtonState( string sAction = "Inventory_Select" )
	{
		//TODO: this can be done better

		if ( sAction == "Inventory_Move" )
			m_pNavigationBar.SetButtonActionName( "ButtonSelect", sAction );
		else
		{
			if ( !m_pFocusedSlotUI.IsSlotSelected() )
				m_pNavigationBar.SetButtonActionName( "ButtonSelect", sAction );
			else
				m_pNavigationBar.SetButtonActionName( "ButtonSelect", "Inventory_Deselect" );
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Returns the inspection screen UI
	SCR_InventoryInspectionUI GetInspectionScreen()
	{
		return m_InspectionScreen;
	}

	//------------------------------------------------------------------------------------------------
	//! Opens the inspection screen for the item slot
	void InspectItem(SCR_InventorySlotUI itemSlot)
	{
		if (m_InspectionScreen)
		{
			m_InspectionScreen.Destroy();
			m_InspectionScreen = null;
		}

		if (!m_Player)
			return;

		if (itemSlot)
			m_InspectionScreen = SCR_InventoryInspectionUI(itemSlot.GetInventoryItemComponent(), itemSlot.GetStorageUI(), m_Player.GetWorld());
	}


	//------------------------------------------------------------------------------------------------
	//! shows only the storages the item can be stored into
	protected void FilterOutStorages( bool bShow = true )
	{
		//Get all slots from the storage list UI
		array<SCR_InventorySlotUI> pSlotsInListUI = new array<SCR_InventorySlotUI>();
		m_pStorageListUI.GetSlots( pSlotsInListUI );
		//foreach ( SCR_InventorySlotUI pSlotFromUI : pSlotsInListUI )
		//	RegisterUIStorage( pSlotFromUI );

		if ( m_pStorageLootUI )
			m_pStorageLootUI.GetSlots( pSlotsInListUI );
		if ( GetActualStorageInCharacterStorageUI() )
			GetActualStorageInCharacterStorageUI().GetSlots( pSlotsInListUI );
		if ( m_pQuickSlotStorage )
			m_pQuickSlotStorage.GetSlots( pSlotsInListUI );
		if ( m_pWeaponStorage )
			m_pWeaponStorage.GetSlots( pSlotsInListUI );

		BaseInventoryStorageComponent pStorageTo;
		foreach ( SCR_InventorySlotUI pStorageSlotUI : pSlotsInListUI )
		{
			if ( !pStorageSlotUI )
				continue;

			pStorageTo = pStorageSlotUI.GetAsStorage();

			if ( bShow )
			{
				if ( !m_pSelectedSlotUI )
					continue;
				if ( pStorageSlotUI.GetAsStorage() == m_pSelectedSlotUI.GetStorageUI().GetCurrentNavigationStorage() )	//it's the same storage as the selected item comes from
				{
					pStorageSlotUI.SetEnabledForMove( 0 );
					continue;
				}

				InventoryItemComponent pInventoryItem = m_pSelectedSlotUI.GetInventoryItemComponent();
				if ( !pInventoryItem )
					continue;

				IEntity pItem = pInventoryItem.GetOwner();

				if ( pStorageTo )
				{
					bool bCanStore = true;

					if ( m_InventoryManager.CanInsertItemInActualStorage( pItem, pStorageTo ) )
					{
						if ( pStorageTo.Type() == SCR_InventoryStorageWeaponsUI )
							if ( !m_pWeaponStorageComp.Contains( pStorageTo.GetOwner() ) )
								bCanStore = false;

						if ( bCanStore )
							pStorageSlotUI.SetEnabledForMove( 1 );
						else
							pStorageSlotUI.SetEnabledForMove( 0 );
					}
					else
					{
						pStorageSlotUI.SetEnabledForMove( 0 );
					}
				}
				else
				{
					pStorageSlotUI.SetEnabledForMove( 0 );
				}

			}
			else
			{
				pStorageSlotUI.SetEnabledForMove( 2 );
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void FilterOutItems( bool bFilterOut )
	{
		array<SCR_InventorySlotUI> aItemsUIInLoot = {};

		//if ( m_pStorageLootUI )
			//aItemsUIInLoot.InsertAll( m_pStorageLootUI.GetUISlots() );
		if ( m_pStorageListUI )
			aItemsUIInLoot.InsertAll( m_pStorageListUI.GetUISlots() );
		//if ( m_pStorageBaseUI )
			//aItemsUIInLoot.InsertAll( m_pStorageBaseUI.GetUISlots() );

		PrintFormat( "INV: Filtering out items %1", 1.5 );

		foreach ( SCR_InventorySlotUI pSlot : aItemsUIInLoot )
		{
			if ( !pSlot.GetStorageComponent() )
			{
				if ( bFilterOut )
				{
					PrintFormat( "INV: Disabling slot %1", pSlot );
					pSlot.SetEnabledForMove( 0 );
				}
				else
				{
					PrintFormat( "INV: Reseting slot %1", pSlot );
					pSlot.SetEnabledForMove( 2 );
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void Action_TryCloseInventory()
	{
		if (m_bWasJustTraversing)
		{
			m_bWasJustTraversing = false;
			return;
		}

		if (IsUsingGamepad())
		{
			if (!m_bStorageSwitchMode)
			{
				SetStorageSwitchMode(true);
				return;
			}
			else
			{
				if (m_pSelectedSlotUI)
				{
					DeselectSlot();				
				}
				else
				{
					Action_CloseInventory();
					return;
				}

				SetStorageSwitchMode(false);
				ResetHighlightsOnAvailableStorages();
				FocusOnSlotInStorage(m_pActiveStorageUI);
				return;
			}
		}
		else
		{
			DeselectSlot();
		}

		Action_CloseInventory();
	}

	//------------------------------------------------------------------------------------------------
	protected void Action_CloseInventory()
	{
		GetGame().GetInputManager().RemoveActionListener( "Inventory_Drag", EActionTrigger.DOWN, Action_DragDown );
		GetGame().GetInputManager().RemoveActionListener( "Inventory", EActionTrigger.DOWN, Action_CloseInventory );
		//m_bColdStart = false; 
		InspectItem(null);
		if (m_pVicinity)
		{
			m_pVicinity.ManipulationComplete();
		}

		/*
		TODO: enable once the performance issue with invokers is solved in characterdamagemanager
		if ( m_CharDamageManager )
			m_CharDamageManager.GetOnDamageStateChanged().Remove( OnDamageStateChanged );
		*/

		auto playerController = GetGame().GetPlayerController();
		if (!playerController)
			return;

		auto menuManager = GetGame().GetMenuManager();
		auto menu = ChimeraMenuPreset.Inventory20Menu;

		auto inventoryMenu = menuManager.FindMenuByPreset( menu ); // prototype inventory
		if (inventoryMenu)
			menuManager.CloseMenuByPreset( menu );
		if  (m_PlayerRenderAttributes)
		{
			m_PlayerRenderAttributes.ResetDeltaRotation();
		}
		if (m_Player)
		{
			SCR_CharacterControllerComponent charController = SCR_CharacterControllerComponent.Cast(m_Player.FindComponent(SCR_CharacterControllerComponent));
			if (charController)
			{
				charController.m_OnPlayerDeath.Remove(Action_CloseInventory);
			}
		}
		if (m_pCharacterWidgetHelper)
			m_pCharacterWidgetHelper.Destroy();
		m_pCharacterWidgetHelper = null;

		HideItemInfo();
		if ( m_pItemInfo )
			m_pItemInfo.Destroy();
		m_pItemInfo = null;

		if (m_InventoryManager)
		{
			m_InventoryManager.m_OnItemAddedInvoker.Remove( OnItemAdded );
			m_InventoryManager.m_OnItemRemovedInvoker.Remove( OnItemRemoved );

			m_InventoryManager.OnInventoryMenuClosed();
		}

		SCR_UISoundEntity.SoundEvent("SOUND_INV_CLOSE");
	}

	//------------------------------------------------------------------------------------------------
	//!
	protected void SimpleFSM( EMenuAction EAction = EMenuAction.ACTION_SELECT  )
	{
		switch (EAction)
		{
			case EMenuAction.ACTION_MOVEINSIDE:
			{
				Action_MoveItemToStorage(m_pActiveStorageUI);
				if (IsUsingGamepad())
					SetStorageSwitchMode(true);
				NavigationBarUpdate();
				if (m_pSelectedSlotUI)
				{
					m_pSelectedSlotUI.SetSelected(false);
					m_pSelectedSlotUI = null;
				}
				FocusOnSlotInStorage(m_pActiveStorageUI);
				ResetHighlightsOnAvailableStorages();
			} break;

			case EMenuAction.ACTION_SELECT:
			{
				if (!m_pFocusedSlotUI)
					return;

				if (m_pSelectedSlotUI)
					m_pSelectedSlotUI.SetSelected(false);

				if (IsUsingGamepad())
				{
					m_pSelectedSlotUI = m_pFocusedSlotUI;
					m_pSelectedSlotUI.SetSelected(true);
					HighlightAvailableStorages(m_pSelectedSlotUI);
				}

				NavigationBarUpdate();
				if (IsUsingGamepad())
					SetStorageSwitchMode(true);
			} break;

			case EMenuAction.ACTION_DESELECT:
			{
				if (m_pSelectedSlotUI)
				{
					ResetHighlightsOnAvailableStorages();
					m_pSelectedSlotUI.SetSelected(false);
					m_pSelectedSlotUI = null;
				}

				NavigationBarUpdate();
			} break;

			case EMenuAction.ACTION_DRAGGED:
			{
				m_EStateMenuItem = EStateMenuItem.STATE_MOVING_ITEM_STARTED;
				SCR_UISoundEntity.SoundEvent("SOUND_INV_CONTAINER_DRAG");
			} break;

			case EMenuAction.ACTION_DROPPED:
			{
				Action_Drop();
				if (m_pActiveHoveredStorageUI)
					m_pActiveHoveredStorageUI.ShowContainerBorder(false);
				m_EStateMenuItem = EStateMenuItem.STATE_IDLE;
				return;
			} break;

			case EMenuAction.ACTION_MOVEBETWEEN:
			{
				if (m_pFocusedSlotUI)
				{
					m_pSelectedSlotUI = m_pFocusedSlotUI;
					SCR_InventoryStorageBaseUI pStorage = m_pFocusedSlotUI.GetStorageUI();
					if (pStorage != m_pStorageLootUI)
					{
						MoveBetweenToVicinity();
						SCR_UISoundEntity.SoundEvent("SOUND_INV_VINICITY_DROP_CLICK");
					}
					else
					{
						MoveBetweenFromVicinity();
						SCR_UISoundEntity.SoundEvent("SOUND_INV_PICKUP_CLICK");
					}
					m_pSelectedSlotUI = null;
				}
				ResetHighlightsOnAvailableStorages();
			} break;

			case EMenuAction.ACTION_UNFOLD:
			{
				if ( m_pFocusedSlotUI.GetStorageUI() == m_pStorageListUI ) //if it is slot in the "storage list ui"
				{
					if ( BaseInventoryStorageComponent.Cast( m_pFocusedSlotUI.GetAsStorage() ) )	// and if it is a storage
					{
						//ShowStorage( m_pFocusedSlotUI.GetAsStorage() ); 		//show the content of the actualy selected
						ToggleStorageContainerVisibility( m_pFocusedSlotUI );
						m_EStateMenuStorage = EStateMenuStorage.STATE_OPENED;
					}
					else
					{
						//CloseOpenedStorage();	// if it is not storage, show nothing
						//ToggleStorageContainerVisibility( m_pFocusedSlotUI.GetAsStorage() );
						m_EStateMenuStorage = EStateMenuStorage.STATE_IDLE;
					}
				}
				else
				{
					TraverseActualSlot();
					NavigationBarUpdate();
					m_EStateMenuStorage = EStateMenuStorage.STATE_OPENED;
				}
			} break;
		}

		if (!IsUsingGamepad())
			m_pSelectedSlotUI = m_pFocusedSlotUI;

		HideItemInfo();
	}

	//------------------------------------------------------------------------------------------------
	//!
	protected void Action_Drop()
	{
		if ( IsFocusedItemInsideDropContainer() != EDropContainer.NOCONTAINER )		//dropped to a container
		{
			if ( m_pFocusedSlotUI )
			{
				if ( IsFocusedItemInsideDropContainer() == EDropContainer.ISINSIDE )
				{
					if ( SCR_InventorySlotStorageUI.Cast( m_pFocusedSlotUI ) )	// storage
					{
						MoveItemToStorageSlot();
					}
					else if ( SCR_InventorySlotWeaponUI.Cast( m_pFocusedSlotUI ) )	// weapon slot
					{
						MoveItemToStorageSlot();
					}
					else if ( SCR_InventorySlotUI.Cast( m_pFocusedSlotUI ) )	// simple slot
					{
						if ( SCR_InventoryStorageQuickSlotsUI.Cast( m_pFocusedSlotUI.GetStorageUI() ) )	//quick slot
						{
							//m_pFocusedSlotUI.m_iQuickSlotIndex
							SetItemToQuickSlotDrop();
						}
						else
						{
							MoveItemToStorageSlot();
							/*
							if ( m_pFocusedSlotUI.GetInventoryItemComponent() )
							{
								MoveItemToStorageSlot();
							}
							else
							{
								//we are inserting into empy slot
								m_InventoryManager.EquipGadget( m_pSelectedSlotUI.GetInventoryItemComponent().GetOwner() );
							}
								*/
						}

					}
				}
			}
			else
			{
					// just container
					MoveItem();
			}
		}
		else
		{
			if (WidgetManager.GetWidgetUnderCursor() == m_wPlayerRender)
			{
				EquipDraggedItem(true);
			}
			else
			{
				//dropped outside of a container
				RemoveItemFromQuickSlotDrop();
			}
		}
		
		ResetHighlightsOnAvailableStorages();
	}

	//------------------------------------------------------------------------------------------------
	void Action_QuickSlotAssign()
	{
		if (m_pItemToAssign && m_pActiveStorageUI == m_pQuickSlotStorage)
		{
			int slotId = m_pQuickSlotStorage.GetFocusedSlotId() + 1;
			SetItemToQuickSlot(slotId, m_pItemToAssign);
			FocusOnSlotInStorage(m_pQuickSlotStorage, slotId - 1);
			m_pItemToAssign = null;
		}
		else
		{
			if (m_pActiveStorageUI != m_pStorageLootUI && m_pActiveStorageUI != m_pQuickSlotStorage)
			{
				m_pItemToAssign = m_pFocusedSlotUI;
				FocusOnSlotInStorage(m_pQuickSlotStorage, 3);
			}
		}

		NavigationBarUpdate();
	}

	//------------------------------------------------------------------------------------------------
	void Action_QuickSlotUnassign()
	{
		if (!m_pFocusedSlotUI)
			return;

		InventoryItemComponent itemComp = m_pFocusedSlotUI.GetInventoryItemComponent();
		if (!itemComp)
			return;

		IEntity item = itemComp.GetOwner();
		if (!item)
			return;

		int slotId = m_pQuickSlotStorage.GetFocusedSlotId();
		slotId = Math.Clamp(slotId, 0, m_StorageManager.GetQuickSlotItems().Count());
		m_pItemToAssign = null;
		m_StorageManager.RemoveItemFromQuickSlot(item);
		ShowQuickSlotStorage();
		FocusOnSlotInStorage(m_pQuickSlotStorage, slotId);
		NavigationBarUpdate();
	}

	//------------------------------------------------------------------------------------------------
	//!
	void MoveItemToStorageSlot()
	{
		if (!m_pSelectedSlotUI)
			return;
		
		InventoryItemComponent pComp = m_pSelectedSlotUI.GetInventoryItemComponent();
		if ( !pComp )
			return;

		IEntity pItem = pComp.GetOwner();
		if (!m_InventoryManager.CanMoveItem(pItem))
			return;
		m_pCallBack.m_pItem = pItem;
		m_pCallBack.m_pMenu = this;
		m_pCallBack.m_pStorageFrom = m_pSelectedSlotUI.GetStorageUI();
		m_pCallBack.m_pStorageTo = m_pFocusedSlotUI.GetStorageUI();

		BaseInventoryStorageComponent pStorageFromComponent = m_pSelectedSlotUI.GetStorageUI().GetCurrentNavigationStorage();
		BaseInventoryStorageComponent pStorageToComponent = m_pFocusedSlotUI.GetAsStorage();

		if ( !m_InventoryManager.EquipAny( m_StorageManager , pItem, 0, m_pCallBack ) )
			m_InventoryManager.InsertItem( pItem, pStorageToComponent, pStorageFromComponent, m_pCallBack );
		/*
		if ( pItem.FindComponent( SCR_GadgetComponent ) )
			m_InventoryManager.EquipGadget( pItem, m_pCallBack );
		else
		*/
	}

	//------------------------------------------------------------------------------------------------
	//!
	void MoveItem( SCR_InventoryStorageBaseUI pStorageBaseUI = null )
	{
		if ( !m_pSelectedSlotUI )
			return;
		InventoryItemComponent pComp = m_pSelectedSlotUI.GetInventoryItemComponent();
		if ( !pComp )
			return;
		IEntity pItem = pComp.GetOwner();
		if (!m_InventoryManager.CanMoveItem(pItem))
			return;
		m_pCallBack.m_pItem = pItem;
		m_pCallBack.m_pMenu = this;
		m_pCallBack.m_pStorageToFocus = m_pSelectedSlotUI.GetStorageUI();

		m_pCallBack.m_pStorageFrom = m_pSelectedSlotUI.GetStorageUI();
		if ( pStorageBaseUI )
			m_pCallBack.m_pStorageTo = pStorageBaseUI;
		else
			m_pCallBack.m_pStorageTo = m_pActiveHoveredStorageUI;

		BaseInventoryStorageComponent pStorageTo = m_pActiveHoveredStorageUI.GetCurrentNavigationStorage();

		if ( pStorageTo && EquipedWeaponStorageComponent.Cast( pStorageTo ) )
		{
			m_InventoryManager.EquipWeapon( pItem, m_pCallBack, m_pCallBack.m_pStorageFrom == m_pStorageLootUI );
			SCR_UISoundEntity.SoundEvent("SOUND_INV_VINICITY_EQUIP_CLICK");
			return;
		}
		else if ( pStorageTo && CharacterInventoryStorageComponent.Cast( pStorageTo ) )
		{
			m_InventoryManager.EquipAny( pStorageTo, pItem, 0, m_pCallBack );
		}
		else if ( m_pCallBack.m_pStorageTo == m_pStorageLootUI )
		{
			MoveToVicinity( pItem );
			SCR_UISoundEntity.SoundEvent("SOUND_INV_VINICITY_DROP_CLICK");
		}
		else
		{
			if ( m_pCallBack.m_pStorageFrom == m_pStorageLootUI )
			{
				MoveFromVicinity();
				SCR_UISoundEntity.SoundEvent("SOUND_INV_PICKUP_CLICK");
			}
			else
			{
				m_InventoryManager.InsertItem( pItem, m_pActiveHoveredStorageUI.GetCurrentNavigationStorage(), m_pCallBack.m_pStorageFrom.GetStorage(), m_pCallBack );
				SCR_UISoundEntity.SoundEvent("SOUND_INV_CONTAINER_DIFR_DROP");
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	protected void EquipWeapon()
	{
		if ( !m_pSelectedSlotUI )
			return;
		IEntity pItem = m_pSelectedSlotUI.GetInventoryItemComponent().GetOwner();
		if ( !pItem )
			return;
		
		if ( m_InventoryManager )
			m_InventoryManager.EquipWeapon( pItem, m_pCallBack );
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	protected void MoveToVicinity( IEntity pItem )
	{
		BaseInventoryStorageComponent pStorageFrom = m_pSelectedSlotUI.GetStorageUI().GetStorage();
			
		if (!m_InventoryManager.CanMoveItem(pItem))
			return;
		 
		m_pCallBack.m_pStorageFrom = m_pSelectedSlotUI.GetStorageUI();
		m_pCallBack.m_pStorageTo = m_pStorageLootUI;
		auto storage = m_pStorageLootUI.GetCurrentNavigationStorage();
		if ( storage )
			m_InventoryManager.InsertItem( pItem, m_pStorageLootUI.GetCurrentNavigationStorage(), pStorageFrom, m_pCallBack );	// moving into the opened storage in the  vicinity
		else
		{
			//droping it on the ground
			auto pSlot = m_pSelectedSlotUI.GetInventoryItemComponent().GetParentSlot();
			if ( pSlot )
				m_InventoryManager.TryRemoveItemFromInventory(pItem, pSlot.GetStorage(), m_pCallBack);
		}	
	}
		
	//------------------------------------------------------------------------------------------------
	//!
	protected void MoveFromVicinity()
	{
		IEntity pItem = m_pSelectedSlotUI.GetInventoryItemComponent().GetOwner();
		if (!m_InventoryManager.CanMoveItem(pItem))
			return;

		m_pCallBack.m_pItem = pItem;
		m_pCallBack.m_pMenu = this;

		m_pCallBack.m_pStorageFrom = m_pStorageLootUI;
		m_pCallBack.m_pStorageTo = m_pActiveHoveredStorageUI;

		m_InventoryManager.InsertItem( pItem, m_pActiveHoveredStorageUI.GetCurrentNavigationStorage(), m_pStorageLootUI.GetCurrentNavigationStorage(), m_pCallBack ); 			//a storage is already opened, try to move it there
	}

	//------------------------------------------------------------------------------------------------
	//!
	protected void MoveBetweenFromVicinity()
	{
		IEntity pItem = m_pSelectedSlotUI.GetInventoryItemComponent().GetOwner();
		if (!m_InventoryManager.CanMoveItem(pItem))
			return;

		m_pCallBack.m_pItem = pItem;
		m_pCallBack.m_pMenu = this;

		m_pCallBack.m_pStorageFrom = m_pStorageLootUI;
		m_pCallBack.m_pStorageTo = m_pStorageListUI;
		m_pCallBack.m_pStorageToFocus = m_pStorageLootUI;
		m_pCallBack.m_sItemToFocus = m_pSelectedSlotUI.GetItemName();

		BaseInventoryStorageComponent pStorageTo = m_StorageManager;
		if ( pItem.FindComponent( SCR_GadgetComponent ) )
		{
			BaseInventoryStorageComponent pStorageComp = m_InventoryManager.FindStorageForItem( pItem, EStoragePurpose.PURPOSE_EQUIPMENT_ATTACHMENT );
			if ( pStorageComp )
				pStorageTo = pStorageComp;
			else
			{
				pStorageComp = m_InventoryManager.FindStorageForItem( pItem, EStoragePurpose.PURPOSE_DEPOSIT );
				if ( pStorageComp )
					pStorageTo = pStorageComp;
			}
		}

		if ( !m_InventoryManager.EquipAny( pStorageTo , pItem, 0, m_pCallBack ) )
			m_InventoryManager.InsertItem( pItem, null, m_pStorageLootUI.GetCurrentNavigationStorage(), m_pCallBack );
	}

	//------------------------------------------------------------------------------------------------
	protected void EquipDraggedItem(bool forceEquip = false)
	{
		if (!m_pSelectedSlotUI)
			return;

		IEntity pItem = m_pSelectedSlotUI.GetInventoryItemComponent().GetOwner();
		if (!pItem)
			return;

		m_pCallBack.m_pItem = pItem;
		m_pCallBack.m_pMenu = this;
		m_pCallBack.m_pStorageFrom = m_pSelectedSlotUI.GetStorageUI();

		m_pCallBack.m_pStorageToFocus = m_pSelectedSlotUI.GetStorageUI();
		m_pCallBack.m_sItemToFocus = m_pSelectedSlotUI.GetItemName();
		m_pCallBack.m_bShouldEquip = forceEquip;

		BaseInventoryStorageComponent pStorageTo = m_StorageManager;
		if (m_pSelectedSlotUI.GetStorageUI().Type() == SCR_InventoryStorageWeaponsUI)
		{
			m_pCallBack.m_pStorageTo = m_pWeaponStorage;
		}
		else
		{
			m_pCallBack.m_pStorageTo = m_pStorageListUI;
		}

		if (pItem.FindComponent(SCR_GadgetComponent))
		{
			BaseInventoryStorageComponent pStorageComp = m_InventoryManager.FindStorageForItem( pItem, EStoragePurpose.PURPOSE_EQUIPMENT_ATTACHMENT );
			if (pStorageComp)
				pStorageTo = pStorageComp;
			else
			{
				pStorageComp = m_InventoryManager.FindStorageForItem(pItem, EStoragePurpose.PURPOSE_DEPOSIT);
				if (pStorageComp)
					pStorageTo = pStorageComp;
			}
		}

		if (pItem.FindComponent(WeaponComponent))
		{
			BaseInventoryStorageComponent pStorageComp = m_InventoryManager.FindStorageForItem(pItem, EStoragePurpose.PURPOSE_WEAPON_PROXY);
			if (pStorageComp)
				pStorageTo = pStorageComp;
			else
			{
				pStorageComp = m_InventoryManager.FindStorageForItem(pItem, EStoragePurpose.PURPOSE_DEPOSIT);;
				if (pStorageComp)
					pStorageTo = pStorageComp;
			}
		}

		bool equip = m_InventoryManager.EquipAny(pStorageTo , pItem, 0, m_pCallBack);
		if (!equip && m_pSelectedSlotUI)
			m_InventoryManager.InsertItem(pItem, null, m_pSelectedSlotUI.GetStorageUI().GetCurrentNavigationStorage(), m_pCallBack);
	}

	//------------------------------------------------------------------------------------------------
	//!
	protected void MoveBetweenToVicinity()
	{
		IEntity pItem = m_pSelectedSlotUI.GetInventoryItemComponent().GetOwner();
		if (!m_InventoryManager.CanMoveItem(pItem))
			return;

		m_pCallBack.m_pItem = pItem;
		m_pCallBack.m_pMenu = this;
		m_pCallBack.m_pStorageToFocus = m_pSelectedSlotUI.GetStorageUI();
		m_pCallBack.m_sItemToFocus = m_pSelectedSlotUI.GetItemName();

		MoveToVicinity( pItem );
	}

	//------------------------------------------------------------------------------------------------
	void DeselectSlot()
	{
		if (m_pSelectedSlotUI)
		{
			m_pSelectedSlotUI.SetSelected(false);
			m_pSelectedSlotUI = null;
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	protected void HandleItemSelection()
	{
		if ( m_pFocusedSlotUI && m_pSelectedSlotUI && m_pFocusedSlotUI != m_pSelectedSlotUI )
		{
			if ( m_bDraggingEnabled )
			{
				m_pFocusedSlotUI.SetSelected( true );			//select new one
				m_pSelectedSlotUI.SetSelected( false );		//deselect the previous one
				m_pSelectedSlotUI = m_pFocusedSlotUI;			//and register the newly selected
				FilterOutStorages( true );
				m_EStateMenuItem = EStateMenuItem.STATE_MOVING_ITEM_STARTED;
			}
		}
		else
		{
			if ( m_bDraggingEnabled )
			{
				if ( m_pSelectedSlotUI )
					m_pSelectedSlotUI.SetSelected( false );		//deselect it
				m_pSelectedSlotUI = null;						//and deregister it
				FilterOutStorages( false );
				m_EStateMenuItem = EStateMenuItem.STATE_IDLE;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SetOpenStorage()
	{
		IEntity m_pStorageToOpen = GetInventoryStorageManager().GetStorageToOpen();
		if (!m_pStorageToOpen)
			return;

		BaseInventoryStorageComponent comp = BaseInventoryStorageComponent.Cast(m_pStorageToOpen.FindComponent(BaseInventoryStorageComponent));
		if (!comp)
			return;

		SCR_InventoryStorageBaseUI storageUI = GetStorageUIFromVicinity(comp);
		if (!storageUI)
			return;
		if (storageUI.IsTraversalAllowed())
		{
			storageUI.Traverse(comp);
		}

		SetStorageSwitchMode(false);
		ButtonWidget lastCloseTraverseButton = storageUI.GetLastCloseTraverseButton();

		if (!FocusOnSlotInStorage(storageUI, 0, false) && lastCloseTraverseButton && IsUsingGamepad())
			GetGame().GetWorkspace().SetFocusedWidget(lastCloseTraverseButton);
		else
			FocusOnSlotInStorage(storageUI);
	}

	//------------------------------------------------------------------------------------------------
	//!
	protected void TraverseActualSlot()
	{
		m_pSelectedSlotUI = m_pFocusedSlotUI;
		auto storage = m_pFocusedSlotUI.GetAsStorage();
		if (storage)
		{
			auto parentUIContainer = m_pFocusedSlotUI.GetStorageUI();
			if (parentUIContainer.IsTraversalAllowed())
			{
				parentUIContainer.Traverse(storage);
			}
			
			ButtonWidget lastCloseTraverseButton = parentUIContainer.GetLastCloseTraverseButton();
			
			if (!FocusOnSlotInStorage(parentUIContainer, 0, false) && lastCloseTraverseButton && IsUsingGamepad())
				GetGame().GetWorkspace().SetFocusedWidget(lastCloseTraverseButton);
			else
				FocusOnSlotInStorage(parentUIContainer);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! when clicked on item / storage slot using controller or mouse
	protected void Action_SelectItem()
	{
		// Ignore if we cannot interact
		if (!GetCanInteract())
			return;
		SimpleFSM( EMenuAction.ACTION_SELECT );
	}

	//------------------------------------------------------------------------------------------------
	protected void Action_DeselectItem()
	{
		SimpleFSM(EMenuAction.ACTION_DESELECT);
		ResetHighlightsOnAvailableStorages();
	}

	//------------------------------------------------------------------------------------------------
	protected void Action_MoveInsideStorage()
	{
		SimpleFSM(EMenuAction.ACTION_MOVEINSIDE);
		ResetHighlightsOnAvailableStorages();
	}

	//------------------------------------------------------------------------------------------------
	protected void Action_EquipItem()
	{
		// Ignore if we cannot interact
		if (!GetCanInteract())
			return;
		if( !m_pFocusedSlotUI )
			return;

		// TODO: rather check the m_eSlotFunction. The functionality of the item is recognized already in the constructor of the SlotUI
		InventoryItemComponent pInvComponent = m_pFocusedSlotUI.GetInventoryItemComponent();
		if( !pInvComponent )
			return;
		m_pItem = pInvComponent.GetOwner();
		if(!m_InventoryManager.CanMoveItem(m_pItem))
			return;

		SCR_GadgetComponent gadgetComp =  SCR_GadgetComponent.Cast( m_pItem.FindComponent( SCR_GadgetComponent ));
		if( gadgetComp )
		{
			//m_InventoryManager.EquipGadget( m_pItem );
		}
		else
		{
			auto weaponComp =  WeaponComponent.Cast(m_pItem.FindComponent( WeaponComponent ));
			if ( weaponComp )
				MoveWeapon();
		}

		FilterOutStorages( false );
		ResetHighlightsOnAvailableStorages();
		SCR_UISoundEntity.SoundEvent("SOUND_INV_VINICITY_EQUIP_CLICK");
	}

	//------------------------------------------------------------------------------------------------
	protected void Action_MoveBetween()
	{
		// Ignore if we cannot interact
		if (!GetCanInteract())
			return;
		SimpleFSM( EMenuAction.ACTION_MOVEBETWEEN );
		ResetHighlightsOnAvailableStorages();
	}

	//------------------------------------------------------------------------------------------------
	protected void Action_Inspect()
	{
		if (GetInspectionScreen())
		{
			m_widget.SetEnabled(true);
			m_widget.SetVisible(true);
			InspectItem(null);
		}
		else
		{
			m_widget.SetEnabled(false);
			m_widget.SetVisible(false);
			InspectItem(m_pFocusedSlotUI);
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	protected void Action_UseItem()
	{
		if ( !m_pFocusedSlotUI )
			return;
		m_pFocusedSlotUI.Use( m_Player );
	}

	//------------------------------------------------------------------------------------------------
	//!
	protected void Action_UnfoldItem()
	{
 		SimpleFSM( EMenuAction.ACTION_UNFOLD );
	}

	//------------------------------------------------------------------------------------------------
	void Action_StepBack(SCR_InventoryStorageBaseUI pParentStorage = null, int traverseStorageIndex = -1)
	{
		//TODO: m_pActiveStorageUI must be set according the active SCR_InventoryStorageBaseUI
		if (pParentStorage && pParentStorage.IsTraversalAllowed())
		{
			pParentStorage.Back(traverseStorageIndex);
			FilterOutStorages(false);
			m_EStateMenuItem = EStateMenuItem.STATE_IDLE;
			SCR_UISoundEntity.SoundEvent("SOUND_INV_CONTAINER_CLOSE");;
			FocusOnSlotInStorage(pParentStorage);
			m_bWasJustTraversing = true;
		}
		ResetHighlightsOnAvailableStorages();
	}

	//------------------------------------------------------------------------------------------------
	void Action_DragDown()
	{
		if ( m_bDraggingEnabled )
			return;
		if ( !m_pFocusedSlotUI )
			return;

		IEntity item = m_pFocusedSlotUI.GetItem().GetOwner();
		if (Vehicle.Cast(item) || TurretControllerComponent.Cast(item.FindComponent(TurretControllerComponent)))
			return;

		if ( m_pFocusedSlotUI && WidgetManager.GetWidgetUnderCursor() != m_pFocusedSlotUI.GetButtonWidget() )
			return;

		if 	(	m_pFocusedSlotUI.Type() != SCR_InventorySlotUI &&
				m_pFocusedSlotUI.Type() != SCR_InventorySlotLBSUI &&
				m_pFocusedSlotUI.Type() != SCR_InventorySlotStorageUI &&
				m_pFocusedSlotUI.Type() != SCR_InventorySlotStorageEmbeddedUI &&
				m_pFocusedSlotUI.Type() != SCR_InventoryStorageWeaponsUI &&
				m_pFocusedSlotUI.Type() != SCR_InventorySlotWeaponUI &&
				m_pFocusedSlotUI.Type() != SCR_InventorySlotQuickSlotUI
			)
			return;

		m_bDraggingEnabled = false;
		WidgetManager.GetMousePos( m_iMouseX, m_iMouseY );
		GetGame().GetInputManager().AddActionListener( "Inventory_Drag", EActionTrigger.PRESSED, Action_OnDrag );
	}

	//------------------------------------------------------------------------------------------------
	void Action_OnDrag()
	{
		int iMouseX, iMouseY;
		WidgetManager.GetMousePos( iMouseX, iMouseY );
		if ( !m_bDraggingEnabled )
		{
			int dX = Math.AbsInt( iMouseX - m_iMouseX );
			int dY = Math.AbsInt( iMouseY - m_iMouseY );
			if ( ( dX < DRAG_THRESHOLD ) && ( dY < DRAG_THRESHOLD ) )
				return;
			if ( !m_pFocusedSlotUI )
				return;

			m_bDraggingEnabled = true;
			SimpleFSM( EMenuAction.ACTION_DRAGGED );
			m_wDragDropContainer.SetVisible( true );

			ItemPreviewManagerEntity manager = GetGame().GetItemPreviewManager();
			if ( manager && m_pSelectedSlotUI )
			{
				HighlightAvailableStorages(m_pSelectedSlotUI);
				ItemPreviewWidget renderPreview = ItemPreviewWidget.Cast( m_pDragDropPreviewImage );
				IEntity previewEntity = null;
				InventoryItemComponent pComp = m_pSelectedSlotUI.GetInventoryItemComponent();
				if ( pComp )
				{
					previewEntity = pComp.GetOwner();
					if (renderPreview)
						manager.SetPreviewItem(renderPreview, previewEntity);
				}
			}
			ContainerSetPos( iMouseX, iMouseY );
			GetGame().GetInputManager().AddActionListener( "Inventory_Drag", EActionTrigger.UP, Action_DragUp );
		}
		else
		{
			ContainerSetPos( iMouseX, iMouseY );
		}
	}

	//------------------------------------------------------------------------------------------------
	void Action_DragUp()
	{
		GetGame().GetInputManager().RemoveActionListener( "Inventory_Drag", EActionTrigger.PRESSED, Action_OnDrag );
		GetGame().GetInputManager().RemoveActionListener( "Inventory_Drag", EActionTrigger.UP, Action_DragUp );
		m_wDragDropContainer.SetVisible( false );
		SimpleFSM( EMenuAction.ACTION_DROPPED );
		m_bDraggingEnabled = false;
		ResetHighlightsOnAvailableStorages();
	}

	//------------------------------------------------------------------------------------------------
	void ContainerSetPos( int iMouseX, int iMouseY )
	{
		float fWidth, fHeight;
		WorkspaceWidget wWorkSpace = GetGame().GetWorkspace();
		m_wDragDropContainer.GetScreenSize( fWidth, fHeight );
		fWidth = wWorkSpace.DPIUnscale( fWidth ) / 2;
		fHeight = wWorkSpace.DPIUnscale( fHeight ) / 2;

		m_pDragDropFrameSlotUI.SetPosX( wWorkSpace.DPIUnscale( iMouseX ) - fWidth );
		m_pDragDropFrameSlotUI.SetPosY( wWorkSpace.DPIUnscale( iMouseY ) - fHeight );
	}

	//------------------------------------------------------------------------------------------------
	// !
	protected void Action_Move( SCR_InventoryStorageBaseUI pStorageBase )
	{
		IEntity pItem = m_pSelectedSlotUI.GetInventoryItemComponent().GetOwner();
		m_InventoryManager.InsertItem( pItem, pStorageBase.GetCurrentNavigationStorage(), m_pFocusedSlotUI.GetStorageUI().GetCurrentNavigationStorage() );
		ResetHighlightsOnAvailableStorages();
	}

	//------------------------------------------------------------------------------------------------
	// !
	protected void Action_LootMove()
	{
		if ( !m_pFocusedSlotUI )
			return;
		IEntity pItem = m_pFocusedSlotUI.GetInventoryItemComponent().GetOwner();
		if (m_InventoryManager.CanMoveItem(pItem))
			return;

		m_pCallBack.m_pItem = pItem;
		m_pCallBack.m_pMenu = this;

		// Loot ===>>> Character
		if ( m_pFocusedSlotUI.GetStorageUI() == m_pStorageLootUI )
		{
			m_pCallBack.m_pStorageFrom = m_pStorageLootUI;
			m_pCallBack.m_pStorageTo = m_pFocusedSlotUI.GetStorageUI();

			if ( m_pDisplayedStorage.Type() == EquipedWeaponStorageComponent )			//default view opened, put the item into the first suitable storage
			{
				m_InventoryManager.InsertItem( pItem, null, null, m_pCallBack );
			}
			else
			{
				m_InventoryManager.InsertItem( pItem, m_pStorageBaseUI.GetCurrentNavigationStorage(), m_pStorageLootUI.GetCurrentNavigationStorage(), m_pCallBack ); 			//a storage is already opened, try to move it there
			}
		}
		else
		// Character ===>>> Loot
		{
			m_pCallBack.m_pStorageFrom = m_pFocusedSlotUI.GetStorageUI();
			m_pCallBack.m_pStorageTo = m_pStorageLootUI;
			auto storage = m_pStorageLootUI.GetCurrentNavigationStorage();
			if ( storage )
				m_InventoryManager.InsertItem( pItem, m_pStorageLootUI.GetCurrentNavigationStorage(), m_pDisplayedStorage, m_pCallBack );	// moving into the opened storage in the  vicinity
			else
			{
				//droping it on the ground
				auto pSlot = m_pFocusedSlotUI.GetInventoryItemComponent().GetParentSlot();
				if ( pSlot )
					m_InventoryManager.TryRemoveItemFromInventory(pItem, pSlot.GetStorage(), m_pCallBack);
			}	
		}
		ResetHighlightsOnAvailableStorages();
	}

	//------------------------------------------------------------------------------------------------
	// !
	protected bool IsStorageInsideLBS( BaseInventoryStorageComponent pStorage, ClothNodeStorageComponent pLBSStorage )
	{
		if ( !pLBSStorage )
			return false;
		array<BaseInventoryStorageComponent> aOutStorages = new array<BaseInventoryStorageComponent>();
		pLBSStorage.GetOwnedStorages( aOutStorages, 1, false );
		return aOutStorages.Find(pStorage) != -1;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnItemAddedListener( IEntity item, notnull BaseInventoryStorageComponent storage )
	{
		if ( storage == m_StorageManager.GetLootStorage() )
			return;

		if ( storage.Type() == EquipedWeaponStorageComponent )	//we are updating the weapon storage on character
		{
			if ( m_pWeaponStorage )
				m_pWeaponStorage.Refresh();
		}
		else if ( storage.Type() == WeaponAttachmentsStorageComponent )	// we are updating a weapon ( attachement / magazin inserted )
		{
			//if ( m_pWeaponStorage && m_pWeaponStorage.GetCurrentNavigationStorage() == storage )
			if ( m_pWeaponStorage )
				m_pWeaponStorage.Refresh();
		}

		UpdateTotalWeightText();
		RefreshUISlotStorages();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnItemRemovedListener( IEntity item, notnull BaseInventoryStorageComponent storage )
	{
		if ( !storage )
			return;
		if ( storage == m_StorageManager.GetLootStorage() )
			return;

		UpdateTotalWeightText();
		RefreshUISlotStorages();
	}

	//------------------------------------------------------------------------------------------------
	override bool OnClick( Widget w, int x, int y, int button )
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	void OnContainerHovered( SCR_InventoryStorageBaseUI pContainer )
	{
		if ( m_bDraggingEnabled )
			pContainer.ShowContainerBorder( true );
		m_pActiveHoveredStorageUI = pContainer;
	}

	//------------------------------------------------------------------------------------------------
	void HighlightAvailableStorages(SCR_InventorySlotUI itemSlot)
	{
		if (!itemSlot)
			return;
		
		InventoryItemComponent itemComp = itemSlot.GetInventoryItemComponent();
		if (!itemComp)
			return;
		
		IEntity itemEntity = itemComp.GetOwner();
		if (!itemEntity)
			return;
		
		InventoryStorageSlot itemParentSlot = itemComp.GetParentSlot();
		BaseInventoryStorageComponent originStorage;
		if (itemParentSlot)
			originStorage = itemParentSlot.GetStorage();
		
		BaseInventoryStorageComponent contStorage;
		float totalWeightWithInsertedItem;
		float totalOccupiedVolumeWithInsertedItem;			
		float itemWeight = itemComp.GetTotalWeight();
		
		foreach (SCR_InventoryStorageBaseUI storageBaseUI: m_aStorages)
		{	
			if (!storageBaseUI)
				continue;
			if (storageBaseUI.Type() == SCR_InventoryStorageLootUI)
				continue;
			contStorage = storageBaseUI.GetStorage();	
			if (!contStorage)
				continue;
			if (originStorage && contStorage == originStorage)
				continue;		
			if (IsStorageInsideLBS(originStorage, ClothNodeStorageComponent.Cast(contStorage)))	
				continue;
			
			totalWeightWithInsertedItem = storageBaseUI.GetTotalRoundedUpWeight(contStorage);
			totalWeightWithInsertedItem += Math.Round(itemWeight * 100) / 100;
			
			totalOccupiedVolumeWithInsertedItem = storageBaseUI.GetOccupiedVolume(contStorage);
			totalOccupiedVolumeWithInsertedItem += itemComp.GetVolume();
			
			storageBaseUI.UpdateTotalWeight(totalWeightWithInsertedItem);
			storageBaseUI.UpdateVolumePercentage(storageBaseUI.GetOccupiedVolumePercentage(contStorage, totalOccupiedVolumeWithInsertedItem));
			
			if (m_InventoryManager.CanInsertItemInActualStorage(itemEntity, contStorage))
				storageBaseUI.SetStorageAsHighlighted(true);
			else	
				storageBaseUI.SetStorageAsHighlighted(false);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void ResetHighlightsOnAvailableStorages()
	{
		foreach (SCR_InventoryStorageBaseUI storageBaseUI: m_aStorages)
		{
			if (!storageBaseUI)
				continue;
			
			storageBaseUI.UpdateVolumePercentage(storageBaseUI.GetOccupiedVolumePercentage(storageBaseUI.GetStorage()));
			storageBaseUI.UpdateTotalWeight(storageBaseUI.GetTotalRoundedUpWeight(storageBaseUI.GetStorage()));
			storageBaseUI.SetStorageAsHighlighted(true);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void OnContainerLeft( SCR_InventoryStorageBaseUI pContainer )
	{		
		pContainer.ShowContainerBorder( false );
		m_pActiveHoveredStorageUI = null;
	}
	
	//------------------------------------------------------------------------------------------------
	void OnContainerFocused(SCR_InventoryStorageBaseUI pContainer)
	{
		pContainer.ShowContainerBorder(true);
		SetActiveStorage(pContainer);
		m_pActiveStorageUI = pContainer;
		NavigationBarUpdate();
	}

	//------------------------------------------------------------------------------------------------
	void OnContainerFocusLeft(SCR_InventoryStorageBaseUI pContainer)
	{
		pContainer.ShowContainerBorder(false);
		SetActiveStorage(pContainer);
		m_pActiveStorageUI = null;
		NavigationBarUpdate();
	}

	//------------------------------------------------------------------------------------------------
	SCR_CharacterControllerComponent GetCharacterController()
	{
		return m_CharController;
	}

	//------------------------------------------------------------------------------------------------
	//! TODO: remove in future
	protected bool MoveOperation( InventoryItemComponent pItemComponent, BaseInventoryStorageComponent pStorageTo )
	{
		if ( !pItemComponent || !pStorageTo )
		{
			Print( "INV: Cannot perform move operation. Either item or storage doesn't exist", LogLevel.DEBUG );
			return false;
		}

			m_InventoryManager.EquipAny( pStorageTo, pItemComponent.GetOwner() );
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! TODO: remove in future
	void MoveWeapon()
	{
		if (!m_pFocusedSlotUI)
			return;
		
		m_pCallBack.m_pStorageFrom = m_pFocusedSlotUI.GetStorageUI();
		
		m_InventoryManager.EquipWeapon( m_pFocusedSlotUI.GetInventoryItemComponent().GetOwner(), m_pCallBack );
	}
	
	//------------------------------------------------------------------------------------------------
	//! 
	protected void RefreshUISlotStorages()
	{
		foreach ( SCR_InventorySlotUI pStorage : m_aShownStorages )
		{
			if ( pStorage )
				pStorage.Refresh();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateTotalWeightText()
	{
		if (!m_wTotalWeightText || !m_StorageManager)
			return;
		
		m_wTotalWeightText.SetText(string.Format("%1 / %2 kg", GetTotalWeight(), m_StorageManager.GetMaxLoad()));
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ShowQuickSlotStorage()
	{
		if( m_wQuickSlotStorage )
		{
			m_wQuickSlotStorage.RemoveHandler( m_wQuickSlotStorage.FindHandler( SCR_InventoryStorageQuickSlotsUI ) );	//remove the handler from the widget
			m_wQuickSlotStorage.RemoveFromHierarchy();
		}
		
		Widget parent = m_widget.FindAnyWidget( "QuickSlots" );
		m_wQuickSlotStorage =  GetGame().GetWorkspace().CreateWidgets( "{A1E61EF091EAC47C}UI/layouts/Menus/Inventory/InventoryQuickSlotsGrid.layout", parent );
				
		if( !m_wQuickSlotStorage )
			return;
		
		OverlaySlot.SetVerticalAlign( m_wQuickSlotStorage, LayoutVerticalAlign.Bottom );
		OverlaySlot.SetHorizontalAlign(m_wQuickSlotStorage, LayoutHorizontalAlign.Left);

		SCR_UISoundEntity.SoundEvent("SOUND_INV_HOTKEY_CONFIRM");
					
		m_wQuickSlotStorage.AddHandler( new SCR_InventoryStorageQuickSlotsUI( null, ELoadoutArea.ELA_None, this ) );
		m_pQuickSlotStorage = SCR_InventoryStorageQuickSlotsUI.Cast( m_wQuickSlotStorage.FindHandler( SCR_InventoryStorageQuickSlotsUI ) );
	}
	
	//------------------------------------------------------------------------------------------------
	// !
	void Action_SelectQuickSlot()
	{
		if ( !GetGame().GetInputManager() )
			return;
		int iSlot = -1;

		int quickSlotCount = m_StorageManager.GetQuickSlotItems().Count();
		for (int id = 1; id <= quickSlotCount; ++id)
		{
			if (GetGame().GetInputManager().GetActionTriggered("InventoryQuickSlot" + id))
				iSlot = id;
		}

		if ( iSlot != -1 )
			SetItemToQuickSlot( iSlot );
	}

	//------------------------------------------------------------------------------------------------
	// !
	protected void RemoveItemFromQuickSlotDrop()
	{
		if ( !m_pSelectedSlotUI )
			return;
		InventoryItemComponent pInventoryComponent = m_pSelectedSlotUI.GetInventoryItemComponent();
		if ( !pInventoryComponent )
			return;
		m_StorageManager.StoreItemToQuickSlot( pInventoryComponent.GetOwner(), m_pSelectedSlotUI.GetSlotIndex() );
		ShowQuickSlotStorage();
	}

	//------------------------------------------------------------------------------------------------
	// !
	protected void SetItemToQuickSlot( int iSlotIndex, SCR_InventorySlotUI slot = null )
	{
		if (!slot)
			slot = m_pFocusedSlotUI;
		if ( !slot )
			return;
		if ( slot.GetStorageUI() && slot.GetStorageUI().Type() == SCR_InventoryStorageLootUI )
			return;	//we don't want to take anything from vicinity to the quickslot
		if ( slot.Type() == SCR_InventorySlotQuickSlotUI )
			return;
		InventoryItemComponent pInventoryComponent = slot.GetInventoryItemComponent();
		if ( !pInventoryComponent )
			return;
		SCR_ItemAttributeCollection pItemAttributes = SCR_ItemAttributeCollection.Cast( pInventoryComponent.GetAttributes() );
		if ( pItemAttributes && ( pItemAttributes.GetItemSize() != ESlotSize.SLOT_1x1 && pItemAttributes.GetItemSize() != ESlotSize.SLOT_2x1 ) )
			return; //so far only items with one line are supported ( issue on the UI side )

		m_StorageManager.StoreItemToQuickSlot( pInventoryComponent.GetOwner(), --iSlotIndex );
		ShowQuickSlotStorage();
	}

	//------------------------------------------------------------------------------------------------
	// !
	protected void SetItemToQuickSlotDrop()
	{
		if ( !m_pFocusedSlotUI )
			return;
		if ( !m_pSelectedSlotUI )
			return;
		if ( m_pSelectedSlotUI.GetStorageUI() && m_pSelectedSlotUI.GetStorageUI().Type() == SCR_InventoryStorageLootUI )
			return;	//we don't want to take anything from vicinity to the quickslot

		IEntity pOriginalEntity;
		if ( ( SCR_InventorySlotQuickSlotUI.Cast( m_pSelectedSlotUI ) ) && ( SCR_InventorySlotQuickSlotUI.Cast( m_pFocusedSlotUI ) ) ) // swapping
		{
			InventoryItemComponent pComp = m_pFocusedSlotUI.GetInventoryItemComponent();
			if ( pComp )
			{
				pOriginalEntity = pComp.GetOwner();
			}
		}

		InventoryItemComponent pInventoryComponent = m_pSelectedSlotUI.GetInventoryItemComponent();
		if ( !pInventoryComponent )
			return;
		SCR_ItemAttributeCollection pItemAttributes = SCR_ItemAttributeCollection.Cast( pInventoryComponent.GetAttributes() );
		if ( pItemAttributes && ( pItemAttributes.GetItemSize() != ESlotSize.SLOT_1x1 && pItemAttributes.GetItemSize() != ESlotSize.SLOT_2x1 ) )
			return; //so far only items with one line are supported ( issue on the UI side )
		int iSlotIndex = m_pFocusedSlotUI.GetSlotIndex();
		m_StorageManager.StoreItemToQuickSlot( pInventoryComponent.GetOwner(), iSlotIndex );
		if ( pOriginalEntity )
			m_StorageManager.StoreItemToQuickSlot( pOriginalEntity, m_pSelectedSlotUI.GetSlotIndex() );
		ShowQuickSlotStorage();
	}

	//------------------------------------------------------------------------------------------------
	protected void Action_SwapItems(SCR_InventorySlotUI item1, SCR_InventorySlotUI item2)
	{
		if (!item1 || !item2)
			return;

		IEntity from = item1.GetInventoryItemComponent().GetOwner();
		IEntity to = item2.GetInventoryItemComponent().GetOwner();

		if (!(m_InventoryManager.CanMoveItem(from) && m_InventoryManager.CanMoveItem(to)))
			return;
		
		m_pCallBack.m_pStorageFrom = m_pSelectedSlotUI.GetStorageUI();
		m_pCallBack.m_pStorageTo = m_pFocusedSlotUI.GetStorageUI();
		m_pCallBack.m_pItem = from;
		m_pCallBack.m_pMenu = this;
		m_pCallBack.m_pStorageToFocus = m_pCallBack.m_pStorageFrom;
		
		//if (m_InventoryManager.CanSwapItemStorages(from, to))
		m_InventoryManager.TrySwapItemStorages(from, to, m_pCallBack); 
	}

	//------------------------------------------------------------------------------------------------
	protected void Action_MoveItemToStorage(SCR_InventoryStorageBaseUI toStorage = null)
	{
		if (!toStorage)
			toStorage = m_pFocusedSlotUI.GetStorageUI();

		if (!toStorage || !m_pSelectedSlotUI)
			return;

		MoveItem(toStorage);
		ResetHighlightsOnAvailableStorages();
	}

	//------------------------------------------------------------------------------------------------
	bool IsUsingGamepad()
	{
		return !GetGame().GetInputManager().IsUsingMouseAndKeyboard();
	}

	//------------------------------------------------------------------------------------------------
	void OnInputDeviceChanged()
	{
		SetStorageSwitchMode(IsUsingGamepad());
		NavigationBarUpdate();
	}

	//------------------------------------------------------------------------------------------------
	void OnItemRemoved( IEntity pItem, BaseInventoryStorageComponent pStorageOwner  )
	{
		//m_StorageManager.RemoveItemFromQuickSlot( pItem );
		ShowQuickSlotStorage();
	}
	
	//------------------------------------------------------------------------------------------------
	void OnItemAdded( IEntity pItem, BaseInventoryStorageComponent pStorageOwner  )
	{
		//m_StorageManager.StoreItemToQuickSlot( pItem );
		ShowQuickSlotStorage();
	}
	
	
	//------------------------------------------------------------------------------------------------
	void SCR_InventoryMenuUI()
	{
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_InventoryMenuUI()
	{
	}
	
	#else
	protected float GetTotalLoadWeight();
	SCR_InventoryStorageManagerComponent GetInventoryStorageManager();
	bool GetCanInteract();
	override void OnMenuOpen();
	protected void Init();
	void ShowStorage();
	void ShowStorage( BaseInventoryStorageComponent storage );
	protected void ShowStoragesList();
	//protected void ShowGadgetStorage();
	protected void ShowEquipedWeaponStorage();
	void ShowItemInfo( string sName = "", string sDescr = "", float sWeight = 0.0 );
	void HideItemInfo();
	void SetSlotFocused( SCR_InventorySlotUI pFocusedSlot );
	SCR_InventoryInspectionUI GetInspectionScreen();
	void InspectItem(SCR_InventorySlotUI itemSlot);
	void FilterOutStorages();
	protected void Action_CloseInventory();
	protected void Action_SelectItem();
	protected void Action_EquipItem();
	protected void Action_Drop();
	protected void Action_Inspect();
	protected void OnItemAddedListener( IEntity item, BaseInventoryStorageComponent storage );
	protected void OnItemRemovedListener( IEntity item, BaseInventoryStorageComponent storage );
	override bool OnClick( Widget w, int x, int y, int button );
	void FilterOutStorages( false ););
	void MoveItem();
	void MoveWeapon();
	void SCR_InventoryMenuUI();
	#endif
};



