//------------------------------------------------------------------------------------------------
class SCR_MapDebugUI : SCR_MapUIBaseComponent
{
	[Attribute("0", UIWidgets.EditBox, desc: "Adjust rotation for debug unit icons", params: "-180 180")]
	protected float m_fUnitIconRotation;
	
	protected bool m_bIsUnitVisible;				// units debug visible					
	protected ref array<MapItem> m_aMapItems = {};	// cached map items 
	
	protected SCR_MapDescriptorDefaults m_DefaultsCfg;
		
	//------------------------------------------------------------------------------------------------
	//! Show infantry units
	protected void ShowUnits()
	{
		m_bIsUnitVisible = !m_bIsUnitVisible;
		
		if (m_bIsUnitVisible)
		{			
			SetPropsVisible(true);
					
			int count = m_MapEntity.GetByType(m_aMapItems, EMapDescriptorType.MDT_UNIT);
			for (int i = 0; i < count; i++)	
			{
				MapItem item = m_aMapItems[i];
				if (!item)
					continue;
								
				IEntity ent = item.Entity();
				if (!ent)
				{
					item.Recycle();
					count--;
					continue;
				}
					
				SCR_CharacterControllerComponent contrComp = SCR_CharacterControllerComponent.Cast(ent.FindComponent(SCR_CharacterControllerComponent));
				if (contrComp && contrComp.IsDead())
				{
					item.SetVisible(false);
					continue;
				}
				
				item.SetInfoText(GetUnitName(ent));
				
				FactionAffiliationComponent factionComp = FactionAffiliationComponent.Cast(ent.FindComponent(FactionAffiliationComponent));
				if (factionComp && factionComp.GetAffiliatedFaction())
				{
					string factionKey = factionComp.GetAffiliatedFaction().GetFactionKey();
					if (factionKey == "USSR")
						item.SetFactionIndex(EFactionMapID.EAST);
					else if (factionKey == "US")
						item.SetFactionIndex(EFactionMapID.WEST);
					else if (factionKey == "FIA")
						item.SetFactionIndex(EFactionMapID.FIA);
					else
						item.SetFactionIndex(EFactionMapID.UNKNOWN);
				}						
			}
		}
		else 
			SetPropsVisible(false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets visibility of a descriptor type props across all layers 
	//! \param state determines visibility
	void SetPropsVisible(bool state)
	{
		for (int i = 0; i < 6; i++)	// TODO unhardcode layers
		{
			MapLayer layer = m_MapEntity.GetLayer(i);
			if (!layer)
				continue;
			
			MapDescriptorProps props;
			foreach (SCR_FactionColorDefaults factionDefaults : m_DefaultsCfg.m_aFactionColors)
			{
				props = layer.GetPropsFor(factionDefaults.m_iFaction, EMapDescriptorType.MDT_UNIT);
				if (props)
				{
					props.SetVisible(state);
				}
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get/update name
	//! \param ent is the subject
	string GetUnitName(IEntity ent)
	{		
		PlayerManager playerMgr = GetGame().GetPlayerManager();
		if (!playerMgr)
			return "";
		
		string name;
		
		int id = playerMgr.GetPlayerIdFromControlledEntity(ent);
		if (id != 0)
			name = playerMgr.GetPlayerName(id);
		else 
		{
			CharacterIdentityComponent charIdentity = CharacterIdentityComponent.Cast(ent.FindComponent(CharacterIdentityComponent));
			if (charIdentity)
				name = charIdentity.GetCharacterFullName();
			else 
				name = "No identity";
		}
		
		
		return name;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Context callback
	protected void PanToPlayer()
	{
		IEntity player = SCR_PlayerController.GetLocalControlledEntity();
		if (!player)
			return;
	
		float targetScreenX, targetScreenY;
		vector playerPos = player.GetOrigin();
		m_MapEntity.WorldToScreen( playerPos[0],  playerPos[2], targetScreenX, targetScreenY ); 
		m_MapEntity.PanSmooth( targetScreenX, targetScreenY );	// zoom to PPU = 1 and pan to player
	}
	
	//------------------------------------------------------------------------------------------------
	//! Zooms to current pixel per unit ratio == 1
	protected void ZoomToPPU1()
	{
		MapWidget mapWidget = MapWidget.Cast( m_RootWidget.FindAnyWidget(SCR_MapConstants.MAP_WIDGET_NAME) );
		vector size = mapWidget.GetSizeInUnits();	
		
		m_MapEntity.ZoomSmooth( size[0] / (size[0] * mapWidget.PixelPerUnit()) );
	}
			
	//------------------------------------------------------------------------------------------------
	//! Update unit map items
	protected void UpdateUnits()
	{
		foreach (MapItem item: m_aMapItems)
		{
			IEntity ent = item.Entity();
			if (ent)
			{
				vector angles = ent.GetAngles();
				item.SetAngle(angles[1] + m_fUnitIconRotation);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void Update()
	{		
		if (m_bIsUnitVisible)
			UpdateUnits();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMapOpen(MapConfiguration config)
	{		
		super.OnMapOpen(config);
		m_DefaultsCfg = config.DescriptorDefsConfig;
		
		if (m_bIsUnitVisible)
		{
			m_bIsUnitVisible = false;
			ShowUnits();
		}
		
		// Add ctx menu entry
		SCR_MapContextualMenuUI ctxMenu = SCR_MapContextualMenuUI.Cast(m_MapEntity.GetMapUIComponent(SCR_MapContextualMenuUI));
		if (ctxMenu)
		{
			
			bool isDebugOn = DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_UI_MAP_DEBUG_OPTIONS, false);
			if (isDebugOn)
			{
				ctxMenu.ContextRegisterDynamic("Map variables dbg", true).m_OnClick.Insert(m_MapEntity.ShowScriptDebug);
				ctxMenu.ContextRegisterDynamic("Zoom to 1px == 1m", true).m_OnClick.Insert(ZoomToPPU1);
				ctxMenu.ContextRegisterDynamic("Pan to player", true).m_OnClick.Insert(PanToPlayer);
				ctxMenu.ContextRegisterDynamic("Show units", true).m_OnClick.Insert(ShowUnits);
			}
		}
	}
};
