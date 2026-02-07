//------------------------------------------------------------------------------------------------
class SCR_SelectSpawnPointSubMenu : SCR_RespawnSubMenuBase
{
	protected SCR_MapEntity m_MapEntity;
	protected ref MapConfiguration m_MapConfigSpawn = new MapConfiguration();

	protected SCR_SpinBoxComponent m_SelectionSpinBox;
	protected Widget m_wSelectionSpinBox;
	protected TextWidget m_wWaitMessage;
	protected TextWidget m_wSpawnPointName;
	protected Widget m_wDeployAvailable;
	protected TextWidget m_wDeployTimer;
	protected Widget m_wNoSpawnPoints;

	protected SCR_NavigationButtonComponent m_PrevSpawnPoint;
	protected SCR_NavigationButtonComponent m_NextSpawnPoint;

	protected bool m_bSmoothPan;
	protected bool m_bUsingSpawnPointGroups;
	protected float m_fZoom;

	protected RplId m_SelectedSpawnPointId = RplId.Invalid();
	protected RplId m_DefaultSpawnPointId = RplId.Invalid();
	protected ref map<RplId, string> m_mSpawnPoints = new map<RplId, string>();

	protected static SCR_SelectSpawnPointSubMenu s_Instance;
	static ref ScriptInvoker Event_OnSpawnPointChanged = new ScriptInvoker();

	[Attribute("SelectionSpinBox")]
	protected string m_sSelectionSpinBox;

	[Attribute("MapMenu")]
	protected string m_sMapMenu;

	[Attribute("Wait")]
	protected string m_sWaitMessage;

	[Attribute("#AR-Vehicle_MobileAssembly_Name")]
	protected LocalizedString m_sMobileAssembly;

	[Attribute("#AR-DeployMenu_SpawnPoint_Name")]
	protected LocalizedString m_sDefaultRespawnPointName;

	//------------------------------------------------------------------------------------------------
	override void GetWidgets()
	{
		super.GetWidgets();
		m_wSelectionSpinBox = GetRootWidget().FindAnyWidget(m_sSelectionSpinBox);
		if (m_wSelectionSpinBox)
		{
			m_SelectionSpinBox = SCR_SpinBoxComponent.Cast(m_wSelectionSpinBox.FindHandler(SCR_SpinBoxComponent));
			m_SelectionSpinBox.GetOnLeftArrowClick().Insert(FocusOnSpawnPointOnly);
			m_SelectionSpinBox.GetOnRightArrowClick().Insert(FocusOnSpawnPointOnly);
			m_SelectionSpinBox.m_OnChanged.Insert(UpdateTimedSpawnPoint);
		}

		Widget mapRootWidget = GetRootWidget().FindAnyWidget(m_sMapMenu);
		if (mapRootWidget)
			m_MapEntity = SCR_MapEntity.GetMapInstance();

		if (!m_MapEntity)
		{
			Debug.Error("MapEntity is missing in the scene, spawn point selection menu won't work properly!");
			Print("MapEntity is needed in the scene for spawn point selection menu to work.", LogLevel.ERROR);
		}

		m_wWaitMessage = TextWidget.Cast(GetRootWidget().FindAnyWidget(m_sWaitMessage));
		if (m_wWaitMessage)
			m_wWaitMessage.SetText(GetRespawnMenuHandler().GetRespawnUnavailableMessage());

		m_wSpawnPointName = TextWidget.Cast(GetRootWidget().FindAnyWidget("SpawnPointName"));
		m_wDeployAvailable = GetRootWidget().FindAnyWidget("DeployCountdown");
		if (m_wDeployAvailable)
		{
			m_wDeployTimer = TextWidget.Cast(m_wDeployAvailable.FindAnyWidget("Time"));
			ButtonWidget back = ButtonWidget.Cast(m_wDeployAvailable.FindAnyWidget("BackToMapBtn"));
			if (back)
			{
				SCR_NavigationButtonComponent nav = SCR_NavigationButtonComponent.Cast(back.FindHandler(SCR_NavigationButtonComponent));
				nav.m_OnActivated.Insert(HideDeployCountdown);
			}
		}

		m_wNoSpawnPoints = GetRootWidget().FindAnyWidget("Warning");

		if (!m_MapEntity || !m_SelectionSpinBox || !m_wWaitMessage)
			Print("The layout is not configured properly and might limit the functionality!", LogLevel.ERROR);
	}

