/**
Entity Fuel Attribute for getting and setting varriables in Editor Attribute window
*/
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_FuelEditorAttribute : SCR_BaseValueListEditorAttribute
{
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity) return null;
		
		IEntity owner =  editableEntity.GetOwner();
		if (!owner) return null;
		FuelManagerComponent fuelManager = FuelManagerComponent.Cast(owner.FindComponent(FuelManagerComponent));
		
		if (!fuelManager) return null;
		
		//Don't show if destroyed
		DamageManagerComponent damageComponent = DamageManagerComponent.Cast(owner.FindComponent(DamageManagerComponent));
		if (damageComponent)
			if (damageComponent.GetState() == EDamageState.DESTROYED)  return null;
		
				
		float fuel = fuelManager.GetTotalFuel() / fuelManager.GetTotalMaxFuel();
		return SCR_BaseEditorAttributeVar.CreateFloat(Math.Round(fuel * 100));
	}
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var) return;
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		
		IEntity owner =  editableEntity.GetOwner();
		if (!owner) return;
		FuelManagerComponent fuelManager = FuelManagerComponent.Cast(owner.FindComponent(FuelManagerComponent));
		if (!fuelManager) return;
				
		array<BaseFuelNode> nodes = new array<BaseFuelNode>;
		fuelManager.GetFuelNodesList(nodes);
		
		for(int i = 0; i < nodes.Count(); i++)
		{
			BaseFuelNode node = nodes.Get(i);
			node.SetFuel(var.GetFloat() / 100 * node.GetMaxFuel());
		}
	}
};