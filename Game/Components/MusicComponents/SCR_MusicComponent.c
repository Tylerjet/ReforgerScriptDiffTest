[EntityEditorProps(category: "GameScripted/Sound", description: "Music component", color: "0 0 255 255")]
class SCR_MusicComponentClass : ScriptComponentClass
{
};

enum EMusicManagerSignal
{
	TimeOfDay,
	RainIntensity,
	WindSpeed
};

//------------------------------------------------------------------------------------------------
class SCR_MusicComponent : ScriptComponent
{	
	protected SCR_MusicManager m_MusicManager;
	protected GameSignalsManager m_GameSignalsManager;
		
	protected ref array<int> m_aMusicManagerSignalIndex = new array<int>;	
				
	// ---------------------------------------------------------
	// Respawn menu events -------------------------------------
	// ---------------------------------------------------------
	protected void OnRespawnMenuOpen()
	{		
		GetGame().GetCallqueue().CallLater(OnRespawnMenuOpenDelay, 100);
	}
	
	//~ Delay to make sure the Respawn menu music is played correctly after the On death music
	protected void OnRespawnMenuOpenDelay()
	{		
		m_MusicManager.PlayMusicOneShot("SOUND_RESPAWNMENU", true, true);
	}
	
	//~ On Spawn
	protected void OnPlayerSpawned()
	{	
		GetGame().GetCallqueue().CallLater(OnPlayerSpawnedDelay, 100);
	}
	
	//~ Delay to make sure the OnSpawn music is played correctly after the respawn menu music
	protected void OnPlayerSpawnedDelay()
	{
		//~ Use as Priority music to make sure it is played even if spawned within own HQ
		m_MusicManager.PlayPriorityAmbientOneShot("SOUND_ONSPAWN", true, false, 1);	
	}
		
	// ---------------------------------------------------------
	// Character controller events -----------------------------
	// ---------------------------------------------------------
	
	//~ When the player died
	protected void OnPlayerDied(int playerId, IEntity player, IEntity killer)
	{	
		if (playerId != SCR_PlayerController.GetLocalPlayerId())
			return;
		
		//~ Use as Priority music as otherwise it will not be played in HQ
		m_MusicManager.PlayPriorityAmbientOneShot("SOUND_ONDEATH", true, false, 1);
	}
	
	//~ When the player is deleted by GM
	protected void OnPlayerDeleted(int playerId, IEntity player)
	{
		//~ If function used uncomment code below
		//if (playerId != SCR_PlayerController.GetLocalPlayerId())
		//	return;
	}
	
		
	// ---------------------------------------------------------
	// ---------------------------------------------------------
	// ---------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{	
		if (SCR_Global.IsEditMode()) 
			return;
		
		ChimeraWorld world = owner.GetWorld();

		if (!world)
			return;
		
		m_MusicManager = SCR_MusicManager.GetInstance();
		if (!m_MusicManager)
			return;
		
		// Get GameSignalsManager
		m_GameSignalsManager = GetGame().GetSignalsManager();
		
		SetEventMask(owner, EntityEvent.INIT);
		
		// Get MusicManager signals Idx
		typename enumType = EMusicManagerSignal;
		int size = enumType.GetVariableCount();
		
		for (int i = 0; i < size; i++)
		{
			m_aMusicManagerSignalIndex.Insert(m_MusicManager.GetSignalIndex(typename.EnumToString(EMusicManagerSignal, i)));
		}
	}
	
	override void EOnInit(IEntity owner)
	{										
		// Register respawn menu screen events
		SCR_RespawnSuperMenu.Event_OnMenuShow.Insert(OnRespawnMenuOpen);
		
		//~ On player spawned and died register
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (gameMode)
		{
			gameMode.GetOnPlayerKilled().Insert(OnPlayerDied);
			gameMode.GetOnPlayerDeleted().Insert(OnPlayerDeleted);
		}
		
		SCR_RespawnComponent.s_OnSpawn.Insert(OnPlayerSpawned);
	}
	
	//this function will be called before the music track is played.
	protected void BeforePlaying()
	{		
		m_MusicManager.SetSignalValue(m_aMusicManagerSignalIndex[EMusicManagerSignal.TimeOfDay], m_GameSignalsManager.GetSignalValue(m_GameSignalsManager.AddOrFindSignal("TimeOfDay")));
		m_MusicManager.SetSignalValue(m_aMusicManagerSignalIndex[EMusicManagerSignal.RainIntensity], m_GameSignalsManager.GetSignalValue(m_GameSignalsManager.AddOrFindSignal("RainIntensity")));
		m_MusicManager.SetSignalValue(m_aMusicManagerSignalIndex[EMusicManagerSignal.WindSpeed], m_GameSignalsManager.GetSignalValue(m_GameSignalsManager.AddOrFindSignal("WindSpeed")));
	}
	
	override void OnDelete(IEntity owner)
	{
		SCR_RespawnSuperMenu.Event_OnMenuShow.Insert(OnRespawnMenuOpen);
		
		//~ On Player spawned deregister
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (gameMode)
		{
			gameMode.GetOnPlayerKilled().Remove(OnPlayerDied);
			gameMode.GetOnPlayerDeleted().Remove(OnPlayerDeleted);
		}
		
		SCR_RespawnComponent.s_OnSpawn.Remove(OnPlayerSpawned);
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_MusicComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_MusicComponent()
	{
	}
};