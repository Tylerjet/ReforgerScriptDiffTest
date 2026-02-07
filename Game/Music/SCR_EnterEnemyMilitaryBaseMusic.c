class SCR_EnterEnemyMilitaryBaseMusic : ScriptedMusic
{
	MusicManager m_MusicManager;
	
	//~ Delay to make sure the OnSpawn music is played correctly after the respawn menu music
	protected void OnEnemyBaseEntered()
	{	
		m_MusicManager.Play(SCR_SoundEvent.SOUND_ONENTERINGENEMYBASE);	
	}
	
	override void Init() 
	{
		ChimeraWorld world = GetGame().GetWorld();

		if (!world)
			return;
		
		m_MusicManager = world.GetMusicManager();
		if (!m_MusicManager)
			return;
		
		
		SCR_GameModeCampaignMP.s_OnEnterEnemyBase.Insert(OnEnemyBaseEntered);
	}
	
	override void OnDelete()
	{
		SCR_GameModeCampaignMP.s_OnEnterEnemyBase.Remove(OnEnemyBaseEntered);
	}
}