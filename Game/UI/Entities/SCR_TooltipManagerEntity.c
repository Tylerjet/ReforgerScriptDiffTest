/*!
Entity which manages custom tooltips.
*/

class SCR_TooltipManagerEntityClass : GenericEntityClass
{
};

//------------------------------------------------------------------------------------------------
enum SCR_ETooltipAlignmentHorizontal
{
	CURSOR,
	LEFT,
	CENTER,
	RIGHT,
	SIDELEFT,
	SIDERIGHT
};

//------------------------------------------------------------------------------------------------
enum SCR_ETooltipAlignmentVertical
{
	CURSOR,
	ABOVE,
	BELOW,
	TOP,
	BOTTOM
};

//------------------------------------------------------------------------------------------------
class SCR_TooltipManagerEntity : GenericEntity
{
	protected static SCR_TooltipManagerEntity s_Instance = null;

	protected vector m_vOffset;											// Offset between mouse position and tooltip position
	protected bool m_bFollowCursor;										// When true, tooltip will be following cursor
	protected Widget m_wTooltip;										// The tooltip widget
	protected Widget m_wHoverWidget;									// When cursor leaves this widget, or this widget gets destroyed, current tooltip will be reset
	protected SCR_ETooltipAlignmentHorizontal m_eHorizontalAlignment;		// Tooltip position in relation to the hover Widget
	protected SCR_ETooltipAlignmentVertical m_eVerticalAlignment;			// Tooltip position in relation to the hover Widget

	protected bool m_bIsRendered;
	protected Widget m_wInteractableWidget;	// Currently focused or hovered widget
	protected WorkspaceWidget m_wWorkspace;

	const int TOOLTIP_Z_ORDER = 10000;	// Z order of tooltip. Tooltip widget is created in Workspace and must be above other widgets.

	//------------------------------------------------------------------------------------------------
	// PUBLIC
	//------------------------------------------------------------------------------------------------
	static SCR_TooltipManagerEntity GetInstance()
	{
		return s_Instance;
	}


	//------------------------------------------------------------------------------------------------
	/*!
	Creates a tooltip from a .layout resource. Previous tooltip is deleted.
	- rsc - .layout resource of the tooltip
	- hoverWidget - widget over which the cursor must stay.
	When cursor leaves the widget, or widget is deleted, tooltip is deleted automatically.
	- followCursor - when true, tooltip will follow cursor. When false, it will stay where it was created at.
	- alignments: tooltip position in relation to the hover widget
	- offset - unscaled offset between cursor pos. and tooltip pos.
	*/
	static Widget CreateTooltip(ResourceName rsc, notnull Widget hoverWidget, bool followCursor = true, vector offset = "15 15 0", SCR_ETooltipAlignmentHorizontal horizontalAlignment = SCR_ETooltipAlignmentHorizontal.CURSOR, SCR_ETooltipAlignmentVertical verticalAlignment = SCR_ETooltipAlignmentVertical.CURSOR)
	{
		SCR_TooltipManagerEntity ent = GetInstance();

		if (!ent)
			return null;

		return ent.Internal_CreateTooltip(rsc, hoverWidget, followCursor, offset, horizontalAlignment, verticalAlignment);
	}


	//------------------------------------------------------------------------------------------------
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


	//------------------------------------------------------------------------------------------------
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


	//------------------------------------------------------------------------------------------------
	// PROTECTED
	//------------------------------------------------------------------------------------------------
	protected Widget Internal_CreateTooltip(ResourceName rsc, Widget hoverWidget, bool followCursor, vector offset, SCR_ETooltipAlignmentHorizontal horizontalAlignment, SCR_ETooltipAlignmentVertical verticalAlignment)
	{
		// If neither of the alignments is set to cursor, there's no point in following the cursor on tick
		m_bFollowCursor = followCursor &&
						!(horizontalAlignment != SCR_ETooltipAlignmentHorizontal.CURSOR &&
						verticalAlignment != SCR_ETooltipAlignmentVertical.CURSOR);

		m_vOffset = offset;
		m_wHoverWidget = hoverWidget;
		m_eHorizontalAlignment = horizontalAlignment;
		m_eVerticalAlignment = verticalAlignment;

		m_bIsRendered = false;

		// Delete the old tooltip
		DeleteTooltip();

		m_wWorkspace = GetGame().GetWorkspace();
		m_wTooltip = m_wWorkspace.CreateWidgets(rsc, m_wWorkspace);

		if (m_wTooltip)
			m_wTooltip.SetZOrder(TOOLTIP_Z_ORDER);

		// Activate lazily - only when creating some tooltip
		SetEventMask(EntityEvent.FRAME);

		return m_wTooltip;
	}

