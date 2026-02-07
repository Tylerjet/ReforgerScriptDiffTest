//------------------------------------------------------------------------------------------------
//! UI Script
//! Inventory Slot UI Layout
class SCR_InventorySlotWeaponSlotsUI : SCR_InventorySlotUI
{
	private static ResourceName m_sPrimaryWeaponSlot = "{71DBF62B460D7EBC}UI/Textures/InventoryIcons/InvenorySlot-Primary-Outline_UI.edds";
	private static ResourceName m_sSecondaryWeaponSlot = "{694BB85E95CC205A}UI/Textures/InventoryIcons/InvenorySlot-Secondary_Outline_UI.edds";
	private static ResourceName m_sGrenadeWeaponSlot = "{8B507AB33B69C40E}UI/Textures/InventoryIcons/InvenorySlot-Grenade-Outline_UI.edds";
	
	protected string m_sWeaponSlotType;
	protected ImageWidget m_wGamepadHintSmall;
	protected ImageWidget m_wGamepadHintLarge;
	protected int m_iWeaponSlotIndex;
	
	//------------------------------------------------------------------------ USER METHODS ----------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	override void Init()
	{
		super.Init();
		
		if (!m_pItem)
		{
			if (m_sWeaponSlotType == "primary")
				SetIcon(m_sPrimaryWeaponSlot);
			else if (m_sWeaponSlotType == "secondary")
				SetIcon(m_sSecondaryWeaponSlot);
			else if (m_sWeaponSlotType == "grenade")
				SetIcon(m_sGrenadeWeaponSlot);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override string SetSlotSize()
	{
		ESlotSize slotSize;
		string slotLayout = SLOT_LAYOUT_1x1;
		
		if (m_sWeaponSlotType == "primary")
			slotSize = ESlotSize.SLOT_2x1;
		else
			slotSize = ESlotSize.SLOT_1x1;
		
		switch (slotSize) 
		{
			case ESlotSize.SLOT_1x1:	{ m_iSizeX = 1; m_iSizeY = 1; } break;
			case ESlotSize.SLOT_2x1:	{ m_iSizeX = 2; m_iSizeY = 1; } break;
		}
		
		return slotLayout;		
	}
	
	//------------------------------------------------------------------------------------------------
	int GetWeaponSlotIndex()
	{
		return m_iWeaponSlotIndex;
	}

	//------------------------------------------------------------------------------------------------		
	override void CheckCompatibility(SCR_InventorySlotUI slot)
	{
		SCR_InventoryMenuUI menu = SCR_InventoryMenuUI.Cast(GetGame().GetMenuManager().FindMenuByPreset(ChimeraMenuPreset.Inventory20Menu));
		if (!menu)
			return;

		Widget incompatible = m_widget.FindAnyWidget("Incompatible");
		if (!incompatible)
			return;

		if (!slot)
		{
			incompatible.SetVisible(false);
			return;
		}

		if (slot.GetSlotedItemFunction() == ESlotFunction.TYPE_WEAPON)
		{
			InventoryItemComponent itemComp = slot.GetInventoryItemComponent();
			if (!itemComp)
				return;
	
			BaseWeaponComponent weaponComp = BaseWeaponComponent.Cast(itemComp.GetOwner().FindComponent(BaseWeaponComponent));
			if (!weaponComp)
				return;

			incompatible.SetVisible(weaponComp.GetWeaponSlotType() != m_sWeaponSlotType);			
		}
		else if (slot.GetSlotedItemFunction() == ESlotFunction.TYPE_MAGAZINE)
		{
			if (!m_pItem)
			{
				incompatible.SetVisible(true);
				return;
			}

			BaseWeaponComponent weapon = BaseWeaponComponent.Cast(m_pItem.GetOwner().FindComponent(BaseWeaponComponent));
			if (!weapon)
			{
				incompatible.SetVisible(true);
				return;	
			}

			InventoryItemComponent itemComp = slot.GetInventoryItemComponent();
			if (!itemComp)
				return;

			MagazineComponent magComp = MagazineComponent.Cast(itemComp.GetOwner().FindComponent(MagazineComponent));
			if (!magComp)
				return;

			BaseMagazineWell well = magComp.GetMagazineWell();
			if (!well)
				return;

			BaseMuzzleComponent muzzle = weapon.GetCurrentMuzzle();
			if (!muzzle.GetMagazineWell())
			{
				incompatible.SetVisible(true);
				return;
			}

			if (!menu.IsWeaponEquipped(m_pItem.GetOwner()) || slot.IsInherited(SCR_ArsenalInventorySlotUI))
			{
				incompatible.SetVisible(true);
				return;
			}

			incompatible.SetVisible(!well.Type().IsInherited(muzzle.GetMagazineWell().Type()));
		}
		else
		{
			incompatible.SetVisible(!menu.CanAddAttachment(slot, this));
		}
	}

	//------------------------------------------------------------------------ COMMON METHODS ----------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	void SCR_InventorySlotWeaponSlotsUI( InventoryItemComponent pComponent = null, SCR_InventoryStorageBaseUI pStorageUI = null, bool bVisible = true, int iSlotIndex = -1, SCR_ItemAttributeCollection pAttributes = null, int iWeaponSlotIndex = 0, string sWeaponSlotType = "" )
	{
		m_iQuickSlotIndex = iSlotIndex;
		m_iWeaponSlotIndex = iWeaponSlotIndex;
		m_sWeaponSlotType = sWeaponSlotType;
		m_Attributes = pAttributes;
		UpdateReferencedComponent( pComponent );
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_InventorySlotWeaponSlotsUI()
	{
	}
};