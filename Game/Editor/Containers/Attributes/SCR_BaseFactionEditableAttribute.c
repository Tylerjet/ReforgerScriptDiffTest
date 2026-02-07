/**
Group Faction Attribute for getting and setting varriables in Editor Attribute window
*/
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_BaseFactionEditableAttribute : SCR_BasePresetsEditorAttribute
{	
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity) 
			return null;
		
		if (!ValidEntity(editableEntity.GetOwner()))
			return null;
		
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return null;
		
		Faction faction = GetFaction(editableEntity.GetOwner());
		
		if (!faction)
			return null;

		return SCR_BaseEditorAttributeVar.CreateInt(factionManager.GetFactionIndex(faction));
	}
	
	//~ Check if entity is valid to set faction
	protected bool ValidEntity(GenericEntity entity)
	{
		return false;
	}
	
	//~ Get the entity faction
	protected Faction GetFaction(GenericEntity entity)
	{
		return null;
	}
	
	//~ Set the entity faction
	protected void SetFaction(GenericEntity entity, Faction faction)
	{
		
	}
	
	//~ Creates the buttons to select faction
	protected override void CreatePresets()
	{			
		SCR_DelegateFactionManagerComponent delegateFactionManager = SCR_DelegateFactionManagerComponent.GetInstance();		
		if (!delegateFactionManager)
			return;
		
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return;
		
		m_aValues.Clear();	
		
		SCR_SortedArray<SCR_EditableFactionComponent> factionDelegates = new SCR_SortedArray<SCR_EditableFactionComponent>;
		int count = delegateFactionManager.GetSortedFactionDelegates(factionDelegates);
			
		SCR_Faction scrFaction;
		
		for(int i = 0; i < count; ++i)
		{
			scrFaction = SCR_Faction.Cast(factionDelegates.Get(i).GetFaction());
			
			if (!scrFaction)
				continue;

			SCR_EditorAttributeFloatStringValueHolder value = new SCR_EditorAttributeFloatStringValueHolder();
			value.SetWithUIInfo(scrFaction.GetUIInfo(), factionManager.GetFactionIndex(factionDelegates.Get(i).GetFaction()));
			m_aValues.Insert(value);
		}
	}
	
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var)
			return;
		
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return;
		
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity)
			return;
		
		Faction faction = factionManager.GetFactionByIndex(var.GetInt());
		if (!faction)
			return;
	
		SetFaction(editableEntity.GetOwner(), faction);
	}
		
}

