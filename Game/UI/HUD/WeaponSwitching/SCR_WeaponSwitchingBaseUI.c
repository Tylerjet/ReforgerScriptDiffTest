[EntityEditorProps(category: "GameScripted/UI/HUD/WeaponSwitchingBar", description: "Concept of quick selection bar")]


class SCR_WeaponSwitchingBaseUI: SCR_InfoDisplay
{
	static private ResourceName							m_ItemPreviewManagerPrefab = "{9F18C476AB860F3B}Prefabs/World/Game/ItemPreviewManager.et";
	static private Widget								s_wQuickSlotStorage;
	static private ref SCR_InventoryStorageQuickSlotsUI	s_pQuickSlotStorage;
	protected InventoryStorageManagerComponent			m_pInventoryManager;
	private InputManager								m_pInputmanager;
	protected IEntity									m_Owner;

	bool 												m_bOpened;

	[Attribute( "{A1E61EF091EAC47C}UI/layouts/Menus/Inventory/InventoryQuickSlotsGrid.layout" )]
	string m_sQuickSlotGridLayout;
	
	//------------------------------------------------------------------------ USER METHODS ----------------------------------------------------------------------
	
	
	//------------------------------------------------------------------------------------------------
	static void RefreshQuickSlots()
	{
		if (s_pQuickSlotStorage)
			s_pQuickSlotStorage.RefreshQuickSlots();
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
		
		//Get Input manager
        m_pInputmanager = GetGame().GetInputManager();
        if( !m_pInputmanager)
            return;

        m_pInputmanager.AddActionListener("Inventory_WeaponSwitching", EActionTrigger.DOWN, Action_OpenQuickSelectionBar );
        m_pInputmanager.AddActionListener("Inventory_WeaponSwitching", EActionTrigger.UP, Action_CloseQuickSelectionBar );
        for (int i = 0; i < 10; ++i)
        {
			m_pInputmanager.AddActionListener("SwitchWeaponCategory" + i.ToString(), EActionTrigger.DOWN, Action_OpenQuickSelectionBar);
			m_pInputmanager.AddActionListener("SwitchWeaponCategory" + i.ToString(), EActionTrigger.UP, Action_CloseQuickSelectionBar);
        }

		// Controller Dpad actions
		m_pInputmanager.AddActionListener("QuickslotDpad", EActionTrigger.DOWN, OnQuickSelectOpen);
		m_pInputmanager.AddActionListener("QuickslotDpad", EActionTrigger.UP, Action_CloseQuickSelectionBar);
		//m_pInputmanager.AddActionListener("QuickslotDpadHold", EActionTrigger.DOWN, OnQuickSelectOpenAlter);		// action ready, requires alternative quickslot activation 
		//m_pInputmanager.AddActionListener("QuickslotDpadHold", EActionTrigger.UP, Action_CloseQuickSelectionBar);
	}

	//------------------------------------------------------------------------------------------------
	protected void RemoveActionListeners()
	{
		if (!m_pInputmanager)
			return;
        m_pInputmanager.RemoveActionListener("Inventory_WeaponSwitching", EActionTrigger.DOWN, Action_OpenQuickSelectionBar );
        m_pInputmanager.RemoveActionListener("Inventory_WeaponSwitching", EActionTrigger.UP, Action_CloseQuickSelectionBar );
		for (int i = 0; i < 10; ++i)
		{
			m_pInputmanager.RemoveActionListener("SwitchWeaponCategory" + i.ToString(), EActionTrigger.DOWN, Action_OpenQuickSelectionBar);
			m_pInputmanager.RemoveActionListener("SwitchWeaponCategory" + i.ToString(), EActionTrigger.UP, Action_CloseQuickSelectionBar);
		}

		m_pInputmanager.RemoveActionListener("QuickslotDpad", EActionTrigger.DOWN, OnQuickSelectOpen);
		m_pInputmanager.RemoveActionListener("QuickslotDpad", EActionTrigger.UP, Action_CloseQuickSelectionBar);
	}

	//------------------------------------------------------------------------------------------------
    void OnQuickSelectOpen(float value, EActionTrigger reason)
	{
		Action_OpenQuickSelectionBar();
		
		if (s_pQuickSlotStorage)
			s_pQuickSlotStorage.SelectSlot((int)value); // value is multiplied directly from input action depending on the key used to call it: UP -> 2 | DOWN -> 3 | LEFT ->4 | RIGHT -> 5 
	}
		
