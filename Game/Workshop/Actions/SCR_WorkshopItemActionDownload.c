//! Action for downloading a specific revision of a mod

void ScriptInvoker_ActionDownloadProgress(SCR_WorkshopItemActionDownload action, int progressSize);
typedef func ScriptInvoker_ActionDownloadProgress;

class SCR_WorkshopItemActionDownload : SCR_WorkshopItemAction
{
	protected ref Revision m_StartVersion;		// Version which was when the download was started
	protected ref Revision m_TargetRevision;
	
	protected bool m_bTargetVersionLatest;	// When true, means we want to download latest version
	
	protected ref SCR_WorkshopCallbackBase m_Callback;
	
	protected float m_fSizeBytes; // Total size of this download.
	
	// True if this action is waiting for details to start the download
	protected bool m_bWaitingForRevisions;
	
	// True when the actual download has been started
	protected bool m_bDownloadStarted;
	
	protected float m_fProgress;
	protected float m_fDownloadedSize;
	
	// State of enabled state when this action was activated
	protected bool m_bAddonEnabledAtStart;
	
	protected ref ScriptInvokerBase<ScriptInvoker_ActionDownloadProgress> m_OnDownloadProgress;
	
	//-----------------------------------------------------------------------------------------------
	ScriptInvokerBase<ScriptInvoker_ActionDownloadProgress> GetOnDownloadProgress()
	{
		if (!m_OnDownloadProgress)
			m_OnDownloadProgress = new ScriptInvokerBase<ScriptInvoker_ActionDownloadProgress>();
		
		return m_OnDownloadProgress;
	}
	
	//-----------------------------------------------------------------------------------------------
	void SCR_WorkshopItemActionDownload(SCR_WorkshopItem wrapper, bool latestVersion, Revision revision = null)
	{
		m_bTargetVersionLatest = latestVersion;
		m_TargetRevision = revision;
		m_StartVersion = wrapper.GetCurrentLocalRevision();
		
		// todo resolve download size
		m_fSizeBytes = m_Wrapper.GetSizeBytes();
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Returns the target version.
	//! !! If you were targeting latest version, but the actual download has not started because it
	//! is waiting for more data from backend, then this will return an empty string until the
	//! data is fetched from backend.
	Revision GetTargetRevision()
	{
		return m_TargetRevision;
	}
	
	
	//-----------------------------------------------------------------------------------------------
	//! Returns the version which was at the moment when the download was started
	Revision GetStartRevision()
	{
		return m_StartVersion;
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
		return m_StartVersion;
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
		SCR_WorkshopCallbackBase callback = new SCR_WorkshopCallbackBase();
		callback.m_OnError.Insert(Callback_OnError);
		callback.m_OnTimeout.Insert(Callback_OnTimeout);
		return callback;
	}
	
	//-----------------------------------------------------------------------------------------------
	protected override bool OnActivate()
	{	
		m_fDownloadedSize = 0;
		
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
			m_Callback = CreateCallback();
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
		
		#ifdef WORKSHOP_DEBUG
		_print("StartDownloadOrLoadDetails(): Revisions are loaded, starting the download");
		#endif
		
		// Instant fail if mod is restricted
		// We know that it's restricted only after we have loaded the details
		if (m_Wrapper.GetRestricted())
		{
			Fail();
			return false;
		}
		
		m_Callback = CreateCallback();
		
		if (m_bTargetVersionLatest)
		{
			m_TargetRevision = m_Wrapper.GetLatestRevision();
			#ifdef WORKSHOP_DEBUG
			_print("StartDownloadOrLoadDetails(): Download of latest version was requested");
			#endif
		}
		
		#ifdef WORKSHOP_DEBUG
		_print(string.Format("StartDownloadOrLoadDetails(): Target version: %1", m_TargetRevision.GetVersion()));
		#endif
			
		bool success = m_Wrapper.Internal_StartDownload(m_TargetRevision, m_Callback);
		
		m_bWaitingForRevisions = false;
		m_bDownloadStarted = true;
		
		return success && !IsFailed(); // The workshop API might have called the Error handler by now
	}
	
	//-----------------------------------------------------------------------------------------------
	override void Internal_Update(float timeSlice)
	{
		if (!m_Wrapper)
			return;
		
		// Check if comms have failed
		if (m_State != STATE_COMPLETED && m_State != STATE_FAILED)
		{
			if (m_Wrapper.GetRequestFailed())
				Fail();
		}
		
		// Complete the action when the addon is at the target revisioon and is downloaded
		if (m_State == STATE_ACTIVE)
		{
			if (m_bWaitingForRevisions)
			{
				// Try to start the download again if we have loaded the details
				if (m_Wrapper.GetRevisionsLoaded())
					StartDownloadOrLoadDetails();
			}
			else
			{
				bool offline = m_Wrapper.GetOffline();
				Revision currentVersion = m_Wrapper.GetCurrentLocalRevision();
				
				//#ifdef WORKSHOP_DEBUG
				//_print(string.Format("Internal_Update: Offline: %1, CurrentVersion: '%2', TargetVersion: '%3'", offline, currentVersion, m_sTargetVersion));
				//#endif
				
				float progress = m_Wrapper.Internal_GetDownloadProgress();
				if (progress > m_fProgress)
				{					
					int currentDownloadSize = m_Wrapper.GetSizeBytes() * progress;
					
					InvokeOnChanged();
					if (m_OnDownloadProgress)
						m_OnDownloadProgress.Invoke(this, currentDownloadSize - m_fDownloadedSize);
					
					m_fProgress = progress;
					m_fDownloadedSize = currentDownloadSize;
				}
				
				if (offline && Revision.AreEqual(currentVersion, m_TargetRevision))
				{
					Complete();
				}
			}
		}
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void Callback_OnError(SCR_WorkshopCallbackBase callback, int code, int restCode, int apiCode)
	{
		//if (code != EBackendError.EBERR_INVALID_STATE) // Ignore this one for now // EApiCode.EACODE_ERROR_OK
		
		Fail();
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void Callback_OnTimeout()
	{
		Fail();
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Try redownload failed addon
	void RetryDownload()
	{
		if (m_Wrapper.GetOffline())
			m_Wrapper.DeleteLocally();
		
		m_Wrapper.RetryDownload(m_TargetRevision);
	}
	
	//-----------------------------------------------------------------------------------------------
	void ForceFail()
	{
		Fail();
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Return size required for current revision
	float GetRequiredDownloadSize()
	{
		if (!m_Wrapper || !m_TargetRevision)
			return -1;
		
		// Downloading from beginning - get whole size
		if (!m_StartVersion)
			return m_Wrapper.GetSizeBytes();
		
		// Get size of update
		float size;
		m_TargetRevision.GetPatchSize(size);
		
		return size;
	}
	
	//-----------------------------------------------------------------------------------------------
	float GetDownloadSize()
	{
		return m_fDownloadedSize;
	}
};