[EntityEditorProps(category: "GameScripted/UI/HUD/WeaponSwitchingBar", description: "Concept of quick selection bar")]


class SCR_WeaponSwitchingBaseUI: SCR_InfoDisplay
{
	static private ResourceName							m_ItemPreviewManagerPrefab = "{9F18C476AB860F3B}Prefabs/World/Game/ItemPreviewManager.et";
	static private Widget								s_wQuickSlotStorage;
	static private ref SCR_InventoryStorageQuickSlotsUI	s_pQuickSlotStorage;
	protected InventoryStorageManagerComponent			m_pInventoryManager;
	protected IEntity									m_Owner;

	bool 												m_bOpened;

	[Attribute( "{A1E61EF091EAC47C}UI/layouts/Menus/Inventory/InventoryQuickSlotsGrid.layout" )]
	string m_sQuickSlotGridLayout;
	
	//------------------------------------------------------------------------ USER METHODS ----------------------------------------------------------------------
	
	
	//------------------------------------------------------------------------------------------------
	static void RefreshQuickSlots()
	{
		if (s_pQuickSlotStorage)
		{
			s_pQuickSlotStorage.RefreshQuickSlots();
			s_pQuickSlotStorage.HighlightLastSelectedSlot();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Init( IEntity owner )
	{
		m_Owner = owner;

		if ( !m_Owner || !m_wRoot )
			return;
		
		m_pInventoryManager = InventoryStorageManagerComponent.Cast( owner.FindComponent( InventoryStorageManagerComponent ) );

		//instantiate the preview manager
		if (!GetGame().GetItemPreviewManager())
		{
			Resource rsc = Resource.Load(m_ItemPreviewManagerPrefab);
			if (rsc.IsValid())
				GetGame().SpawnEntityPrefabLocal(rsc);
		}

		InputManager inputManager = GetGame().GetInputManager();;
        inputManager.AddActionListener("Inventory_WeaponSwitching", EActionTrigger.UP, QuickBarClosedOnly );
        inputManager.AddActionListener("Inventory_WeaponSwitching", EActionTrigger.DOWN, Action_OpenQuickSelectionBar );
        inputManager.AddActionListener("Inventory_WeaponSwitching", EActionTrigger.UP, Action_CloseQuickSelectionBar );

        for (int i = 0; i < 10; ++i)
        {
			inputManager.AddActionListener("SwitchWeaponCategory" + i.ToString(), EActionTrigger.DOWN, Action_OpenQuickSelectionBar);
			inputManager.AddActionListener("SwitchWeaponCategory" + i.ToString(), EActionTrigger.UP, Action_CloseQuickSelectionBar);
        }
	}

	//------------------------------------------------------------------------------------------------
	protected void RemoveActionListeners()
	{
		InputManager inputManager = GetGame().GetInputManager();
        inputManager.RemoveActionListener("Inventory_WeaponSwitching", EActionTrigger.UP, QuickBarClosedOnly );
        inputManager.RemoveActionListener("Inventory_WeaponSwitching", EActionTrigger.DOWN, Action_OpenQuickSelectionBar );
        inputManager.RemoveActionListener("Inventory_WeaponSwitching", EActionTrigger.UP, Action_CloseQuickSelectionBar );

		for (int i = 0; i < 10; ++i)
		{
			inputManager.RemoveActionListener("SwitchWeaponCategory" + i.ToString(), EActionTrigger.DOWN, Action_OpenQuickSelectionBar);
			inputManager.RemoveActionListener("SwitchWeaponCategory" + i.ToString(), EActionTrigger.UP, Action_CloseQuickSelectionBar);
		}
	}

	//------------------------------------------------------------------------------------------------
    void Action_OpenQuickSelectionBar()
    {
		if (m_bOpened)
			return;

    	GetGame().GetInputManager().AddActionListener("CharacterSwitchWeapon", EActionTrigger.VALUE, Action_SelectSlot );
    	GetGame().GetInputManager().AddActionListener("CharacterDropItem", EActionTrigger.DOWN, Action_DropItem );
    	m_bOpened = true;
    	GetGame().GetCallqueue().Remove( ShowQuickSlots );		// if there's a delayed Show method from the previous quick bar usage, purge it
		if ( !m_wRoot )
			return;
		if( s_wQuickSlotStorage )
		{
			s_wQuickSlotStorage.RemoveHandler( s_wQuickSlotStorage.FindHandler( SCR_InventoryStorageQuickSlotsUI ) );
			s_wQuickSlotStorage.RemoveFromHierarchy();
		}
		Widget parent = m_wRoot.FindAnyWidget( "QuickSlots" );
		s_wQuickSlotStorage =  GetGame().GetWorkspace().CreateWidgets( m_sQuickSlotGridLayout, parent );
		if( !s_wQuickSlotStorage )
			return;
		s_pQuickSlotStorage = new SCR_InventoryStorageQuickSlotsUI( null, -1, null );
		s_wQuickSlotStorage.AddHandler( s_pQuickSlotStorage );
		s_pQuickSlotStorage.HighlightLastSelectedSlot();
		s_pQuickSlotStorage.SetInitialQuickSlot();
		
		Show( true, UIConstants.FADE_RATE_DEFAULT, true );
		BlurWidget wBlur = BlurWidget.Cast( m_wRoot.FindAnyWidget( "wBlur" ) );
		if ( wBlur ) 
			wBlur.SetVisible( true );
		
		SCR_InventoryStorageManagerComponent invMan = SCR_InventoryStorageManagerComponent.Cast(m_pInventoryManager);
		if (invMan)
			invMan.m_OnQuickBarOpenInvoker.Invoke(true);
		
		SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_INV_HOTKEY_OPEN);
    }
	
    //------------------------------------------------------------------------------------------------
    void Action_CloseQuickSelectionBar()
    {
    	GetGame().GetInputManager().RemoveActionListener("CharacterSwitchWeapon", EActionTrigger.VALUE, Action_SelectSlot );
    	GetGame().GetInputManager().RemoveActionListener("CharacterDropItem", EActionTrigger.DOWN, Action_DropItem);
    	m_bOpened = false;
		if ( !m_wRoot )
			return;	
		BlurWidget wBlur = BlurWidget.Cast( m_wRoot.FindAnyWidget( "wBlur" ) );
		if ( wBlur ) 
			wBlur.SetVisible( false );
		
		if ( s_pQuickSlotStorage && s_pQuickSlotStorage.UseItemInSlot() )
			GetGame().GetCallqueue().CallLater( ShowQuickSlots, 2000, false, false, UIConstants.FADE_RATE_DEFAULT, true );
		else
			Show( false, UIConstants.FADE_RATE_DEFAULT, true );
		
		SCR_InventoryStorageManagerComponent invMan = SCR_InventoryStorageManagerComponent.Cast(m_pInventoryManager);
		if (invMan)
			invMan.m_OnQuickBarOpenInvoker.Invoke(false);
		
		SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_INV_HOTKEY_CLOSE);
    }

