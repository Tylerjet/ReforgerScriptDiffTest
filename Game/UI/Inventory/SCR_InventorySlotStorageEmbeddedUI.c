
//------------------------------------------------------------------------------------------------
//! UI Script
//! Inventory Slot UI Layout
class SCR_InventorySlotStorageEmbeddedUI : SCR_InventorySlotStorageUI
{		
	//------------------------------------------------------------------------ USER METHODS ------------------------------------------------------------------------							
	
	//------------------------------------------------------------------------------------------------
	override void SetSelected( bool select )
	{
		super.SetSelected( select );
		
		if ( !m_wSelectedEffect )
			return;
		if ( select )
		{
			m_wSelectedEffect.SetVisible( true );
			if( this.Type() != SCR_InventorySlotStorageUI )
				m_wMoveWhatEffect.SetVisible( true );
		}
		else
		{
			m_wSelectedEffect.SetVisible( false );
			m_wMoveWhatEffect.SetVisible( false );
		}
	}
	
		
	//------------------------------------------------------------------------------------------------
	void SCR_InventorySlotStorageEmbeddedUI( InventoryItemComponent pComponent = null, SCR_InventoryStorageBaseUI pStorageUI = null, bool bVisible = true, int iSlotIndex = -1, SCR_ItemAttributeCollection pAttributes = null )
	{
		m_pStorageUI = pStorageUI;
	}
	

	//------------------------------------------------------------------------------------------------
	void ~SCR_InventorySlotStorageEmbeddedUI()
	{
	}
};