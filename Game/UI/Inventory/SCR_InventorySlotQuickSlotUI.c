//------------------------------------------------------------------------------------------------
//! UI Script
//! Inventory Slot UI Layout
class SCR_InventorySlotQuickSlotUI : SCR_InventorySlotUI
{
	ResourceName m_sPrimarySecondaryWeapon = "{F565C4A388964DBE}UI/Textures/InventoryIcons/InventorySlot_gadget.edds";	//TODO: icon for weapon
	
	protected ImageWidget m_wGamepadHintSmall;
	protected ImageWidget m_wGamepadHintLarge;
	protected const string s_aIconNames[10] = {
		"X", "Y", "A", "B", "", "DPAD_up", "DPAD_down", "DPAD_left", "DPAD_right", ""
	};
	//ResourceName m_sPrimarySecondaryWeapon = "{E6BDE9DF9368C48C}UI/Textures/WeaponIcons/weapon_AK74.edds";
	//------------------------------------------------------------------------ USER METHODS ----------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	override void Init()
	{
		super.Init();
		// preparation for the icon
		//if ( m_iQuickSlotIndex < 2 )				//Primary or secondary weapon
		//	SetIcon( "{E6BDE9DF9368C48C}UI/Textures/WeaponIcons/weapon_AK74.edds" );
	}
	
	//------------------------------------------------------------------------------------------------
	//! should be the slot visible?
	override void SetSlotVisible( bool bVisible )
	{
		super.SetSlotVisible( bVisible );
		
		if (!GetGame().GetInputManager().IsUsingMouseAndKeyboard())
		{
			m_wGamepadHintSmall = ImageWidget.Cast(m_widget.FindAnyWidget("GamepadHintSmall"));
			m_wGamepadHintLarge = ImageWidget.Cast(m_widget.FindAnyWidget("GamepadHintLarge"));
			SCR_InventoryStorageQuickSlotsUI storage = SCR_InventoryStorageQuickSlotsUI.Cast(GetStorageUI());
			if (storage)
				SetQuickSlotHint(storage.GetGamepadIcons(), m_iQuickSlotIndex);
		}

		if( bVisible )
		{
			if ( !m_pItem )	//if item is not available, it's the empty slot, show the large number
			{
				m_wTextQuickSlotLarge = TextWidget.Cast( m_widget.FindAnyWidget( "TextQuickSlotLarge" ) );			
				SetQuickSlotIndexVisible( m_wTextQuickSlotLarge, true );
				SetQuickSlotHintVisible(m_wGamepadHintLarge);
			}
			else
			{
				SetQuickSlotIndexVisible( m_wTextQuickSlot, true );
				SetQuickSlotHintVisible(m_wGamepadHintSmall);
			}
		}
		else
		{
			m_wTextQuickSlotLarge = null;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override string SetSlotSize()
	{
		ESlotSize slotSize;
		string slotLayout = SLOT_LAYOUT_1x1;
				
		if ( m_iQuickSlotIndex < 2 )
			slotSize = ESlotSize.SLOT_2x1;
		else
			slotSize = m_Attributes.GetItemSize();
		switch ( slotSize ) 
		{
			case ESlotSize.SLOT_1x1:	{ slotLayout = SLOT_LAYOUT_1x1; m_iSizeX = 1; m_iSizeY = 1; } break;
			case ESlotSize.SLOT_2x1:	{ slotLayout = SLOT_LAYOUT_2x1; m_iSizeX = 2; m_iSizeY = 1; } break;
			case ESlotSize.SLOT_2x2:	{ slotLayout = SLOT_LAYOUT_2x2; m_iSizeX = 2; m_iSizeY = 2; } break;
			case ESlotSize.SLOT_3x3:	{ slotLayout = SLOT_LAYOUT_3x3; m_iSizeX = 3; m_iSizeY = 3; } break;
		}
		return slotLayout;
	}

	//------------------------------------------------------------------------------------------------
	void SetQuickSlotHintVisible(ImageWidget hintWidget)
	{
		if (!GetGame().GetInputManager().IsUsingMouseAndKeyboard() && hintWidget.IsEnabled()) 
			hintWidget.SetVisible(true);		
	}

	//------------------------------------------------------------------------------------------------
	void SetQuickSlotHint(ResourceName texture, int id)
	{
		if (!m_wGamepadHintSmall || !m_wGamepadHintLarge || GetStorageUI().GetInventoryMenuHandler()) // show hints only in quickslot bar
			return;

		string quad = s_aIconNames[id];

		if (!quad.IsEmpty())
		{
			m_wGamepadHintSmall.LoadImageFromSet(0, texture, quad);
			m_wGamepadHintSmall.SetEnabled(true);
			m_wGamepadHintLarge.LoadImageFromSet(0, texture, quad);		
			m_wGamepadHintLarge.SetEnabled(true);
		}
	}
	
	//------------------------------------------------------------------------ COMMON METHODS ----------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	void SCR_InventorySlotQuickSlotUI( InventoryItemComponent pComponent = null, SCR_InventoryStorageBaseUI pStorageUI = null, bool bVisible = true, int iSlotIndex = -1, SCR_ItemAttributeCollection pAttributes = null )
	{
		m_iQuickSlotIndex = iSlotIndex;
		m_Attributes = pAttributes;
		
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_InventorySlotQuickSlotUI()
	{
	}
};