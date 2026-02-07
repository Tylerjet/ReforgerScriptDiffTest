/*
Specific workshop item actions, inherited from base action class.
*/


//! Action for downloading a specific revision of a mod
class SCR_WorkshopItemActionDownload : SCR_WorkshopItemAction
{	
	protected string m_sStartVersion;		// Version which was when the download was started
	protected string m_sTargetVersion;
	
	protected bool m_bTargetVersionLatest;	// When true, means we want to download latest version
	
	protected ref SCR_WorkshopCallbackBase m_Callback;
	
	protected float m_fSizeBytes; // Total size of this download.
	
	// True if this action is waiting for details to start the download
	protected bool m_bWaitingForRevisions;
	
	// True when the actual download has been started
	protected bool m_bDownloadStarted;
	
	protected float m_fProgress;
	
	// State of enabled state when this action was activated
	protected bool m_bAddonEnabledAtStart;
	
	//-----------------------------------------------------------------------------------------------
	void SCR_WorkshopItemActionDownload(SCR_WorkshopItem wrapper, bool latestVersion, string version = string.Empty)
	{
		m_bTargetVersionLatest = latestVersion;
		m_sTargetVersion = version;
		m_sStartVersion = wrapper.GetCurrentLocalVersion();
		
		// todo resolve download size
		m_fSizeBytes = m_Wrapper.GetSizeBytes();
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Returns the target version.
	//! !! If you were targeting latest version, but the actual download has not started because it
	//! is waiting for more data from backend, then this will return an empty string until the
	//! data is fetched from backend.
	string GetTargetVersion()
	{
		return m_sTargetVersion;
	}
	
	
	//-----------------------------------------------------------------------------------------------
	//! Returns the version which was at the moment when the download was started
	string GetStartVersion()
	{
		return m_sStartVersion;
	}
	
	//-----------------------------------------------------------------------------------------------
	bool GetTargetedAtLatestVersion()
	{
		return m_bTargetVersionLatest;
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Returns true when this is an update, false when regular download.
	bool GetUpdate()
	{
		return !m_sStartVersion.IsEmpty();
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Returns current download progress if active or paused, 1 if complete, otherwise 0.
	float GetProgress()
	{
		if (IsCompleted())
			return 1.0;
		return m_fProgress;
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Returns download size. Might return 0 until the actiion is activated.
	float GetSizeBytes()
	{
		return m_fSizeBytes;
	}
	
	//-----------------------------------------------------------------------------------------------
	protected SCR_WorkshopCallbackBase CreateCallback()
	{
		SCR_WorkshopCallbackBase callback = new SCR_WorkshopCallbackBase;
		callback.m_OnError.Insert(Callback_OnError);
		callback.m_OnTimeout.Insert(Callback_OnTimeout);
		return callback;
	}
	
	//-----------------------------------------------------------------------------------------------
	protected override bool OnActivate()
	{	
		// Check if addon was enabled, when action is finished it will be restored
		m_bAddonEnabledAtStart = m_Wrapper.GetEnabled();
		
		// If we have versions already, we start downloading immediately
		// Otherwise we load details and start waiting
		return StartDownloadOrLoadDetails();
	}
	
	//-----------------------------------------------------------------------------------------------
	protected override bool OnReactivate()
	{
		return OnActivate();
	}
	
	//-----------------------------------------------------------------------------------------------
	protected override bool OnCancel()
	{
		bool success = m_Wrapper.Internal_CancelDownload();
		
		return success && !IsFailed();
	}
	
	//-----------------------------------------------------------------------------------------------
	protected override bool OnPause()
	{
		bool success;
		
		if (m_bDownloadStarted)
			success = m_Wrapper.Internal_PauseDownload();
		else
			success = true;
		
		return success && !IsFailed();
	}
	
	//-----------------------------------------------------------------------------------------------
	protected override bool OnResume()
	{
		if (m_bDownloadStarted)
		{
			m_Callback = this.CreateCallback();
			bool success = m_Wrapper.Internal_ResumeDownload(m_Callback);		
			return success && !IsFailed();
		}
		else
		{
			// If download didn't actually start, we must not resume but start a download
			return StartDownloadOrLoadDetails();
		}
	}
	
	//-----------------------------------------------------------------------------------------------
	protected override void OnFail()
	{
		// Try to cancel the download at least
		m_Wrapper.Internal_CancelDownload();
	}
	
	//-----------------------------------------------------------------------------------------------
	protected override void OnComplete()
	{
		// Restore 'enabled' state
		m_Wrapper.SetEnabled(m_bAddonEnabledAtStart);
	}
	
	//-----------------------------------------------------------------------------------------------
	//! We can't start download before details are loaded, because details have the revisions.
	//! Starts download immediately or loads details of workshop item.
	protected bool StartDownloadOrLoadDetails()
	{
		#ifdef WORKSHOP_DEBUG
		_print("StartDownloadOrLoadDetails()");
		#endif
		
		if (m_Wrapper.GetRevisionsLoaded())
		{
			#ifdef WORKSHOP_DEBUG
			_print("StartDownloadOrLoadDetails(): Revisions are loaded, starting the download");
			#endif
			
			// Instant fail if mod is restricted
			// We know that it's restricted only after we have loaded the details
			if (m_Wrapper.GetRestricted())
			{
				this.Fail();
				return false;
			}
			
			m_Callback = this.CreateCallback();
			
			if (m_bTargetVersionLatest)
			{
				m_sTargetVersion = m_Wrapper.GetLatestVersion();
				#ifdef WORKSHOP_DEBUG
				_print("StartDownloadOrLoadDetails(): Download of latest version was requested");
				#endif
			}
			
			#ifdef WORKSHOP_DEBUG
			_print(string.Format("StartDownloadOrLoadDetails(): Target version: %1", m_sTargetVersion));
			#endif
				
			bool success = m_Wrapper.Internal_StartDownload(m_sTargetVersion, m_Callback);
			
			m_bWaitingForRevisions = false;
			m_bDownloadStarted = true;
			
			return success && !IsFailed(); // The workshop API might have called the Error handler by now
		}
		else
		{
			#ifdef WORKSHOP_DEBUG
			_print("StartDownloadOrLoadDetails(): Revisions are not loaded, waiting");
			#endif
			
			m_bWaitingForRevisions = true;
			m_Wrapper.LoadDetails();
			
			return true;
		}
	}
	
	//-----------------------------------------------------------------------------------------------
	override void Internal_Update()
	{
		if (!m_Wrapper)
			return;
		
		// Check if comms have failed
		if (m_State != STATE_COMPLETED && m_State != STATE_FAILED)
		{
			if (m_Wrapper.GetRequestFailed())
				this.Fail();
		}
		
		// Complete the action when the addon is at the target revisioon and is downloaded
		if (m_State == STATE_ACTIVE)
		{
			if (m_bWaitingForRevisions)
			{
				// Try to start the download again if we have loaded the details
				if (m_Wrapper.GetRevisionsLoaded())
					this.StartDownloadOrLoadDetails();
			}
			else
			{
				bool offline = m_Wrapper.GetOffline();
				string currentVersion = m_Wrapper.GetCurrentLocalVersion();
				
				//#ifdef WORKSHOP_DEBUG
				//_print(string.Format("Internal_Update: Offline: %1, CurrentVersion: '%2', TargetVersion: '%3'", offline, currentVersion, m_sTargetVersion));
				//#endif
				
				float progress = m_Wrapper.Internal_GetDownloadProgress();
				if (progress > m_fProgress)
				{					
					InvokeOnChanged();
					m_fProgress = progress;
				}
				
				if (offline && currentVersion == m_sTargetVersion)
				{
					this.Complete();
				}
			}
		}
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void Callback_OnError(SCR_WorkshopCallbackBase callback, int code, int restCode, int apiCode)
	{
		if (code != EBackendError.EBERR_INVALID_STATE) // Ignore this one for now // EApiCode.EACODE_ERROR_OK
			this.Fail();
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void Callback_OnTimeout()
	{
		this.Fail();
	}
};


//-----------------------------------------------------------------------------------------------
//! Action for downloading latest dependencies. Doesn't download this addon, but only downloads dependencies.
class SCR_WorkshopItemActionDownloadDependenciesLatest : SCR_WorkshopItemActionComposite
{	
	// Optionally it is possible to provide dependencies externally,
	// In this case only these will be downloaded.
	ref array<ref SCR_WorkshopItem> m_aProvidedDependencies;
	
	//-----------------------------------------------------------------------------------------------
	//! You must provide an array of dependencies to download.
	void SCR_WorkshopItemActionDownloadDependenciesLatest(SCR_WorkshopItem wrapper, array<ref SCR_WorkshopItem> dependencies)
	{
		if (dependencies.Count() > 0)
		{
			// Copy the provided dependencies
			m_aProvidedDependencies = new array<ref SCR_WorkshopItem>;
			foreach (auto dep : dependencies)
				m_aProvidedDependencies.Insert(dep);
			
			// Create actions for all dependencies
			#ifdef WORKSHOP_DEBUG
			_print("New() Creating actions:");
			#endif
			m_aActions.Clear();
			foreach(SCR_WorkshopItem dep : m_aProvidedDependencies)
			{
				if (!dep.GetOffline() || dep.GetUpdateAvailable())
				{
					#ifdef WORKSHOP_DEBUG
					_print(string.Format("  Dependency: %1 - Creating action", dep.GetName()));
					#endif
					
					SCR_WorkshopItemAction action = dep.DownloadLatestVersion();
					m_aActions.Insert(action);
				}
				else
				{
					#ifdef WORKSHOP_DEBUG
					_print(string.Format("  Dependency: %1 - Download is not required.", dep.GetName()));
					#endif
				}
			}
		}
	}
	
	//-----------------------------------------------------------------------------------------------
	protected override bool OnActivate()
	{	
		bool success = true;
		
		// Activate all created actions
		foreach (auto action : m_aActions)
		{
			if (action.IsPaused())
				success = success && action.Resume();
			else if (action.IsInactive())
				success = success && action.Activate();
		}
			
		return success;
	}
	
	//-----------------------------------------------------------------------------------------------
	protected override bool OnReactivate()
	{
		return OnActivate();
	}
	
	//-----------------------------------------------------------------------------------------------
	protected override bool OnCancel()
	{
		CancelAll();
		return true;
	}
	
	//-----------------------------------------------------------------------------------------------
	protected override bool OnPause()
	{
		PauseAll();
		return true;
	}
	
	//-----------------------------------------------------------------------------------------------
	protected override bool OnResume()
	{
		ResumeAll();
		return true;
	}
	
	//-----------------------------------------------------------------------------------------------
	protected override void OnFail()
	{
		// Can't do anything
	}
	
	//-----------------------------------------------------------------------------------------------
	override void Internal_Update()
	{
		if (!m_Wrapper)
			return;
		
		// Check if comms have failed
		if (m_State != STATE_COMPLETED && m_State != STATE_FAILED)
		{
			if (m_Wrapper.GetRequestFailed())
				this.Fail();
		}
		
		// Unregister dependnecy actions which were canceled
		array<SCR_WorkshopItemAction> removeActions = new array<SCR_WorkshopItemAction>;
		foreach (SCR_WorkshopItemAction action : m_aActions)
		{
			if (action.IsCanceled() || action.IsPaused() || action.IsFailed()) // todo should paused actions be removed too?
				removeActions.Insert(action);
		}
		UnregisterActions(removeActions);
		
		// Complete when all actions are completed
		if (AllCompleted())
		{
			this.Complete();
		}
	}
};


//-----------------------------------------------------------------------------------------------
//! Action for reporting an item
class SCR_WorkshopItemActionReport : SCR_WorkshopItemAction
{
	protected EWorkshopReportType m_eReportType;
	protected string m_sMessage;
	ref SCR_WorkshopCallbackBase m_Callback;
	
	
	//-----------------------------------------------------------------------------------------------
	void SCR_WorkshopItemActionReport(SCR_WorkshopItem wrapper, EWorkshopReportType eReport, string sMessage)
	{
		m_eReportType = eReport;
		m_sMessage = sMessage;
	}
	
	
	//-----------------------------------------------------------------------------------------------
	protected override bool OnActivate()
	{
		m_Callback = new SCR_WorkshopCallbackBase();
		m_Callback.m_OnTimeout.Insert(Callback_OnErrorOrTimeout);
		m_Callback.m_OnError.Insert(Callback_OnErrorOrTimeout);
		m_Callback.m_OnSuccess.Insert(Callback_OnSuccess);
		
		bool success = m_Wrapper.Internal_Report(m_eReportType, m_sMessage, m_Callback);
		
		return success;
	}
	
	//-----------------------------------------------------------------------------------------------
	protected override bool OnReactivate()
	{
		return this.OnActivate();
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void Callback_OnErrorOrTimeout()
	{
		this.Fail();
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void Callback_OnSuccess()
	{
		this.Complete();
	}
};



//-----------------------------------------------------------------------------------------------
//! Action for canceling report of an item
class SCR_WorkshopItemActionCancelReport : SCR_WorkshopItemAction
{
	ref SCR_WorkshopCallbackBase m_Callback;
	
	
	//-----------------------------------------------------------------------------------------------
	protected override bool OnActivate()
	{
		m_Callback = new SCR_WorkshopCallbackBase();
		m_Callback.m_OnTimeout.Insert(Callback_OnErrorOrTimeout);
		m_Callback.m_OnError.Insert(Callback_OnErrorOrTimeout);
		m_Callback.m_OnSuccess.Insert(Callback_OnSuccess);
		
		bool success = m_Wrapper.Internal_CancelReport(m_Callback);
		
		return success;
	}
	
	//-----------------------------------------------------------------------------------------------
	protected override bool OnReactivate()
	{
		return this.OnActivate();
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void Callback_OnErrorOrTimeout()
	{
		this.Fail();
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void Callback_OnSuccess()
	{
		this.Complete();
	}
};