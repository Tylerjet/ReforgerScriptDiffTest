[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_ArsenalFactionEditorAttribute : SCR_BasePresetsEditorAttribute
{	
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return null;
		
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity)
			return null;
		
		SCR_FactionControlComponent factionComponent = SCR_FactionControlComponent.Cast(editableEntity.GetOwner().FindComponent(SCR_FactionControlComponent));
		if (!factionComponent)
			return null;
		
		Faction faction = factionComponent.GetFaction();
		if (!faction)
			return null;
		
		return SCR_BaseEditorAttributeVar.CreateInt(factionManager.GetFactionIndex(faction));
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
		
		SCR_FactionControlComponent factionComponent = SCR_FactionControlComponent.Cast(editableEntity.GetOwner().FindComponent(SCR_FactionControlComponent));
		if (!factionComponent)
			return;
		
		factionComponent.SetFaction(factionManager.GetFactionByIndex(var.GetInt()));
	}
	
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
};