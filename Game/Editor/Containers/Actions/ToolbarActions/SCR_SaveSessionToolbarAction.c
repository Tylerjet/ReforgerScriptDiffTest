[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_SaveSessionToolbarAction : SCR_EditorToolbarAction
{
	[Attribute(desc: "When enabled, the operation will always bring up a save dialog.")]
	protected bool m_bSaveAs;
	
	[Attribute(ESaveType.USER.ToString(), UIWidgets.ComboBox, "Save file type.", enums: ParamEnumArray.FromEnum(ESaveType))]
	protected ESaveType m_eSaveType;
	
	//---------------------------------------------------------------------------------------------
	override bool IsServer()
	{
		//--- The action opens local UI
		return false;
	}
	
	//---------------------------------------------------------------------------------------------
	override bool CanBeShown(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		// PS prevent from using on server  
		if (System.GetPlatform() == EPlatform.PS5 || System.GetPlatform() == EPlatform.PS4 || System.GetPlatform() == EPlatform.PS5_PRO)
		{
			ServerInfo serverInfo = GetGame().GetServerInfo();
			if (!Replication.IsServer() && serverInfo && serverInfo.IsCrossplay())
				return false;
		}
		
		//--- Disallow on client, or in MP for "Save" version ("Save As" is allowed in MP)
		if (!Replication.IsServer() || (!m_bSaveAs && Replication.IsRunning()))
			return false;
		
		//--- Disallow if editor save struct is not configured
		return GetGame().GetSaveManager().ScenarioCanBeSaved();
	}
	
	//---------------------------------------------------------------------------------------------
	override bool CanBePerformed(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		return CanBeShown(hoveredEntity, selectedEntities, cursorWorldPosition, flags);
	}
	
	//---------------------------------------------------------------------------------------------
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition,int flags, int param = -1)
	{
		SCR_SaveManagerCore saveManager = GetGame().GetSaveManager();
		
		// PS save behavior
		if (System.GetPlatform() == EPlatform.PS5 || System.GetPlatform() == EPlatform.PS4 || System.GetPlatform() == EPlatform.PS5_PRO)
		{
			if (m_bSaveAs || !saveManager.OverrideCurrentSave(m_eSaveType))
				GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.EditorSaveDialog);
			
			return;
		}
		
		WorldSaveItem saveItem;
		string saveName = SCR_SaveWorkshopManager.GetInstance().GetCurrentSave(saveItem);
		
		// Create new world save item for save without item
		if (!saveName.IsEmpty() && !saveItem)
		{
			WorldSaveManifest manifest = new WorldSaveManifest();
			
			string saveCustomName = saveManager.GetCustomName(saveName);
			manifest.m_sName = saveCustomName;
			manifest.m_aFileNames = {saveName};
		
			// Enable all dependencies by default
			manifest.m_aDependencyIds = {};
			GameProject.GetLoadedAddons(manifest.m_aDependencyIds);
			
			saveManager.Save(ESaveType.USER, saveCustomName, manifest);
			return;
		} 
		
		// Create a new save
		if (m_bSaveAs || !saveItem || !SCR_SaveWorkshopManager.CanOverrideSave(saveItem))
		{
			new SCR_CreateNewSaveDialog();
			return;
		}
		
		// Save current state
		saveName = saveManager.GetCustomName(saveName);
		
		saveManager.Save(m_eSaveType, saveName);
	}
};