//------------------------------------------------------------------------------------------------
class SCR_SelectFactionSubMenu : SCR_RespawnSubMenuBase
{
	protected static SCR_SelectFactionSubMenu s_Instance;
	protected SCR_SpinBoxComponent m_FactionSelectionSpinBox;
	protected HorizontalLayoutWidget m_wAvailableFactions;
	protected Widget m_wPlayerList;
	protected Widget m_wOptionalMessage;
	protected Widget m_wSelectFaction;
	protected Widget m_wWarningMsg;

	protected ref map<SCR_FactionMenuTile, Faction> m_mAvailableFactions = new ref map<SCR_FactionMenuTile, Faction>();

	[Attribute("AvailableFactions")]
	protected string m_sAvailableFactions;

	[Attribute("FactionSelectionSpinBox")]
	protected string m_sFactionSelection;

	[Attribute("PlayerList")]
	protected string m_sPlayerListWidget;

	[Attribute("FactionImage")]
	protected string m_sFactionImage;

	[Attribute("{A008695164D0A624}UI/layouts/WidgetLibrary/WLib_ButtonImage.layout")]
	protected ResourceName m_sButtonImage;

	[Attribute("{F010750A52F76A5D}UI/layouts/Menus/RoleSelection/PlayerName.layout")]
	protected ResourceName m_sPlayerName;

	[Attribute("{2EFEA2AF1F38E7F0}UI/Textures/Icons/icons_wrapperUI-64.imageset")]
	protected ResourceName m_sPlatformIcons;

	[Attribute("PermanentFaction")]
	protected string m_sOptionalMessage;
	
	[Attribute("Tiles")]
	protected string m_sTileContainer;

	protected int m_iPlayerLimit = -1;

