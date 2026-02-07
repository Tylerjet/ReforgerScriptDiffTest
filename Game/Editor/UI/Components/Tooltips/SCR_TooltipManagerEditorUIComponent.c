/** @ingroup Editor_UI Editor_UI_Components
*/
class SCR_TooltipManagerEditorUIComponent: SCR_BaseEditorUIComponent
{
	const float INERTIA_THRESHOLD = 0.001; //--- Squared value
	
	[Attribute()]
	protected ref array<ref SCR_TooltipEditorEntry> m_Tooltips;
	
	[Attribute(defvalue: "0.1", desc: "How long (s) it will take before the tooltips appear.")]
	protected float m_fDelay;
	
	[Attribute(defvalue: "10", desc: "How fast is the fade-in effect")]
	protected float m_fFadeInSpeed;
	
	[Attribute(defvalue: "0.03", desc: "Inertia strength. Larger values mean more inertia.")]
	private float m_fInertiaStrength;
	
	protected InputManager m_InputManager;
	protected SCR_LayersEditorComponent m_LayersManager;
	protected SCR_TooltipAreaEditorUIComponent m_TooltipArea;
	protected SCR_CursorEditorUIComponent m_Cursor;
	protected SCR_BaseTooltipEditorUIComponent m_Tooltip;
	protected SCR_UIInfo m_Info;
	protected Managed m_InfoInstance;
	protected bool m_bTooltipShown;

