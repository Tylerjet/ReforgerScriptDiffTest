//------------------------------------------------------------------------------------------------
//! modded version for to be used with the inventory 2.0 
class SCR_PickUpItemAction : SCR_InventoryAction
{
	#ifndef DISABLE_INVENTORY
	//------------------------------------------------------------------------------------------------
	override protected void PerformActionInternal(SCR_InventoryStorageManagerComponent manager, IEntity pOwnerEntity, IEntity pUserEntity)
	{
		manager.InsertItem( pOwnerEntity );
		
		// Play sound
		RplComponent rplComp = RplComponent.Cast(pOwnerEntity.FindComponent(RplComponent));
		if (rplComp)
			manager.PlayItemSound(rplComp.Id(), "SOUND_PICK_UP");
		else
		{
			SimpleSoundComponent simpleSoundComp = SimpleSoundComponent.Cast(pOwnerEntity.FindComponent(SimpleSoundComponent));
			if (simpleSoundComp)
			{
				vector mat[4];
				pOwnerEntity.GetWorldTransform(mat);
				
				simpleSoundComp.SetTransformation(mat);
				simpleSoundComp.PlayStr("SOUND_PICK_UP");
			}
		}
	}
	
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
 	{
		if ( !super.CanBePerformedScript( user ) )
			return false;
		
		auto pInventoryManager = SCR_InventoryStorageManagerComponent.Cast( user.FindComponent( SCR_InventoryStorageManagerComponent ) );
		if ( !pInventoryManager )
			return false;
		
		pInventoryManager.SetReturnCodeDefault();
		
		if ( !pInventoryManager.IsAnimationReady() || !pInventoryManager.CanInsertItem( m_Item.GetOwner(), EStoragePurpose.PURPOSE_DEPOSIT ) )
		{
			SetCannotPerformReason( GetReason( pInventoryManager ) );
			return false;
		}
		
		return true;
 	}
	#endif
};