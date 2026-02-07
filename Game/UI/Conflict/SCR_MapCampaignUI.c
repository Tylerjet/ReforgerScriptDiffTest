//------------------------------------------------------------------------------
class SCR_MapCampaignUI : SCR_MapUIBaseComponent
{
	const int MULTIPLIER = 3;
	protected Widget m_ConflictUIRoot;
	protected Widget m_wMobileAssembly;
	protected Widget m_wConfigLayout;

	protected ResourceName m_sBaseElement = "{E48F14D6D3C54BE5}UI/layouts/Campaign/BaseElement.layout";
	protected ResourceName m_sBasesConfigLayout = "{BD375D3D799220E5}UI/layouts/Map/BasesMapSettings.layout";
	protected ResourceName m_sSpawnPointElement = "{E78DE3FD19654C1B}UI/layouts/Campaign/SpawnPointElement.layout";

	protected ref map<Widget, SCR_MapUIElement> m_mIcons = new map<Widget, SCR_MapUIElement>();
	static ref ScriptInvoker Event_OnIconsInit = new ScriptInvoker();
	
	protected bool m_bIsEditor; // Map opened in editor with gamemaster rights (!limited)

	[Attribute("0")] // temporary switch between using ui icons and map descriptors
	bool m_bUseNew;

	[Attribute("0")]
	bool m_bUseAggregation = true;

	protected int m_iIconSize = 32;
	protected string m_sSelectedFaction = "All";

	ref array<ref SCR_FactionColorDefaults> s_aFactionColors = {};
	
	[Attribute("{EAB5D9841F081D07}UI/layouts/Campaign/TaskElementNew.layout")]
	protected ResourceName m_sMapTaskIconResourceName;

	//------------------------------------------------------------------------------
	override void Update(float timeSlice)
	{
		UpdateIcons();
	}

	//------------------------------------------------------------------------------
	protected void UpdateIcons()
	{
		foreach (Widget widget, SCR_MapUIElement icon : m_mIcons)
		{
			if (!icon || !widget)
				continue;
			vector pos = icon.GetPos();
			float x, y;
			m_MapEntity.WorldToScreen(pos[0], pos[2], x, y, true);

			x = GetGame().GetWorkspace().DPIUnscale(x);
			y = GetGame().GetWorkspace().DPIUnscale(y);

			FrameSlot.SetPos(widget, x, y);
		}
	}
	
	//------------------------------------------------------------------------------
	protected void InitMobileAssembly(string factionKey, bool deployed)
	{
		if (m_wMobileAssembly)
		{
			m_wMobileAssembly.SetVisible(deployed);
			return;
		}

		FactionManager fm = GetGame().GetFactionManager();
		if (!fm)
			return;

		SCR_CampaignFaction faction = SCR_CampaignFaction.Cast(fm.GetFactionByKey(factionKey));
		if (!faction)
			return;
		
		SCR_CampaignFaction playerFaction = SCR_CampaignFaction.Cast(SCR_RespawnSystemComponent.GetLocalPlayerFaction());
		if (faction != playerFaction)
			return;

		IEntity box = faction.GetDeployedMobileAssembly(); // don't ask me why i named it 'box'
		if (!box)
			return;

		SCR_CampaignMobileAssemblyComponent assembly = SCR_CampaignMobileAssemblyComponent.Cast(box.FindComponent(SCR_CampaignMobileAssemblyComponent));
		if (!assembly)
			return;

		Widget w = GetGame().GetWorkspace().CreateWidgets(m_sBaseElement, m_ConflictUIRoot);
		if (!w)
			return;

		m_wMobileAssembly = w;

		SCR_CampaignMapUIBase handler = SCR_CampaignMapUIBase.Cast(w.FindHandler(SCR_CampaignMapUIBase));
		if (!handler)
			return;

		handler.SetParent(this);
		handler.InitMobile(assembly);
		m_mIcons.Set(w, handler);

		FrameSlot.SetSizeToContent(w, true);
		FrameSlot.SetAlignment(w, 0.5, 0.5);
	}

