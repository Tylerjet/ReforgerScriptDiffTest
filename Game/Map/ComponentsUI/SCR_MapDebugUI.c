//------------------------------------------------------------------------------------------------
class SCR_MapDebugUI : SCR_MapUIBaseComponent
{
	[Attribute("0", UIWidgets.EditBox, desc: "Adjust rotation for debug unit icons", params: "-180 180")]
	protected float m_fUnitIconRotation;
	
	[Attribute("{BCD5479864D1A79A}UI/Textures/Map/topographicIcons/icons_topographic_map.imageset", UIWidgets.ResourceNamePicker, "Imageset source for display icon" )]
	protected ResourceName m_sIconImageset;
	
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
				
				string name;
				array<string> nameParams = {};
				GetUnitName(ent, name, nameParams);
				item.SetInfoText(name, nameParams);
				
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
	//! \param[out] name Name or formatting of name
	//! \param[out] nameParams If uses formating: Firstname, Alias and Surname (Alias can be an empty string)
	void GetUnitName(IEntity ent, out string name, out notnull array<string> nameParams)
	{				
		PlayerManager playerMgr = GetGame().GetPlayerManager();
		if (!playerMgr)
			return;

		int id = playerMgr.GetPlayerIdFromControlledEntity(ent);
		if (id != 0)
			name = playerMgr.GetPlayerName(id);
		else 
		{
			SCR_CharacterIdentityComponent scrCharIdentity = SCR_CharacterIdentityComponent.Cast(ent.FindComponent(SCR_CharacterIdentityComponent));
			if (scrCharIdentity)
			{
				scrCharIdentity.GetFormattedFullName(name, nameParams);
			}
			else
			{
				CharacterIdentityComponent charIdentity = CharacterIdentityComponent.Cast(ent.FindComponent(CharacterIdentityComponent));
				if (charIdentity)
					name = charIdentity.GetIdentity().GetName();
				else 
					name = "No identity";
			}
				
		}
		
		return;
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
		m_MapEntity.ZoomSmooth(1);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Zooms to the beginning of higher layer
	protected void ZoomLayerUp()
	{
		int index = m_MapEntity.GetLayerIndex();
		MapLayer layer = m_MapEntity.GetLayer(index);
		if (layer)
		{
			if (layer.GetCeiling() <= m_MapEntity.GetMinZoom())	// max layer
				return;
						
			m_MapEntity.ZoomSmooth(layer.GetCeiling() - 0.1);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Zooms to the ceiling of lower layer
	protected void ZoomLayerDown()
	{		
		int index = m_MapEntity.GetLayerIndex();
		if (index == 0)		// min layer
			return;
		
		MapLayer layer = m_MapEntity.GetLayer(index-1);
		if (layer)
			m_MapEntity.ZoomSmooth(layer.GetCeiling());
	}
	
	//------------------------------------------------------------------------------------------------
	//! Toggle map light
	protected void ToggleLight()
	{
		SCR_MapLightUI lightUI = SCR_MapLightUI.Cast(SCR_MapEntity.GetMapInstance().GetMapUIComponent(SCR_MapLightUI));
		if (lightUI)
			lightUI.ToggleActive();
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
	protected void OnRadialMenuOpen()
	{
		SCR_MapContextualMenuUI ctxMenu = SCR_MapContextualMenuUI.Cast(m_MapEntity.GetMapUIComponent(SCR_MapContextualMenuUI));
		
		bool isDebugOn = DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_UI_MAP_DEBUG_OPTIONS, false);
		if (isDebugOn)
		{
			SCR_MapMenuCategory category = ctxMenu.AddRadialCategory("Debug Options");
			category.SetIcon(m_sIconImageset, "view-point");
			
			SCR_MapMenuEntry entry = ctxMenu.AddRadialEntry("Map variables dbg", "Debug Options");
			entry.m_OnClick.Insert(m_MapEntity.ShowScriptDebug);
			entry.SetIcon(m_sIconImageset, "culvet");
			
			entry = ctxMenu.AddRadialEntry("Zoom to 1px == 1m", "Debug Options");
			entry.m_OnClick.Insert(ZoomToPPU1);
			entry.SetIcon(m_sIconImageset, "single-tree");
			
			entry = ctxMenu.AddRadialEntry("Zoom up a layer", "Debug Options"); 
			entry.m_OnClick.Insert(ZoomLayerUp);
			entry.SetIcon(m_sIconImageset, "ancient-fortification");
			
			entry = ctxMenu.AddRadialEntry("Zoom down a layer", "Debug Options");
			entry.m_OnClick.Insert(ZoomLayerDown);
			entry.SetIcon(m_sIconImageset, "hospital");
			
			entry = ctxMenu.AddRadialEntry("Pan to player", "Debug Options");
			entry.m_OnClick.Insert(PanToPlayer);
			entry.SetIcon(m_sIconImageset, "hill-marker");
			
			entry = ctxMenu.AddRadialEntry("Show units", "Debug Options");
			entry.m_OnClick.Insert(ShowUnits);
			entry.SetIcon(m_sIconImageset, "crane");
			
			entry = ctxMenu.AddRadialEntry("Toggle Light", "Debug Options");
			entry.m_OnClick.Insert(ToggleLight);
			entry.SetIcon(m_sIconImageset, "smoke-stack");
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void Update(float timeSlice)
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
		
		SCR_MapContextualMenuUI ctxMenu = SCR_MapContextualMenuUI.Cast(m_MapEntity.GetMapUIComponent(SCR_MapContextualMenuUI));
		if (ctxMenu)
			ctxMenu.GetOnMenuOpenInvoker().Insert(OnRadialMenuOpen);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMapClose(MapConfiguration config)
	{
		SCR_MapContextualMenuUI ctxMenu = SCR_MapContextualMenuUI.Cast(m_MapEntity.GetMapUIComponent(SCR_MapContextualMenuUI));
		if (ctxMenu)
			ctxMenu.GetOnMenuOpenInvoker().Remove(OnRadialMenuOpen);
	}
};
