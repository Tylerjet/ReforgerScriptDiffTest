/**
Arsenal Faction Attribute for getting and setting varriables in Editor Attribute window
*/
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_ArsenalFactionEditorAttribute : SCR_BaseFactionEditableAttribute
{	
	protected override bool ValidEntity(GenericEntity entity)
	{
		SCR_ArsenalComponent arsenalComponent = SCR_ArsenalComponent.Cast(entity.FindComponent(SCR_ArsenalComponent));
		if (!arsenalComponent)
			return false;
		
		SCR_FactionControlComponent factionComponent = SCR_FactionControlComponent.Cast(entity.FindComponent(SCR_FactionControlComponent));
		if (!factionComponent)
			return false;
		
		return true;
	}
	
	protected override Faction GetFaction(GenericEntity entity)
	{
		return SCR_FactionControlComponent.Cast(entity.FindComponent(SCR_FactionControlComponent)).GetFaction();
	}
	
	protected override void SetFaction(GenericEntity entity, Faction faction)
	{
		SCR_FactionControlComponent factionComponent = SCR_FactionControlComponent.Cast(entity.FindComponent(SCR_FactionControlComponent));
		if (!factionComponent)
			return;
		
		factionComponent.SetFaction(faction);
	}
};