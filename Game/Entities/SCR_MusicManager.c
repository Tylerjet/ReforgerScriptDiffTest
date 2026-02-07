[EntityEditorProps(category: "GameScripted/Sound", description: "Music Manager", color: "0 0 255 255")]
class SCR_MusicManagerClass: MusicManagerClass
{
};
class SCR_MusicManager: MusicManager
{
	protected static SCR_MusicManager s_Instance;
	protected EventHandlerManagerComponent m_EventHandlerManager;
	
	protected MusicManagerController m_MusicManagerController;
	protected bool m_bIsPlayingAmbientMusic = true;
	protected bool m_bIsPlayingPriorityAmbientMusic = false;
	
	//Local only. Normally disabling ambient music locally also disables priority ambient music. In specific situations (like entering a base) this still needs to be true. If server disables ambient music then priority ambient music will never play
	protected bool m_bLocalAllowPriorityAmbientMusic = true; 
	
	/*!
	Get SCR_MusicManager instance
	\return music manager instance to play oneshots
	*/
	static SCR_MusicManager GetInstance()
	{
		if (!s_Instance)
		{
			MusicManagerController musicController = MusicManagerController.GetController();
			if (!musicController) 
				return null;
		
			s_Instance = SCR_MusicManager.Cast(musicController.GetMusicManager());
		}

		return s_Instance;
	}
	
	/*!
	Terminates One shot music and enables the automated ambient music system
	*/
	void TerminateOneShot()
	{
		if (!m_bIsPlayingAmbientMusic || m_bIsPlayingPriorityAmbientMusic)
		{
			m_MusicManagerController.Terminate();
			m_bIsPlayingAmbientMusic = true;
			m_bIsPlayingPriorityAmbientMusic = false;
		}
	}
	
	/*!
	Plays an ambient music one shot sound once
	Will never play if ambient music is disabled. 
	Ambient music is always disabled in editor. Add music here that GM should not hear
	If enabled it will request the music controller to play ONCE the song requested
	\param song Sound to play
	\param force: If true, it will play anyways
	\param persistent: if true, it won't get stopped by music disruptions (bullets/explosions)
	\param priority: 0 = min priority. Only one requested sound will play per frame.
	*/
	void PlayAmbientOneShot(string song, bool force, bool persistent, int priority = 0)
	{
		if (!m_MusicManagerController)
			return;
		
		if (!m_MusicManagerController.IsAmbientMusicEnabled())
			return;
		
		m_bIsPlayingPriorityAmbientMusic = false;
		m_bIsPlayingAmbientMusic = true;
		m_MusicManagerController.PlayOneShot(song, force, persistent, priority);
	}
	
	/*!
	Plays a Priority ambient music one shot sound once
	Priority ambient music are a kind of Ambient music that is played if the player enters an area, spawn music, etc. This is played in situations user does not want the ambient music to ever play but still want some specific ambient one shot to play.
	By default disabling Ambient music locally also disables priority ambient music but an exception can be made for it
	If server has disabled ambient music then priority ambient music will never play
	Priority ambient music is always disabled in editor. Use PlayMusicOneShot if you want the GM to hear it as well.
	Will request the music controller to play ONCE the song requested
	\param song Sound to play
	\param force: If true, it will play anyways
	\param persistent: if true, it won't get stopped by music disruptions (bullets/explosions)
	\param priority: 0 = min priority. Only one requested sound will play per frame.
	*/
	void PlayPriorityAmbientOneShot(string song, bool force, bool persistent = true, int priority = 10)
	{
		if (!m_MusicManagerController)
			return;
		
		if (!IsAmbientMusicAuthorizedByServer() || !m_bLocalAllowPriorityAmbientMusic)
			return;
		
		//Remove listen as it should not set ambient true on the next track
		if ((!m_bIsPlayingAmbientMusic || m_bIsPlayingPriorityAmbientMusic) && m_EventHandlerManager)
			m_EventHandlerManager.RemoveScriptHandler("BeforePlaying", this, this.OnNextTrackCalled, false);
		
		m_bIsPlayingAmbientMusic = false;
		m_bIsPlayingPriorityAmbientMusic = true;
		m_MusicManagerController.PlayOneShot(song, force, persistent, priority);
		
		//Delay registering with one frame to make sure that if any non-ambient music was playing it will deregister correctly
		GetGame().GetCallqueue().CallLater(DelayedRegisterNextTrack);
			
	}
	
