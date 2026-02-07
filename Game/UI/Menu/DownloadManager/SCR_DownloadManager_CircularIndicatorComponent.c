/*!
Component which visualizes state of download manager with the small circular indicator.
*/

class SCR_DownloadManager_CircularIndicatorComponent : ScriptedWidgetComponent
{
	protected const float FADE_OUT_WAIT_TIME = 2.5;
	
	protected ref SCR_DownloadManager_CircleProgressIndicatorWidgets m_Widgets = new SCR_DownloadManager_CircleProgressIndicatorWidgets();
	
	protected Widget m_wRoot;
	
	protected ref SCR_FadeInOutAnimator m_Animator;
	
	
	//------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wRoot = w;
		m_Widgets.Init(w);
		
		m_Animator = new SCR_FadeInOutAnimator(m_wRoot, WidgetAnimator.FADE_RATE_FAST, WidgetAnimator.FADE_RATE_SLOW, FADE_OUT_WAIT_TIME, fadeOutSetVisibleFalse: true);
		if (!SCR_Global.IsEditMode())
			m_Animator.FadeOutInstantly();
		
		// I can't use MenuManager.GetOwnerMenu because it can't find the menu on HandlerAttached, so we use call queue instead.
		GetGame().GetCallqueue().CallLater(OnFrame, 1, true);
		
		m_Widgets.m_ButtonComponent.m_OnClicked.Insert(OnButtonClick);
	}
	
	
	
	//------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{	
		GetGame().GetCallqueue().Remove(OnFrame);
	}
	
	
	
	//------------------------------------------------------------------------------------------
	protected void OnFrame()
	{
		if (!m_wRoot)
			return;

		float tDelta = ftime / 1000.0; // ftime is milliseconds!
		
		SCR_DownloadManager mgr = SCR_DownloadManager.GetInstance();
		if (mgr && !SCR_Global.IsEditMode())
		{
			UpdateAllWidgets();
		}
		
		m_Animator.Update(tDelta);
	}
	
	
	
	//------------------------------------------------------------------------------------------
	protected void UpdateAllWidgets()
	{
		SCR_DownloadManager mgr = SCR_DownloadManager.GetInstance();
		string downloadStateText;
		int nCompleted, nTotal;
		mgr.GetDownloadQueueState(nCompleted, nTotal);
		
				
		
		// Progress bar, progress text
		m_Widgets.m_DownloadDoneImage.SetVisible(nTotal == 0);
		m_Widgets.m_QueueSizeText.SetVisible(nTotal > 0);		
		if (nTotal > 0)
		{
			// Get total progress of all downloads
			array<ref SCR_WorkshopItemActionDownload> downloadQueue = mgr.GetDownloadQueue();
			float progress = SCR_DownloadManager.GetDownloadActionsProgress(downloadQueue);
			
			// Progress bar
			m_Widgets.m_ProgressCircle.SetMaskProgress(progress);
			
			// Queue size text
			string queueSizeText = string.Format("%1", nTotal - nCompleted);
			m_Widgets.m_QueueSizeText.SetText(queueSizeText);
		}
		else
		{
			m_Widgets.m_QueueSizeText.SetText(string.Empty);
			m_Widgets.m_ProgressCircle.SetMaskProgress(1.0);
		}
		
		m_Animator.ForceVisible(nTotal > 0);
	}
	
	
	
	//------------------------------------------------------------------------------------------
	protected void OnButtonClick()
	{
		SCR_DownloadManager_Dialog.Create();
	}
};