	//------------------------------------------------------------------------------
	protected void InitConfigLayout()
	{
		if (m_wConfigLayout)
			return;

		if (m_sBasesConfigLayout.IsEmpty())
			return;

		m_wConfigLayout = GetGame().GetWorkspace().CreateWidgets(m_sBasesConfigLayout, m_RootWidget);
		m_wConfigLayout.SetVisible(false);
		FrameSlot.SetSizeToContent(m_wConfigLayout, true);

		Widget checkboxLines = m_wConfigLayout.FindAnyWidget("CheckboxLines");
		if (checkboxLines)
		{
			SCR_CampaignBaseManager baseManager = SCR_CampaignBaseManager.GetInstance();
			SCR_CheckboxComponent checkboxHandler = SCR_CheckboxComponent.Cast(checkboxLines.FindHandler(SCR_CheckboxComponent));
			if (checkboxHandler)
			{
				checkboxHandler.SetChecked(baseManager.GetShowMapLinks());
				checkboxHandler.m_OnChanged.Insert(baseManager.UpdateShowMapLinks);
			}
		}

		Widget toggleNames = m_wConfigLayout.FindAnyWidget("NameToggle");
		if (toggleNames)
		{
			SCR_CheckboxComponent nameHandler = SCR_CheckboxComponent.Cast(toggleNames.FindHandler(SCR_CheckboxComponent));
			if (nameHandler)
				nameHandler.SetChecked(true);
		}

		Widget opacity = m_wConfigLayout.FindAnyWidget("OpacitySlider");
		if (opacity)
		{
			SCR_SliderComponent opacityHandler = SCR_SliderComponent.Cast(opacity.FindHandler(SCR_SliderComponent));
			if (opacityHandler)
				opacityHandler.SetValue(m_mIcons.GetElement(0).GetRoot().GetOpacity() * 100);
		}

		Widget iconSize = m_wConfigLayout.FindAnyWidget("IconSize");
		if (iconSize)
		{
			SCR_SliderComponent sizeHandler = SCR_SliderComponent.Cast(iconSize.FindHandler(SCR_SliderComponent));
			if (sizeHandler)
				sizeHandler.SetValue(m_mIcons.GetElement(0).GetIconSize()[0] - 16);
		}
	}

	//------------------------------------------------------------------------------
	// Loads the current state
	protected void UpdateConfigLayout()
	{
		SCR_CampaignBaseManager baseManager = SCR_CampaignBaseManager.GetInstance();
		
		Widget checkboxLines = m_wConfigLayout.FindAnyWidget("CheckboxLines");
		if (checkboxLines)
		{
			SCR_CheckboxComponent handler = SCR_CheckboxComponent.Cast(checkboxLines.FindHandler(SCR_CheckboxComponent));
			if (handler)
				handler.SetChecked(baseManager.GetShowMapLinks());
		}

		Widget toggleNames = m_wConfigLayout.FindAnyWidget("NameToggle");
		if (toggleNames)
		{
			SCR_CheckboxComponent toggle = SCR_CheckboxComponent.Cast(toggleNames.FindHandler(SCR_CheckboxComponent));
			if (toggle)
				toggle.m_OnChanged.Insert(SetNameVisible);
		}

		Widget opacitySlider = m_wConfigLayout.FindAnyWidget("OpacitySlider");
		if (opacitySlider)
		{
			SCR_SliderComponent slider = SCR_SliderComponent.Cast(opacitySlider.FindHandler(SCR_SliderComponent));
			if (slider)
				slider.m_OnChanged.Insert(SetIconOpacity);
		}

		Widget iconSizeSlider = m_wConfigLayout.FindAnyWidget("IconSize");
		if (iconSizeSlider)
		{
			SCR_SliderComponent iconSize = SCR_SliderComponent.Cast(iconSizeSlider.FindHandler(SCR_SliderComponent));
			if (iconSize)
				iconSize.m_OnChanged.Insert(SetIconSize);
		}

		Widget factionSelector = m_wConfigLayout.FindAnyWidget("FactionSelect");
		if (factionSelector)
		{
			SCR_ComboBoxComponent selector = SCR_ComboBoxComponent.Cast(factionSelector.FindHandler(SCR_ComboBoxComponent));
			if (selector)
				selector.m_OnChanged.Insert(SetSelectedFaction);
		}
	}

