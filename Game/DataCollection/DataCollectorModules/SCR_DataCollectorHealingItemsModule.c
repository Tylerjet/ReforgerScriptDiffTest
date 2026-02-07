[BaseContainerProps()]
class SCR_DataCollectorHealingItemsModule : SCR_DataCollectorModule
{
	protected ref map<int, IEntity> m_mTrackedPlayers = new map<int, IEntity>();

	//------------------------------------------------------------------------------------------------
	protected override void AddInvokers(IEntity player)
	{
		//Print("SCR_DataCollectorHealingItemsModule:AddInvokers: Adding invokers");
		super.AddInvokers(player);
		if (!player)
			return;

		SCR_ChimeraCharacter chimeraPlayer = SCR_ChimeraCharacter.Cast(player);
		if (!chimeraPlayer)
			return;

		SCR_CharacterControllerComponent characterController = SCR_CharacterControllerComponent.Cast(chimeraPlayer.GetCharacterController());
		if (!characterController)
			return;

		characterController.m_OnItemUseEndedInvoker.Insert(HealingItemUsed);
	}

	//------------------------------------------------------------------------------------------------
	protected override void RemoveInvokers(IEntity player)
	{
		super.AddInvokers(player);
		if (!player)
			return;

		SCR_ChimeraCharacter chimeraPlayer = SCR_ChimeraCharacter.Cast(player);
		if (!chimeraPlayer)
			return;

		SCR_CharacterControllerComponent characterController = SCR_CharacterControllerComponent.Cast(chimeraPlayer.GetCharacterController());
		if (!characterController)
			return;

		characterController.m_OnItemUseEndedInvoker.Remove(HealingItemUsed);
	}

