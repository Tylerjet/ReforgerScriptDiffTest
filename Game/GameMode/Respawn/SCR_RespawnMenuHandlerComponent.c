//------------------------------------------------------------------------------------------------
class SCR_RespawnMenuHandlerComponentClass : SCR_RespawnHandlerComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_RespawnMenuHandlerComponent : SCR_RespawnHandlerComponent
{
	[Attribute("1", category: "Respawn Handler")]
	protected bool m_bAllowFactionChange;

	[Attribute("1", category: "Respawn Handler")]
	protected bool m_bAllowFactionSelection;

	[Attribute("1", category: "Respawn Handler")];
	protected bool m_bAllowLoadoutSelection;

	[Attribute("1", category: "Respawn Handler")]
	protected bool m_bAllowSpawnPointSelection;
	
	[Attribute("1", category: "Respawn Handler")]
	protected bool m_bAllowQuickDeploy;

	[Attribute("1", category: "Respawn Handler")]
	protected float m_fMenuOpenDelay;

	protected float m_fMenuOpenDelayCounter = 0;

	[Attribute("#AR-DeployMenu_NoSpawnPoints_Text", uiwidget: UIWidgets.EditBox, "Message shown when no spawn points are available", category: "Respawn Handler")]
	protected LocalizedString m_sRespawnUnavailable;

	[Attribute("", uiwidget: UIWidgets.EditBox, "Optional message shown in faction selection screen", category: "Respawn Handler")]
	protected LocalizedString m_sFactionMenuMessage;

	protected ref SimplePreload m_Preload;

	protected ref set<int> m_aSpawnQueue = new set<int>();
	protected ref set<int> m_aProcessingQueue = new set<int>();

	/*!
		Whether local client is enqueued for respawning, ie. menu should be open.
	*/
	bool m_bLocalPlayerEnqueued;

	/*!
		Marks local player as enqueued.
	*/
	protected override void OnLocalPlayerEnqueued()
	{
		m_bLocalPlayerEnqueued = true;
		super.OnLocalPlayerEnqueued();
	}

	/*!
		Marks local player as dequeued.
	*/
	protected override void OnLocalPlayerDequeued()
	{
		m_bLocalPlayerEnqueued = false;
		super.OnLocalPlayerDequeued();
	}

	/*!
		Returns whether respawn menu can be open.
	*/
	protected bool CanOpenRespawnMenu()
	{
		// if editor
		// if whatever
		
		// TODO@AS:
		// TODO@LK:
		// CanRequestRespawn() returns only whether we have unlocked RequestRespawn() lock on PlayerController
		// that is completely fine, but this condition should probably be handled in the menu
		// so whenever !CanRequestRespawn() all buttons are grayed out and the user is informed.
		// 
		// I'll try to expose the lock of PlayerController futher to scripts, so we can actually hook up
		// on that and now rely on this silly bool - then we will have:
		// a) timeout -> sent request, no response within x time
		// b) response -> sent request, received response:
		//     b1) ok -> spawned, all is ok
		//     b2) err -> not spawned, something gone wrong
		//
		// and this whole thing can be nuked as well		
		PlayerController pc = GetGame().GetPlayerController();
		if (!pc)
			return false;

		SCR_PlayerController playerController = SCR_PlayerController.Cast(pc);

		//--- Never open respawn menu when possessing or control entity
		bool hasEntity = playerController.IsPossessing() || playerController.GetControlledEntity();
		
		return playerController.CanRequestRespawn() && !SCR_EditorManagerEntity.IsOpenedInstance() && !hasEntity && GetGameMode().GetState() != SCR_EGameModeState.POSTGAME;
	}

	/*!
		Called every frame. Handles opening an closing of respawn menu based
		on player eligibility for respawn and respawn menu logic.
	*/
	protected override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (m_Preload)
		{
			bool finished = m_Preload.Update(timeSlice);
			if (finished)
			{
				m_Preload = null;
			}
			return;
		}

		if (m_pGameMode.IsMaster() && !m_aSpawnQueue.IsEmpty())
		{
			PlayerManager pm = GetGame().GetPlayerManager();
			int count = m_aSpawnQueue.Count();
			int pid;
			for (int index = 0; index < count; ++index)
			{
				pid = m_aSpawnQueue[index];
				if (m_pGameMode.CanPlayerRespawn(pid))
				{
					m_aProcessingQueue.Insert(pid);
					pm.GetPlayerController(pid).RequestRespawn();
					m_aSpawnQueue.Remove(index);
				}
			}
		}

		// Make sure that we open menu only when we can
		bool isOpen = SCR_RespawnSystemComponent.IsRespawnMenuOpened();
		if (m_bLocalPlayerEnqueued && CanOpenRespawnMenu())
		{
			if (m_fMenuOpenDelayCounter > 0)
				m_fMenuOpenDelayCounter -= timeSlice;

			if (!isOpen && m_fMenuOpenDelayCounter <= 0)
				SCR_RespawnSystemComponent.OpenRespawnMenu();
		}
		else
		{
			if (isOpen)
			{
				m_fMenuOpenDelayCounter = m_fMenuOpenDelay;
				SCR_RespawnSystemComponent.CloseRespawnMenu();
			}
		}

		super.EOnFrame(owner, timeSlice);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerRegistered(int playerId)
	{
		super.OnPlayerRegistered(playerId);

		if (!m_bAllowFactionSelection)
			RandomizePlayerFaction(playerId);
	}

