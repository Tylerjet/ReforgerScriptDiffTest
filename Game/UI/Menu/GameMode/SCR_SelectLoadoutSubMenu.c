//------------------------------------------------------------------------------------------------
class SCR_SelectLoadoutSubMenu : SCR_RespawnSubMenuBase
{
	protected static SCR_SelectLoadoutSubMenu s_Instance;
	protected SCR_LoadoutManager m_LoadoutManager;

	protected SCR_BasePlayerLoadout m_SelectedLoadout;
	protected ref map<SCR_LoadoutMenuTile, SCR_BasePlayerLoadout> m_mAvailableLoadouts = new ref map<SCR_LoadoutMenuTile, SCR_BasePlayerLoadout>();
	
	[Attribute("Tiles")]
	protected string m_sTileContainer;
	
	[Attribute("NoLoadoutSaved")]
	protected string m_sNoArsenalLoadoutMessageID;

	override void GetWidgets()
	{
		super.GetWidgets();
		Widget tileSelection = GetRootWidget().FindAnyWidget("TileSelection");
		if (tileSelection)
			m_TileSelection = SCR_DeployMenuTileSelection.Cast(tileSelection.FindHandler(SCR_DeployMenuTileSelection));
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_SuperMenuBase parentMenu)
	{
		if (!GetGame().GetLoadoutManager())
		{
			Print("No loadout manager present in the current world!", LogLevel.ERROR);
			return;
		}

		super.OnMenuOpen(parentMenu);

		m_bIsLastAvailableTab = !GetRespawnMenuHandler().GetAllowSpawnPointSelection();
		CreateConfirmButton();
		CreateQuickDeployButton();

		PlayerManager pm = GetGame().GetPlayerManager();
		SCR_RespawnComponent rc = SCR_RespawnComponent.Cast(pm.GetPlayerRespawnComponent(m_iPlayerId));
		rc.GetLoadoutLockInvoker().Insert(LockLoadoutTiles);

		m_LoadoutManager = GetGame().GetLoadoutManager();
		m_sConfirmButtonText = m_sButtonTextSelectLoadout;
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuShow(SCR_SuperMenuBase parentMenu)
	{
		if (!GetGame().GetLoadoutManager())
		{
			Print("No loadout manager present in the current world!", LogLevel.ERROR);
			return;
		}

		super.OnMenuShow(parentMenu);

		UpdateLoadouts();
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateLoadouts()
	{
		Faction faction = m_RespawnSystemComponent.GetPlayerFaction(m_iPlayerId);
		array<ref SCR_BasePlayerLoadout> loadouts = {};
		Widget gallery = GetRootWidget().FindAnyWidget(m_sTileContainer);
		SCR_GalleryComponent gallery_component = SCR_GalleryComponent.Cast( gallery.GetHandler(0));
		gallery_component.ClearAll();	

		int loadoutCnt;
		if (m_LoadoutManager)
		{
			if (faction)
			{
				loadoutCnt = m_LoadoutManager.GetPlayerLoadoutsByFaction(faction, loadouts);
			}
			else
			{
				loadouts = m_LoadoutManager.GetPlayerLoadouts();
				loadoutCnt = loadouts.Count();
			}
		}

		for (int lid = 0; lid < loadoutCnt; ++lid)
		{
			SCR_LoadoutMenuTile tile = SCR_LoadoutMenuTile.InitializeTile(m_TileSelection, loadouts[lid]);
			if (!loadouts[lid].IsLoadoutAvailableClient())
			{
				tile.DisableAndShowMessage(m_sNoArsenalLoadoutMessageID);
				continue;
			}
			
			m_mAvailableLoadouts.Set(tile, loadouts[lid]);
			tile.m_OnClicked.Insert(HandleOnConfirm);
		}

		m_TileSelection.Init();
	}

	//------------------------------------------------------------------------------------------------
	protected int GetWeaponSlots(IEntitySource prefab, out array<IEntityComponentSource> slots)
	{
		if (!prefab)
			return -1;

		int componentsCount = prefab.GetComponentCount();
		for (int i = 0; i < componentsCount; ++i)
		{
			IEntityComponentSource compSrc = prefab.GetComponent(i);
			if (compSrc.GetClassName() == "CharacterWeaponSlotComponent")
				slots.Insert(compSrc);
		}

		return slots.Count();
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_BasePlayerLoadout GetSelectedLoadout()
	{
		if (m_TileSelection)
		{
			SCR_LoadoutMenuTile tile = SCR_LoadoutMenuTile.Cast(m_TileSelection.GetFocusedTile());
			m_SelectedLoadout = m_mAvailableLoadouts.Get(tile);
		}

		return m_SelectedLoadout;
	}

	//------------------------------------------------------------------------------------------------
	protected override bool ConfirmSelection()
	{
		if (GetSelectedLoadout())
		{
			SetDeployAvailable();
			return RequestLoadout(GetSelectedLoadout());
		}
		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected void LockLoadoutTiles(bool locked)
	{
		m_bButtonsUnlocked = !locked;
		m_TileSelection.SetTilesEnabled(!locked);
		SCR_RespawnSuperMenu.Cast(m_ParentMenu).SetLoadingVisible(locked);
	}

	//------------------------------------------------------------------------------------------------
	void HandleOnLoadoutAssigned(int playerId, SCR_BasePlayerLoadout loadout)
	{
	}

	//------------------------------------------------------------------------------------------------
	static SCR_SelectLoadoutSubMenu GetInstance()
	{
		return s_Instance;
	}

	//------------------------------------------------------------------------------------------------
	void SCR_SelectLoadoutSubMenu()
	{
		s_Instance = this;
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_SelectLoadoutSubMenu()
	{
		s_Instance = null;
	}
};