	/*!
	Plays a music music one shot sound once
	Will request the music controller to play ONCE the song requested
	Music is always enabled in editor. Add music here that GM should hear
	\param song: Sound to play
	\param force: If true, it will play anyways
	\param persistent: if true, it won't get stopped by music disruptions (bullets/explosions)
	\param priority: 0 = min priority. Only one requested sound will play per frame. Music Generally should override any other audio. So keep priority of 100 or higher
	*/
	void PlayMusicOneShot(string song, bool force, bool persistent = true, int priority = 100)
	{
		if (!m_MusicManagerController)
			return;
		
		//Remove listen as it should not set ambient true on the next track
		if ((!m_bIsPlayingAmbientMusic || m_bIsPlayingPriorityAmbientMusic) && m_EventHandlerManager)
			m_EventHandlerManager.RemoveScriptHandler("BeforePlaying", this, this.OnNextTrackCalled, false);
		
		m_bIsPlayingAmbientMusic = false;
		m_bIsPlayingPriorityAmbientMusic = false;
		m_MusicManagerController.PlayOneShot(song, force, persistent, priority);
		
		//Delay registering with one frame to make sure that if any non-ambient music was playing it will deregister correctly
		GetGame().GetCallqueue().CallLater(DelayedRegisterNextTrack);
	}
	
	//Register for next track being loaded in, which will set ambient music on true again
	protected void DelayedRegisterNextTrack()
	{
		//Register
		if (m_EventHandlerManager)
			m_EventHandlerManager.RegisterScriptHandler("BeforePlaying", this, this.OnNextTrackCalled, false);
	}
	
	//Next track is called so set isPlaying ambient true again
	protected void OnNextTrackCalled()
	{
		m_bIsPlayingAmbientMusic = true;
		m_bIsPlayingPriorityAmbientMusic = false;
		
		//unregister
		if (m_EventHandlerManager)
			m_EventHandlerManager.RemoveScriptHandler("BeforePlaying", this, this.OnNextTrackCalled, false);
	}
	
