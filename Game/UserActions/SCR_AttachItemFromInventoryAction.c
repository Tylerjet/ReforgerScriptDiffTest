//------------------------------------------------------------------------------------------------
//! Equip weapon attachment
class SCR_AttachItemFromInventoryAction : SCR_InventoryAction
{
	#ifndef DISABLE_INVENTORY
	SCR_InventoryStorageManagerComponent m_InventoryManager;
	InventoryItemComponent m_ItemComponent;
	BaseWeaponComponent m_WeaponComponent;
	
	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{		
		GenericEntity owner = GenericEntity.Cast(pOwnerEntity);
		if (owner)
		{
			m_ItemComponent = InventoryItemComponent.Cast(owner.FindComponent(InventoryItemComponent));
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void PerformActionInternal(SCR_InventoryStorageManagerComponent manager, IEntity pOwnerEntity, IEntity pUserEntity)
	{
		WeaponAttachmentsStorageComponent storage = WeaponAttachmentsStorageComponent.Cast(m_WeaponComponent.GetOwner().FindComponent(WeaponAttachmentsStorageComponent));
		if (!m_InventoryManager.CanMoveItemToStorage(GetOwner(), storage, -1))
		{
			BaseMagazineComponent magazine = BaseMagazineComponent.Cast(pOwnerEntity.FindComponent(BaseMagazineComponent));
			CharacterControllerComponent ctrl = CharacterControllerComponent.Cast(pUserEntity.FindComponent(CharacterControllerComponent));
			if (ctrl && magazine)
			{
				ctrl.ReloadWeaponWith(pOwnerEntity);
			}
			else
			{
				manager.TrySwapItemStorages(pOwnerEntity, storage.Get(0));
			}
			manager.PlayItemSound(pOwnerEntity, "SOUND_SWITCH");
		}
		else
		{
			manager.EquipWeaponAttachment(pOwnerEntity, pUserEntity);		
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{		
		if (!m_ItemComponent)
			return false;

		if (!m_ItemComponent.GetParentSlot())
			return false;

		ChimeraCharacter character = ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		if(!character)
			return false;
		
		CharacterControllerComponent controller = character.GetCharacterController();	
		if(!controller)
			return false;	
			
		if(!controller.GetInspect())
			return false;
		
		if(!m_InventoryManager)
			m_InventoryManager = SCR_InventoryStorageManagerComponent.Cast(character.FindComponent(SCR_InventoryStorageManagerComponent));
		if(!m_InventoryManager)
			return false;
		
		BaseWeaponManagerComponent weaponManager = controller.GetWeaponManagerComponent();
		if(!weaponManager)
			return false;

		m_WeaponComponent = controller.GetWeaponManagerComponent().GetCurrentWeapon();
		if(!m_WeaponComponent || m_WeaponComponent.GetOwner() == m_ItemComponent.GetParentSlot().GetOwner())
			return false;	
		
		auto storage = WeaponAttachmentsStorageComponent.Cast(m_WeaponComponent.GetOwner().FindComponent(WeaponAttachmentsStorageComponent));
		if (!storage)
			return false;
		
		bool result = m_InventoryManager.CanMoveItemToStorage(GetOwner(), storage, -1);
		if (!result)
		{
			IEntity existingAttachment = storage.Get(0);
			if (existingAttachment)
				result = m_InventoryManager.CanSwapItemStorages(GetOwner(), existingAttachment);
		}

		return result;
	}	
	
	override bool GetActionNameScript(out string outName)
	{
		if (!m_ItemComponent)
			return false;
		UIInfo actionInfo = GetUIInfo();
		UIInfo itemInfo = m_ItemComponent.GetUIInfo();
		if (actionInfo && itemInfo)
		{
			outName = string.Format("%1%2", actionInfo.GetName(), itemInfo.GetName());
			return true;
		}
		else
		{
			return false;
		}			
	}
	#endif
};