	//------------------------------------------------------------------------------
	protected void SetSelectedFaction(SCR_ComboBoxComponent combo, int id)
	{
		m_sSelectedFaction = combo.GetCurrentItem();
	}

	//------------------------------------------------------------------------------
	protected void SetNameVisible(SCR_CheckboxComponent c, bool visible)
	{
		foreach (Widget w, SCR_MapUIElement i : m_mIcons)
		{
			if (m_sSelectedFaction == "All")
				i.ShowName(visible);
			else
				if (SCR_CampaignMapUIBase.Cast(i).GetFactionKey() == m_sSelectedFaction)
					i.ShowName(visible);
		}
	}

	//------------------------------------------------------------------------------
	protected void SetIconSize(SCR_SliderComponent s, float size)
	{
		int minSize = 16;
		m_iIconSize = minSize + size;

		foreach (Widget w, SCR_MapUIElement i : m_mIcons)
		{
			i.SetIconSize(m_iIconSize);
		}

		Aggregate();
	}

	//------------------------------------------------------------------------------
	protected void SetIconOpacity(SCR_SliderComponent s, float alpha)
	{
		foreach (Widget w, SCR_MapUIElement i : m_mIcons)
		{
			if (m_sSelectedFaction == "All")
				i.SetOpacity(alpha / 100);
			else
				if (SCR_CampaignMapUIBase.Cast(i).GetFactionKey() == m_sSelectedFaction)
					i.SetOpacity(alpha / 100);
		}
	}

	//------------------------------------------------------------------------------
	protected void ToggleConfigLayout()
	{
		SCR_MapCursorModule cursorModule = SCR_MapCursorModule.Cast(SCR_MapEntity.GetMapInstance().GetMapModule(SCR_MapCursorModule));
		if (cursorModule && cursorModule.GetCursorState() & EMapCursorState.CS_CONTEXTUAL_MENU)
			return;

		if (!m_wConfigLayout)
			return;

		m_wConfigLayout.SetVisible(!m_wConfigLayout.IsVisible());
		
		UpdateConfigLayout();
	}

	//------------------------------------------------------------------------------
	protected void RegisterActionListeners()
	{
		GetGame().GetInputManager().AddActionListener("MapToggleShowSettings", EActionTrigger.DOWN, ToggleConfigLayout);
	}

	//------------------------------------------------------------------------------
	protected void InitBases()
	{
		SCR_CampaignBaseManager baseManager = SCR_CampaignBaseManager.GetInstance();
		array<SCR_CampaignBase> bases = baseManager.GetBases();
		SCR_CampaignFaction faction = SCR_CampaignFaction.Cast(SCR_RespawnSystemComponent.GetLocalPlayerFaction());
		
		if (!faction)
			return;

		int cnt = bases.Count();
		for (int i = 0; i < cnt; ++i)
		{
			SCR_CampaignBase base = bases[i];
			
			if (!base.GetIsEnabled())
				continue;
			
			if (base.GetIsHQ() && base.GetOwningFaction() != faction)
			{
				IEntity ent = base;
				MapDescriptorComponent mapDesc;
				IEntity sibling;
		
				// Hide all map descriptors
				while (ent)
				{
					mapDesc = MapDescriptorComponent.Cast(ent.FindComponent(MapDescriptorComponent));
					
					if (mapDesc && mapDesc.Item())
						mapDesc.Item().SetVisible(false);
					
					sibling = ent.GetSibling();
					
					if (sibling)
						ent = sibling;
					else
						ent = ent.GetChildren();
				}
				
				array<SCR_CampaignServiceComponent> services = {};
				base.GetAllBaseServices(services);
				
				foreach (SCR_CampaignServiceComponent service : services)
				{
					IEntity serviceEntity = service.GetOwner();
					
					if (!serviceEntity)
						continue;
					
					mapDesc = SCR_CampaignServiceMapDescriptorComponent.Cast(serviceEntity.FindComponent(SCR_CampaignServiceMapDescriptorComponent));
					
					if (!mapDesc)
						continue;
					
					mapDesc.Item().SetVisible(false);
				}
				
				continue;
			}

			Widget w = GetGame().GetWorkspace().CreateWidgets(m_sBaseElement, m_ConflictUIRoot);
			SCR_CampaignMapUIBase handler = SCR_CampaignMapUIBase.Cast(w.FindHandler(SCR_CampaignMapUIBase));
			
			if (!handler)
				return;

			handler.SetParent(this);
			handler.InitBase(base);
			m_mIcons.Set(w, handler);
			base.SetBaseUI(handler);

			FrameSlot.SetSizeToContent(w, true);
			FrameSlot.SetAlignment(w, 0.5, 0.5);
		}

		Event_OnIconsInit.Invoke();

		if (faction)
		{
			string factionKey = faction.GetFactionKey();
			InitMobileAssembly(factionKey, faction.GetDeployedMobileAssembly() != null);
			InitTasks(faction);
		}

		SCR_GameModeCampaignMP gameMode = SCR_GameModeCampaignMP.GetInstance();
		if (gameMode)
			gameMode.s_OnMobileAssemblyDeployChanged.Insert(InitMobileAssembly);
		
		UpdateIcons();	// Initialization
	}
	