	//------------------------------------------------------------------------------------------------
	override void HandleOnFactionAssigned(int playerID, Faction assignedFaction)
	{
		SCR_RespawnSuperMenu menu = SCR_RespawnSuperMenu.GetInstance();
		if (menu)
		{
			menu.HandleOnFactionAssigned(playerID, assignedFaction);
		}

		if (!m_bAllowLoadoutSelection)
			RandomizePlayerLoadout(playerID);
	}

	//------------------------------------------------------------------------------------------------
	override void HandleOnLoadoutAssigned(int playerID, SCR_BasePlayerLoadout assignedLoadout)
	{
		SCR_RespawnSuperMenu menu = SCR_RespawnSuperMenu.GetInstance();
		if (menu)
		{
			menu.HandleOnLoadoutAssigned(playerID, assignedLoadout);
		}

		if (!m_bAllowSpawnPointSelection)
			RandomizePlayerSpawnPoint(playerID);
	}

	//------------------------------------------------------------------------------------------------
	override void HandleOnSpawnPointAssigned(int playerID, SCR_SpawnPoint spawnPoint)
	{
		SCR_RespawnSuperMenu menu = SCR_RespawnSuperMenu.GetInstance();
		if (menu)
		{
			menu.HandleOnSpawnPointAssigned(playerID, spawnPoint);
		}

		if (spawnPoint && SCR_PlayerController.GetLocalPlayerId() == playerID)
		{
			m_Preload = SimplePreload.Preload(spawnPoint.GetOrigin());
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerConnected(int playerId)
	{
		super.OnPlayerConnected(playerId);

		SCR_SelectFactionSubMenu menu = SCR_SelectFactionSubMenu.GetInstance();
		if (menu)
		{
			menu.HandleOnPlayerConnected(playerId);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerDisconnected(int playerId)
	{
		super.OnPlayerDisconnected(playerId);

		SCR_SelectFactionSubMenu menu = SCR_SelectFactionSubMenu.GetInstance();
		if (menu)
		{
			menu.HandleOnPlayerDisconnected(playerId);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerSpawned(int playerId, IEntity controlledEntity)
	{
		int index = m_aProcessingQueue.Find(playerId);
		if (m_aProcessingQueue.Contains(playerId))
			m_aProcessingQueue.Remove(index);
	}

	//------------------------------------------------------------------------------------------------
	void RequestRespawn(int playerId)
	{
		if (!m_pGameMode.IsMaster())
			return;

		if (m_aSpawnQueue.Contains(playerId) || m_aProcessingQueue.Contains(playerId))
			return;

		m_aSpawnQueue.Insert(playerId);
	}

	//------------------------------------------------------------------------------------------------
	void RequestQuickRespawn(int playerId)
	{
		if (!m_pGameMode.IsMaster())
			return;

		SCR_RespawnSystemComponent respawnSystem = SCR_RespawnSystemComponent.GetInstance();
		Faction playerFaction = respawnSystem.GetPlayerFaction(playerId);
		if (!playerFaction)
		{
			FactionManager factionManager = GetGame().GetFactionManager();
			if (!factionManager)
				return;

			array<Faction> factions = {};
			int factionsCount = factionManager.GetFactionsList(factions);
			if (factionsCount == 0)
			{
				Print("Could not randomize player faction, no factions are available!", LogLevel.ERROR);
				return;
			}

			array<SCR_Faction> availableFactions = {};
			foreach (Faction faction : factions)
			{
				SCR_Faction scriptedFaction = SCR_Faction.Cast(faction);
				if (!scriptedFaction)
					continue;
				if (scriptedFaction.IsPlayable() && respawnSystem.CanSetFaction(playerId, respawnSystem.GetFactionIndex(scriptedFaction)))
					availableFactions.Insert(scriptedFaction);
			}

			int lastPlayerCount = int.MAX;
			int factionIndex;
			// balance factions
			foreach (SCR_Faction faction : availableFactions)
			{
				if (faction.GetPlayerCount() < lastPlayerCount)
				{
					lastPlayerCount = faction.GetPlayerCount();
					factionIndex = respawnSystem.GetFactionIndex(faction);
				}
			}

			respawnSystem.DoSetPlayerFaction(playerId, factionIndex);
		}

		RandomizePlayerGroup(playerId);
		RandomizePlayerLoadout(playerId);
		RandomizePlayerSpawnPoint(playerId);

		RequestRespawn(playerId);
	}

	//------------------------------------------------------------------------------------------------
	bool GetAllowFactionChange()
	{
		return m_bAllowFactionChange;
	}

	//------------------------------------------------------------------------------------------------
	bool GetAllowFactionSelection()
	{
		return m_bAllowFactionSelection;
	}

	//------------------------------------------------------------------------------------------------
	bool GetAllowLoadoutSelection()
	{
		return m_bAllowLoadoutSelection;
	}

	//------------------------------------------------------------------------------------------------
	bool GetAllowSpawnPointSelection()
	{
		return m_bAllowSpawnPointSelection;
	}

	//------------------------------------------------------------------------------------------------
	bool GetAllowQuickDeploy()
	{
		return m_bAllowQuickDeploy;
	}

	//------------------------------------------------------------------------------------------------
	LocalizedString GetRespawnUnavailableMessage()
	{
		return m_sRespawnUnavailable;
	}

	//------------------------------------------------------------------------------------------------
	LocalizedString GetFactionMenuMessage()
	{
		return m_sFactionMenuMessage;
	}
};