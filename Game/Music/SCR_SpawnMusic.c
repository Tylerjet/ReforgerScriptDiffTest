class SCR_SpawnMusic : ScriptedMusic
{
	MusicManager m_MusicManager;
	
	//~ On Spawn
	protected void OnPlayerSpawned()
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