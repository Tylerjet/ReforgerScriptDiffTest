class SCR_AITryUseEquipedItem : AITaskScripted
{
	[Attribute( defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "If item to interact with is a consumable, Is Consumable must be true" )]
	private bool m_bIsConsumable;
	
	// -----------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		IEntity controlledEntity;
		controlledEntity = owner.GetControlledEntity();
		
		if (!controlledEntity)
			return ENodeResult.FAIL;

		CharacterControllerComponent controller = CharacterControllerComponent.Cast(controlledEntity.FindComponent(CharacterControllerComponent));
		if (!controller)
			return false;

		if (controller.CanUseItem() || m_bIsConsumable)
		{
			if (controller.TryUseEquippedItem())
				return ENodeResult.SUCCESS;
		}

		// If consumable is true but tryUse failed, The node has failed
		if (m_bIsConsumable)
			return ENodeResult.FAIL;
			
		SCR_GadgetManagerComponent gadgetManager = SCR_GadgetManagerComponent.GetGadgetManager(controlledEntity);
		if (gadgetManager)
		{
			gadgetManager.ToggleHeldGadget(true);
			return ENodeResult.SUCCESS;
		}

		return ENodeResult.FAIL;
	}
	
	// -----------------------------------------------------------------------------------------------
	protected override string GetNodeMiddleText()
	{
		if (m_bIsConsumable)
			return "Consume the currently held item";
		else
			return "Toggle state of the currently held item";		
	}
	
	// -----------------------------------------------------------------------------------------------
	protected override string GetOnHoverDescription()
	{
		return "Scripted Node: Character will consume the currently held consumable, or toggle the currently held item if possible.";
	}
	
	// -----------------------------------------------------------------------------------------------
	protected override bool VisibleInPalette()
	{
		return true;
	}

};