	//------------------------------------------------------------------------------------------------
	override void GetWidgets()
	{
		super.GetWidgets();
		Widget tileSelection = GetRootWidget().FindAnyWidget("TileSelection");
		if (tileSelection)
			m_TileSelection = SCR_DeployMenuTileSelection.Cast(tileSelection.FindHandler(SCR_DeployMenuTileSelection));

		m_wPlayerList = GetRootWidget().FindAnyWidget(m_sPlayerListWidget);
		m_wOptionalMessage = GetRootWidget().FindAnyWidget(m_sOptionalMessage);

		m_wWarningMsg = GetRootWidget().FindAnyWidget("NoFactionWarning");
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_SuperMenuBase parentMenu)
	{
		if (!GetGame().GetFactionManager())
		{
			Print("No faction manager present in the current world!", LogLevel.ERROR);
			return;
		}

		super.OnMenuOpen(parentMenu);

		if (GetGame().GetMissionHeader())
			m_iPlayerLimit = SCR_MissionHeader.Cast(GetGame().GetMissionHeader()).m_iPlayerCount;

		m_sConfirmButtonText = m_sButtonTextSelectFaction;

		SCR_RespawnMenuHandlerComponent rsMenuHandler = GetRespawnMenuHandler();
		m_bIsLastAvailableTab = !rsMenuHandler.GetAllowLoadoutSelection() &&
								!rsMenuHandler.GetAllowSpawnPointSelection();

		CreateConfirmButton();
		CreateQuickDeployButton();

		if (m_wOptionalMessage && !rsMenuHandler.GetAllowFactionChange() && rsMenuHandler.GetPermanentFactionMessage() != string.Empty)
		{
			m_wOptionalMessage.SetVisible(true);
			TextWidget w = TextWidget.Cast(m_wOptionalMessage.FindAnyWidget("Message"));
			w.SetText(rsMenuHandler.GetPermanentFactionMessage());
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuShow(SCR_SuperMenuBase parentMenu)
	{
		if (!GetGame().GetFactionManager())
		{
			Print("No faction manager present in the current world!", LogLevel.ERROR);
			return;
		}

		super.OnMenuShow(parentMenu);
		
		InitFactionList();
		PlayerManager pm = GetGame().GetPlayerManager();
		SCR_RespawnComponent rc = SCR_RespawnComponent.Cast(pm.GetPlayerRespawnComponent(m_iPlayerId));
		rc.GetFactionLockInvoker().Insert(LockFactionTiles);

		UpdateUnassignedPlayerList();
		UpdateFactionPlayerList();
		
		m_wWarningMsg.SetVisible(!s_bPlayableFactionsAvailable);
		GetRootWidget().FindAnyWidget("GalleryOverlay").SetVisible(s_bPlayableFactionsAvailable);
		Widget title = GetRootWidget().FindAnyWidget("TitleWidget");
		if (title)
			title.SetVisible(s_bPlayableFactionsAvailable);
	}

	//------------------------------------------------------------------------------------------------
	protected Faction GetSelectedFaction()
	{
		if (m_TileSelection)
		{
			SCR_FactionMenuTile tile = SCR_FactionMenuTile.Cast(m_TileSelection.GetFocusedTile());
			m_SelectedFaction = m_mAvailableFactions.Get(tile);
		}

		return m_SelectedFaction;
	}

	//------------------------------------------------------------------------------------------------
	protected void InitFactionList()
	{
		Widget gallery = GetRootWidget().FindAnyWidget(m_sTileContainer);
		SCR_GalleryComponent gallery_component = SCR_GalleryComponent.Cast( gallery.GetHandler(0));
		gallery_component.ClearAll();	
		array<Faction> factions = {};
		int count = m_FactionManager.GetFactionsList(factions);
		s_bPlayableFactionsAvailable = false;
		for (int i = 0; i < count; ++i)
		{
			SCR_Faction faction = SCR_Faction.Cast(factions[i]);
			
			faction.GetOnFactionPlayableChanged().Insert(UpdateFactionTile);
			
			if (!faction.IsPlayable())
				continue;

			s_bPlayableFactionsAvailable = true;
			SCR_FactionMenuTile tile = SCR_FactionMenuTile.InitializeTile(m_TileSelection, faction);
			m_mAvailableFactions.Set(tile, faction);
			tile.m_OnClicked.Insert(HandleOnConfirm);
		}

		m_TileSelection.Init();
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateFactionTile(Faction faction, bool available)
	{
		SCR_DeployMenuTile factionTile = m_mAvailableFactions.GetKeyByValue(faction);
		if (factionTile)
			factionTile.ShowTile(available);
		s_bPlayableFactionsAvailable = false;
		foreach (SCR_DeployMenuTile tile, Faction f : m_mAvailableFactions)
		{
			if (tile && tile.IsEnabled())
			{
				s_bPlayableFactionsAvailable = true;
				break;
			}
		}

		SetQuickDeployAvailable();
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateFactionPlayerList()
	{
		if (!m_FactionManager)
			return;

		foreach (SCR_FactionMenuTile tile, Faction faction : m_mAvailableFactions)
		{
			if (!tile)
				continue;

			SCR_Faction scriptedFaction = SCR_Faction.Cast(faction);
			if (scriptedFaction)
				tile.SetPlayerCount(scriptedFaction.GetPlayerCount());

			Widget list = tile.GetPlayerList();
			Widget widget = list.GetChildren();
			while (widget)
			{
				Widget child = widget;
				widget = widget.GetSibling();
				child.RemoveFromHierarchy();
			}

			array<int> players = {};
			PlayerManager pm = GetGame().GetPlayerManager();
			pm.GetAllPlayers(players);

			foreach (int playerId : players)
			{
				Faction playerFaction = m_RespawnSystemComponent.GetPlayerFaction(playerId);

				if (playerFaction != faction)
					continue;

				widget = GetGame().GetWorkspace().CreateWidgets(m_sPlayerName, list);
				if (!widget)
					continue;

				TextWidget text = TextWidget.Cast(widget.FindAnyWidget("Text"));
				if (!text)
					continue;

				string name = pm.GetPlayerName(playerId);
				text.SetText(name);

				PlatformKind platform = pm.GetPlatformKind(playerId);
				string platformName = SCR_Global.GetPlatformName(platform);

				ImageWidget img = ImageWidget.Cast(widget.FindAnyWidget("Platform"));
				if (img && !platformName.IsEmpty())
				{
					img.SetVisible(true);
					img.LoadImageFromSet(0, m_sPlatformIcons, platformName);
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateUnassignedPlayerList()
	{
		if (!m_wPlayerList)
			return;

		Widget w = m_wPlayerList.GetChildren();
		while (w)
		{
			Widget child = w;
			w = w.GetSibling();
			child.RemoveFromHierarchy();
		}

		array<int> players = {};
		PlayerManager pm = GetGame().GetPlayerManager();
		pm.GetAllPlayers(players);

		foreach (int playerId : players)
		{
			if (!pm.GetPlayerController(playerId))
				continue;

			Faction playerFaction = m_RespawnSystemComponent.GetPlayerFaction(playerId);
			if (playerFaction)
				continue;

			w = GetGame().GetWorkspace().CreateWidgets(m_sPlayerName, m_wPlayerList);
			if (!w)
				continue;

			TextWidget text = TextWidget.Cast(w.FindAnyWidget("Text"));
			if (!text)
				continue;

			string name = pm.GetPlayerName(playerId);
			text.SetText(name);

			PlatformKind platform = pm.GetPlatformKind(playerId);
			string platformName = SCR_Global.GetPlatformName(platform);

			ImageWidget img = ImageWidget.Cast(w.FindAnyWidget("Platform"));
			if (img && !platformName.IsEmpty())
			{
				img.SetVisible(true);
				img.LoadImageFromSet(0, m_sPlatformIcons, platformName);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected override bool ConfirmSelection()
	{
		if (GetSelectedFaction())
		{
			SetDeployAvailable();
			return RequestFaction(GetSelectedFaction());
		}
		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected void LockFactionTiles(bool locked)
	{
		m_bButtonsUnlocked = !locked;
		m_TileSelection.SetTilesEnabled(!locked);
		SCR_RespawnSuperMenu.Cast(m_ParentMenu).SetLoadingVisible(locked);
	}

	//------------------------------------------------------------------------------------------------
	void HandleOnFactionAssigned(int playerId, Faction faction)
	{
		UpdateUnassignedPlayerList();
		UpdateFactionPlayerList();
	}

	//------------------------------------------------------------------------------------------------
	void HandleOnPlayerConnected(int playerId)
	{
		UpdateUnassignedPlayerList();
		UpdateFactionPlayerList();
	}

	//------------------------------------------------------------------------------------------------
	void HandleOnPlayerDisconnected(int playerId)
	{
		UpdateUnassignedPlayerList();
		UpdateFactionPlayerList();
	}

	//------------------------------------------------------------------------------------------------
	static SCR_SelectFactionSubMenu GetInstance()
	{
		return s_Instance;
	}

	//------------------------------------------------------------------------------------------------
	void SCR_SelectFactionSubMenu()
	{
		s_Instance = this;
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_SelectFactionSubMenu()
	{
		s_Instance = null;
	}
};