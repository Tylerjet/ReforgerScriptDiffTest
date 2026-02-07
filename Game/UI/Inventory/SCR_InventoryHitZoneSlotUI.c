class SCR_InventoryHitZoneUI : SCR_InventoryAttachmentPointUI
{
	protected SCR_InventoryHitZonePointContainerUI m_pParentContainer;

	//------------------------------------------------------------------------------------------------
	override bool OnDrop(SCR_InventorySlotUI slot)
	{
		IEntity item = slot.GetInventoryItemComponent().GetOwner();
		if (!item)
			return true;

		SCR_ConsumableItemComponent comp = SCR_ConsumableItemComponent.Cast(item.FindComponent(SCR_ConsumableItemComponent));
		if (!comp)
			return true;

		SCR_ConsumableEffectHealthItems effect = SCR_ConsumableEffectHealthItems.Cast(comp.GetConsumableEffect());
		if (!effect)
			return true;

		if (!effect.CanApplyEffect(m_Player, m_Player))
			return true;

		SCR_GadgetManagerComponent gadgetMgr = SCR_GadgetManagerComponent.GetGadgetManager(m_Player);
		gadgetMgr.SetGadgetMode(item, EGadgetMode.IN_HAND);

		ChimeraCharacter character = ChimeraCharacter.Cast(m_Player);
		SCR_CharacterControllerComponent charCtrl = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
		charCtrl.m_OnGadgetStateChangedInvoker.Insert(OnApplyEffect);

		return true;
	}

	protected void OnApplyEffect(IEntity gadget, bool isInHand, bool isOnGround)
	{
		if (isInHand)
			GetGame().GetCallqueue().CallLater(ApplyLater, 100, false, gadget); // hotfix because m_OnGadgetStateChangedInvoker doesn't wait for anim to be complete
	}

	protected void ApplyLater(IEntity gadget)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(m_Player);
		// todo: unlock item when complete!
		InventoryItemComponent itemComp = InventoryItemComponent.Cast(gadget.FindComponent(InventoryItemComponent));
		itemComp.RequestUserLock(character, true);

		SCR_CharacterControllerComponent charCtrl = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
		SCR_ConsumableItemComponent comp = SCR_ConsumableItemComponent.Cast(gadget.FindComponent(SCR_ConsumableItemComponent));
		SCR_ConsumableEffectHealthItems effect = SCR_ConsumableEffectHealthItems.Cast(comp.GetConsumableEffect());
		if (effect)
			effect.ActivateEffect(character, character, gadget, effect.GetAnimationParameters(character, m_pParentContainer.GetHitZoneGroup()));
	}

	//------------------------------------------------------------------------------------------------
	void SCR_InventoryHitZoneUI(
		BaseInventoryStorageComponent storage,
		LoadoutAreaType slotID = null,
		SCR_InventoryMenuUI menuManager = null,
		int iPage = 0,
		array<BaseInventoryStorageComponent> aTraverseStorage = null,
		SCR_InventoryHitZonePointContainerUI parent = null)
	{
		m_Storage = storage;
		m_MenuHandler 	= menuManager;
		m_eSlotAreaType = slotID;
		m_iMaxRows 		= 1;
		m_iMaxColumns 	= 1;
		m_iMatrix = new SCR_Matrix(m_iMaxColumns, m_iMaxRows);
		m_pParentContainer = parent;
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