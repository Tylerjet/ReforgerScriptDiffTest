//------------------------------------------------------------------------------------------------
//! UI Script
//! Inventory Slot UI Layout
class SCR_InventorySlotQuickSlotUI : SCR_InventorySlotUI
{
	ResourceName m_sPrimarySecondaryWeapon = "{F565C4A388964DBE}UI/Textures/InventoryIcons/InventorySlot_gadget.edds";	//TODO: icon for weapon
	
	protected ImageWidget m_wGamepadHintSmall;
	protected ImageWidget m_wGamepadHintLarge;
	protected RichTextWidget m_wKeybindText;

	//ResourceName m_sPrimarySecondaryWeapon = "{E6BDE9DF9368C48C}UI/Textures/WeaponIcons/weapon_AK74.edds";
	//------------------------------------------------------------------------ USER METHODS ----------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	override void Init()
	{
		super.Init();
		GetGame().GetGame().OnInputDeviceIsGamepadInvoker().Insert(UpdateHints);
		// preparation for the icon
		//if ( m_iQuickSlotIndex < 2 )				//Primary or secondary weapon
		//	SetIcon( "{E6BDE9DF9368C48C}UI/Textures/WeaponIcons/weapon_AK74.edds" );
	}
	
	//------------------------------------------------------------------------------------------------
	//! should be the slot visible?
	override void SetSlotVisible( bool bVisible )
	{
		super.SetSlotVisible( bVisible );

		m_wKeybindText = RichTextWidget.Cast(m_widget.FindAnyWidget("KeybindRichText"));
		if (m_wKeybindText && !GetStorageUI().GetInventoryMenuHandler())
		{
			EInputDeviceType currentDevice = GetGame().GetInputManager().GetLastUsedInputDevice();
			InputBinding binding = GetGame().GetInputManager().CreateUserBinding();

			string deviceString = "keyboard";
			if (currentDevice == EInputDeviceType.GAMEPAD)
				deviceString = "gamepad";

			string name = string.Format("%1%2", "SwitchWeaponCategory", (m_iQuickSlotIndex + 1));

			array<string> bindings = {};
			if (deviceString == "gamepad" &&
				!binding.GetBindings(name, bindings, currentDevice))
				return;

			if (name == "SwitchWeaponCategory10")
				name = "SwitchWeaponCategory0";

			m_wKeybindText.SetText(string.Format("<action name='%1' preset='%2' device='" + deviceString + "' scale='1.25'/>", name, ""));
		}

		if( bVisible )
		{
			if ( !m_pItem )	//if item is not available, it's the empty slot, show the large number
			{
				m_wTextQuickSlotLarge = TextWidget.Cast( m_widget.FindAnyWidget( "TextQuickSlotLarge" ) );
				SetQuickSlotIndexVisible( m_wTextQuickSlotLarge, true );
				SetQuickSlotHintVisible(m_wKeybindText);
			}
			else
			{
				SetQuickSlotIndexVisible( m_wTextQuickSlot, true );
				SetQuickSlotHintVisible(m_wKeybindText);
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

	void UpdateHints()
	{
		// SetQuickSlotHintVisible(m_wKeybindText);
		m_wKeybindText.SetVisible(!GetGame().GetInputManager().IsUsingMouseAndKeyboard());
	}

	//------------------------------------------------------------------------------------------------
	void SetQuickSlotHintVisible(RichTextWidget hintWidget)
	{
		if (!GetGame().GetInputManager().IsUsingMouseAndKeyboard() && hintWidget.IsEnabled()) 
			hintWidget.SetVisible(true);		
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