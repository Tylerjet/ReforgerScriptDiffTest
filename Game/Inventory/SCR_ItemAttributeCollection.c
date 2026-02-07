class SCR_ItemAttributeCollection: ItemAttributeCollection
{
	[Attribute("2", UIWidgets.ComboBox, "Slot size", "", ParamEnumArray.FromEnum(ESlotSize))]
	protected ESlotSize m_Size;
	
	[Attribute("2", UIWidgets.ComboBox, "Slot size", "", ParamEnumArray.FromEnum(ESlotID))]
	protected ESlotID m_SlotType;

	[Attribute("1", UIWidgets.CheckBox, "Sets item movable by drag'n'drop")]
	protected bool m_bDraggable;

	private ItemPhysicalAttributes m_PhysAttributes;
	
	//------------------------------------------------------------------------------------------------
	float GetVolume() 
	{
		if (m_PhysAttributes != null)
		{
			return m_PhysAttributes.GetVolume();
		}
		return 0.0;
	}

	
	//------------------------------------------------------------------------------------------------
	float GetWeight()
	{
		if (m_PhysAttributes != null)
		{
			return m_PhysAttributes.GetWeight();
		}
		return 0.0;
	};

	//------------------------------------------------------------------------------------------------
	bool IsDraggable()
	{
		return m_bDraggable;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetSlotSize( ESlotSize slotSize ) 	
	{
		m_Size = slotSize; 
	}
	
	//------------------------------------------------------------------------------------------------
	void SetSlotType( ESlotID slotID ) 	
	{
		m_SlotType = slotID; 
	}
	
	//------------------------------------------------------------------------------------------------
	ESlotID GetSlotType() 		{ return m_SlotType; }

	//------------------------------------------------------------------------------------------------
	int GetSlotSum()
	{
		int iRetVal = 0;
		switch( m_Size )
		{
			case ESlotSize.SLOT_1x1: { iRetVal = 1; } break;
			case ESlotSize.SLOT_2x1: { iRetVal = 2; } break;
			case ESlotSize.SLOT_2x2: { iRetVal = 4; } break;
			case ESlotSize.SLOT_3x3: { iRetVal = 9; } break;
		}
		return iRetVal;
	}
			
	//------------------------------------------------------------------------------------------------
	//! size of the slot the item fits in
	ESlotSize GetItemSize()	{ return m_Size; }

	override protected void OnInitCollection(IEntityComponentSource src)
	{
		m_PhysAttributes = ItemPhysicalAttributes.Cast(FindAttribute(ItemPhysicalAttributes));
	}
};
