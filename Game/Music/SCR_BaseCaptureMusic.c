class SCR_BaseCaptureMusic : ScriptedMusic
{
	MusicManager m_MusicManager;
	
	//~ Delay to make sure the OnSpawn music is played correctly after the respawn menu music
	protected void OnBaseCapture()
	{		
		m_MusicManager.Play(SCR_SoundEvent.SOUND_ONBASECAPTURE);	
	}
	
	override void Init() 
	{
		ChimeraWorld world = GetGame().GetWorld();

		if (!world)
			return;
		
		m_MusicManager = world.GetMusicManager();
		if (!m_MusicManager)
			return;
		
		
		SCR_CampaignBase.s_OnBaseCapture.Insert(OnBaseCapture);
	}
	
	override void OnDelete()
	{
		SCR_CampaignBase.s_OnBaseCapture.Remove(OnBaseCapture);
	}
}