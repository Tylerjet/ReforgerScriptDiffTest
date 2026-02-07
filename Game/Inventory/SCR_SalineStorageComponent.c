class SCR_SalineStorageComponentClass : SCR_EquipmentStorageComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_SalineMovedCallback : ScriptedInventoryOperationCallback
{
	SCR_CharacterInventoryStorageComponent m_CharInventoryStorageComp;
	IEntity m_SalineBag;
	float m_fItemRegenerationDuration;
	
	//------------------------------------------------------------------------------------------------
	override protected void OnComplete()
	{
		m_CharInventoryStorageComp.RemoveItemFromQuickSlot(m_SalineBag);
		//Destroy saline bag when the healing effect wears off
		GetGame().GetCallqueue().CallLater(DestroysalineBag, m_fItemRegenerationDuration * 1000, false, m_SalineBag);
	}

	//------------------------------------------------------------------------------------------------
	protected void DestroysalineBag(IEntity item)
	{
		if (!item)
			return;
		
		RplComponent.DeleteRplEntity(item, false);
	}
};

//------------------------------------------------------------------------------------------------
class SCR_SalineStorageComponent : SCR_EquipmentStorageComponent
{
	ref SCR_SalineMovedCallback m_SalineMovedCallback = new SCR_SalineMovedCallback();
	
	//------------------------------------------------------------------------------------------------
	bool AddSalineBagToSlot(IEntity target, ECharacterHitZoneGroup eHitZoneGroup, IEntity salineBag, float itemRegenerationDuration)
	{
		if (!salineBag)
			return false;
		
		SCR_SalineStorageComponent SalineStorageComp = SCR_SalineStorageComponent.Cast(target.FindComponent(SCR_SalineStorageComponent));
		if (!SalineStorageComp)
			return false;
		
		SCR_InventoryStorageManagerComponent storageMan = SCR_InventoryStorageManagerComponent.Cast(GetOwner().FindComponent(SCR_InventoryStorageManagerComponent));
		if (!storageMan)
			return false;
		
		SCR_SalineBagStorageSlot salineTargetSlot;
		for (int i, count = SalineStorageComp.GetSlotsCount(); i < count; i++)
		{
			salineTargetSlot = SCR_SalineBagStorageSlot.Cast(SalineStorageComp.GetSlot(i));
			if (!salineTargetSlot)
				continue;

			if (salineTargetSlot.GetAssociatedHZGroup() != eHitZoneGroup)
				continue;
			
			if (salineTargetSlot.GetItem(i))
			{
				Debug.Error("salineBagSlot already contained some item");
				return false;
			}

			break;
		}
		
		if (!salineTargetSlot)
			return false;
		
		SCR_CharacterInventoryStorageComponent charInventoryStorageComp = SCR_CharacterInventoryStorageComponent.Cast(GetOwner().FindComponent(SCR_CharacterInventoryStorageComponent));
		if (!charInventoryStorageComp)
			return false;

		m_SalineMovedCallback.m_SalineBag = salineBag;
		m_SalineMovedCallback.m_CharInventoryStorageComp = charInventoryStorageComp;
		m_SalineMovedCallback.m_fItemRegenerationDuration = itemRegenerationDuration;
		
		if (storageMan.TryMoveItemToStorage(salineBag, SalineStorageComp, salineTargetSlot.GetID(), m_SalineMovedCallback))
			return true;
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool ShouldPreviewAttachedItems()
	{
		return true;
	}

};