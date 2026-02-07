class SCR_PhotoSaveModeEditorUIComponent : SCR_PhotoModeEditorUIComponent
{
	protected const string IMAGE_PATH_TEST = "UI/TestImg.png";
	
	[Attribute("Toolbar_CaptureButton")]
	protected string m_sCaptureButton;
	
	[Attribute("")]
	protected string m_sCaptureAction;
	
	[Attribute("use_screenshot")]
	protected string m_sConfirmScreenshotDialogTag;
	
	[Attribute("EditorMenuConfirm")]
	protected string m_sConfirmScreenshotAction;
	
	[Attribute("InputButtonRoot0")]
	protected string m_sActionButton;
	
	
	protected SCR_EventHandlerComponent m_CaptureButtonEvents;
	protected SCR_ConfigurableDialogUi m_UseScreenshotDialog;
	protected PixelRawData m_CapturedData;
	
	protected ref ScriptInvokerString m_OnUseScreenshot;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		// UI setup
		Widget captureButton = w.FindAnyWidget(m_sCaptureButton);
		m_CaptureButtonEvents = SCR_EventHandlerComponent.Cast(captureButton.FindHandler(SCR_EventHandlerComponent));
		m_CaptureButtonEvents.GetOnClick().Insert(CaptureImage);
		
		Widget inputButton = captureButton.FindAnyWidget(m_sActionButton);
		SCR_InputButtonComponent inputButtonCmp = SCR_InputButtonComponent.Cast(inputButton.FindHandler(SCR_InputButtonComponent));
		inputButtonCmp.SetAction(m_sConfirmScreenshotAction);
		
		GetGame().GetInputManager().AddActionListener(m_sConfirmScreenshotAction, EActionTrigger.DOWN, CaptureImage);
		
		// Inputs 
		GetGame().GetInputManager().AddActionListener(m_sCaptureAction, EActionTrigger.DOWN, CaptureImage);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Capture image of current camere view 
	protected void CaptureImage()
	{
		System.MakeScreenshotRawData(OnScreenshot, 0, 0, 1920, 1080, 1920, 1080);
	}
	
	//---------------------------------------------------------------------------------------------
	protected void OnScreenshot(PixelRawData data, int imageWidth, int imageHeight, int stride)
	{
		#ifdef WORKBENCH
		//Workbench.SavePixelRawData(IMAGE_PATH_TEST, data, imageWidth, imageHeight, stride);
		#endif
		
		m_CapturedData = data;
		
		// Create comfirm dialog
		m_UseScreenshotDialog = SCR_SaveWorkshopManagerUI.CreateDialog(m_sConfirmScreenshotDialogTag);
		m_UseScreenshotDialog.m_OnConfirm.Insert(OnScreenshotUseDialogConfirm);
		m_UseScreenshotDialog.m_OnCancel.Insert(OnScreenshotUseDialogCancel);
		
		
	}
	
	//---------------------------------------------------------------------------------------------
	protected void OnScreenshotUseDialogConfirm()
	{
		string screenshot = IMAGE_PATH_TEST;
		
		if (m_OnUseScreenshot)
			m_OnUseScreenshot.Invoke(screenshot);
		
		SCR_SaveWorkshopManager manager = SCR_SaveWorkshopManager.GetInstance();
		SCR_EditedSaveManifest editedManifest = manager.GetEditedSaveManifest();
		WorldSaveManifest manifest = editedManifest.GetManifest();
		WorldSaveItem save; 
		manager.GetCurrentSave(save);
		
		if (editedManifest.GetEditingValue() == "thumbnail")
		{
			save.SaveJPEG(m_CapturedData, 1920, 1080, 50, 4, true);
			//editedManifest.m_Manifest.m_sPreview = screenshot;
		}
		else if (editedManifest.GetEditingValue() == "gallery")
		{	
			if (!manifest.m_aScreenshots)
				manifest.m_aScreenshots = {};
			
			for (int i = manifest.m_aScreenshots.Count() - 1; i >= 0; i--)
			{
				if (manifest.m_aScreenshots[i] == string.Empty)
					manifest.m_aScreenshots.Remove(i);
			}
			
			manifest.m_aScreenshots.Insert(screenshot);
		}
		
		manager.SetEditedSaveManifest(manifest, editedManifest.GetEditingValue(), "img");
		
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		SCR_EditorManagerEntity editorManager = core.GetEditorManager();
		
		SCR_EditorManagerEntity.GetInstance().SetCurrentMode(EEditorMode.EDIT);
	}
	
	//---------------------------------------------------------------------------------------------
	protected void OnScreenshotUseDialogCancel()
	{
		m_CapturedData = null;
	}
}