	//------------------------------------------------------------------------------------------------
	protected void InitTasksMarkers()
	{
		SCR_BaseTaskManager taskManager = GetTaskManager();
		if (!taskManager)
			return;
		
		int taskCount;
		array<SCR_BaseTask> availableTasks = {};
		if (m_bIsEditor)
		{
			taskCount = taskManager.GetTasks(availableTasks);
		}
		else
		{
			taskCount = taskManager.GetFilteredTasks(availableTasks, SCR_RespawnSystemComponent.GetLocalPlayerFaction());
		}
		
		SCR_MapUITask handler;
		SCR_Faction f;
		ButtonWidget taskIconButton;
		Widget overlayWidget;
		ButtonWidget taskAssignButton;
		Widget widget;
		Widget w;
		ImageWidget image;
		SCR_TaskSelectButton iconButtonHandler;
		SCR_TaskOverlayButton overlayButtonHandler;
		SCR_TaskAssignButton assignButtonHandler;
		
		for (int i = 0; i < taskCount; ++i)
		{
			w = GetGame().GetWorkspace().CreateWidgets(m_sMapTaskIconResourceName, m_ConflictUIRoot);
			if (!w)
				break;
			
			SCR_Faction targetFaction = SCR_Faction.Cast(availableTasks[i].GetTargetFaction());
			
			image = ImageWidget.Cast(w.FindAnyWidget("TaskIconBackground"));
			if (image)
				availableTasks[i].SetTaskIconWidget(image);
			
			image = ImageWidget.Cast(w.FindAnyWidget("TaskIconSymbol"));
			if (image)
			{
				if (targetFaction)
					image.SetColor(targetFaction.GetOutlineFactionColor());

				availableTasks[i].SetWidgetIcon(image);	
			}
			
			widget = Widget.Cast(w.FindAnyWidget("TaskIconOutline"));
			if (widget)
			{
				if (targetFaction)
					widget.SetColor(targetFaction.GetOutlineFactionColor());
			}
			
			widget = Widget.Cast(w.FindAnyWidget("TaskIconHover"));
			if (widget)
			{
				widget.SetEnabled(false);
				widget.SetOpacity(0);
			}
			
			widget = Widget.Cast(w.FindAnyWidget("Border"));
			if (widget)
			{
				widget.SetEnabled(false);
				widget.SetOpacity(0);
			}
			
			widget = Widget.Cast(w.FindAnyWidget("TaskTitleButton"));
			if (widget)
			{
				widget.SetVisible(false);
			}
			
			widget = Widget.Cast(w.FindAnyWidget("Assignee"));
			if (widget)
			{
				widget.SetEnabled(false);
				widget.SetOpacity(0);
			}
							
			handler = SCR_MapUITask.Cast(w.FindHandler(SCR_MapUITask));
			if (!handler)
				continue;

			handler.SetParent(this);
			handler.InitTask(availableTasks[i]);
			m_mIcons.Set(w, handler);
			
			taskIconButton = ButtonWidget.Cast(w.FindAnyWidget("TaskIconButton"));
			if (taskIconButton)
			{
				iconButtonHandler = SCR_TaskSelectButton.Cast(taskIconButton.FindHandler(SCR_TaskSelectButton));
				if (iconButtonHandler)
					iconButtonHandler.SetRootWidgetHandler(handler);
			}
						
			overlayWidget = w.FindAnyWidget("OverlayWidget");
			if (overlayWidget)
			{
				overlayButtonHandler = SCR_TaskOverlayButton.Cast(overlayWidget.FindHandler(SCR_TaskOverlayButton));
				if (overlayButtonHandler)
					overlayButtonHandler.SetRootWidgetHandler(handler);
			}
			
			taskAssignButton = ButtonWidget.Cast(w.FindAnyWidget("TaskTitleButton"));
			if (taskAssignButton)
			{
				assignButtonHandler = SCR_TaskAssignButton.Cast(taskAssignButton.FindHandler(SCR_TaskAssignButton));
				if (assignButtonHandler)
					assignButtonHandler.SetRootWidgetHandler(handler);
			}
	
			FrameSlot.SetSizeToContent(w, true);
			FrameSlot.SetAlignment(w, 0.5, 0.5);
		}
		
			UpdateIcons();	// Initialization
	}

