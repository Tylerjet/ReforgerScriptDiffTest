//------------------------------------------------------------------------------------------------
//! UI Script
//! Inventory Storage UI Layout
class SCR_InventoryStorageLootUI : SCR_InventoryStorageBaseUI
{
	private GenericEntity m_Character;
	private CharacterVicinityComponent m_Vicinity;
//	protected string											sGridPath 					= "centerFrame.gridFrame.size.grid";	
	
	//------------------------------------------------------------------------------------------------
	// ! creates the slot
	protected override SCR_InventorySlotUI CreateSlotUI( InventoryItemComponent pComponent, SCR_ItemAttributeCollection pAttributes = null )
	{
		if ( WeaponAttachmentsStorageComponent.Cast( pComponent ) )
		{
			return SCR_InventorySlotWeaponUI( pComponent, this, true );						//creates the slot 
		}
		else if ( BaseInventoryStorageComponent.Cast( pComponent ) )
		{
			return SCR_InventorySlotStorageEmbeddedUI( pComponent, this, true );			//creates the slot 
		}
		else
		{
			return new SCR_InventorySlotUI( pComponent, this, true );			//creates the slot 
		}
	}

	//------------------------------------------------------------------------ USER METHODS ------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	override void Init()
	{
		if (!m_Character)
			return;
		//get the workspace
		if ( m_workspaceWidget == null ) 
			m_workspaceWidget = GetGame().GetWorkspace();
		
		//Get inventory manager and the actual character's topmost storage
		m_InventoryManager = SCR_InventoryStorageManagerComponent.Cast( m_Character.FindComponent( SCR_InventoryStorageManagerComponent ) );
		m_InventoryStorage = SCR_CharacterInventoryStorageComponent.Cast( m_Character.FindComponent( SCR_CharacterInventoryStorageComponent ) );
		m_Vicinity = CharacterVicinityComponent.Cast( m_Character.FindComponent(CharacterVicinityComponent) );
		
		if( m_InventoryStorage )
		{
			m_wGrid = GridLayoutWidget.Cast( m_widget.FindAnyWidget( m_sGridPath ) );	
			m_wStorageName = TextWidget.Cast( m_widget.FindAnyWidget( "TextC" ) );
			//Set the proper ratio according to the number of columns and rows
			SizeLayoutWidget wSizeContainer = SizeLayoutWidget.Cast( m_widget.FindAnyWidget( "SizeLayout0" ) );
			if ( wSizeContainer )
				wSizeContainer.SetAspectRatio( m_iMaxColumns / m_iMaxRows );
		}
		if( !m_wStorageName )
			return;
		m_wStorageName.SetText( "#AR-Inventory_Vicinity" );
	}
	
	//------------------------------------------------------------------------ COMMON METHODS ----------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached( Widget w )
	{
		super.HandlerAttached( w );
		InitPaging();
		SetRowsAndColumns();
		CreateEmptyLayout();
		CreateSlots();
		SortSlots();
		ShowPage(0);			
	}		
	
	//------------------------------------------------------------------------------------------------
	protected override void GetAllItems( out notnull array<IEntity> pItemsInStorage, BaseInventoryStorageComponent pStorage = null )
	{
		if( !m_Vicinity )
			return;
		
		if ( !pStorage )
			m_Vicinity.GetAvailableItems(pItemsInStorage);
		else
		{
			array<BaseInventoryStorageComponent> pStorages = new array<BaseInventoryStorageComponent>();
			array<IEntity> pItems = new array<IEntity>();
			pStorage.GetOwnedStorages( pStorages, 1, false );
			foreach ( BaseInventoryStorageComponent pStor : pStorages )
			{
				if (! pStor )
					continue;
				pStor.GetAll( pItems );
				pItemsInStorage.Copy( pItems );
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_InventoryStorageLootUI( BaseInventoryStorageComponent storage, ELoadoutArea slotID = ELoadoutArea.ELA_None, SCR_InventoryMenuUI menuManager = null, int iPage = 0, array<BaseInventoryStorageComponent> aTraverseStorage = null, GenericEntity character = null )
	{
		m_iMaxRows = 6;
		m_iMaxColumns = 6;
		m_iMatrix = new SCR_Matrix( m_iMaxColumns, m_iMaxRows );
		SetSlotAreaID( slotID );
		m_Character = character;
		m_bEnabled = true;
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_InventoryStorageLootUI()
	{
	}
};