	/*!
	Show tooltip with given params.
	\param info UI info with all visual information
	\param type Tooltip type, influences which layout will be selected
	\param instance Instance, e.g., entity, which can be accessed for more information
	*/
	void SetInfo(SCR_UIInfo info, EEditorTooltip type = 0, Managed instance = null)
	{
		if (!m_TooltipArea)
			return;
		
		if (info && info == m_Info)
			return;
		
		if (m_Tooltip)
			ResetInfo();
		
		foreach (SCR_TooltipEditorEntry tooltipEntry: m_Tooltips)
		{
			if (tooltipEntry.GetTooltipType() == type)
			{
				ResourceName layout = tooltipEntry.GetTooltipLayout();
				if (!layout.IsEmpty())
				{
					Widget tooltipWidget = GetWidget().GetWorkspace().CreateWidgets(layout, m_TooltipArea.GetAreaWidget());
					FrameSlot.SetOffsets(tooltipWidget, 0, 0, 0, 0);
					FrameSlot.SetSizeToContent(tooltipWidget, true);
					
					m_Tooltip = SCR_BaseTooltipEditorUIComponent.Cast(tooltipWidget.FindHandler(SCR_BaseTooltipEditorUIComponent));
					if (m_Tooltip)
					{				
						if (m_Tooltip.SetTooltip(info, instance))
						{
							//OnMenuUpdate(0);
							GetGame().GetCallqueue().CallLater(PlayAnimation, m_fDelay * 1000, false);
						}
						else
						{
							tooltipWidget.RemoveFromHierarchy();
							tooltipWidget = null;
						}
					}
					else
					{
						Print(string.Format("SCR_BaseTooltipEditorUIComponent not found in tooltip '%1'", layout), LogLevel.WARNING);
						return;
					}
				}
				else
				{
					Print(string.Format("Layout for tooltip of type '%1' not defined!", typename.EnumToString(EEditorTooltip, type)), LogLevel.WARNING);
					return;
				}
			}
		}
		
		//--- Ignore when tooltip type is undefined (it may be intentionally so)
		if (!m_Tooltip)
			return;
			
		m_Info = info;
		m_InfoInstance = instance;
	}
	/*!
	Hide currently shown tooltip.
	*/
	void ResetInfo()
	{
		if (m_Tooltip)
		{
			m_Tooltip.GetWidget().RemoveFromHierarchy();
			m_Tooltip = null;
			GetGame().GetCallqueue().Remove(PlayAnimation);
			ResetAnimation();
		}
		m_Info = null;
		m_InfoInstance = null;
	}
	/*!
	Refresh currently shown tooltip, in case it's UI info changed (e.g., action state was toggled).
	*/
	void RefreshInfo(SCR_UIInfo info, EEditorTooltip type = 0, Managed instance = null)
	{
		if (m_InfoInstance && instance == m_InfoInstance)
			SetInfo(info, type, instance)
	}
	protected void PlayAnimation()
	{
		//m_TooltipArea.GetWidget().SetOpacity(1);
		WidgetAnimator.PlayAnimation(m_TooltipArea.GetWidget(), WidgetAnimationType.Opacity, 1, m_fFadeInSpeed);
		m_bTooltipShown = true;
	}
	protected void ResetAnimation()
	{
		WidgetAnimator.StopAnimation(m_TooltipArea.GetWidget(), WidgetAnimationType.Opacity);
		m_TooltipArea.GetWidget().SetOpacity(0);
		m_bTooltipShown = false;
	}
	protected void ShowEntityTooltip(SCR_EditableEntityComponent entity)
	{
		if (!entity)
			return;
		
		if (m_LayersManager)
			entity = m_LayersManager.GetParentBelowCurrentLayer(entity);
		
		if (entity)
			SetInfo(entity.GetInfo(), EEditorTooltip.ENTITY, entity);
	}
	protected void OnHover(EEditableEntityState state, set<SCR_EditableEntityComponent> entitiesInsert, set<SCR_EditableEntityComponent> entitiesRemove)
	{
		if (!GetMenu().IsFocused())
			return;
		
		//--- Entity tooltip not available when context menu is opened
		SCR_ContextMenuActionsEditorUIComponent contextMenu = SCR_ContextMenuActionsEditorUIComponent.Cast(GetRootComponent().FindComponent(SCR_ContextMenuActionsEditorUIComponent));
		if (contextMenu && contextMenu.IsContextMenuOpen())
			return;
		
		if (entitiesRemove && !entitiesRemove.IsEmpty())
		{
			ResetInfo();
		}
		if (entitiesInsert && !entitiesInsert.IsEmpty())
		{
			ShowEntityTooltip(entitiesInsert[0]);
		}
	}
	protected void OnInputDeviceUserChanged(EInputDeviceType oldDevice, EInputDeviceType newDevice)
	{
		if (SCR_Global.IsChangedMouseAndKeyboard(oldDevice, newDevice))
			return;
		
		MenuRootBase menu = GetMenu();
		if (!menu) return;
		
		/*
		if (m_InputManager.IsUsingMouseAndKeyboard())
		{
			menu.GetOnMenuUpdate().Insert(OnMenuUpdate);
		}
		else
		{
			menu.GetOnMenuUpdate().Remove(OnMenuUpdate);
		}
		*/
	}
	protected void OnRadialMenuToggle(IEntity owner, bool isOpened)
	{
		if (isOpened)
		{
			//--- Hide entity tooltip when radial menu is opened
			ResetInfo();
		}
		else
		{
			//--- Show entity tooltip when radial menu is closed again
			SCR_BaseEditableEntityFilter hoverFilter = SCR_BaseEditableEntityFilter.GetInstance(EEditableEntityState.HOVER);
			if (hoverFilter) ShowEntityTooltip(hoverFilter.GetFirstEntity());
		}
	}
	protected void OnMenuUpdate(float tDelta)
	{
		if (!m_Tooltip) return;
		
		m_Tooltip.UpdateTooltip(m_InfoInstance);
		
		if (!m_InputManager.IsUsingMouseAndKeyboard())
			return;

		Widget widget = m_TooltipArea.GetAreaWidget();
		WorkspaceWidget workspace = widget.GetWorkspace();
		
		float posX, posY;
		int offsetTop, offsetRight, offsetBottom, offsetLeft;
		int screenW = workspace.GetWidth();
		int screenH = workspace.GetHeight();
		screenW = workspace.DPIUnscale(screenW);
		screenH = workspace.DPIUnscale(screenH);
		
		m_TooltipArea.GetOffsets(offsetTop, offsetRight, offsetBottom, offsetLeft);
		
		if (m_Cursor)
		{
			vector cursorPos = m_Cursor.GetCursorPos();
			posX = cursorPos[0];
			posY = cursorPos[1];
		}
		else
		{
			int mouseX, mouseY;
			WidgetManager.GetMousePos(mouseX, mouseY);
			posX = workspace.DPIUnscale(mouseX);
			posY = workspace.DPIUnscale(mouseY);
		}
		
		float tooltipX, tooltipY, tooltipW, tooltipH;
		
		//--- Get unscaled position. Using scaled one and converting would cause a jitter due to conversion imprecision.
		vector tooltipPos = FrameSlot.GetPos(widget);
		tooltipX = tooltipPos[0];
		tooltipY = tooltipPos[1];
		
		//--- Get actual sceen size. Cannot use FrameSLot.GetSize, that returns configured, not actual size.
		widget.GetScreenSize(tooltipW, tooltipH);
		tooltipW = workspace.DPIUnscale(tooltipW);
		tooltipH = workspace.DPIUnscale(tooltipH);
		
		//--- Prevent screen overflow
		if (posX > screenW - tooltipW - offsetRight)
			posX -= tooltipW + offsetLeft;
		else
			posX += offsetRight;
		
		if (posY > screenH - tooltipH - offsetBottom)
			posY -= tooltipH + offsetTop;
		else
			posY += offsetBottom;
		
		if (vector.DistanceSq(Vector(tooltipX, tooltipY, 0), Vector(posX, posY, 0)) < INERTIA_THRESHOLD)
			return;
		
		//--- Smooth movement, important especially when changing direction close to screen edges
		if (m_bTooltipShown && tDelta != 0)
		{
			float progress = Math.Min(tDelta * m_fInertiaStrength, 1);
			posX = Math.Lerp(tooltipX, posX, progress);
			posY = Math.Lerp(tooltipY, posY, progress);
		}
		
		FrameSlot.SetPos(widget, posX, posY);
	}
	protected void OnMenuFocusLost()
	{
		ResetInfo();
	}
	
