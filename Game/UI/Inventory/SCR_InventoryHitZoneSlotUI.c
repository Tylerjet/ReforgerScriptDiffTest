class SCR_InventoryHitZoneUI : SCR_InventoryAttachmentPointUI
{
	protected SCR_InventoryHitZonePointContainerUI m_pParentContainer;
	protected bool m_bTourniquetted;

	//------------------------------------------------------------------------------------------------
	override bool OnDrop(SCR_InventorySlotUI slot)
	{
		IEntity item = slot.GetInventoryItemComponent().GetOwner();
		if (!item)
			return true;

		if (!CanApplyItem(item))
			return true;

		SCR_GadgetManagerComponent gadgetMgr = SCR_GadgetManagerComponent.GetGadgetManager(m_Player);
		gadgetMgr.SetGadgetMode(item, EGadgetMode.IN_HAND);

		ChimeraCharacter character = ChimeraCharacter.Cast(m_Player);
		SCR_CharacterControllerComponent charCtrl = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
		if (charCtrl.GetRightHandItem() == item || charCtrl.GetAttachedGadgetAtLeftHandSlot() == item)
		{
			ApplyLater(item);
			return true;
		}
		
		charCtrl.m_OnGadgetStateChangedInvoker.Insert(OnApplyEffect);

		return true;
	}

	protected void OnApplyEffect(IEntity gadget, bool isInHand, bool isOnGround)
	{
		if (!m_pParentContainer)
			return;

		if (isInHand)
			GetGame().GetCallqueue().CallLater(ApplyLater, 100, false, gadget); // hotfix because m_OnGadgetStateChangedInvoker doesn't wait for anim to be complete
	}

	//------------------------------------------------------------------------------------------------
	bool CanApplyItem(notnull IEntity item)
	{
		SCR_ConsumableItemComponent comp = SCR_ConsumableItemComponent.Cast(item.FindComponent(SCR_ConsumableItemComponent));
		if (!comp)
			return false;

		SCR_ConsumableEffectHealthItems effect = SCR_ConsumableEffectHealthItems.Cast(comp.GetConsumableEffect());

		return effect && effect.CanApplyEffectToHZ(m_Player, m_Player, m_pParentContainer.GetHitZoneGroup());
	}

	protected void ApplyLater(IEntity gadget)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(m_Player);
		InventoryItemComponent itemComp = InventoryItemComponent.Cast(gadget.FindComponent(InventoryItemComponent));

		SCR_CharacterControllerComponent charCtrl = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
		SCR_ConsumableItemComponent comp = SCR_ConsumableItemComponent.Cast(gadget.FindComponent(SCR_ConsumableItemComponent));
		SCR_ConsumableEffectHealthItems effect = SCR_ConsumableEffectHealthItems.Cast(comp.GetConsumableEffect());
		if (effect)
			effect.ActivateEffect(character, character, gadget, effect.GetAnimationParameters(character, m_pParentContainer.GetHitZoneGroup()));

		itemComp.RequestUserLock(character, true);
		charCtrl.m_OnGadgetStateChangedInvoker.Remove(OnApplyEffect);
	}

	void SetTourniquetted(bool tourniquetted)
	{
		m_bTourniquetted = tourniquetted;
	}

	bool IsTourniquetted()
	{
		return m_bTourniquetted;
	}

	SCR_InventoryMenuUI GetMenuHandler()
	{
		return m_pParentContainer.GetInventoryHandler();
	}

	//------------------------------------------------------------------------------------------------
	void SCR_InventoryHitZoneUI(
		BaseInventoryStorageComponent storage,
		LoadoutAreaType slotID = null,
		SCR_InventoryMenuUI menuManager = null,
		int iPage = 0,
		array<BaseInventoryStorageComponent> aTraverseStorage = null,
		SCR_InventoryHitZonePointContainerUI parent = null, bool isTourniquetted = false)
	{
		m_Storage = storage;
		m_MenuHandler 	= menuManager;
		m_eSlotAreaType = slotID;
		m_iMaxRows 		= 1;
		m_iMaxColumns 	= 1;
		m_iMatrix = new SCR_Matrix(m_iMaxColumns, m_iMaxRows);
		m_pParentContainer = parent;
		m_bTourniquetted = isTourniquetted;
	}
}

//------------------------------------------------------------------------------------------------
class SCR_InventoryHitZoneSlotUI : SCR_InventorySlotUI
{
	override bool OnDrop(SCR_InventorySlotUI slot)
	{
		return m_pStorageUI.OnDrop(slot);
	}
}