	//------------------------------------------------------------------------------------------------
	protected void Internal_DeleteTooltip()
	{
		if (m_wTooltip)
			GetGame().GetWorkspace().RemoveChild(m_wTooltip);
	}


	//------------------------------------------------------------------------------------------------
	protected override void EOnFrame(IEntity owner, float timeSlice)
	{
		m_wWorkspace = GetGame().GetWorkspace();

		// Bail, deactivate, delete tooltip if hover widget is different or is gone
		if (GetGame().GetInputManager().GetLastUsedInputDevice() == EInputDeviceType.MOUSE)
			m_wInteractableWidget = WidgetManager.GetWidgetUnderCursor();
		else
			m_wInteractableWidget = m_wWorkspace.GetFocusedWidget();

		if (!m_wHoverWidget || m_wHoverWidget != m_wInteractableWidget)
		{
			Internal_DeleteTooltip();
			ClearEventMask(EntityEvent.FRAME);
			return;
		}

		// Bail, deactivate if there is no tooltip any more (smth else deleted it or we deleted it)
		if (!m_wTooltip)
		{
			ClearEventMask(EntityEvent.FRAME);
			return;
		}

		// Check for first render
		if (!m_bIsRendered)
		{
			// Check if the widget has been rendered
			float tooltipSizeX, tooltipSizeY;
			m_wTooltip.GetScreenSize(tooltipSizeX, tooltipSizeY);
			m_bIsRendered = tooltipSizeX != 0 && tooltipSizeY != 0;

			UpdatePosition();
			return;
		}

		// Update current tooltip position if required
		if (m_bFollowCursor)
			UpdatePosition();
	}


	//------------------------------------------------------------------------------------------------
	protected void UpdatePosition()
	{
		//! Tooltip position
		float xTooltip, yTooltip;
		m_wTooltip.GetScreenPos(xTooltip, yTooltip);

		//! Tooltip Size
		float xTooltipSize, yTooltipSize;
		m_wTooltip.GetScreenSize(xTooltipSize, yTooltipSize);

		//! Cursor position
		int xCursor, yCursor;
		WidgetManager.GetMousePos(xCursor, yCursor);

		//! HoverWidget position
		float xHover, yHover;
		m_wHoverWidget.GetScreenPos(xHover, yHover);

		//! HoverWidget size
		float xHoverSize, yHoverSize;
		m_wHoverWidget.GetScreenSize(xHoverSize, yHoverSize);

		//! Alignments
		SCR_ETooltipAlignmentHorizontal alignmentHorizontal = GetCorrectedHorizontalAlignment(xCursor, xHover, xHoverSize, xTooltipSize);
		SCR_ETooltipAlignmentVertical alignmentVertical = m_eVerticalAlignment;

		//! Offset
		vector offset = GetCorrectedOffset(xCursor, xHover, xHoverSize, xTooltipSize);

		//! Tooltip alignment
		float xTooltipAlignment;
		float yTooltipAlignment;
		
		//! Calculate correct anchors and alignments
		switch (alignmentHorizontal)
		{
			case SCR_ETooltipAlignmentHorizontal.CURSOR:
				xTooltip = xCursor;
				break;

			case SCR_ETooltipAlignmentHorizontal.LEFT:
				xTooltip = xHover;
				break;

			case SCR_ETooltipAlignmentHorizontal.CENTER:
				xTooltip = xHover + (xHoverSize * 0.5);
				xTooltipAlignment = 0.5;
				break;

			case SCR_ETooltipAlignmentHorizontal.RIGHT:
				xTooltip = xHover + xHoverSize;
				xTooltipAlignment = 1;
				break;

			case SCR_ETooltipAlignmentHorizontal.SIDELEFT:
				xTooltip = xHover;
				xTooltipAlignment = 1;
				break;

			case SCR_ETooltipAlignmentHorizontal.SIDERIGHT:
				xTooltip = xHover + xHoverSize;
				break;
		}

		switch (alignmentVertical)
		{
			case SCR_ETooltipAlignmentVertical.CURSOR:
				yTooltip = yCursor;
				break;

			case SCR_ETooltipAlignmentVertical.ABOVE:
				yTooltip = yHover;
				yTooltipAlignment = 1;
				break;

			case SCR_ETooltipAlignmentVertical.BELOW:
				yTooltip = yHover + yHoverSize;
				break;

			case SCR_ETooltipAlignmentVertical.TOP:
				yTooltip = yHover;
				break;

			case SCR_ETooltipAlignmentVertical.BOTTOM:
				yTooltip = yHover + yHoverSize;
				yTooltipAlignment = 1;
				break;
		}

		SetTooltipPosition(xTooltip, yTooltip, offset, xTooltipAlignment, yTooltipAlignment);
	}


