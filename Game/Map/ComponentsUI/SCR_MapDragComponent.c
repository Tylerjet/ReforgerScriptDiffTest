//------------------------------------------------------------------------------------------------
//! Component for dragging widgets
class SCR_MapDragComponent : SCR_MapUIBaseComponent
{
	static bool s_bIsDragging = false;
	
	static protected ref ScriptInvoker s_OnDragWidget = new ScriptInvoker;
	static protected Widget s_DraggedWidget;
	
	protected SCR_MapCursorModule m_CursorModule;
	protected SCR_MapCursorInfo m_CursorInfo;
	
	//------------------------------------------------------------------------------------------------
	//! Get OnDragWidget invoker
	static ScriptInvoker GetOnDragWidgetInvoker()
	{
		return s_OnDragWidget;
	}
	
	//------------------------------------------------------------------------------------------------
	//! set draggable widget
	static void SetDraggedWidget(Widget w)
	{
		if (!s_bIsDragging)
			s_DraggedWidget = w;
	}
	
	//------------------------------------------------------------------------------------------------
	//! unset draggable widget
	static void UnsetDraggedWidget(Widget w)
	{
		if (s_bIsDragging)
			s_bIsDragging = false;
		
		s_DraggedWidget = null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Begin drag
	//! \return Returns true if there is a draggable widget under the cursor
	static bool StartDrag()
	{
		if (s_DraggedWidget && !s_bIsDragging)
		{
			s_bIsDragging = true;
			s_OnDragWidget.Invoke(s_DraggedWidget);
			
			return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! End drag
	static void EndDrag()
	{
		s_bIsDragging = false;
		s_DraggedWidget = null;
		s_OnDragWidget.Invoke(null);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Drag widget
	void DragWidget(Widget widget, SCR_MapCursorInfo cursorInfo)
	{				
		int screenX = GetGame().GetWorkspace().GetWidth();
		int screenY = GetGame().GetWorkspace().GetHeight();
		
		float widgetX, widgetY, fx, fy, minX, minY, maxX, maxY;
		
		// mouse position difference
		fx = cursorInfo.x - cursorInfo.lastX; 
		fy = cursorInfo.y - cursorInfo.lastY;

		// no change
		if (fx == 0 && fy == 0)
			return;

		// new pos
		vector pos = FrameSlot.GetPos(widget);
		fx = fx + pos[0];
		fy = fy + pos[1];

		//! get widget size
		widget.GetScreenSize(widgetX, widgetY);

		//! get max screen size
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		minX = workspace.DPIUnscale(-widgetX/2);
		minY = workspace.DPIUnscale(-widgetY/2);
		maxX = workspace.DPIUnscale(screenX) - workspace.DPIUnscale(widgetX/2);
		maxY = workspace.DPIUnscale(screenY) - workspace.DPIUnscale(widgetY/2);

		// avoid moving the element off screen		
		if (fx < minX) fx = minX;
		if (fy < minY) fy = minY;
		if (fx > maxX) fx = maxX;
		if (fy > maxY) fy = maxY;
		
		// set new postion
		FrameSlot.SetPos(widget, fx, fy);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMapOpen(MapConfiguration config)
	{
		super.OnMapOpen(config);
		
		m_CursorModule = SCR_MapCursorModule.Cast(m_MapEntity.GetMapModule(SCR_MapCursorModule));
		m_CursorInfo = m_CursorModule.GetCursorInfo();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMapClose(MapConfiguration config)
	{
		s_OnDragWidget.Clear();
		s_DraggedWidget = null;
		s_bIsDragging = false;
		
		super.OnMapClose(config);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Update()
	{
		if (s_bIsDragging)
		{
			DragWidget(s_DraggedWidget, m_CursorInfo);
		}
	}
};
