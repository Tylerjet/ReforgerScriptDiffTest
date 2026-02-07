class SCR_SpawnMusic : ScriptedMusic
{
	MusicManager m_MusicManager;
	
		//~ On Spawn
	protected void OnPlayerSpawned()
	{	
		//~ Todo: implement queing of music properly on the code side
		GetGame().GetCallqueue().CallLater(OnPlayerSpawnedDelay, 100);
	}
	
	//~ Delay to make sure the OnSpawn music is played correctly after the respawn menu music
	protected void OnPlayerSpawnedDelay()
	{		
		//~ Never play spawn music if player Main entity is dead/deleted
		if (SCR_PlayerController.GetLocalMainEntity() == null)
			return;
		
		m_MusicManager.Play(SCR_SoundEvent.SOUND_ONSPAWN);	
	}
	
	override void Init() 
	{
		ChimeraWorld world = GetGame().GetWorld();

		if (!world)
			return;
		
		m_MusicManager = world.GetMusicManager();
		if (!m_MusicManager)
			return;
		
		SCR_RespawnComponent.s_OnSpawn.Insert(OnPlayerSpawned);
	}
	
	override void OnDelete()
	{
		SCR_RespawnComponent.s_OnSpawn.Remove(OnPlayerSpawned);
	}
}