	//------------------------------------------------------------------------------------------------
	protected void InitMap()
	{
		if (m_MapEntity)
		{
			m_MapConfigSpawn = m_MapEntity.SetupMapConfig(EMapEntityMode.SPAWNSCREEN, "{418989FA279F1257}Configs/Map/MapSpawnMenu.conf", GetRootWidget());
			m_MapEntity.OpenMap(m_MapConfigSpawn);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuUpdate(SCR_SuperMenuBase parentMenu, float tDelta)
	{
		super.OnMenuUpdate(parentMenu, tDelta);

		if (m_MapEntity && m_MapEntity.IsOpen() && m_InputManager)
		{
				m_InputManager.ActivateContext("MapContext");
				m_InputManager.ActivateContext("DeployMenuMapContext");
			}
		}

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuOpen(parentMenu);

		m_sConfirmButtonText = m_sButtonTextSelectDeploy;
		m_bIsLastAvailableTab = true; // TODO: hardcoded as for now this tab will always be the last

		m_PrevSpawnPoint = CreateNavigationButton("SpawnPointPrev", "#AR-DeployMenu_PreviousSpawnPoint");
		m_PrevSpawnPoint.GetRootWidget().SetZOrder(-1);
		AlignableSlot.SetPadding(m_PrevSpawnPoint.GetRootWidget(), 10, 0, 0, 0);
		m_NextSpawnPoint = CreateNavigationButton("SpawnPointNext", "#AR-DeployMenu_NextSpawnPoint");
		m_NextSpawnPoint.GetRootWidget().SetZOrder(-1);
		AlignableSlot.SetPadding(m_NextSpawnPoint.GetRootWidget(), 10, 0, 0, 0);

		if (m_PrevSpawnPoint)
			m_PrevSpawnPoint.m_OnActivated.Insert(SelectPrevSpawnPoint);
		if (m_NextSpawnPoint)
			m_NextSpawnPoint.m_OnActivated.Insert(SelectNextSpawnPoint);
		CreateConfirmButton();
		m_ConfirmButton.SetAction("MenuDeploy");
		if (m_ConfirmButton)
			m_ConfirmButton.m_OnToggled.Insert(UntoggleQuick);
		if (m_QuickDeployButton)
			m_QuickDeployButton.m_OnToggled.Insert(UntoggleConfirm);	
	}
	
	//------------------------------------------------------------------------------------------------
	void UntoggleQuick()
	{
		if (m_QuickDeployButton)
			m_QuickDeployButton.SetToggled(false, true, false);
	}
	
	//------------------------------------------------------------------------------------------------
	void UntoggleConfirm()
	{
		if (m_ConfirmButton)
			m_ConfirmButton.SetToggled(false, true, false);
	}

	void OnMapOpen()
	{
		UpdateAndShowSelection();
		if (!SelectLastSelectedSpawnPoint())
			SelectSpawnPoint();
		FocusOnSelectedSpawnPoint();
		UpdateTimedSpawnPoint();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuShow(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuShow(parentMenu);

		// set the map to fullscreen
		Widget parentRoot = m_ParentMenu.GetRootWidget();
		Widget blur = parentRoot.FindAnyWidget("Blur0");
		if (blur)
			blur.SetVisible(false);

		parentRoot.AddChild(m_wRoot);
		FrameSlot.SetAnchorMin(m_wRoot, 0, 0);
		FrameSlot.SetAnchorMax(m_wRoot, 1, 1);
		FrameSlot.SetOffsets(m_wRoot, 0, 0, 0, 0);
		m_wRoot.SetZOrder(-1);

		if (m_MapEntity && !m_MapEntity.IsOpen())
		{
			InitMap();
			m_MapEntity.OpenMap(m_MapConfigSpawn);
			// m_MapEntity.SetZoom(m_fZoom);
		}

		PlayerManager pm = GetGame().GetPlayerManager();
		SCR_RespawnComponent rc = SCR_RespawnComponent.Cast(pm.GetPlayerRespawnComponent(m_iPlayerId));
		rc.GetSpawnPointLockInvoker().Insert(LockConfirmButton);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuHide(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuHide(parentMenu);

		Widget parentRoot = m_ParentMenu.GetRootWidget();
		Widget blur = parentRoot.FindAnyWidget("Blur0");
		if (blur)
			blur.SetVisible(true);

		if (m_MapEntity)
			m_MapEntity.CloseMap();
	}

	//------------------------------------------------------------------------------------------------
	protected void SelectPrevSpawnPoint()
	{
		int i = m_SelectionSpinBox.GetCurrentIndex()-1;
		if (i < 0)
			i = m_SelectionSpinBox.GetNumItems()-1;
		m_SelectionSpinBox.SetCurrentItem(i);
		SelectSpawnPoint();
		UntoggleConfirm();
	}

	//------------------------------------------------------------------------------------------------
	protected void SelectNextSpawnPoint()
	{
		int i = m_SelectionSpinBox.GetCurrentIndex()+1;
		if (i > m_SelectionSpinBox.GetNumItems()-1)
			i = 0;
		m_SelectionSpinBox.SetCurrentItem(i);
		SelectSpawnPoint();
		UntoggleConfirm();
	}

	//------------------------------------------------------------------------------------------------
	protected void SetButtonsEnabled(bool enabled)
	{
		if (m_PrevSpawnPoint)
			m_PrevSpawnPoint.SetEnabled(enabled, false);
		if (m_NextSpawnPoint)
			m_NextSpawnPoint.SetEnabled(enabled, false);
	}

	//------------------------------------------------------------------------------------------------
	void SelectSpawnPoint()
	{
		string name = m_SelectionSpinBox.GetCurrentItem();
		m_SelectedSpawnPointId = m_mSpawnPoints.GetKeyByValue(name);
		FocusOnSelectedSpawnPoint();
		SCR_SpawnPoint selectedSpawnPoint = SCR_SpawnPoint.GetSpawnPointByRplId(m_SelectedSpawnPointId);
		if (selectedSpawnPoint)
			m_bTimedSpawnPoint = (selectedSpawnPoint.Type() == SCR_PlayerRadioSpawnPointCampaign);
		Event_OnSpawnPointChanged.Invoke(selectedSpawnPoint);
		m_wSpawnPointName.SetText(name);
	}

	//------------------------------------------------------------------------------------------------
	protected bool SelectLastSelectedSpawnPoint()
	{
		if (!m_SelectedSpawnPointId.IsValid())
			return false;

		string name = m_mSpawnPoints.Get(m_SelectedSpawnPointId);
		int id = m_SelectionSpinBox.m_aElementNames.Find(name);
		if (id == -1)
			m_SelectedSpawnPointId = m_DefaultSpawnPointId;
		else
			m_SelectionSpinBox.SetCurrentItem(id);

		FocusOnSelectedSpawnPoint();
		Event_OnSpawnPointChanged.Invoke(SCR_SpawnPoint.GetSpawnPointByRplId(m_SelectedSpawnPointId));
		m_wSpawnPointName.SetText(name);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void SetConfirmButtonToggled(SCR_SpawnPoint sp)
	{
		if (!m_RespawnSystemComponent)
			return;

		Faction playerFaction = m_RespawnSystemComponent.GetPlayerFaction(m_iPlayerId);
		if (!playerFaction)
			return;

		if (sp.GetFactionKey() == playerFaction.GetFactionKey())
			return;

		if (m_SelectedSpawnPointId == SCR_SpawnPoint.GetSpawnPointRplId(sp))
			m_ConfirmButton.SetToggled(false);

		UpdateTimedSpawnPoint();
	}

	void UpdateTimedSpawnPoint()
	{
		SCR_SpawnPoint selectedSpawnPoint = SCR_SpawnPoint.GetSpawnPointByRplId(m_SelectedSpawnPointId);
		if (selectedSpawnPoint)
			m_bTimedSpawnPoint = (selectedSpawnPoint.Type() == SCR_PlayerRadioSpawnPointCampaign);
		else
			m_bTimedSpawnPoint = false;
		Event_OnSpawnPointChanged.Invoke(selectedSpawnPoint);
	}

	//------------------------------------------------------------------------------------------------
	protected void RemoveSpawnPointFromList(SCR_SpawnPoint sp)
	{
		if (!m_SelectionSpinBox)
			return;

		RplId id = SCR_SpawnPoint.GetSpawnPointRplId(sp);
		string name = m_mSpawnPoints.Get(id);
		int nameId = m_SelectionSpinBox.m_aElementNames.Find(name);
		if (nameId != -1)
			m_SelectionSpinBox.RemoveItem(nameId);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSelectedFromMap(MapItem item)
	{
		if (!m_RespawnSystemComponent || !item)
			return;

		SCR_SpawnPoint sp = SCR_SpawnPoint.Cast(item.Entity());
		if (!sp)
		{
			IEntity child = item.Entity().GetChildren();
			while (child)
			{
				if (SCR_SpawnPoint.Cast(child))
				{
					sp = SCR_SpawnPoint.Cast(child);
					break;
				}
				child = child.GetSibling();
			}

			if (!sp)
				return;
		}

		if (sp.GetFactionKey() == m_RespawnSystemComponent.GetPlayerFaction(m_iPlayerId).GetFactionKey())
		{
			RplId id = SCR_SpawnPoint.GetSpawnPointRplId(sp);

			string spName = m_mSpawnPoints.Get(id);
			m_SelectedSpawnPointId = id;

			FocusOnSelectedSpawnPoint();
			int nameId = m_SelectionSpinBox.m_aElementNames.Find(spName);
			if (nameId != -1)
				m_SelectionSpinBox.SetCurrentItem(nameId);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void FocusOnSelectedSpawnPoint()
	{
		if (!m_MapEntity)
			return;

		CanvasWidget canvas = m_MapEntity.GetMapWidget();
		if (!canvas)
			return;

		SCR_SpawnPoint sp = SCR_SpawnPoint.GetSpawnPointByRplId(m_SelectedSpawnPointId);

		if (!sp)
			return;

		SCR_MapDescriptorComponent descr = GetMapDescriptorFromHierarchy(sp);
		if (!descr || !descr.Item())
			return;

		int x, y;
		vector itemPos = descr.Item().GetPos();

		m_MapEntity.WorldToScreen(itemPos[0], itemPos[2], x, y);
		if (m_bSmoothPan)
		{
			m_MapEntity.PanSmooth(x, y);
		}
		else
		{
			// menu opened, set the pan directly
			// note(koudelkaluk): hotfix since SetPan() doesn't seem to work atm
			m_MapEntity.ZoomPanSmooth(m_MapEntity.GetCurrentZoom(), itemPos[0], itemPos[2], 0.01);
			//m_MapEntity.SetPan(x, y);
			m_bSmoothPan = true;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void FocusOnSpawnPointOnly()
	{
		string name = m_SelectionSpinBox.GetCurrentItem();
		m_SelectedSpawnPointId = m_mSpawnPoints.GetKeyByValue(name);
		FocusOnSelectedSpawnPoint();
		UntoggleConfirm();
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateAndShowSelection()
	{
		if (!m_RespawnSystemComponent || !m_MapEntity || !m_MapEntity.IsOpen())
			return;

		Faction faction = m_RespawnSystemComponent.GetPlayerFaction(m_iPlayerId);
		if (!faction)
			return;

		string factionKey = faction.GetFactionKey();
		array<SCR_SpawnPoint> spawnPoints = SCR_SpawnPoint.GetSpawnPointsForFaction(factionKey);
		int cnt = spawnPoints.Count();
		SetButtonsEnabled(cnt > 1);

		for (int i = 0; i < cnt; ++i)
		{
			SCR_SpawnPoint sp = spawnPoints[i];

			RplId id = SCR_SpawnPoint.GetSpawnPointRplId(sp);
			if (!id.IsValid())
				continue;

			if (m_mSpawnPoints.Get(id))
				continue;

			string spName = m_sDefaultRespawnPointName;
			// todo: bases and mobile hqs also could have ui info?
			if ((sp.GetParent() != null ) && sp.GetParent().Type() == SCR_CampaignBase)
			{
				SCR_CampaignBase base = SCR_CampaignBase.Cast(sp.GetParent());
				spName = base.GetBaseName();

				if (base.GetType() == CampaignBaseType.MAIN)
					m_DefaultSpawnPointId = id;

			}
			else if ((sp.GetParent() != null ) && sp.GetParent().Type() == GenericEntity)
			{
				spName = m_sMobileAssembly;
			}
			else
			{
				SCR_UIInfo info = sp.GetInfo();
				if (info)
					spName = info.GetName();

				if (m_SelectionSpinBox.m_aElementNames.Find(spName) > -1)
				{
					int spId = i+1;
					spName = string.Format("%1 %2", spName, spId);
				}

				if (id == 0)
					m_DefaultSpawnPointId = id;
			}

			m_mSpawnPoints.Set(id, spName);
			if (m_SelectionSpinBox.m_aElementNames.Find(spName) == -1)
				m_SelectionSpinBox.AddItem(spName);
		}

		foreach (RplId id, string name : m_mSpawnPoints)
		{
			SCR_SpawnPoint sp = SCR_SpawnPoint.GetSpawnPointByRplId(id);
			if (!sp)
				continue;
			if (sp.GetFactionKey() != factionKey)
			{
				if (sp == SCR_SpawnPoint.GetSpawnPointByRplId(SCR_SpawnPoint.s_LastUsed))
					SCR_SpawnPoint.s_LastUsed = RplId.Invalid();
				int nameId = m_SelectionSpinBox.m_aElementNames.Find(name);
				if (nameId != -1)
					m_SelectionSpinBox.RemoveItem(nameId);
				m_mSpawnPoints.Remove(id);
			}
		}

		if (!m_RespawnSystemComponent.CanSetSpawnPoint(m_iPlayerId, SCR_SpawnPoint.s_LastUsed))
			m_SelectedSpawnPointId = m_DefaultSpawnPointId;
		else
			m_SelectedSpawnPointId = SCR_SpawnPoint.s_LastUsed;

		SetDeployAvailable();
		UpdateTimedSpawnPoint();
	}

	//------------------------------------------------------------------------------------------------
	// return topmost map descriptor from hierarchy
	static SCR_MapDescriptorComponent GetMapDescriptorFromHierarchy(IEntity ent)
	{
		SCR_MapDescriptorComponent descr = SCR_MapDescriptorComponent.Cast(ent.FindComponent(SCR_MapDescriptorComponent));
		IEntity parent = ent.GetParent();

		while (parent)
		{
			SCR_MapDescriptorComponent dc = SCR_MapDescriptorComponent.Cast(parent.FindComponent(SCR_MapDescriptorComponent));
			if (dc)
				descr = dc;
			parent = parent.GetParent();
		}

		return descr;
	}

	//------------------------------------------------------------------------------------------------
	protected override bool ConfirmSelection()
	{
		if (m_SelectedSpawnPointId == RplId.Invalid())
			SelectSpawnPoint();
		SCR_SpawnPoint.s_LastUsed = m_SelectedSpawnPointId;

		return RequestSpawnPoint(SCR_SpawnPoint.GetSpawnPointByRplId(m_SelectedSpawnPointId));
	}

	//------------------------------------------------------------------------------------------------
	static SCR_SelectSpawnPointSubMenu GetInstance()
	{
		return s_Instance;
	}

	//------------------------------------------------------------------------------------------------
	protected void HideDeployCountdown()
	{
		SetDeployCountdown(false);
		m_bCountdownHidden = true;
	}

	//------------------------------------------------------------------------------------------------
	void SetDeployCountdown(bool show, string time = string.Empty)
	{
		if (m_bCountdownHidden)
			return;

		if (m_wDeployTimer && m_wDeployAvailable)
		{
			m_wDeployAvailable.SetVisible(show);
			m_wDeployTimer.SetText(time);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void LockConfirmButton(bool locked)
	{
		m_bButtonsUnlocked = !locked;
		SCR_RespawnSuperMenu.Cast(m_ParentMenu).SetLoadingVisible(locked);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void CreateConfirmButton()
	{
		Widget btn = GetRootWidget().FindAnyWidget("DeployBtn");
		if (btn)
		{
			m_ConfirmButton = SCR_NavigationButtonComponent.Cast(btn.FindHandler(SCR_NavigationButtonComponent));
			m_ConfirmButton.m_OnActivated.Insert(HandleOnConfirm);
			m_ConfirmButton.SetToggleable(true);
		}
	}

	//------------------------------------------------------------------------------------------------
	void HandleOnSpawnPointAssigned(int playerId, SCR_SpawnPoint spawnPoint)
	{
	}

	//------------------------------------------------------------------------------------------------
	protected override void SetDeployAvailable()
	{
		super.SetDeployAvailable();

		if (m_wWaitMessage)
			m_wWaitMessage.SetVisible(!m_bSpawnPointsAvailable);
		if (m_wSpawnPointName)
			m_wSpawnPointName.SetVisible(m_bSpawnPointsAvailable);
		if (m_wNoSpawnPoints)
			m_wNoSpawnPoints.SetVisible(!m_bSpawnPointsAvailable);
		if (m_wSelectionSpinBox)
			m_wSelectionSpinBox.SetVisible(m_bSpawnPointsAvailable);
	}

	//------------------------------------------------------------------------------------------------
	void SCR_SelectSpawnPointSubMenu()
	{
		if (SCR_GameModeCampaignMP.GetInstance())
		{
			SCR_CampaignBase.s_OnBaseOwnerChanged.Insert(UpdateAndShowSelection);
			SCR_CampaignBase.s_OnSpawnPointOwnerChanged.Insert(UpdateAndShowSelection);
			SCR_CampaignMobileAssemblyComponent.s_OnSpawnPointOwnerChanged.Insert(UpdateAndShowSelection);
		}

		SCR_MapUIElement.Event_OnPointSelected.Insert(OnSelectedFromMap);
		SCR_MapEntity.GetOnMapOpen().Insert(OnMapOpen);

		SCR_SpawnPoint.Event_SpawnPointFactionAssigned.Insert(SetConfirmButtonToggled);
		SCR_SpawnPoint.Event_OnSpawnPointCountChanged.Insert(UpdateAndShowSelection);
		SCR_SpawnPoint.Event_SpawnPointFactionAssigned.Insert(UpdateAndShowSelection);
		SCR_SpawnPoint.Event_SpawnPointFactionAssigned.Insert(UpdateTimedSpawnPoint);
		SCR_SpawnPoint.Event_SpawnPointRemoved.Insert(RemoveSpawnPointFromList);

		s_Instance = this;
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_SelectSpawnPointSubMenu()
	{
		if (SCR_GameModeCampaignMP.GetInstance())
		{
			SCR_CampaignBase.s_OnBaseOwnerChanged.Remove(UpdateAndShowSelection);
			SCR_CampaignBase.s_OnSpawnPointOwnerChanged.Remove(UpdateAndShowSelection);
			SCR_CampaignMobileAssemblyComponent.s_OnSpawnPointOwnerChanged.Remove(UpdateAndShowSelection);
		}

		SCR_MapUIElement.Event_OnPointSelected.Remove(OnSelectedFromMap);
		SCR_MapEntity.GetOnMapOpen().Remove(OnMapOpen);

		SCR_SpawnPoint.Event_SpawnPointFactionAssigned.Remove(SetConfirmButtonToggled);
		SCR_SpawnPoint.Event_OnSpawnPointCountChanged.Remove(UpdateAndShowSelection);
		SCR_SpawnPoint.Event_SpawnPointFactionAssigned.Remove(UpdateAndShowSelection);
		SCR_SpawnPoint.Event_SpawnPointFactionAssigned.Remove(UpdateTimedSpawnPoint);
		SCR_SpawnPoint.Event_SpawnPointRemoved.Remove(RemoveSpawnPointFromList);

		if (m_PrevSpawnPoint)
			m_PrevSpawnPoint.m_OnActivated.Remove(SelectPrevSpawnPoint);
		if (m_NextSpawnPoint)
			m_NextSpawnPoint.m_OnActivated.Remove(SelectNextSpawnPoint);
		if (m_ConfirmButton)
			m_ConfirmButton.m_OnToggled.Remove(UntoggleQuick);
		if (m_QuickDeployButton)
			m_QuickDeployButton.m_OnToggled.Remove(UntoggleConfirm);

		s_Instance = null;
	}
};