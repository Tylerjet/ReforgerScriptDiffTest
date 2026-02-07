/*!
Download manager entry for handling addon download state
*/

//------------------------------------------------------------------------------------------------
void ScriptInvoker_DownloadManagerEntry(SCR_DownloadManagerEntry entry);
typedef func ScriptInvoker_DownloadManagerEntry;

//------------------------------------------------------------------------------------------------
class SCR_DownloadManagerEntry : SCR_ScriptedWidgetComponent
{
	// State messages
	protected const string STATE_DOWNLOADING_PERCENTAGE = "#AR-ValueUnit_Percentage";
	protected const string STATE_CANCELED = "#AR-Workshop_Canceled";
	protected const string STATE_NO_CONNECTION = "#AR-Workshop_WarningNoConnection";
	protected const string STATE_DOWNLOAD_FAIL = "#AR-Workshop_DownloadFail";
	
	[Attribute("{3262679C50EF4F01}UI/Textures/Icons/icons_wrapperUI.imageset", UIWidgets.ResourceNamePicker, desc: "Imageset for the icon", params: "imageset")]
	protected ResourceName m_sIconImageset;
	
	protected ref SCR_DownloadManagerEntryWidgets m_Widgets = new SCR_DownloadManagerEntryWidgets();
	protected ref SCR_BackendImageComponent m_BackendImage;
	
	protected ref SCR_WorkshopItemActionDownload m_Action;
	protected ref SCR_WorkshopItem m_Item;
	
	protected bool m_bShowButtons = false;
	protected EDownloadManagerActionState m_iState = EDownloadManagerActionState.INACTIVE;
	
	protected ref ScriptInvokerBase<ScriptInvoker_DownloadManagerEntry> m_OnUpdate;
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerBase<ScriptInvoker_DownloadManagerEntry> GetOnUpdate()
	{
		if (!m_OnUpdate)
			m_OnUpdate = new ScriptInvokerBase<ScriptInvoker_DownloadManagerEntry>();
		
		return m_OnUpdate;
	}
	
	//------------------------------------------------------------------------------------------------
	// Override 
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		m_Widgets.Init(w);
		
		m_Widgets.m_PauseBtnComponent.m_OnClicked.Insert(OnClickPause);
		m_Widgets.m_ResumeBtnComponent.m_OnClicked.Insert(OnClickResume);
		m_Widgets.m_CancelBtnComponent.m_OnClicked.Insert(OnClickCancel);
		m_Widgets.m_RetryBtnComponent.m_OnClicked.Insert(OnClickRetry);
		
