// Script File
//Get all characters. Check if A group (or more) is selected. Check if all characters in the group are selected. Ignore if true. If only of the same group do not preform

[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_CreateNewGroupContextAction : SCR_BaseContextAction
{	
	override bool CanBeShown(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		SCR_LayersEditorComponent layersManager = SCR_LayersEditorComponent.Cast(SCR_BaseEditorComponent.GetInstance(SCR_LayersEditorComponent));
		if (!layersManager)
			return false;
		
		SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		if (!core)
			return false;
		
		foreach	(SCR_EditableEntityComponent entity : selectedEntities)
		{
			//if (core.CanSetParentOfType(entity.GetEntityType(), EEditableEntityType.GROUP) && lobby.GetPlayerId(entity.GetOwner()) <= 0)
			if (entity.GetEntityType() == EEditableEntityType.CHARACTER && SCR_PossessingManagerComponent.GetPlayerIdFromControlledEntity(entity.GetOwner()) <= 0)
				return true;
		}
		
		return false;
	}
	
	//Can always preform if has AI characters
	override bool CanBePerformed(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{		
		SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		if (!core)
			return false;
		
		SCR_EditableEntityComponent parentGroup = null;
		int entityCount = 0;
		
		//Make sure that if all selected characters are in the same group and the whole group is selected that the player cannot create a new group (as this would change nothing)
		foreach	(SCR_EditableEntityComponent entity : selectedEntities)
		{
			//if (core.CanSetParentOfType(entity.GetEntityType(), EEditableEntityType.GROUP) && lobby.GetPlayerId(entity.GetOwner()) <= 0)
			if (entity.GetEntityType() == EEditableEntityType.CHARACTER && SCR_PossessingManagerComponent.GetPlayerIdFromControlledEntity(entity.GetOwner()) <= 0)
			{
				SCR_EditableEntityComponent newParentGroup = entity.GetParentEntity();
				
				entityCount++;
				
				if (parentGroup == null)
					parentGroup = newParentGroup;
				else if (parentGroup != newParentGroup)
					return true;
			}
		}
		
		return (parentGroup && parentGroup.GetChildrenCount() != entityCount);
	}
	
	
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition,int flags, int param = -1)
	{
		SCR_LayersEditorComponent layersManager = SCR_LayersEditorComponent.Cast(SCR_BaseEditorComponent.GetInstance(SCR_LayersEditorComponent));
		if (!layersManager)
			return;
		
		SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		if (!core)
			return;
		
		set<SCR_EditableEntityComponent> entitiesToMove = new set<SCR_EditableEntityComponent>;
		set<SCR_EditableEntityComponent> firstEntity = new set<SCR_EditableEntityComponent>;
		
		//Makes sure that if the hovered entity is a character that the created group will be that faction
		//if (hoveredEntity && core.CanSetParentOfType(hoveredEntity.GetEntityType(), EEditableEntityType.GROUP) && lobby.GetPlayerId(hoveredEntity.GetOwner()) <= 0)
		if (hoveredEntity && hoveredEntity.GetEntityType() == EEditableEntityType.CHARACTER && SCR_PossessingManagerComponent.GetPlayerIdFromControlledEntity(hoveredEntity.GetOwner()) <= 0)
			firstEntity.Insert(hoveredEntity);
		
		foreach(SCR_EditableEntityComponent entity: selectedEntities)
		{
			//if (core.CanSetParentOfType(entity.GetEntityType(), EEditableEntityType.GROUP)  && lobby.GetPlayerId(entity.GetOwner()) <= 0)
			if (entity.GetEntityType() == EEditableEntityType.CHARACTER && SCR_PossessingManagerComponent.GetPlayerIdFromControlledEntity(entity.GetOwner()) <= 0)
			{
				if (entity == hoveredEntity)
					continue;
				
				if (!firstEntity.IsEmpty())
					entitiesToMove.Insert(entity);
				else
					 firstEntity.Insert(entity);
			}
		}
		
		if (firstEntity.IsEmpty())
			return;
		
		
		layersManager.SplitGroupAndAddCharacters(SCR_EditableCharacterComponent.Cast(firstEntity[0]), entitiesToMove); 
	}

	
};