class SCR_AITargetStatsScreen
{
	ref array<Widget> m_aAccuracyDisplays = {};
	ref array<Widget> m_aDistributionDisplays = {};
	ref array<AIAgent> m_aOwnersArray = {};
	
	ResourceName m_sAccuracyLayout = "{71DA647DA43C6448}UI/layouts/Test/AccuracyDebugSubpanel.layout";
	ResourceName m_sDistributionLayout = "{F46E711F17F445C0}UI/layouts/Test/AccuracyDebugDistribution.layout";
	string m_aBarNames[3] = {"Image0", "Image1", "Image2"};
	string m_sPanelName = "Panel";
	int m_iBulletsCounted;
	float m_fDotSize = 4;
	float m_fDotOpacity = 0.6;
	float m_fTableHeight = 100;
	Widget m_wRootTarget;
	Widget m_wRootGraph;
	static Widget m_wRoot;
	
	void SCR_AITargetStatsScreen()
	{
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_AI_CONFIGURE_STATISTICSDEBUG,"","AI Accuracywidget debug","AI");
		DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_AI_CONFIGURE_STATISTICSDEBUG,0);	
		
		// Create a horizontal layout at first
		if (!m_wRoot)
		{
			m_wRoot = GetGame().GetWorkspace().CreateWidgetInWorkspace(WidgetType.VerticalLayoutWidgetTypeID, 0,0,0,0, WidgetFlags.VISIBLE, null, 0);
			if (!m_wRoot)
				return;
			FrameSlot.SetAnchorMin(m_wRoot, 0, 0);
			FrameSlot.SetAnchorMax(m_wRoot, 1, 1);
			FrameSlot.SetOffsets(m_wRoot, 40, 40, 40, 40);
		}
		
		m_wRootTarget = GetGame().GetWorkspace().CreateWidget(WidgetType.HorizontalLayoutWidgetTypeID, WidgetFlags.VISIBLE, null, 0, m_wRoot);
		m_wRootGraph = GetGame().GetWorkspace().CreateWidget(WidgetType.HorizontalLayoutWidgetTypeID, WidgetFlags.VISIBLE, null, 0, m_wRoot);
		if (!m_wRootTarget)
			return;
		
		LayoutSlot.SetPadding(m_wRootTarget, 0, 10, 0, 0);
		
		GetGame().GetWorkspace().CreateWidgets("{1664A3BA018F3151}UI/layouts/Debug/AccuracyDebugLegend.layout", GetGame().GetWorkspace());
	}
	
	void AddAccuracyDisplay()
	{
		if (!m_wRootTarget)
			return;

		Widget display = GetGame().GetWorkspace().CreateWidgets(m_sAccuracyLayout, m_wRootTarget);
		if (!display)
			return;
		
		m_aAccuracyDisplays.Insert(display);
	}
	
	void AddDistributionDisplay()
	{
		if (!m_wRootGraph)
			return;

		Widget display = GetGame().GetWorkspace().CreateWidgets(m_sDistributionLayout, m_wRootGraph);
		if (!display)
			return;
		
		m_aDistributionDisplays.Insert(display);
	}
	
	void DrawPoint(AIAgent owner, vector point, int target, Color drawColor)
	{
		while (target >= m_aAccuracyDisplays.Count())
		{
			AddAccuracyDisplay();
		}
		
		Widget root = m_aAccuracyDisplays[target];
		if (!root)
			return;
		
		ImageWidget dot = ImageWidget.Cast(GetGame().GetWorkspace().CreateWidget(WidgetType.ImageWidgetTypeID, WidgetFlags.VISIBLE, drawColor, 0, root.GetChildren()));
		if (!dot)
			return;
		
		FrameSlot.SetAnchorMin(dot, 0.5,0.5);
		FrameSlot.SetAnchorMax(dot, 0.5,0.5);
		FrameSlot.SetAlignment(dot, 0.5,0.5);
		FrameSlot.SetPos(dot, point[1], point[0]);
		FrameSlot.SetSize(dot, m_fDotSize, m_fDotSize);
		dot.SetOpacity(m_fDotOpacity);
	}

	void DrawGraphDot(AIAgent owner, vector point, int target, Color drawColor, ECharacterStance stance)
	{
		while (target >= m_aDistributionDisplays.Count())
		{
			AddDistributionDisplay();
		}
		
		Widget root = m_aDistributionDisplays[target];
		if (!root)
			return;
		
		Widget panel = root.FindAnyWidget(m_sPanelName);
		Widget bars = root.FindAnyWidget("BarsLayout");
		if (!bars || !panel)
			return;

		Widget currentBar = GetChildByIndex(bars, point[0]);
		if (!currentBar)
			return;
				
		Widget widgetToShow = currentBar.FindAnyWidget(m_aBarNames[stance]);
		if (!widgetToShow)
			return;
		
		LayoutSlot.SetPadding(widgetToShow, 0, -m_fTableHeight * point[1], 0, 0);
	}
	
	Widget GetChildByIndex(Widget parent, int index)
	{
		if (!parent)
			return null;
		
		Widget child = parent.GetChildren();
		for (int i; i <= index; i++)
		{
			if (!child)
				return null;
			
			if (i == index)
				return child;
			
			child = child.GetSibling();
		}
		return null;
	}
	
	void ResetWidgets()
	{
		if (m_aAccuracyDisplays)
		{
			foreach (Widget display : m_aAccuracyDisplays)
			{
				display.RemoveFromHierarchy();
			}
			m_aAccuracyDisplays.Clear();
		}
		if (m_aDistributionDisplays)
		{
			foreach (Widget display : m_aDistributionDisplays)
			{
				display.RemoveFromHierarchy();
			}
			m_aDistributionDisplays.Clear();
		}
	}
	
	void Update(float timeslice)
	{
		if (!DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AI_CONFIGURE_STATISTICSDEBUG))
			return;
		
		DbgUI.Text("U to reset all widgets");
		DbgUI.Text("K to disable AI");
		DbgUI.Text("L to toggle accuracy display");
		DbgUI.Text("N to toggle target distribution display");
		
		if (Debug.KeyState(KeyCode.KC_U))
		{
			Debug.ClearKey(KeyCode.KC_U);
			ResetWidgets();
		}
		
		if (Debug.KeyState(KeyCode.KC_K))
		{
			Debug.ClearKey(KeyCode.KC_K);
			foreach (AIAgent testSoldier : m_aOwnersArray)
			{
				IEntity testSoldierEntity = testSoldier.GetControlledEntity();
				AIControlComponent controlComponent = AIControlComponent.Cast(testSoldierEntity.FindComponent(AIControlComponent));
				
				if (!controlComponent) 
					return;
				
				if (controlComponent.IsAIActivated())
					controlComponent.DeactivateAI();
				else
					controlComponent.ActivateAI();
			}
		}
		
		if (Debug.KeyState(KeyCode.KC_L))
		{
			Debug.ClearKey(KeyCode.KC_L);

			if (!m_wRootTarget)
				return;
			
			m_wRootTarget.SetVisible(!m_wRootTarget.IsVisible());
		}

		if (Debug.KeyState(KeyCode.KC_N))
		{
			Debug.ClearKey(KeyCode.KC_N);
			
			if (!m_wRootGraph)
				return;
			
			m_wRootGraph.SetVisible(!m_wRootGraph.IsVisible());
		}
	}	
};