	//------------------------------------------------------------------------------------------------
	protected void ShowAllSpawnPoints()
	{
		array<SCR_SpawnPoint> spawnPoints = {};
		Faction playerFaction = SCR_RespawnSystemComponent.GetLocalPlayerFaction();
		if (m_bIsEditor)
		{
			spawnPoints = SCR_SpawnPoint.GetSpawnPoints();
		}
		else if (playerFaction)
		{
			spawnPoints = SCR_SpawnPoint.GetSpawnPointsForFaction(playerFaction.GetFactionKey());
		}
		
		foreach (SCR_SpawnPoint sp : spawnPoints)
		{
			ShowSpawnPoint(sp);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void ShowSpawnPoint(SCR_SpawnPoint sp)
	{
		if (!sp)
			return;
		
		// todo: hotfix for icon duplicates in conflict, figure out a proper solution :wink:
		if (SCR_GameModeCampaignMP.GetInstance() && sp.Type() == SCR_SpawnPoint || sp.Type() == SCR_CampaignSpawnPointGroup)
			return;

		if (!SCR_SelectSpawnPointSubMenu.GetInstance() && sp.GetVisibleInDeployMapOnly())
			return;

		Widget w = GetGame().GetWorkspace().CreateWidgets(m_sSpawnPointElement, m_ConflictUIRoot);
		SCR_MapUISpawnPoint handler = SCR_MapUISpawnPoint.Cast(w.FindHandler(SCR_MapUISpawnPoint));
		if (!handler)
			return;

		handler.Init(sp);
		handler.SetParent(this);
		m_mIcons.Set(w, handler);

		FrameSlot.SetSizeToContent(w, true);
		FrameSlot.SetAlignment(w, 0.5, 0.5);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void HideSpawnPoint(SCR_SpawnPoint sp)
	{
		SCR_MapUISpawnPoint spHandler;
		foreach (Widget w, SCR_MapUIElement handler: m_mIcons)
		{
			spHandler = SCR_MapUISpawnPoint.Cast(handler);
			if (spHandler && spHandler.GetSpawnPoint() == sp)
			{
				w.RemoveFromHierarchy();
				m_mIcons.Remove(w);
				return;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreatePlayerSpawnPositionHint()
	{		
		SCR_GameModeCampaignMP gamemode = SCR_GameModeCampaignMP.GetInstance();
		
		if ( !gamemode )
			return;
		
		if (!gamemode.CanShowPlayerSpawn())
			return;
		
		gamemode.SetMapCampaignUI(this);
		
		vector playerLocation = gamemode.GetPlayerSpawnPos();
		
		Widget indicator = GetGame().GetWorkspace().CreateWidgets("{FD71287E68006A26}UI/layouts/Campaign/CampaignPlayerMapIndicator.layout", m_ConflictUIRoot);
		
		SCR_CampaignMapUIPlayerHighlight highlightHandler = SCR_CampaignMapUIPlayerHighlight.Cast(indicator.FindHandler(SCR_CampaignMapUIPlayerHighlight));
		
		if (!highlightHandler)
			return;
		
		m_mIcons.Insert(indicator, highlightHandler);
		
		highlightHandler.SetParent(this);
		highlightHandler.SetPos(playerLocation);
		
		FrameSlot.SetPosX(indicator, playerLocation[0]);
		FrameSlot.SetPosY(indicator, playerLocation[1]);
		FrameSlot.SetSizeToContent(indicator, true);
		FrameSlot.SetAlignment(indicator, 0.5, 0.5);
		
		//Called only after spawn, to center map and zoom to player position.
		if (!gamemode.WasMapOpened())
		{			
			m_MapEntity.ZoomPanSmooth(1.5,playerLocation[0],playerLocation[2]);
			gamemode.SetMapOpened(true);
		}
		
		//Set image from ImageSet 
		ResourceName imageSet = "{5E8F77F38C2B9B9F}UI/Imagesets/MilitarySymbol/ICO.imageset";
		ImageWidget image = ImageWidget.Cast(indicator.FindAnyWidget("Image"));
		if(image)
		{
			image.LoadImageFromSet(0, imageSet, "PlayerSpawnHint");
			image.SetColor(Color.Orange);
		}
		
		//Sets time to spawn icon
		RichTextWidget timeWidget = RichTextWidget.Cast(indicator.FindAnyWidget("TimeString"));
		TimeContainer timeContainer = gamemode.GetSpawnTime();
		if (timeContainer)
		{
			string hours = timeContainer.m_iHours.ToString();
    		string minutes = timeContainer.m_iMinutes.ToString();
		
			if (timeContainer.m_iHours < 10)
				hours = "0" + hours;
		
			if (timeContainer.m_iMinutes < 10)
				minutes = "0" + minutes;
		
			timeWidget.SetTextFormat("%1:%2",hours, minutes);
		}
		else
			timeWidget.SetText("N/A");
		
		HideSpawnPositionHint(m_MapEntity.GetLayerIndex());
		m_MapEntity.GetOnLayerChanged().Insert(HideSpawnPositionHint);
	}
	
	//------------------------------------------------------------------------------------------------
	//!Called to hide Spawn Position hint if player is killed (or his entity disappear for some reason)
	void RemoveSpawnPositionHint()
	{
		SCR_CampaignMapUIPlayerHighlight highlightHandler;
		foreach (Widget w, SCR_MapUIElement handler: m_mIcons)
		{
			highlightHandler = SCR_CampaignMapUIPlayerHighlight.Cast(handler);
			if(highlightHandler)
			{
				w.RemoveFromHierarchy();
				m_mIcons.Remove(w);
				break;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void HideSpawnPositionHint(float layer)
	{
		SCR_CampaignMapUIPlayerHighlight highlightHandler;
		foreach (Widget w, SCR_MapUIElement handler: m_mIcons)
		{
			highlightHandler = SCR_CampaignMapUIPlayerHighlight.Cast(handler);
			if(highlightHandler)
			{	
				if(layer > 1)
				{
					w.SetVisible(false);
					return;
				}
				w.SetVisible(true);
			}
		}
	}
	//------------------------------------------------------------------------------------------------
	protected void OnSpawnPointFactionAssigned(SCR_SpawnPoint sp)
	{
		Faction playerFaction = SCR_RespawnSystemComponent.GetLocalPlayerFaction();
		if (m_bIsEditor || playerFaction.GetFactionKey() == sp.GetFactionKey())
		{
			ShowSpawnPoint(sp);
		}
		else
		{
			HideSpawnPoint(sp);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void InitTasks(Faction faction)
	{
		SCR_BaseTaskManager tm = GetTaskManager();
		if (!tm)
			return;

		array<SCR_BaseTask> tasks = {};
		tm.GetFilteredTasks(tasks, faction);

		foreach (SCR_BaseTask t : tasks)
		{
			t.CreateMapUIIcon();
		}
	}

	//------------------------------------------------------------------------------------------------
	void RemoveIcon(SCR_MapUIElement icon)
	{
		Widget w = m_mIcons.GetKeyByValue(icon);
		delete w;
	}

	//------------------------------------------------------------------------------------------------
	override void OnMapOpen(MapConfiguration config)
	{
		m_ConflictUIRoot = m_RootWidget.FindAnyWidget("ConflictFrame");
		m_ConflictUIRoot.SetVisible(true);

		if (s_aFactionColors.IsEmpty())
			s_aFactionColors = config.DescriptorDefsConfig.m_aFactionColors;

		if (SCR_GameModeCampaignMP.GetInstance())
			InitBases();
		
		m_bIsEditor = SCR_EditorManagerEntity.IsOpenedInstance(false);
		
		InitTasksMarkers();
		ShowAllSpawnPoints();
		CreatePlayerSpawnPositionHint();
		
		SCR_SpawnPoint.Event_SpawnPointFactionAssigned.Insert(OnSpawnPointFactionAssigned);
		SCR_MapEntity.GetOnMapPan().Insert(UpdateIcons);
		SCR_MapEntity.GetOnMapZoom().Insert(UpdateIcons);
		if (m_bUseAggregation)
		{
			SCR_MapEntity.GetOnMapZoom().Insert(Aggregate);
			GetGame().GetCallqueue().CallLater(Aggregate, 100, false);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnMapClose(MapConfiguration config)
	{
		SCR_GameModeCampaignMP gameMode = SCR_GameModeCampaignMP.GetInstance();
		if (gameMode)
			gameMode.s_OnMobileAssemblyDeployChanged.Remove(InitMobileAssembly);

		SCR_SpawnPoint.Event_SpawnPointFactionAssigned.Remove(OnSpawnPointFactionAssigned);
		SCR_MapEntity.GetOnMapPan().Remove(UpdateIcons);
		SCR_MapEntity.GetOnMapZoom().Remove(UpdateIcons);
		if (m_bUseAggregation)
			SCR_MapEntity.GetOnMapZoom().Remove(Aggregate);

		if (m_wConfigLayout)
			m_wConfigLayout.RemoveFromHierarchy();

		GetGame().GetInputManager().RemoveActionListener("MapToggleShowSettings", EActionTrigger.DOWN, ToggleConfigLayout);

		delete m_wMobileAssembly;
		foreach (Widget w, SCR_MapUIElement i : m_mIcons)
		{
			delete w;
		}

		m_mIcons.Clear();
		m_MapEntity.GetOnLayerChanged().Remove(HideSpawnPositionHint);
	}

	//------------------------------------------------------------------------------------------------
	// todo(koudelkaluk): precalculate zoom level
	protected void Aggregate()
	{
		/*array<vector> positions = {};
		set<SCR_MapUIElement> hidden = new set<SCR_MapUIElement>();

		foreach (Widget w, SCR_MapUIElement e : m_mIcons)
		{
			if (e.Type() == SCR_CampaignMapUIPlayerHighlight)
				continue;
			
			e.SetVisible(true);
			
			
			
			for (int i = 0; i < positions.Count(); ++i)
			{
				float dist = vector.DistanceSq(FrameSlot.GetPos(w), positions[i]);
				if (dist < m_iIconSize * m_iIconSize * MULTIPLIER)
					hidden.Insert(e);
			}

			positions.Insert(FrameSlot.GetPos(w));
		}

		foreach (SCR_MapUIElement b : hidden)
		{
			b.SetVisible(false);
		}*/
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_MapCampaignUI()
	{
		delete m_wMobileAssembly;
		foreach (Widget w, SCR_MapUIElement i : m_mIcons)
		{
			delete w;
		}
	}
};