	//------------------------------------------------------------------------------------------------
    void Action_OpenQuickSelectionBar()
    {
		if (m_bOpened)
			return;

    	m_pInputmanager.AddActionListener("CharacterSwitchWeapon", EActionTrigger.VALUE, Action_SelectSlot );
    	m_bOpened = true;
    	GetGame().GetCallqueue().Remove( Show );		// if there's a delayed Show method from the previous quick bar usage, purge it
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
		
		Show( true, WidgetAnimator.FADE_RATE_DEFAULT, true );
		BlurWidget wBlur = BlurWidget.Cast( m_wRoot.FindAnyWidget( "wBlur" ) );
		if ( wBlur ) 
			wBlur.SetVisible( true );
		
		SCR_InventoryStorageManagerComponent invMan = SCR_InventoryStorageManagerComponent.Cast(m_pInventoryManager);
		if (invMan)
			invMan.m_OnQuickBarOpenInvoker.Invoke(true);
    }
	
    //------------------------------------------------------------------------------------------------
    void Action_CloseQuickSelectionBar()
    {
    	m_pInputmanager.RemoveActionListener("CharacterSwitchWeapon", EActionTrigger.VALUE, Action_SelectSlot );
    	m_bOpened = false;
		if ( !m_wRoot )
			return;	
		BlurWidget wBlur = BlurWidget.Cast( m_wRoot.FindAnyWidget( "wBlur" ) );
		if ( wBlur ) 
			wBlur.SetVisible( false );
		
		if ( s_pQuickSlotStorage && s_pQuickSlotStorage.UseItemInSlot() )
			GetGame().GetCallqueue().CallLater( Show, 2000, false, false, WidgetAnimator.FADE_RATE_DEFAULT, true );
		else
			Show( false, WidgetAnimator.FADE_RATE_DEFAULT, true );
		
		SCR_InventoryStorageManagerComponent invMan = SCR_InventoryStorageManagerComponent.Cast(m_pInventoryManager);
		if (invMan)
			invMan.m_OnQuickBarOpenInvoker.Invoke(false);
    }
	
	//------------------------------------------------------------------------------------------------
    void Action_SelectSlot()
	{
		if (!m_bOpened || !s_pQuickSlotStorage || !m_pInputmanager)
			return;
		
		int targetSlot = -1;

		if (m_pInputmanager.GetActionTriggered("SwitchWeaponCategory1"))
			targetSlot = 0;
		else if (m_pInputmanager.GetActionTriggered("SwitchWeaponCategory2"))
			targetSlot = 1;
		else if (m_pInputmanager.GetActionTriggered("SwitchWeaponCategory3"))
			targetSlot = 2;
		else if (m_pInputmanager.GetActionTriggered("SwitchWeaponCategory4"))
			targetSlot = 3;
		else if (m_pInputmanager.GetActionTriggered("SwitchWeaponCategory5"))
			targetSlot = 4;
		else if (m_pInputmanager.GetActionTriggered("SwitchWeaponCategory6"))
			targetSlot = 5;
		else if (m_pInputmanager.GetActionTriggered("SwitchWeaponCategory7"))
			targetSlot = 6;
		else if (m_pInputmanager.GetActionTriggered("SwitchWeaponCategory8"))
			targetSlot = 7;
		else if (m_pInputmanager.GetActionTriggered("SwitchWeaponCategory9"))
			targetSlot = 8;
		else if (m_pInputmanager.GetActionTriggered("SwitchWeaponCategory0"))
			targetSlot = 9;

		if ( targetSlot == - 1 )
			s_pQuickSlotStorage.SelectSlot( m_pInputmanager.GetActionValue("CharacterSwitchWeapon") );
		else
			s_pQuickSlotStorage.SelectSlot( targetSlot );
	}
	
	//------------------------------------------------------------------------------------------------
	override void UpdateValues(IEntity owner, float timeSlice)
	{
		if (!m_bOpened)
			return;

	    GetGame().GetInputManager().ActivateContext("WeaponSelectionContext");
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