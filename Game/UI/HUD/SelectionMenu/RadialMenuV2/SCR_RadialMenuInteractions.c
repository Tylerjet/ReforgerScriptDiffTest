//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_RadialMenuInteractions
{
	//! Default radial menu handling input references
	const static string INPUT_CONTEXT = "RadialMenuContext";
	const static string INPUT_RADIAL_X = "RadialX";
	const static string INPUT_RADIAL_Y = "RadialY";
	const static string INPUT_RADIAL_X2 = "RadialX2";
	const static string INPUT_RADIAL_Y2 = "RadialY2";
	const static string INPUT_TOGGLE = "CharacterSwitchWeaponRadial";
	const static string INPUT_TRIGGER_ACTION = "RadialTrigger";
	const static string INPUT_PAGE_NEXT = "RadialNext";
	const static string INPUT_PAGE_PREVIOUS = "RadialPrevious";
	
	//protected int m_iHandlerId;
	
	//! The minimum input magnitude for selection to be valid
	protected float m_fMinInputMagnitude = 0.25;
	
	//! Last selected element or null if none
	protected BaseSelectionMenuEntry m_pCurrentSelection;
	
	//! Filter used for filtering active/inactive actions
	protected ref SCR_RadialMenuFilter m_pFilter = new SCR_RadialMenuFilter();
	
	protected InputManager m_InputManager;
	
	protected bool m_bCanOpenMenu = true;
	protected bool m_bIsMenuOpen = false;
	protected bool m_bUsingLeftStick = true;
	protected bool m_bEntryTriggered;
	
	protected string m_sHandlingContext;
	protected string m_sInputToggle;
	protected string m_sInput_RadialX;
	protected string m_sInput_RadialY;
	protected string m_sInputPerform;
	protected string m_sInputPageNext; 
	protected string m_sInputPagePrev;
	
	protected bool m_bHoldToOpen;
	protected float m_fPointerAngle = 0;
	protected ERadialMenuPerformType m_iEntryPerformType;
	protected IEntity m_Owner;
	
	//! Script invokers
	ref ScriptInvoker<IEntity, bool> onMenuToggleInvoker;
	ref ScriptInvoker<IEntity> onPerformInputCallInvoker;
	ref ScriptInvoker<IEntity, vector, float, bool> onThumbstickMoveInvoker;
	ref ScriptInvoker m_OnPageSwitch = new ref ScriptInvoker;
	
	//------------------------------------------------------------------------------------------------
	protected void OnMenuToggle()
	{
		if (!m_bCanOpenMenu)
			return;
		
		m_bIsMenuOpen = !m_bIsMenuOpen;
		
		if (!m_bIsMenuOpen && m_iEntryPerformType == ERadialMenuPerformType.OnClose)
			onPerformInputCallInvoker.Invoke(m_Owner);
		
		onMenuToggleInvoker.Invoke(m_Owner, m_bIsMenuOpen);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnPerformInputCall()
	{
		if (m_iEntryPerformType == ERadialMenuPerformType.OnPressPerformInput)
			onPerformInputCallInvoker.Invoke(m_Owner);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnThumbstickMove(vector direction, float angle, bool isMouseInput)
	{
		onThumbstickMoveInvoker.Invoke(m_Owner, direction, angle, isMouseInput);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Setting of input renferences - all has default strings 
	void SetHandling(IEntity owner, bool leftStick = true, string toggle = INPUT_TOGGLE, string conxtext = INPUT_CONTEXT, string trigger = INPUT_TRIGGER_ACTION)
	{
		m_Owner = owner;
		m_sHandlingContext = conxtext;
		m_sInputToggle = toggle;
		m_sInputPerform = trigger;
		
		// Stick use
		m_bUsingLeftStick = leftStick;
		if (leftStick)
		{
			m_sInput_RadialX = INPUT_RADIAL_X;
			m_sInput_RadialY = INPUT_RADIAL_Y;
		}
		else
		{
			m_sInput_RadialX = INPUT_RADIAL_X2;
			m_sInput_RadialY = INPUT_RADIAL_Y2;
		}
		
		if (!m_InputManager)
			return;
		
		m_InputManager.AddActionListener(m_sInputToggle, EActionTrigger.DOWN, OnMenuToggle);
		if (m_bHoldToOpen)
			m_InputManager.AddActionListener(m_sInputToggle, EActionTrigger.UP, OnMenuToggle);
		
		m_InputManager.AddActionListener(m_sInputPerform, EActionTrigger.DOWN, OnPerformInputCall);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Unput references for paging 
	void SetHandlingPaging(string pageNext = INPUT_PAGE_NEXT, string pagePrev = INPUT_PAGE_PREVIOUS)
	{
		m_sInputPageNext = pageNext;
		m_sInputPagePrev = pagePrev;
		
		if (!m_InputManager)
			return;
		
		m_InputManager.AddActionListener(m_sInputPageNext, EActionTrigger.DOWN, OnPageNext);
		m_InputManager.AddActionListener(m_sInputPagePrev, EActionTrigger.DOWN, OnPagePrev);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnPageNext() { m_OnPageSwitch.Invoke(1) }
	
	//------------------------------------------------------------------------------------------------
	protected void OnPagePrev() { m_OnPageSwitch.Invoke(-1) }
	
	//------------------------------------------------------------------------------------------------
	protected float GetClampedAngle(float x, float y, int elementCount)
	{
		if (elementCount == 0)
			return 0.0;

		float angle = Math.Atan2(x,y);
		float step = (2 * Math.PI) / (float)elementCount;
		float as = angle/step;
		float reg = Math.Round(as);

		return reg * step;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Calls for entry performing
	void PerformInteractions(IEntity owner)
	{
		m_InputManager.ActivateContext(m_sHandlingContext);
		PointingAngle(owner);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Handle pointing direciton of INPUT_AXIS
	void PointingAngle(IEntity owner)
	{
		if (!m_InputManager)
			return;
		
		string actionX, actionY;
		bool usingMouse = m_InputManager.IsUsingMouseAndKeyboard();
		if (usingMouse)
		{
			actionX = "RadialMouseX";
			actionY = "RadialMouseY";
		}
		else
		{
			actionX = m_sInput_RadialX;
			actionY = m_sInput_RadialY;
		}

		vector v;
		v[0] = m_InputManager.GetActionValue(actionX);
		v[1] = m_InputManager.GetActionValue(actionY);

		if (usingMouse)
			v.Normalize();
		
		m_fPointerAngle = Math.Atan2(v[0], v[1]);
		OnThumbstickMove(v, m_fPointerAngle, usingMouse);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetHandlerId(int id)
	{
		//m_iHandlerId = id;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetHandlingContext(string inputStr)
	{
		m_sHandlingContext = inputStr;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetInputToggle(string inputStr)
	{
		m_InputManager.RemoveActionListener(m_sInputToggle, EActionTrigger.DOWN, OnMenuToggle);
		m_sInputToggle = inputStr;
		m_InputManager.AddActionListener(m_sInputToggle, EActionTrigger.DOWN, OnMenuToggle);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetInputRadialX(string inputStr)
	{
		m_sInput_RadialX = inputStr;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetInputRadialY(string inputStr)
	{
		m_sInput_RadialY = inputStr;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetInputPerform(string inputStr)
	{
		m_sInputPerform = inputStr;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetHoldToggleToOpen(bool isTrue)
	{
		m_bHoldToOpen = isTrue;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetEntryPerformType(ERadialMenuPerformType type)
	{
		m_iEntryPerformType = type;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetCanOpenMenu(bool canOpen)
	{
		m_bCanOpenMenu = canOpen;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetCanOpenMenu()
	{
		return m_bCanOpenMenu;
	}

	//------------------------------------------------------------------------------------------------
	void SCR_RadialMenuInteractions()
	{
		m_InputManager = GetGame().GetInputManager();
		
		// Define invokers
		onMenuToggleInvoker = new ref ScriptInvoker<int, IEntity, bool>();
		onPerformInputCallInvoker = new ref ScriptInvoker<int, IEntity, float>();
		onThumbstickMoveInvoker = new ref ScriptInvoker<int, IEntity, vector, float, bool>();
		
		//m_iHandlerId = handlerId;
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_RadialMenuInteractions()
	{
		if (!m_InputManager)
			return;
		
		m_InputManager.RemoveActionListener(m_sInputPageNext, EActionTrigger.DOWN, OnPageNext);
		m_InputManager.RemoveActionListener(m_sInputPagePrev, EActionTrigger.DOWN, OnPagePrev);
		m_InputManager.RemoveActionListener(m_sInputToggle, EActionTrigger.UP, OnMenuToggle);
		m_InputManager.RemoveActionListener(m_sInputPerform, EActionTrigger.DOWN, OnPerformInputCall);
	}
};
