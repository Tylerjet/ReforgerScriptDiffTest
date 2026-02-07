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
		
		// Set current game version
		TextWidget text = TextWidget.Cast(w.FindAnyWidget("VersionText"));
		if (text)
		{
			string platformName = "";	
			PlatformService pls = GetGame().GetPlatformService();
			if (pls)
				platformName = m_aPlatformStrings[pls.GetLocalPlatformKind()];

			text.SetTextFormat("%1 (%2)", GetGame().GetBuildVersion(), platformName);
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