	override void HandlerAttachedScripted(Widget w)
	{
		m_InputManager = GetGame().GetInputManager();
		if (!m_InputManager) return;
		
		GetGame().OnInputDeviceUserChangedInvoker().Insert(OnInputDeviceUserChanged);
		
		MenuRootBase menu = GetMenu();
		if (menu)
		{
			menu.GetOnMenuUpdate().Insert(OnMenuUpdate);
			menu.GetOnMenuFocusLost().Insert(OnMenuFocusLost);
		}
		
		MenuRootComponent menuRoot = GetRootComponent();
		if (!menuRoot) return;
		
		m_Cursor = SCR_CursorEditorUIComponent.Cast(menuRoot.FindComponent(SCR_CursorEditorUIComponent));
		//if (!m_Cursor) return;
		
		SCR_RadialMenuManagerEditorComponent radialMenuHost = SCR_RadialMenuManagerEditorComponent.Cast(SCR_RadialMenuManagerEditorComponent.GetInstance(SCR_RadialMenuManagerEditorComponent));
		if (radialMenuHost)
		{
			radialMenuHost.GetEditorRadialMenuOpened().Insert(OnRadialMenuToggle);
		}
		
		m_TooltipArea = SCR_TooltipAreaEditorUIComponent.Cast(menuRoot.FindComponent(SCR_TooltipAreaEditorUIComponent, true));
		if (!m_TooltipArea) return;
		
		m_TooltipArea.GetWidget().SetOpacity(0);
		
		SCR_BaseEditableEntityFilter hoverFilter = SCR_BaseEditableEntityFilter.GetInstance(EEditableEntityState.HOVER);
		if (hoverFilter) hoverFilter.GetOnChanged().Insert(OnHover);
		
		m_LayersManager = SCR_LayersEditorComponent.Cast(SCR_LayersEditorComponent.GetInstance(SCR_LayersEditorComponent));
		
		m_fInertiaStrength = 1 / Math.Max(m_fInertiaStrength, 0.001);
	}
	override void HandlerDeattached(Widget w)
	{
		if (GetGame().OnInputDeviceUserChangedInvoker())
			GetGame().OnInputDeviceUserChangedInvoker().Remove(OnInputDeviceUserChanged);
		
		MenuRootBase menu = GetMenu();
		if (menu)
		{
			menu.GetOnMenuUpdate().Remove(OnMenuUpdate);
			menu.GetOnMenuFocusLost().Remove(OnMenuFocusLost);
		}
		
		if (m_TooltipArea) m_TooltipArea.ClearTooltips();
		
		SCR_BaseEditableEntityFilter hoverFilter = SCR_BaseEditableEntityFilter.GetInstance(EEditableEntityState.HOVER);
		if (hoverFilter) hoverFilter.GetOnChanged().Remove(OnHover);
		
		SCR_RadialMenuManagerEditorComponent radialMenuHost = SCR_RadialMenuManagerEditorComponent.Cast(SCR_RadialMenuManagerEditorComponent.GetInstance(SCR_RadialMenuManagerEditorComponent));
		if (radialMenuHost)
		{
			radialMenuHost.GetEditorRadialMenuOpened().Remove(OnRadialMenuToggle);
		}
	}
};

[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditorTooltip, "m_TooltipType")]
class SCR_TooltipEditorEntry
{
	[Attribute("0", UIWidgets.ComboBox, "", enums: ParamEnumArray.FromEnum(EEditorTooltip))]
	protected EEditorTooltip m_TooltipType;
	
	[Attribute(params: "layout")]
	protected ResourceName m_TooltipLayout;
	
	EEditorTooltip GetTooltipType()
	{
		return m_TooltipType;
	}
	ResourceName GetTooltipLayout()
	{
		return m_TooltipLayout;
	}
};

enum EEditorTooltip
{
	DEFAULT,
	MODE,
	ENTITY,
	CONTEXT_ACTION,
	TOOLBAR_ACTION
};