	//------------------------------------------------------------------------------------------------
	protected void HealingItemUsed(IEntity item, bool ActionCompleted, SCR_ConsumableEffectAnimationParameters animParams)
	{
		if (!item || !ActionCompleted)
			return;

		SCR_ConsumableItemComponent consumableItem = SCR_ConsumableItemComponent.Cast(item.FindComponent(SCR_ConsumableItemComponent));
		if (!consumableItem)
			return;

		IEntity user = consumableItem.GetCharacterOwner();
		if (!user)
			return;

		IEntity target = consumableItem.GetTargetCharacter();
		if (!target)
			target = user;

		int userID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(user);
		if (userID == 0) // Non-player character
			return;

		EConsumableType typeId = consumableItem.GetConsumableType();

		switch (typeId)
		{
			case EConsumableType.None: return;
			case EConsumableType.Bandage:GetGame().GetDataCollector().GetPlayerData(userID).AddBandageUse(target == user); return;
			case EConsumableType.Tourniquet: GetGame().GetDataCollector().GetPlayerData(userID).AddTourniquetUse(target == user); return;
			case EConsumableType.Saline: GetGame().GetDataCollector().GetPlayerData(userID).AddSalineUse(target == user); return;
			case EConsumableType.Morphine: GetGame().GetDataCollector().GetPlayerData(userID).AddMorphineUse(target == user); return;
			default: Print("SCR_DataCollectorHealingItemsModule:HealingItemUsed: Error: Unidentified Healing item typeId. Value is " + typeId + ".", LogLevel.ERROR); return;
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerSpawned(int playerID, IEntity controlledEntity)
	{
		m_mTrackedPlayers.Insert(playerID, controlledEntity);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerDisconnected(int playerID, IEntity controlledEntity = null)
	{
		m_mTrackedPlayers.Remove(playerID);
	}

#ifdef ENABLE_DIAG
	//------------------------------------------------------------------------------------------------
	override void OnControlledEntityChanged(IEntity from, IEntity to)
	{
		super.OnControlledEntityChanged(from, to);

		if (to)
		{
			int playerID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(to);
			m_mTrackedPlayers.Insert(playerID, to);
		}
		else if (from)
		{
			int playerID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(from);
			m_mTrackedPlayers.Remove(playerID);
		}
	}
#endif

	//------------------------------------------------------------------------------------------------
	override void Update(IEntity owner, float timeTick)
	{
		//If there's no data collector, do nothing
		if (!GetGame().GetDataCollector())
			return;

		m_fTimeSinceUpdate += timeTick;

		if (m_fTimeSinceUpdate<TIME_TO_UPDATE)
			return;

		SCR_PlayerData playerData;
		int playerId;

		for (int i = m_mTrackedPlayers.Count() - 1; i >= 0; i--)
		{
			playerId = m_mTrackedPlayers.GetKey(i);
			playerData = GetGame().GetDataCollector().GetPlayerData(playerId);

			//DEBUG display
#ifdef ENABLE_DIAG
			if (m_StatsVisualization)
			{
				m_StatsVisualization.Get(SCR_EHealingItemsModuleStats.BANDAGESSELF).SetText(playerData.GetCurrentBandageSelf().ToString());
				m_StatsVisualization.Get(SCR_EHealingItemsModuleStats.BANDAGESFRIENDLIES).SetText(playerData.GetCurrentBandageFriends().ToString());
				m_StatsVisualization.Get(SCR_EHealingItemsModuleStats.TOURNIQUETSSELF).SetText(playerData.GetCurrentTourniquetSelf().ToString());
				m_StatsVisualization.Get(SCR_EHealingItemsModuleStats.TOURNIQUETSFRIENDLIES).SetText(playerData.GetCurrentTourniquetFriends().ToString());
				m_StatsVisualization.Get(SCR_EHealingItemsModuleStats.SELINESELF).SetText(playerData.GetCurrentSelineSelf().ToString());
				m_StatsVisualization.Get(SCR_EHealingItemsModuleStats.SELINEFRIENDLIES).SetText(playerData.GetCurrentSelineFriends().ToString());
				m_StatsVisualization.Get(SCR_EHealingItemsModuleStats.MORPHINESELF).SetText(playerData.GetCurrentMorphineSelf().ToString());
				m_StatsVisualization.Get(SCR_EHealingItemsModuleStats.MORPHINEFRIENDLIES).SetText(playerData.GetCurrentMorphineFriends().ToString());
			}
#endif
		}
		m_fTimeSinceUpdate = 0;
	}

#ifdef ENABLE_DIAG
	//------------------------------------------------------------------------------------------------
	override void CreateVisualization()
	{
		super.CreateVisualization();
		if (!m_StatsVisualization)
			return;

		CreateEntry("Bandages on self: ", 0, SCR_EHealingItemsModuleStats.BANDAGESSELF);
		CreateEntry("Bandages on friendlies: ", 0, SCR_EHealingItemsModuleStats.BANDAGESFRIENDLIES);
		CreateEntry("Tourniquets on self: ", 0, SCR_EHealingItemsModuleStats.TOURNIQUETSSELF);
		CreateEntry("Tourniquets on friendlies: ", 0, SCR_EHealingItemsModuleStats.TOURNIQUETSFRIENDLIES);
		CreateEntry("seline on self: ", 0, SCR_EHealingItemsModuleStats.SELINESELF);
		CreateEntry("seline on friendlies: ", 0, SCR_EHealingItemsModuleStats.SELINEFRIENDLIES);
		CreateEntry("Morphine on self: ", 0, SCR_EHealingItemsModuleStats.MORPHINESELF);
		CreateEntry("Morphine on friendlies: ", 0, SCR_EHealingItemsModuleStats.MORPHINEFRIENDLIES);
	}
#endif
};

#ifdef ENABLE_DIAG
enum SCR_EHealingItemsModuleStats
{
	BANDAGESSELF,
	BANDAGESFRIENDLIES,
	TOURNIQUETSSELF,
	TOURNIQUETSFRIENDLIES,
	SELINESELF,
	SELINEFRIENDLIES,
	MORPHINESELF,
	MORPHINEFRIENDLIES
};
#endif