		m_Widgets.m_HorizontalButtons.SetVisible(false);
		m_Widgets.m_PauseBtn.SetVisible(false);
		m_Widgets.m_ResumeBtn.SetVisible(false);
		m_Widgets.m_CancelBtn.SetVisible(false);
		m_Widgets.m_RetryBtn.SetVisible(false);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		m_bShowButtons = true;
		UpdateButtons();

		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW,int x, int y)
	{
		m_bShowButtons = false;
		UpdateButtons();
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	// Custom
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! Initializes the line in interactive mode.
	//! It will be able to interact with the download action.
	void InitForDownloadAction(SCR_WorkshopItem item, SCR_WorkshopItemActionDownload action)
	{
		m_Item = item;
		m_Action = action;
		
		m_Action.m_OnChanged.Insert(UpdateProgressWidgets);
		
		m_BackendImage = SCR_BackendImageComponent.Cast(GetRootWidget().FindHandler(SCR_BackendImageComponent));
		
		// Setup static 
		SetupWidgets();
		
		UpdateAllWidgets();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetupWidgets()
	{
		// Image 
		BackendImage img = m_Action.GetWorkshopItem().GetWorkshopItem().Thumbnail();
		m_BackendImage.SetImage(img);
		
		// Name
		m_Widgets.m_NameText.SetText(m_Action.GetAddonName());
		
		// Versions
		m_Widgets.m_VersionText.SetText(m_Action.GetTargetRevision().GetVersion());
	}
	
	//------------------------------------------------------------------------------------------------
	//! Based on current download action state setup state simplified for download manager UI grouping
	//! Fucntion is static to compare download action state before entry is created
	static EDownloadManagerActionState DownloadActionState(SCR_WorkshopItemActionDownload action)
	{
		if (!action)
		{
			Print("Download manager entry state can't be update due to missing download action");
			return EDownloadManagerActionState.INACTIVE;
		}
		
		// Running  
		if (!action.IsCompleted() && !action.IsFailed() && !action.IsCanceled())
			return EDownloadManagerActionState.RUNNING;
		
		// Failed
		if (action.IsFailed() || action.IsCanceled())
			return EDownloadManagerActionState.FAILED;
		
		// Downloaded
		if (action.IsCompleted() && !action.IsFailed() && !action.IsCanceled())
			return EDownloadManagerActionState.DOWNLOADED;
		
		// No state
		return EDownloadManagerActionState.INACTIVE;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Updates all widgets. Only relevant in the mode InitForDownloadAction
	protected void UpdateAllWidgets()
	{
		if (!m_Action)
			return;
		
		UpdateProgressWidgets();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Update all text displaying progress
	protected void UpdateProgressWidgets()
	{
		m_iState = DownloadActionState(m_Action);
		
		// Progress bar
		bool running = (m_iState == EDownloadManagerActionState.RUNNING); 
		
		m_Widgets.m_Progress.SetVisible(running || m_iState == EDownloadManagerActionState.FAILED);
		if (running)
			m_Widgets.m_ProgressComponent.SetValue(m_Action.GetProgress());
		
		// Download size
		string addonSize = SCR_ByteFormat.GetReadableSize(m_Action.GetSizeBytes());
		
		if (running || m_iState == EDownloadManagerActionState.FAILED)
		{
			// Show progress
			string downloadSize = SCR_ByteFormat.ContentDownloadFormat(m_Action.GetSizeBytes() * m_Action.GetProgress());
			
			m_Widgets.m_DownloadedText.SetText(string.Format("%1/%2", downloadSize, addonSize));
		}
		else
		{
			m_Widgets.m_DownloadedText.SetText(addonSize);
		}
		
		// Update buttons 
		UpdateButtons();
		
		// Show message 
		string msg, imgName;
		Color col;
		bool colorText;
		StateMessage(msg, imgName, col, colorText);
		
		m_Widgets.m_DownloadStateText.SetText(msg);
		m_Widgets.m_DownloadIcon.LoadImageFromSet(0, m_sIconImageset, imgName);
		
		if (col)
		{
			m_Widgets.m_DownloadIcon.SetColor(col);
			m_Widgets.m_DownloadStateText.SetColor(col);
			m_Widgets.m_ProgressComponent.SetSliderColor(col);
		}
		
		// Invoke
		if (m_OnUpdate)
			m_OnUpdate.Invoke(this);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateButtons()
	{
		m_Widgets.m_HorizontalButtons.SetVisible(m_bShowButtons);

		if (!m_bShowButtons)
			return;
		
		bool pause, resume, cancel, retry;
		CanDoActions(pause, resume, cancel, retry);
		
		m_Widgets.m_PauseBtn.SetVisible(pause);
		m_Widgets.m_ResumeBtn.SetVisible(resume);
		m_Widgets.m_CancelBtn.SetVisible(cancel);
		m_Widgets.m_RetryBtn.SetVisible(retry);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Store which actions can be used for entry in current state
	void CanDoActions(out bool canPause, out bool canResume, out bool canCancel, out bool canRetry)
	{	
		canPause = m_iState == EDownloadManagerActionState.RUNNING && !m_Action.IsPaused();
		canResume = m_iState == EDownloadManagerActionState.RUNNING && m_Action.IsPaused();
		canCancel = m_iState == EDownloadManagerActionState.RUNNING;
		canRetry = m_iState == EDownloadManagerActionState.FAILED;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Setup state message variables
	protected void StateMessage(out string message, out string imageName, out Color color, out bool colorText)
	{
		// Downloading
		if (m_Action.IsActive() && !m_Action.IsCompleted() && !m_Action.IsPaused())
		{
			float progress = m_Action.GetProgress();
			string progressStr = WidgetManager.Translate(STATE_DOWNLOADING_PERCENTAGE, Math.Round(progress * 100.0));
			
			message = progressStr;
			imageName = "downloading";
			color = UIColors.CONTRAST_CLICKED_HOVERED;
			colorText = false;
			return;
		}
			
		// Paused 
		if (m_Action.IsPaused())
		{
			float progress = m_Action.GetProgress();
			string progressStr = WidgetManager.Translate(STATE_DOWNLOADING_PERCENTAGE, Math.Round(progress * 100.0));
			
			message = progressStr;
			imageName = "download-pause";
			color = UIColors.CONTRAST_DEFAULT;
			colorText = false;
			return;
		}
		
		// Canceled 
		if (m_Action.IsCanceled())
		{
			message = STATE_CANCELED; // TODO:should be canceled
			imageName = "cancelCircle";
			color = UIColors.WARNING;
			colorText = true;
			return;
		}
		
		// Failed 
		if (m_Action.IsFailed())
		{
			message = FailReason();
			imageName = "warning";
			color = UIColors.WARNING;
			colorText = true;
			return;
		}
		
		// Success
		if (m_Action.IsCompleted())
		{
			message = "#AR-Workshop_TabName_Downloaded";
			imageName = "okCircle";
			color = UIColors.CONFIRM;
			colorText = true;
			return;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return message with details why entry wasn't downloaded
	protected string FailReason()
	{
		// Communication with server failed
		if (!GetGame().GetBackendApi().IsActive())
			return STATE_NO_CONNECTION;
		
		// Addon specific fail
		return STATE_DOWNLOAD_FAIL;
	}
	
	//------------------------------------------------------------------------------------------------
	void OnClickPause()
	{
		if (!m_Action)
			return;
		
		if (m_Action.IsActive() && !m_Action.IsPaused())
			m_Action.Pause();
		
		UpdateProgressWidgets();
	}
	
	//------------------------------------------------------------------------------------------------
	void OnClickResume()
	{
		if (!m_Action)
			return;
		
		if (m_Action.IsPaused())
			m_Action.Resume();
		else if (m_Action.IsInactive())
			m_Action.Activate();
		
		UpdateProgressWidgets();
	}
	
	//------------------------------------------------------------------------------------------------
	void OnClickCancel()
	{
		if (m_Action)
			m_Action.Cancel();
		
		UpdateProgressWidgets();
	}
	
	//------------------------------------------------------------------------------------------------
	void OnClickRetry()
	{
		if (m_Action)
			m_Action.RetryDownload();

		UpdateProgressWidgets();
	}
	
	//------------------------------------------------------------------------------------------------
	// Get set
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	SCR_WorkshopItem GetItem()
	{
		return m_Item;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_WorkshopItemActionDownload GetDownloadAction()
	{
		return m_Action;
	}
	
	//------------------------------------------------------------------------------------------------
	EDownloadManagerActionState GetState()
	{
		return m_iState;
	}
}