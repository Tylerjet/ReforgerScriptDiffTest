class SCR_UniversalInventoryStorageComponentClass: UniversalInventoryStorageComponentClass
{
};

// Current storage variant allows dynamic scaling of slots and handles Move/Insert/Remove operations
// it will accept any entity for insertion and will remove/add it's visibility flag when inserted/removed from storage
// see CharacterInventoryStorageComponent for example of custom storage inheritance from current class
class SCR_UniversalInventoryStorageComponent : UniversalInventoryStorageComponent
{
	
	
	[Attribute( "0", UIWidgets.EditBox, "How much weight it can carry")]
	protected float m_fMaxWeight;
	
	#ifndef DISABLE_INVENTORY
	private SCR_ItemAttributeCollection 							pAttributes;
	protected float 												m_fWeight;
	protected float 												m_fVolume;
	protected SCR_InventoryStorageManagerComponent					pInventoryManager;
	
	//protected float										m_fWeightSum			= 0.0;
	//protected float										m_fVolumeSum			= 0.0;
	
	
	
	//------------------------------------------------------------------------ USER METHODS ------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	private SCR_ItemAttributeCollection GetAttributeCollection( IEntity item )
	{
		InventoryItemComponent pItemComp = GetItemComponent( item );
		if( !pItemComp )
			return null;
		return SCR_ItemAttributeCollection.Cast( pItemComp.GetAttributes() );
	}
	
	//------------------------------------------------------------------------------------------------
	protected InventoryItemComponent GetItemComponent( IEntity pItem )
	{
		return InventoryItemComponent.Cast( pItem.FindComponent( InventoryItemComponent ) );
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool IsWeightOk( float fWeight ) 
	{ 
		if (!pAttributes)
		{
			return false;
		}
		fWeight += ( GetTotalWeight() - pAttributes.GetWeight() );
		return m_fMaxWeight >= fWeight;		
	}	
	
	//------------------------------------------------------------------------------------------------
	override bool CanStoreItem(IEntity item, int slotID)
	{
		
		InventoryItemComponent pItemComp = GetItemComponent( item );
		if( !pItemComp )
			return false;

		bool bVolumeOK = PerformVolumeValidation( item );
		if( !bVolumeOK )
		{
			if( pInventoryManager )	
				pInventoryManager.SetReturnCode( EInventoryRetCode.RETCODE_ITEM_TOO_BIG );
		}
		bool bWeightOK = IsWeightOk( pItemComp.GetTotalWeight() );
		if( !bWeightOK )
		{
			if( pInventoryManager )	
				pInventoryManager.SetReturnCode( EInventoryRetCode.RETCODE_ITEM_TOO_HEAVY );
		}
		return bVolumeOK && bWeightOK;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanRemoveItem(IEntity item)
	{
		//TODO: Define conditions for removing the item
		if( item )
			return true;
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnRemovedFromSlot(IEntity item, int slotID)
	{
		GenericEntity pGenComp = GenericEntity.Cast( item );
		auto pItemComponent = InventoryItemComponent.Cast( pGenComp.FindComponent( InventoryItemComponent ) );
		pItemComponent.ShowOwner();
		pItemComponent.EnablePhysics();
		
		SCR_ItemAttributeCollection itemAttributeCollection = GetAttributeCollection( item );
		if( !itemAttributeCollection )
			return;
		
		m_fWeight -= pItemComponent.GetTotalWeight();
		m_fVolume -= itemAttributeCollection.GetVolume();
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnAddedToSlot(IEntity item, int slotID)
	{
		GenericEntity pGenComp = GenericEntity.Cast( item );
		auto pItemComponent = InventoryItemComponent.Cast( pGenComp.FindComponent( InventoryItemComponent ) );
		if( !pItemComponent )
			return;
		pItemComponent.HideOwner();
		pItemComponent.DisablePhysics();
		pItemComponent.ActivateOwner(false);
				
		SCR_ItemAttributeCollection itemAttributeCollection = GetAttributeCollection( item );
		if( !itemAttributeCollection )
			return;
		
		m_fWeight += pItemComponent.GetTotalWeight();
		m_fVolume += itemAttributeCollection.GetVolume();
		
	}
	
		
	//------------------------------------------------------------------------ COMMON METHODS ----------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	override void OnManagerChanged(InventoryStorageManagerComponent manager)
	{
		pInventoryManager = SCR_InventoryStorageManagerComponent.Cast( manager );
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_UniversalInventoryStorageComponent( IEntityComponentSource src, IEntity ent, IEntity parent )
	{
		
		pAttributes = SCR_ItemAttributeCollection.Cast( GetAttributes() );
		if( !pAttributes )
			return;

		m_fWeight = pAttributes.GetWeight();
		//m_fVolume = pAttributes.GetVolume();
		m_fVolume = 0; //empty from the start
		
	}
	#else
	private SCR_ItemAttributeCollection GetAttributeCollection( IEntity item );
	protected InventoryItemComponent GetItemComponent( IEntity pItem );
	protected bool IsVolumeOk( float fVolume );	
	protected bool IsWeightOk( float fWeight );
	override bool CanStoreItem(IEntity item, int slotID);
	override bool CanRemoveItem(IEntity item);
	override void OnRemovedFromSlot(IEntity item, int slotID);
	protected override void OnAddedToSlot(IEntity item, int slotID);
	override void OnManagerChanged(InventoryStorageManagerComponent manager);
//	void SCR_UniversalInventoryStorageComponent( IEntityComponentSource src, IEntity ent, IEntity parent );

	#endif
	
};