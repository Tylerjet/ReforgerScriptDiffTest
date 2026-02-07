
/** @file */


// -------------------------------------------------------------------------
// parent class for invoking custom analytics events
//
//
class StatsApi
{

	void StatsApi()
	{
	}

	void ~StatsApi()
	{
	}

	/**
	\brief Allow sending of generic application event with custom data
	*/
	proto native void ApplicationEvent();

	/**
	\brief Allow sending of editor start event
	*/
	proto native int EditorStart();
	
	/**
	\brief Allow sending of generic editor event with custom data
	*/
	proto native int EditorEvent();
	
	/**
	\brief Allow sending of editor close event
	*/
	proto native int EditorClosed();

	/**
	\brief Increment Editor usage counter for a specific Player
	*/
	proto native void IncrementEditorCounter(int iPlayerID);

	/**
	\brief Allow sending of generic mod related event with custom data
	*/
	proto native int ModEvent(Managed params);

	/**
	\brief Allow sending of generic player event with custom data
	*/
	proto native int PlayerEvent( int iPlayerID, Managed params );
	
	/**
	\brief Allow sending of player score event with custom data
	*/
	proto native int PlayerScore( int iPlayerID, Managed params  );

	/**
	\brief Allow sending of generic session event with custom data
	*/
	proto native void SessionEvent();


	/**
	\brief Event generated when Aplication start
	*/
	void OnApplicationInit()
	{
	}

	/**
	\brief Generic Application event
	*/
	void OnApplicationEvent()
	{
	}

	/**
	\brief Event generated when Aplication is closed
	*/
	void OnApplicationClose()
	{
	}

	/**
	\brief Event generated when Editor is opened
	*/
	void OnEditorStart()
	{
	}

	/**
	\brief Generic Editor event
	*/
	void OnEditorEvent()
	{
	}

	/**
	\brief Event generated when Editor is closed
	*/
	void OnEditorClosed()
	{
	}

	/**
	\brief Generic event related to Mods
	*/
	void OnModEvent()
	{
	}

	/**
	\brief Event generated on Player connection to MP game
	*/
	void OnPlayerConnect( int iPlayerID )
	{
	}

	/**
	\brief Event generated when Player disconnects from MP game
	*/
	void OnPlayerDisconnect( int iPlayerID )
	{
	}
	
	/**
	\brief Event generated when Player is kicked from MP game
	*/
	void OnPlayerKicked( int iPlayerID )
	{
	}

	/**
	\brief Generic Player event
	*/
	void OnPlayerEvent( int iPlayerID )
	{
	}

	/**
	\brief Event generated when Player is spawned in MP game
	*/
	void OnPlayerSpawn( int iPlayerID )
	{
	}

	/**
	\brief Event generated when Player score is updated in MP game
	*/
	void OnPlayerScore( int iPlayerID )
	{
	}

	/**
	\brief Event generated when Player dies during MP game
	*/
	void OnPlayerDie( int iPlayerID )
	{
	}

	/**
	\brief Event generated when new Session is started
	*/
	void OnSessionStart()
	{
	}

	/**
	\brief Generic Session event
	*/
	void OnSessionEvent()
	{
	}

	/**
	\brief Event generated when new Session is ended
	*/
	void OnSessionEnd()
	{
	}


}


