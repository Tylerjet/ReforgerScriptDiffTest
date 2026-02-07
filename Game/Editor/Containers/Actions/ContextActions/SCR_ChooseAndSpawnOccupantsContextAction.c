[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_ChooseAndSpawnOccupantsContextAction : SCR_SelectedEntitiesContextAction
{
	[Attribute(desc: "Compartments Types to fill when chosing entities.", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ECompartmentType))]
	protected ref array<ECompartmentType> m_aCompartmentsTypes;
	
	override bool CanBeShown(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{		
		if (!hoveredEntity)
			return false;
		
		SCR_BaseCompartmentManagerComponent compartmentManager = SCR_BaseCompartmentManagerComponent.Cast(hoveredEntity.GetOwner().FindComponent(SCR_BaseCompartmentManagerComponent));
		
		return compartmentManager && compartmentManager.HasDefaultOccupantsDataForTypes(m_aCompartmentsTypes);
	}
	
	override bool CanBePerformed(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		SCR_EditableVehicleComponent vehicle = SCR_EditableVehicleComponent.Cast(hoveredEntity);
		return vehicle && vehicle.CanOccupyVehicleWithCharacters(m_aCompartmentsTypes, false, checkOccupyingFaction: false);
	}
	
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition,int flags, int param = -1)
	{
		//SCR_EditableVehicleComponent vehicle = SCR_EditableVehicleComponent.Cast(hoveredEntity);
		//vehicle.OccupyVehicleWithDefaultCharacters(m_aCompartmentsTypes);
		
		
		//Print("Needs label content browser update in order to create a config and set the correct filters!");
		//Print("Open editor browser with placing flag and filter on all characters and Groups of specific sizes");
		
		if (!hoveredEntity)
			return;
		
		SCR_ContentBrowserEditorComponent contentBrowserManager = SCR_ContentBrowserEditorComponent.Cast(SCR_ContentBrowserEditorComponent.GetInstance(SCR_ContentBrowserEditorComponent, true));
		if (contentBrowserManager)
		{
			//~ Always active Characters. Not always active groups and filter on size
			SCR_EditorContentBrowserDisplayConfig newConfig = new SCR_EditorContentBrowserDisplayConfig();
			contentBrowserManager.OpenBrowserExtended(hoveredEntity, newConfig);
		}
	}
};