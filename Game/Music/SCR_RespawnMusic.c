class SCR_RespawnMusic : ScriptedMusic
{
	MusicManager m_MusicManager;
	
	protected void OnRespawnMenuOpen()
	{		
		GetGame().GetCallqueue().CallLater(m_MusicManager.Play, 300, false, SCR_SoundEvent.SOUND_RESPAWNMENU);
	}
		
	protected void OnRespawnMenuClosed()
	{
		GetGame().GetCallqueue().Remove(m_MusicManager.Play);
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