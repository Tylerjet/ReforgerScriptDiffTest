/*!
Entity which manages custom tooltips.
*/

class SCR_TooltipManagerEntityClass : GenericEntityClass
{
};

class SCR_TooltipManagerEntity : GenericEntity
{
	protected static SCR_TooltipManagerEntity s_Instance = null;
	
	protected vector m_vOffset;			// Offset between mouse position and tooltip position
	protected bool m_bFollowCursor;		// When true, tooltip will be following cursor
	protected Widget m_wTooltip;		// The tooltip widget
	protected Widget m_wHoverWidget;	// When cursor leaves this widget, or this widget gets destroyed, current tooltip will be reset

	const int TOOLTIP_Z_ORDER = 10000;	// Z order of tooltip. Tooltip widget is created in Workspace and must be above other widgets.	
	
	
	
	
	//--------------------------------------------------------------------------------------------------------------
	// PUBLIC
	
	
	//--------------------------------------------------------------------------------------------------------------
	static SCR_TooltipManagerEntity GetInstance()
	{
		return s_Instance;
	}
	
	
	//--------------------------------------------------------------------------------------------------------------
	/*!
	Creates a tooltip from a .layout resource. Previous tooltip is deleted.
	- rsc - .layout resource of the tooltip
	- hoverWidget - widget over which the cursor must stay.
	When cursor leaves the widget, or widget is deleted, tooltip is deleted automatically.
	- followCursor - when true, tooltip will follow cursor. When false, it will stay where it was created at.
	- offset - unscaled offset between cursor pos. and tooltip pos.
	*/
	static Widget CreateTooltip(ResourceName rsc, notnull Widget hoverWidget, bool followCursor = true, vector offset = "15 15 0")
	{	
		SCR_TooltipManagerEntity ent = GetInstance();
		
		if (!ent)
			return null;
		
		return ent.Internal_CreateTooltip(rsc, hoverWidget, followCursor, offset);
	}
	
	
	//--------------------------------------------------------------------------------------------------------------
	/*!
	Deletes current tooltip, if it exists.
	*/
	static void DeleteTooltip()
	{
		SCR_TooltipManagerEntity ent = GetInstance();
		
		if (!ent)
			return;
		
		ent.Internal_DeleteTooltip();
	}
	
	
	//--------------------------------------------------------------------------------------------------------------
	/*!
	Returns current tooltip widget, or null if there is none.
	*/
	static Widget GetTooltip()
	{
		SCR_TooltipManagerEntity ent = GetInstance();
		
		if (!ent)
			return null;
		
		return ent.m_wTooltip;
	}
	
	
	
	
	
	//--------------------------------------------------------------------------------------------------------------
	// PROTECTED
	
	
	
	//--------------------------------------------------------------------------------------------------------------
	protected override void EOnFrame(IEntity owner, float timeSlice)
	{	
		// Bail, deactivate, delete tooltip if hover widget is different or is gone
		if (WidgetManager.GetWidgetUnderCursor() != m_wHoverWidget || !m_wHoverWidget)
		{
			Internal_DeleteTooltip();
			SetFlags(EntityFlags.ACTIVE, false);			
			return;
		}
		
		// Bail, deactivate if there is no tooltip any more (smth else deleted it or we deleted it)
		if (!m_wTooltip)
		{
			SetFlags(EntityFlags.ACTIVE, false);
			return;
		}
			
		// Update current tooltip position if required
		if (m_wTooltip && m_bFollowCursor)
			UpdatePosition();
	}
	
	
	//--------------------------------------------------------------------------------------------------------------
	protected void UpdatePosition()
	{
		int xCursor, yCursor;
		WidgetManager.GetMousePos(xCursor, yCursor);
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		float xTooltip = workspace.DPIUnscale(xCursor + m_vOffset[0]);
		float yTooltip = workspace.DPIUnscale(yCursor + m_vOffset[1]);
		FrameSlot.SetPos(m_wTooltip, xTooltip, yTooltip);
	}
	
	
	//--------------------------------------------------------------------------------------------------------------
	protected Widget Internal_CreateTooltip(ResourceName rsc, Widget hoverWidget, bool followCursor, vector offset)
	{
		// Activate lazily - only when creating some tooltip
		SetFlags(EntityFlags.ACTIVE, true);
				
		m_bFollowCursor = followCursor;
		m_vOffset = offset;
		m_wHoverWidget = hoverWidget;
		
		Widget workspace = GetGame().GetWorkspace();
		
		// Delete the old tooltip
		DeleteTooltip();
		
		m_wTooltip = GetGame().GetWorkspace().CreateWidgets(rsc, workspace);
		if (m_wTooltip)
		{
			m_wTooltip.SetZOrder(TOOLTIP_Z_ORDER);
			UpdatePosition();
		}
			
		return m_wTooltip;
	}
	
	
	//--------------------------------------------------------------------------------------------------------------
	protected void Internal_DeleteTooltip()
	{
		if (m_wTooltip)
			GetGame().GetWorkspace().RemoveChild(m_wTooltip);
	}
	
	
	//--------------------------------------------------------------------------------------------------------------
	void SCR_TooltipManagerEntity(IEntitySource src, IEntity parent)
	{
		s_Instance = this;
		
		SetEventMask(EntityEvent.FRAME);
	}
	
	
	//--------------------------------------------------------------------------------------------------------------
	void ~SCR_TooltipManagerEntity()
	{
		Internal_DeleteTooltip();
	}
};