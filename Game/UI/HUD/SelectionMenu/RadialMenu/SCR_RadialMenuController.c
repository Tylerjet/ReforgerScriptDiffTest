/*!
Class created for quick application of radial menu into components
It finds global menu, holds controller settings and data required for menu
*/

//------------------------------------------------------------------------------------------------
[BaseContainerProps(configRoot: true)]
class SCR_RadialMenuController
{
	[Attribute("1")]
	protected bool m_sEnableControl;

	[Attribute()]
	protected ref SCR_SelectionMenuControllerInputs m_RMControls;

	[Attribute()]
	protected ref SCR_SelectionMenuData m_Data;

	protected SCR_RadialMenu m_RadialMenu;
	protected IEntity m_Owner;

	// Callbacks
	protected ref ScriptInvoker<SCR_RadialMenuController, bool> m_OnControllerChanged;

	//------------------------------------------------------------------------------------------------
	protected void InvokeOnControllerChanged(bool hasControl)
	{
		if (m_OnControllerChanged)
			m_OnControllerChanged.Invoke(this, hasControl);
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnControllerChanged()
	{
		if (!m_OnControllerChanged)
			m_OnControllerChanged = new ScriptInvoker();

		return m_OnControllerChanged;
	}

	//------------------------------------------------------------------------------------------------
	//! Take control over selected menu
	//! Filling no menu will take control over the global radial menu
	void Control(IEntity owner, SCR_RadialMenu radialMenu = null)
	{
		// Clear previous inputs
		if (radialMenu && m_RMControls)
		{
			GetGame().GetInputManager().RemoveActionListener(
				m_RMControls.m_sOpenAction, EActionTrigger.DOWN, OnOpenRadialMenuInput);

			m_RadialMenu.GetOnControllerChanged().Remove(OnMenuControllerChanged);
		}

		m_Owner = owner;

		// Setup radial menu
		if (radialMenu == null)
			radialMenu = SCR_RadialMenu.GlobalRadialMenu();

		m_RadialMenu = radialMenu;

		if (m_RadialMenu)
			m_RadialMenu.GetOnControllerChanged().Insert(OnMenuControllerChanged);

		// Inputs
		if (m_RMControls)
		{
			GetGame().GetInputManager().AddActionListener(
				m_RMControls.m_sOpenAction, EActionTrigger.DOWN, OnOpenRadialMenuInput);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Stop opening controls of current menu
	void StopControl(bool closeMenu = false)
	{
		if (m_RadialMenu)
		{
			m_RadialMenu.GetOnControllerChanged().Remove(OnMenuControllerChanged);

			if (closeMenu)
				m_RadialMenu.Close();

			m_RadialMenu = null;
		}

		// Inputs
		if (m_RMControls)
		{
			GetGame().GetInputManager().RemoveActionListener(
				m_RMControls.m_sOpenAction, EActionTrigger.DOWN, OnOpenRadialMenuInput);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Return true if cached menu is controlled by this owner entity
	bool HasControl()
	{
		if (!m_RadialMenu)
		{
			Print("[SCR_RadialMenuController] - has no menu selected to control!", LogLevel.WARNING);
			return false;
		}

		if (!m_RadialMenu.GetControllerInputs())
			return false;

		return m_RadialMenu.GetControllerInputs().m_Owner == m_Owner;
	}

	//------------------------------------------------------------------------------------------------
	void UpdateMenuData()
	{
		if (m_RadialMenu)
			m_RadialMenu.AddEntries(m_Data.GetEntries(), true);
	}

	//------------------------------------------------------------------------------------------------
	// Callbacks
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	protected void OnOpenRadialMenuInput()
	{
		if (!m_sEnableControl || !m_RadialMenu)
			return;

		m_RadialMenu.SetController(m_Owner, m_RMControls);
		m_RadialMenu.Open();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMenuControllerChanged(SCR_SelectionMenu menu, SCR_SelectionMenuControllerInputs controller)
	{
		// Check menu
		if (menu != m_RadialMenu)
			return;

		// Check control
		bool hasControl = m_Owner == controller.m_Owner;

		if (!hasControl)
			StopControl();

		InvokeOnControllerChanged(hasControl);
	}

	//------------------------------------------------------------------------------------------------
	// Get
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	void SetEnableControl(bool enable)
	{
		m_sEnableControl = enable;
	}

	//------------------------------------------------------------------------------------------------
	bool GetEnableControl()
	{
		return m_sEnableControl;
	}

	//------------------------------------------------------------------------------------------------
	SCR_RadialMenu GetRadialMenu()
	{
		return m_RadialMenu;
	}

	//------------------------------------------------------------------------------------------------
	SCR_SelectionMenuControllerInputs GetControls()
	{
		return m_RMControls;
	}

	//------------------------------------------------------------------------------------------------
	SCR_SelectionMenuData GetData()
	{
		return m_Data;
	}
};
