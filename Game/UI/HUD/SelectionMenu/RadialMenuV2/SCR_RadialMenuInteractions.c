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
	const static string INPUT_TOGGLE = "";
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
	ref ScriptInvoker<IEntity> onAttemptMenuOpenInvoker = new ScriptInvoker();
	ref ScriptInvoker<IEntity, bool> onMenuToggleInvoker;
	ref ScriptInvoker<IEntity> onPerformInputCallInvoker;
	ref ScriptInvoker<IEntity> onMenuOpenFailed = new ScriptInvoker();
	ref ScriptInvoker<IEntity, vector, float, bool> onThumbstickMoveInvoker;
	ref ScriptInvoker m_OnPageSwitch = new ref ScriptInvoker;
	
	//------------------------------------------------------------------------------------------------
	void Open()
	{
		if (m_bIsMenuOpen)
			return;
		
		onAttemptMenuOpenInvoker.Invoke(m_Owner);
		
		if (!m_bCanOpenMenu || !CanBeOpened())
		{
			onMenuOpenFailed.Invoke(m_Owner);
			return;
		}
		
		m_bIsMenuOpen = true;
		onMenuToggleInvoker.Invoke(m_Owner, true);
	}
	
	//------------------------------------------------------------------------------------------------
	void Close()
	{
		if (!m_bIsMenuOpen)
			return;
		
		m_bIsMenuOpen = false;
		onMenuToggleInvoker.Invoke(m_Owner, false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Additional conditions for opening the menu
	protected bool CanBeOpened()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnToggleInput(float value, EActionTrigger reason)
	{
		if (!m_bCanOpenMenu && !m_bIsMenuOpen)
		{
			onMenuOpenFailed.Invoke(m_Owner);
			return;
		}
		
		bool open;
		
		if (m_bHoldToOpen)
			open = reason == EActionTrigger.DOWN;
		else
			open = !open;
		
		if (m_bCanOpenMenu && !open && (m_iEntryPerformType == ERadialMenuPerformType.OnClose || m_iEntryPerformType == ERadialMenuPerformType.OnCloseOrPress))
			onPerformInputCallInvoker.Invoke(m_Owner);
		
		if (open)
			Open();
		else
			Close();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnPerformInput(float value, EActionTrigger reason)
	{
		if (m_bCanOpenMenu && (m_iEntryPerformType == ERadialMenuPerformType.OnPressPerformInput || m_iEntryPerformType == ERadialMenuPerformType.OnCloseOrPress))
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
		
		// Stick use
		// TODO - separate context when using left stick isntead of letting CharacterForward and CharacterRight through
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
		
		SetInputToggle(toggle);
		SetInputPerform(trigger);
	}
	
	//------------------------------------------------------------------------------------------------
	string GetHandlingContext()
	{
		return m_sHandlingContext;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Unput references for paging 
	void SetHandlingPaging(string pageNext = INPUT_PAGE_NEXT, string pagePrev = INPUT_PAGE_PREVIOUS)
	{
		InputManager inputManager = GetGame().GetInputManager();
		if (!inputManager)
			return;
		
		if (!m_sInputPageNext.IsEmpty())
			inputManager.RemoveActionListener(m_sInputPageNext, EActionTrigger.DOWN, OnPageNext);
		
		if (!m_sInputPagePrev.IsEmpty())
			inputManager.RemoveActionListener(m_sInputPagePrev, EActionTrigger.DOWN, OnPagePrev);
		
		m_sInputPageNext = pageNext;
		m_sInputPagePrev = pagePrev;
		
		if (!m_sInputPageNext.IsEmpty())
			inputManager.AddActionListener(m_sInputPageNext, EActionTrigger.DOWN, OnPageNext);
		
		if (!m_sInputPagePrev.IsEmpty())
			inputManager.AddActionListener(m_sInputPagePrev, EActionTrigger.DOWN, OnPagePrev);
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
		InputManager inputManager = GetGame().GetInputManager();
		if (!inputManager)
			return;
		
		inputManager.ActivateContext(m_sHandlingContext);
		
		// Force quit when context is missing
		if (m_bHoldToOpen && !m_sInputToggle.IsEmpty() && inputManager.GetActionValue(m_sInputToggle) < 0.5)
		{
			Close();
			return;
		}
		
		PointingAngle(owner);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Handle pointing direciton of INPUT_AXIS
	void PointingAngle(IEntity owner)
	{
		InputManager inputManager = GetGame().GetInputManager();
		if (!inputManager)
			return;
		
		string actionX, actionY;
		bool usingMouse = inputManager.IsUsingMouseAndKeyboard();
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
		v[0] = inputManager.GetActionValue(actionX);
		v[1] = inputManager.GetActionValue(actionY);

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
		InputManager inputManager = GetGame().GetInputManager();
		if (!inputManager)
			return;
		
		if (!m_sInputToggle.IsEmpty())
		{
			inputManager.RemoveActionListener(m_sInputToggle, EActionTrigger.DOWN, OnToggleInput);
			if (m_bHoldToOpen)
				inputManager.RemoveActionListener(m_sInputToggle, EActionTrigger.UP, OnToggleInput);
		}
		
		m_sInputToggle = inputStr;
		
		if (!m_sInputToggle.IsEmpty())
		{
			inputManager.AddActionListener(m_sInputToggle, EActionTrigger.DOWN, OnToggleInput);
			if (m_bHoldToOpen)
				inputManager.AddActionListener(m_sInputToggle, EActionTrigger.UP, OnToggleInput);
		}
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
		InputManager inputManager = GetGame().GetInputManager();
		if (!inputManager)
			return;
		
		if (!m_sInputPerform.IsEmpty())
			inputManager.RemoveActionListener(m_sInputPerform, EActionTrigger.DOWN, OnPerformInput);
		
		m_sInputPerform = inputStr;
		
		if (!m_sInputPerform.IsEmpty())
			inputManager.AddActionListener(m_sInputPerform, EActionTrigger.DOWN, OnPerformInput);
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
		
		if (m_bIsMenuOpen && !m_bCanOpenMenu)
			Close();
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetCanOpenMenu()
	{
		return m_bCanOpenMenu;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnMenuOpenFailed()
	{
		return onMenuOpenFailed;
	}

	//------------------------------------------------------------------------------------------------
	void SCR_RadialMenuInteractions()
	{
		// Define invokers
		onMenuToggleInvoker = new ref ScriptInvoker<int, IEntity, bool>();
		onPerformInputCallInvoker = new ref ScriptInvoker<int, IEntity, float>();
		onThumbstickMoveInvoker = new ref ScriptInvoker<int, IEntity, vector, float, bool>();
		
		//m_iHandlerId = handlerId;
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_RadialMenuInteractions()
	{
		InputManager inputManager = GetGame().GetInputManager();
		if (!inputManager)
			return;
		
		inputManager.RemoveActionListener(m_sInputPageNext, EActionTrigger.DOWN, OnPageNext);
		inputManager.RemoveActionListener(m_sInputPagePrev, EActionTrigger.DOWN, OnPagePrev);
		inputManager.RemoveActionListener(m_sInputPerform, EActionTrigger.DOWN, OnPerformInput);
		inputManager.RemoveActionListener(m_sInputToggle, EActionTrigger.DOWN, OnToggleInput);
		if (m_bHoldToOpen)
			inputManager.RemoveActionListener(m_sInputToggle, EActionTrigger.UP, OnToggleInput);
	}
};