	//------------------------------------------------------------------------------------------------
	vector GetCorrectedOffset(int xCursor, int xHover, int xHoverSize, int xTooltipSize)
	{
		//! Offset
		vector offset;
		for (int i = 0; i < 2; i++)
		{
			offset[i] = m_wWorkspace.DPIScale(m_vOffset[i]);
		}

		//! TODO: solve flickering issue when the mouse is in the restricted area at tooltip creation
		//! Horizontal: prevent the tooltip from moving away from the hover widget border if it's aligned above or below it
		if (IsSlidingHorizontally())
		{
			if (IsOverflowingLeft(xCursor, xHover) || IsOverflowingRight(xCursor, xHover, xHoverSize, xTooltipSize))
				offset[0] = 0;
		}
		
		//! TODO: vertical

		return offset;
	}


	//------------------------------------------------------------------------------------------------
	SCR_ETooltipAlignmentHorizontal GetCorrectedHorizontalAlignment(int xCursor, int xHover, int xHoverSize, int xTooltipSize)
	{
		//! Horizontal: prevent the tooltip from moving away from the hover widget border if it's aligned above or below it
		if (IsSlidingHorizontally())
		{
			if (IsOverflowingLeft(xCursor, xHover))
				return SCR_ETooltipAlignmentHorizontal.LEFT;

			else if (IsOverflowingRight(xCursor, xHover, xHoverSize, xTooltipSize))
				return SCR_ETooltipAlignmentHorizontal.RIGHT;
		}

		return m_eHorizontalAlignment;
	}


	//------------------------------------------------------------------------------------------------
	bool IsOverflowingLeft(int xCursor, int xHover)
	{
		if (IsSlidingHorizontally())
			return xCursor < xHover + m_wWorkspace.DPIScale(m_vOffset[0] * -1);

		return false;
	}


	//------------------------------------------------------------------------------------------------
	bool IsOverflowingRight(int xCursor, int xHover, int xHoverSize, int xTooltipSize)
	{
		if (IsSlidingHorizontally())
			return xCursor + m_wWorkspace.DPIScale(m_vOffset[0]) > (xHover + xHoverSize) - xTooltipSize;

		return false;
	}


	//------------------------------------------------------------------------------------------------
	bool IsSlidingHorizontally()
	{
		return m_bIsRendered && m_eHorizontalAlignment == SCR_ETooltipAlignmentHorizontal.CURSOR && m_eVerticalAlignment != SCR_ETooltipAlignmentVertical.CURSOR;
	}


	//------------------------------------------------------------------------------------------------
	void SetTooltipPosition(int xTooltip, int yTooltip, vector offset, float xTooltipAlignment = 0, float yTooltipAlignment = 0)
	{
		//! Screen size
		float xWorkspaceSize, yWorkspaceSize;
		m_wWorkspace.GetScreenSize(xWorkspaceSize, yWorkspaceSize);

		if (xWorkspaceSize == 0)
			xWorkspaceSize = 1;

		if (yWorkspaceSize == 0)
			yWorkspaceSize = 1;

		FrameSlot.SetAnchor(m_wTooltip, (xTooltip + offset[0]) / xWorkspaceSize, (yTooltip + offset[1]) / yWorkspaceSize);
		FrameSlot.SetAlignment(m_wTooltip, xTooltipAlignment, yTooltipAlignment);
		FrameSlot.SetPos(m_wTooltip, 0, 0);

		//! TODO: handle alignment regarding to cursor (as of now to make the tooltip appear to the left of the mouse you must set negative offset)
	}


	//------------------------------------------------------------------------------------------------
	void SCR_TooltipManagerEntity(IEntitySource src, IEntity parent)
	{
		s_Instance = this;
		SetFlags(EntityFlags.NO_TREE | EntityFlags.NO_LINK);
	}


	//------------------------------------------------------------------------------------------------
	void ~SCR_TooltipManagerEntity()
	{
		Internal_DeleteTooltip();
	}
};
