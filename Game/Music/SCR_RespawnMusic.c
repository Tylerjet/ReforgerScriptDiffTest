class SCR_RespawnMusic : ScriptedMusic
{
	MusicManager m_MusicManager;
	
	protected void OnRespawnMenuOpen()
	{		
		m_MusicManager.Play(SCR_SoundEvent.SOUND_RESPAWNMENU);
	}
		
	protected void OnRespawnMenuClosed()
	{
		m_MusicManager.Stop(SCR_SoundEvent.SOUND_RESPAWNMENU);
	}
	
	override void Init() 
	{
		ChimeraWorld world = GetGame().GetWorld();

		if (!world)
			return;
		
		m_MusicManager = world.GetMusicManager();
		if (!m_MusicManager)
			return;
		
			// Register respawn menu screen events
		SCR_RespawnSuperMenu.Event_OnMenuShow.Insert(OnRespawnMenuOpen);
		SCR_RespawnSuperMenu.Event_OnMenuHide.Insert(OnRespawnMenuClosed);
		SCR_RespawnSuperMenu.Event_OnMenuClose.Insert(OnRespawnMenuClosed);
	}
	
	override void OnDelete()
	{
		SCR_RespawnSuperMenu.Event_OnMenuShow.Remove(OnRespawnMenuOpen);
		SCR_RespawnSuperMenu.Event_OnMenuHide.Remove(OnRespawnMenuClosed);
		SCR_RespawnSuperMenu.Event_OnMenuClose.Remove(OnRespawnMenuClosed);
	}
}