	/*!
	Enables/disables ambient music for all clients
	\param value: true to unmute clients, false to mute them
	\param playerAlteredAuthorization player ID of player that changed the value for notifications
	*/
	void AuthorizeAmbientMusicForClients(bool value, int playerAlteredAuthorization = -1)
	{
		if (Replication.IsClient() || !m_MusicManagerController)
			return;
		
		if (IsAmbientMusicAuthorizedByServer() == value)
			return;
		
		m_MusicManagerController.AuthorizeAmbientMusicForClients(value);
		
		if (!value)
		{
			//Terminate any ambient music that was already playing
			BroadcastTerminateAmbientMusic();
			Rpc(BroadcastTerminateAmbientMusic);
		}
		
		if (playerAlteredAuthorization > 0)
		{
			if (value)
				SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_ATTRIBUTES_ENABLED_AMBIENT_MUSIC, playerAlteredAuthorization);
			else 
				SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_ATTRIBUTES_DISABLED_AMBIENT_MUSIC, playerAlteredAuthorization);
		}
	}
	
	//Terminate any ambient music that was already playing
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void BroadcastTerminateAmbientMusic()
	{
		if (GetIsPlayingAmbientOrPriorityAmbientMusic())
		{
			if (m_bIsPlayingPriorityAmbientMusic && m_EventHandlerManager)
				m_EventHandlerManager.RemoveScriptHandler("BeforePlaying", this, this.OnNextTrackCalled, false);
			
			m_bIsPlayingAmbientMusic = false;
			m_bIsPlayingPriorityAmbientMusic = false;
			m_MusicManagerController.Terminate();
		}
	}
	
		
	/*!
	Enables/disables ALL music music for all clients
	\param mute: true to mute clients, false to unmute them
	\param playerAlteredMute player ID of player that changed the mute for notifications
	*/
	void MuteMusicForClients(bool mute, int playerAlteredMute = -1)
	{
		if (Replication.IsClient() || !m_MusicManagerController)
			return;
		
		if (IsMuted() == mute)
			return;
		
		m_MusicManagerController.MuteClients(mute);
		
		if (playerAlteredMute > 0)
		{
			if (mute)
				SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_ATTRIBUTES_MUTE_MUSIC, playerAlteredMute);
			else 
				SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_ATTRIBUTES_UNMUTE_MUSIC, playerAlteredMute);
		}
	}
	
	
	/*!
	Enables ambient music specifically for current instance of the game.
	Careful! For a client to play ambient music they need to be authorized by the server + have it enabled!
	Ambient music will NOT be enabled without server authorization.
	*/
	void EnableLocalAmbientMusic()
	{
		if (!m_MusicManagerController)
			return;
		
		TerminateOneShot();
		m_bLocalAllowPriorityAmbientMusic = true;
		m_MusicManagerController.EnableAmbientMusic(true);
	}
	
	/*!
	Disables ambient music specifically for current instance of the game.
	Careful! For a client to play ambient music they need to be authorized by the server + have it enabled!
	Ambient music will NOT be enabled without server authorization.
	\param allowPriorityAmbientMusic: false it will also disable priority ambient music, True it will still allow priority ambient music to be played (Local only and if server authorization music is disabled it will still never play)
	*/
	void DisableLocalAmbientMusic(bool allowPriorityAmbientMusic = false)
	{
		if (!m_MusicManagerController)
			return;
		
		if (GetIsPlayingAmbientOrPriorityAmbientMusic())
		{
			if (m_bIsPlayingAmbientMusic)
			{
				m_MusicManagerController.Terminate();
			}
			else if (m_bIsPlayingPriorityAmbientMusic && !allowPriorityAmbientMusic)
			{				
				if (m_EventHandlerManager)
					m_EventHandlerManager.RemoveScriptHandler("BeforePlaying", this, this.OnNextTrackCalled, false);
				
				m_bIsPlayingPriorityAmbientMusic = false;
				m_MusicManagerController.Terminate();
			}
		}
			
		m_bIsPlayingAmbientMusic = false;
		m_bLocalAllowPriorityAmbientMusic = allowPriorityAmbientMusic;
		m_MusicManagerController.EnableAmbientMusic(false);
	}
	
	/*!
	Get if currently playing ambient music or priority ambient music
	\return true if currently playing ambient music or priority ambient music. Will always return false if ambient music and priority ambient music are not playing
	*/
	bool GetIsPlayingAmbientOrPriorityAmbientMusic()
	{		
		return m_bIsPlayingAmbientMusic || m_bIsPlayingPriorityAmbientMusic;
	}
	
	
	override protected void EOnInit(IEntity owner)
	{
		if (SCR_Global.IsEditMode(this)) 
			return;
		
		m_MusicManagerController = MusicManagerController.GetController();
		if (!m_MusicManagerController)
		{
			Print("SCR_MusicManager could not find MusicManagerController!", LogLevel.ERROR);
			return;
		}
		
		if (GetInstance() != this)
			Print("More then one SCR_MusicManager in the world! This will cause issues", LogLevel.WARNING);
		
		m_EventHandlerManager = EventHandlerManagerComponent.Cast(owner.FindComponent(EventHandlerManagerComponent));
		if (!m_EventHandlerManager)
			Print("SCR_MusicManager could not find EventHandlerManagerComponent!", LogLevel.ERROR);
	}
		
	
	void SCR_MusicManager(IEntitySource src, IEntity parent)
	{
		SetFlags(EntityFlags.ACTIVE, false);
		SetEventMask(EntityEvent.INIT);
	}
};
