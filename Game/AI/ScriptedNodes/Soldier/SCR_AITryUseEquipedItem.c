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
			return ENodeResult.FAIL;

		if (!controller.CanUseItem())
			return ENodeResult.FAIL;

		IEntity gadget = controller.GetAttachedGadgetAtLeftHandSlot();

		// If the equiped item is not consumable, execute right away		
		if (!m_bIsConsumable)
		{
			if (controller.TryUseItem(gadget))
				return ENodeResult.SUCCESS;
			else
				return ENodeResult.FAIL;
		}

		if (!gadget)
			return ENodeResult.FAIL;
		
		SCR_ConsumableItemComponent consumableItemComp = SCR_ConsumableItemComponent.Cast(gadget.FindComponent(SCR_ConsumableItemComponent));
		if (!consumableItemComp)
			return ENodeResult.FAIL;
		
		SCR_ConsumableEffectBase consumableEffect = consumableItemComp.GetConsumableEffect();
		if (!consumableEffect)
			return ENodeResult.FAIL;
		
		if (!consumableEffect.CanApplyEffect(controlledEntity, owner))
			return ENodeResult.FAIL;
	
		if (consumableEffect.ActivateEffect(controlledEntity, controlledEntity, gadget))
			return ENodeResult.SUCCESS;

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