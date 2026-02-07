class SCR_WatermarkComponent: ScriptedWidgetComponent
{
	[Attribute("5")]
	private float m_fDefaultOffsetX;

	[Attribute("5")]
	private float m_fDefaultOffsetY;

	[Attribute("5")]
	private float m_fEditorOffsetX;
	
	[Attribute("40")]
	private float m_fEditorOffsetY;

	[Attribute("#AR-Watermark_Development")]
	private string m_sWatermarkNameWorkbench;
		
	[Attribute("#AR-Watermark_EarlyAccess_Steam")]
	private string m_sWatermarkNamePC;
	
	[Attribute("#AR-Watermark_EarlyAccess_Xbox")]
	private string m_sWatermarkNameXbox;
	
	[Attribute("#AR-Watermark_Development")]
	private string m_sWatermarkNamePlaystation;

	[Attribute("WatermarkText")]
	private string m_sWatermarkWidgetName;

	[Attribute("VersionText")]
	private string m_sVersionTextWidgetName;
		
	private Widget m_wRoot;

	private ref array <string> m_aPlatformStrings = {
		"Dummy",
		"#AR-UI_Xbox",
		"#AR-UI_PSN",
		"#AR-UI_Steam"
	};

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wRoot = w;
		w.SetZOrder(1000); // Show it above everything

		// Select the correct watermark name for current platform
		PlatformService pls = GetGame().GetPlatformService();
		PlatformKind platform;
		if (pls)
			platform = pls.GetLocalPlatformKind();
		
		TextWidget watermarkText = TextWidget.Cast(w.FindAnyWidget("WatermarkText"));
		if (watermarkText)
		{
			array<string> names = 
			{
				m_sWatermarkNameWorkbench, 
				m_sWatermarkNameXbox, 
				m_sWatermarkNamePlaystation, 
				m_sWatermarkNamePC
			};
			
			if (platform >= 0 && platform < names.Count())
				watermarkText.SetTextFormat("%1", names[pls.GetLocalPlatformKind()]);
		}
		
		// Set current game version
		TextWidget text = TextWidget.Cast(w.FindAnyWidget("VersionText"));
		if (text)
		{
			string platformName = "";
			if (pls)
				platformName = m_aPlatformStrings[platform];

			string experimentalNotice = "";
			if (GetGame().IsExperimentalBuild())
				experimentalNotice = " - Experimental";

			text.SetTextFormat("%1%2 (%3)", GetGame().GetBuildVersion(), experimentalNotice, platformName);
		}
		
		SCR_EditorManagerCore editorManagerCore = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (editorManagerCore) 
			editorManagerCore.Event_OnEditorManagerInitOwner.Insert(OnEditorManagerInit);
		
		ChangeVisualization(false);
	}

	//------------------------------------------------------------------------------------------------
	void ChangeVisualization(bool isInEditor)
	{
		if (isInEditor)
			FrameSlot.SetPos(m_wRoot, -m_fEditorOffsetX, m_fEditorOffsetY);
		else
			FrameSlot.SetPos(m_wRoot, -m_fDefaultOffsetX, m_fDefaultOffsetY);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnEditorOpen()
	{
		ChangeVisualization(true);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnEditorClose()
	{
		ChangeVisualization(false);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnEditorManagerInit()
	{
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (editorManager)
		{
			ChangeVisualization(editorManager.IsOpened());
			
			editorManager.GetOnOpened().Insert(OnEditorOpen);
			editorManager.GetOnClosed().Insert(OnEditorClose);
		}
	}
};
