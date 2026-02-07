[BaseContainerProps()]
class SCR_InfoDisplaySlotHandler : SCR_InfoDisplayHandler
{
	[Attribute()]
	protected string m_sGroupName;

	[Attribute()]
	protected string m_sSlotName;

	protected float m_fWidth, m_fHeight;
	protected Widget m_wSlot;
	protected Widget m_wContent;
	protected SCR_HUDGroupUIComponent m_GroupComponent;
	protected SCR_HUDSlotUIComponent m_SlotComponent;
	protected WorkspaceWidget m_wWorkspace;
	protected bool m_bIsVisible;

	//------------------------------------------------------------------------------------------------
	SCR_HUDSlotUIComponent GetSlotUIComponent()
	{
		return m_SlotComponent;
	}

	//------------------------------------------------------------------------------------------------
	SCR_HUDGroupUIComponent GetGroupUIComponent()
	{
		return m_GroupComponent;
	}

	string GetGroupName()
	{
		return m_sGroupName;
	}

	string GetSlotName()
	{
		return m_sSlotName;
	}

	void SetSlotComponent(SCR_HUDSlotUIComponent slot)
	{
		m_SlotComponent = slot;
	}

	void SetGroupComponent(SCR_HUDGroupUIComponent group)
	{
		m_GroupComponent = group;
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnStart(notnull SCR_InfoDisplay display)
	{
#ifdef WORKBENCH
		if (!m_bCanUpdate)
			Print("SCR_InfoDisplaySlotHandler requires CanUpdate to be set to true!", LogLevel.WARNING);
#endif
		m_wWorkspace = GetGame().GetWorkspace();

		SCR_HUDManagerComponent hudManager = null;

		if (!GrabHudComponents(m_GroupComponent, m_SlotComponent, hudManager))
		{
			Print("Failed To Create A Slot: " + m_sSlotName + ". One of the properties of the Info Dispaly is not defined properly, witch is preventing the initialization.", LogLevel.ERROR);
			return;
		}

		m_wSlot = m_SlotComponent.GetRootWidget();
		if (!m_wSlot)
			return;

		if (m_wSlot.GetChildren())
		{
			Print("Slot Already Taken: " + m_sSlotName + ". The Slot an Info Display is trying to occupy is already occupied!", LogLevel.ERROR);
			return;
		}

		Widget displayWidget = m_wWorkspace.CreateWidgets(display.m_LayoutPath, m_wSlot);
		if (!displayWidget)
			return;

		display.SetRootWidget(displayWidget);
		display.SetContentWidget(displayWidget);
		display.RegisterToHudManager();

		m_bIsVisible = displayWidget.IsVisible();
		m_GroupComponent.AddSlotToGroup(m_SlotComponent);

		m_SlotComponent.Initialize();
		m_SlotComponent.SetContentWidget(displayWidget);
		m_SlotComponent.GetOnResize().Insert(OnSlotResize);
		
		Print(m_OwnerDisplay.Type());
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnStop(notnull SCR_InfoDisplay display)
	{
		if (m_SlotComponent)
		{
			//m_SlotComponent.SetHeight(0, true);
			//m_SlotComponent.SetWidth(0, true);
			m_SlotComponent.GetOnResize().Remove(OnSlotResize);
		}

		if (m_GroupComponent)
		{
			//m_GroupComponent.RemoveSlotFromGroup(m_SlotComponent);
			m_GroupComponent.ResizeGroup();
		}

		//if (m_wContent)
			//m_wContent.RemoveFromHierarchy();
	}

	//------------------------------------------------------------------------------------------------
	protected bool GrabHudComponents(out SCR_HUDGroupUIComponent groupComponent, out SCR_HUDSlotUIComponent slotComponent, out SCR_HUDManagerComponent hudManager)
	{
		hudManager = SCR_HUDManagerComponent.GetHUDManager();
		if (!hudManager)
			return false;

		slotComponent = hudManager.GetSlotComponentByName(m_sSlotName);
		if (!slotComponent)
			return false;

		Widget slotWidget = slotComponent.GetRootWidget();
		if (!slotWidget)
			return false;

		Widget groupWidget = slotWidget.GetParent();
		if (!groupWidget)
			return false;

		groupComponent = SCR_HUDGroupUIComponent.Cast(groupWidget.FindHandler(SCR_HUDGroupUIComponent));
		if (!groupComponent)
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	private void OnSlotResize(notnull SCR_HUDSlotUIComponent slot)
	{
		m_fWidth = slot.GetWidth();
		m_fHeight = slot.GetHeight();
	}

	//------------------------------------------------------------------------------------------------
	override void OnUpdate(float timeSlice)
	{
		if (!m_wSlot)
			return;

		float width, height;
		m_wSlot.GetScreenSize(width, height);

		width = m_wWorkspace.DPIUnscale(width);
		height = m_wWorkspace.DPIUnscale(height);

		bool isVisible = m_wSlot.IsVisible();

		if (!float.AlmostEqual(width, m_fWidth, 0.1) || !float.AlmostEqual(height, m_fHeight, 0.1) || m_bIsVisible != isVisible)
		{
			m_GroupComponent.ResizeGroup();
			m_fHeight = height;
			m_fWidth = width;
			m_bIsVisible = isVisible;
		}

	}
};
