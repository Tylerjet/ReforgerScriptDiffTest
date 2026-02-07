[EntityEditorProps(category: "GameScripted/Components", description: "Mine inventory item component.")]
class SCR_MineInventoryItemComponentClass : SCR_PlaceableInventoryItemComponentClass
{
	
};

//------------------------------------------------------------------------------------------------
class SCR_MineInventoryItemComponent : SCR_PlaceableInventoryItemComponent
{
	//------------------------------------------------------------------------------------------------
	override bool ShouldHideInVicinity()
	{
		SCR_PressureTriggerComponent triggerComponent = SCR_PressureTriggerComponent.Cast(GetOwner().FindComponent(SCR_PressureTriggerComponent));
		if (!triggerComponent)
			return false;
		
		return triggerComponent.IsActivated();
	}
};