	//------------------------------------------------------------------------------------------------
	// called when the quickbar is closed by letting go the tab key
	protected void QuickBarClosedOnly()
	{
		s_pQuickSlotStorage.SetQuickBarClosed();
	}

	//------------------------------------------------------------------------------------------------
    void Action_SelectSlot()
	{
		if (!m_bOpened || !s_pQuickSlotStorage)
			return;
		
		int targetSlot = -1;

		InputManager inputManager = GetGame().GetInputManager();
		if (inputManager.GetActionTriggered("SwitchWeaponCategory1"))
			targetSlot = 0;
		else if (inputManager.GetActionTriggered("SwitchWeaponCategory2"))
			targetSlot = 1;
		else if (inputManager.GetActionTriggered("SwitchWeaponCategory3"))
			targetSlot = 2;
		else if (inputManager.GetActionTriggered("SwitchWeaponCategory4"))
			targetSlot = 3;
		else if (inputManager.GetActionTriggered("SwitchWeaponCategory5"))
			targetSlot = 4;
		else if (inputManager.GetActionTriggered("SwitchWeaponCategory6"))
			targetSlot = 5;
		else if (inputManager.GetActionTriggered("SwitchWeaponCategory7"))
			targetSlot = 6;
		else if (inputManager.GetActionTriggered("SwitchWeaponCategory8"))
			targetSlot = 7;
		else if (inputManager.GetActionTriggered("SwitchWeaponCategory9"))
			targetSlot = 8;
		else if (inputManager.GetActionTriggered("SwitchWeaponCategory0"))
			targetSlot = 9;

		if ( targetSlot == - 1 )
			s_pQuickSlotStorage.SelectSlot( inputManager.GetActionValue("CharacterSwitchWeapon") );
		else
			s_pQuickSlotStorage.SelectSlot( targetSlot );
	}
	
	//------------------------------------------------------------------------------------------------
	void Action_DropItem()
	{
		InventoryItemComponent selectedItem = s_pQuickSlotStorage.GetCurrentSlotItem();
		if (selectedItem)
		{
			SCR_QuickSlotRefreshCB cb = new SCR_QuickSlotRefreshCB();
			m_pInventoryManager.TryRemoveItemFromStorage(selectedItem.GetOwner(), selectedItem.GetParentSlot().GetStorage(), cb);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void UpdateValues(IEntity owner, float timeSlice)
	{
		if (!m_bOpened)
			return;

	    GetGame().GetInputManager().ActivateContext("WeaponSelectionContext");
	}

	protected void ShowQuickSlots(bool show, float speed = UIConstants.FADE_RATE_DEFAULT, bool force = false)
	{
		Show(show, speed, force);
	}
		
	//------------------------------------------------------------------------ COMMON METHODS ----------------------------------------------------------------------
   	
	//------------------------------------------------------------------------------------------------
	override event void OnStartDraw(IEntity owner)
	{
		super.OnStartDraw( owner );
		Init( owner );
		Show( false, 0, true );
	}	
	
	//------------------------------------------------------------------------------------------------
	override event void OnStopDraw(IEntity owner)
	{
		super.OnStopDraw(owner);
		RemoveActionListeners();
		s_pQuickSlotStorage = null;
	}
};

class SCR_QuickSlotRefreshCB : ScriptedInventoryOperationCallback
{
	override void OnComplete()
	{
		SCR_WeaponSwitchingBaseUI.RefreshQuickSlots();
	}
}