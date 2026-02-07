class SCR_AIEquipRightHandItem : AITaskScripted
{
	[Attribute("0", UIWidgets.ComboBox, "Item interest type", "", ParamEnumArray.FromEnum(ECommonItemType) )]
	private ECommonItemType m_eItemOfInterestType;

	[Attribute("0", UIWidgets.ComboBox, "Gadget type", "", ParamEnumArray.FromEnum(EGadgetType) )]
	private EGadgetType m_eGadgetType;
	
	private bool m_bIsEquipped = false;
	private IEntity m_OwnerEntity;
	private CharacterControllerComponent controller;

	// -----------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		"ItemType",
		"GadgetType"
	};
	override TStringArray GetVariablesIn()
    {
        return s_aVarsIn;
    }

	// -----------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		m_OwnerEntity = owner.GetControlledEntity();
		if (!m_OwnerEntity)
			NodeError(this, owner, "Missing owner entity!");
	}

	// -----------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_bIsEquipped)
		{
			GetVariableIn("ItemType",m_eItemOfInterestType);
			GetVariableIn("GadgetType",m_eGadgetType);

			if (m_eItemOfInterestType != 0)
				m_bIsEquipped = AIEquipInventoryItem(m_eItemOfInterestType);
			else if (m_eGadgetType != 0)
				m_bIsEquipped = AIEquipGadgetItem(m_eGadgetType);
		}

		if (!m_bIsEquipped)
		    return ENodeResult.FAIL;

	    return ENodeResult.SUCCESS;
	}


	// -----------------------------------------------------------------------------------------------
	bool AIEquipInventoryItem(ECommonItemType wantedEntityType)
	{
		controller = CharacterControllerComponent.Cast(m_OwnerEntity.FindComponent(CharacterControllerComponent));
		if (!controller)
			return false;

		SCR_InventoryStorageManagerComponent inventoryManager = SCR_InventoryStorageManagerComponent.Cast(m_OwnerEntity.FindComponent(SCR_InventoryStorageManagerComponent));

		if (!inventoryManager)
			return false;

		SCR_HoldableItemPredicate predicate = new SCR_HoldableItemPredicate();
		predicate.wanted = wantedEntityType;

		IEntity item = inventoryManager.FindItem(predicate);
		if (!item)
			return false;

		controller.TakeGadgetInLeftHand(item, 1);

		return true;
	}

	// -----------------------------------------------------------------------------------------------
	bool AIEquipGadgetItem(EGadgetType wantedGadgetType)
	{
		SCR_GadgetManagerComponent gadgetManager = SCR_GadgetManagerComponent.GetGadgetManager(m_OwnerEntity);
		if (!gadgetManager)
			return false;

		IEntity item = gadgetManager.GetGadgetByType(wantedGadgetType);
		if (!item)
			return false;

		gadgetManager.SetGadgetMode(item, EGadgetMode.IN_HAND);

		return true;
	}
	
	// -----------------------------------------------------------------------------------------------
	protected override string GetNodeMiddleText()
	{
		if (m_eItemOfInterestType != 0)
			return "Item to be equiped: " + typename.EnumToString(ECommonItemType,m_eItemOfInterestType);
		else if (m_eGadgetType != 0)
			return "Item to be equiped: " + typename.EnumToString(EGadgetType,m_eGadgetType);
		else 
			return "No valid item selected";
	}
	
	// -----------------------------------------------------------------------------------------------
	protected override bool VisibleInPalette()
	{
		return true;
	}
	
	// -----------------------------------------------------------------------------------------------
	protected override string GetOnHoverDescription()
	{
		return "Scripted Node: Make the AI to equip a gadget item or a common item";
	}
};
