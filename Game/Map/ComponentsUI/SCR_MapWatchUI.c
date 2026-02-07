//------------------------------------------------------------------------------------------------
class SCR_MapWatchUI : SCR_MapRTWBaseUI
{
	protected SCR_WristwatchComponent m_WristwatchComp;
			
	//------------------------------------------------------------------------------------------------
	override void SetWidgetNames()
	{
		WIDGET_NAME = "WatchFrame";
		RT_WIDGET_NAME = "WatchRTW";
		WORLD_NAME = "WatchUIWorld";
	}
	
	//------------------------------------------------------------------------------------------------
	override void InitPositionVectors()
	{
		m_WristwatchComp = SCR_WristwatchComponent.Cast( m_RTEntity.FindComponent(SCR_WristwatchComponent) );
		
		m_vPrefabPos = "0 0 0";
		m_vCameraPos = "0 0.14 0";
		m_vCameraAngle = "0 -90 0";
	}
	
	//------------------------------------------------------------------------------------------------
	//! Update for the wristwatch entity within preview world
	protected void UpdatePreviewEntity()
	{								
		if (m_WristwatchComp)
		{						
			// activate, apply proc anims
			if (m_WristwatchComp.GetMode() != EGadgetMode.IN_HAND)
				m_WristwatchComp.SetMapMode();
			
			// tick the frame
			m_WristwatchComp.UpdateTime();
		}
		
		// update		
		BaseWorld previewWorld = m_RTWorld.GetRef();
		previewWorld.UpdateEntities();
	}
	
	//------------------------------------------------------------------------------------------------
	override string GetPrefabResource()
	{
		ResourceName prefabName = string.Empty;
		
		SCR_GadgetManagerComponent gadgetManager = SCR_GadgetManagerComponent.GetGadgetManager(SCR_PlayerController.GetLocalControlledEntity());
		if (gadgetManager)
		{
			IEntity watch = gadgetManager.GetGadgetByType(EGadgetType.WRISTWATCH);
			
			SCR_WristwatchComponent watchComp = SCR_WristwatchComponent.Cast( watch.FindComponent(SCR_GadgetComponent) );
			if (watchComp)
				prefabName = watchComp.GetWatchPrefab();
		}
		
		return prefabName;
	}
	

	//------------------------------------------------------------------------------------------------
	//! Set compass visibility
	//! \param visible is true/false switch
	override void SetVisible(bool visible)
	{
		if (visible)
		{
			SCR_GadgetManagerComponent gadgetManager = SCR_GadgetManagerComponent.GetGadgetManager(SCR_PlayerController.GetLocalControlledEntity());
			if (!gadgetManager)
				return;
			
			// No wristwatch equipped
			if (!gadgetManager.GetGadgetByType(EGadgetType.WRISTWATCH))
				return;
			
			super.SetVisible(visible);
					
			// Start anim
			ScriptCallQueue queue = GetGame().GetCallqueue();
			if (queue)
				queue.CallLater(UpdatePreviewEntity, 0, true);
		}
		else 
		{	
			ScriptCallQueue queue = GetGame().GetCallqueue();
			if (queue)
				queue.Remove(UpdatePreviewEntity);
			
			super.SetVisible(visible);
		}
	}
			
	//------------------------------------------------------------------------------------------------
	override void OnMapOpen(MapConfiguration config)
	{
		super.OnMapOpen(config);
		
		if (m_bWantedVisible)
		{
			SetVisible(true);
			
			if (m_ToolMenuEntry) 
				m_ToolMenuEntry.SetColor(UIColors.CONTRAST_COLOR);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMapClose(MapConfiguration config)
	{
		m_bWantedVisible = m_bIsVisible;
		SetVisible(false);
		
		super.OnMapClose(config);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Init()
	{
		SCR_MapToolMenuModule toolMenu = SCR_MapToolMenuModule.Cast(m_MapEntity.GetMapModule(SCR_MapToolMenuModule));
		if (toolMenu)
		{
			m_ToolMenuEntry = toolMenu.RegisterToolMenuEntry(SCR_MapToolMenuModule.ICONS_IMAGESET, "watch", m_bIsVisible, 2); // add to menu
			m_ToolMenuEntry.m_OnClick.Insert(ToggleVisible);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void Update()
	{		
		// Drag compass
		if (m_bIsVisible && m_bIsDragged)
		{
			// save position for map reopen
			WorkspaceWidget workspace = g_Game.GetWorkspace();
			m_wFrame.GetScreenPos(m_fPosX, m_fPosY);
			m_fPosX = workspace.DPIUnscale(m_fPosX);
			m_fPosY = workspace.DPIUnscale(m_fPosY);
		}
	}
};
