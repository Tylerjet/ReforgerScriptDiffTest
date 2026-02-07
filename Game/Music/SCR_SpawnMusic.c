class SCR_SpawnMusic : ScriptedMusic
{
	MusicManager m_MusicManager;
	
	//~ On Spawn
	protected void OnPlayerSpawned()
	{	
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
		
		SCR_RespawnComponent.SGetOnSpawn().Insert(OnPlayerSpawned);
	}
	
	override void OnDelete()
	{
		SCR_RespawnComponent.SGetOnSpawn().Remove(OnPlayerSpawned);
	}
}