[EntityEditorProps(category: "GameScripted/Sound", description: "Music component", color: "0 0 255 255")]
class SCR_MusicEditorComponentClass: SCR_BaseEditorComponentClass
{
};
class SCR_MusicEditorComponent: SCR_BaseEditorComponent
{
	SCR_EditorManagerEntity m_EditorManager;
	protected bool m_bMusicWasMutedByEditor;
	protected SCR_MusicManager m_MusicManager;
	
	protected void OnEditorOpened()
	{
		//Add a short delay to make sure any ambient music that started to play is terminated correctly
		GetGame().GetCallqueue().CallLater(OpenedEditorDelay, 500);
	}
	
	protected void OpenedEditorDelay()
	{
		if (!m_EditorManager.IsOpened() || m_EditorManager.IsLimited())
			return;
		
		m_bMusicWasMutedByEditor = true;
		m_MusicManager.TerminateOneShot();
		m_MusicManager.DisableLocalAmbientMusic();
	}
	
	protected void OnEditorClosed()
	{
		bool playerInHQ = false;
		
		//~ Make sure to check if player is in HQ and not enable abmient music again
		SCR_GameModeCampaignMP GameModeCampaign = SCR_GameModeCampaignMP.GetInstance();
		if (GameModeCampaign)
		{
			SCR_CampaignFaction pFaction = SCR_CampaignFaction.Cast(GameModeCampaign.GetLastPlayerFaction());
		
			if (pFaction)
				playerInHQ = pFaction.GetMainBase() && GameModeCampaign.IsPlayerInBase(pFaction.GetMainBase());
		}
		
		//Checks this in cause the limited mode changed
		if (m_bMusicWasMutedByEditor && !playerInHQ)
			m_MusicManager.EnableLocalAmbientMusic();
		
		m_bMusicWasMutedByEditor = false;
	}
	
	override void EOnEditorInit()
	{
		if (SCR_Global.IsEditMode()) 
			return;
		
		super.EOnEditorInit();
		
		m_MusicManager = SCR_MusicManager.GetInstance();
		if (!m_MusicManager)
			return;
		
		m_EditorManager = SCR_EditorManagerEntity.GetInstance();
		if (m_EditorManager)
		{
			m_EditorManager.GetOnOpened().Insert(OnEditorOpened);
			m_EditorManager.GetOnClosed().Insert(OnEditorClosed);
			
			if (m_EditorManager.IsOpened())
				OnEditorOpened();
		}
	}
	
	override void OnDelete(IEntity owner)
	{
		super.OnDelete(owner);
		
		if (m_EditorManager)
		{
			m_EditorManager.GetOnOpened().Remove(OnEditorOpened);
			m_EditorManager.GetOnClosed().Remove(OnEditorClosed);
		}
	}
};
