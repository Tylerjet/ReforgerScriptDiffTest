class SCR_DeviceSpecificComponent: ScriptedWidgetComponent
{
	[Attribute("0", UIWidgets.Auto, "")]
	private bool m_bControllerOnly;
	
	private Widget m_Widget;

	protected void OnInputDeviceUserChanged()
	{
		ArmaReforgerScripted game = GetGame();
		if (!game) return;
		
		InputManager inputManager = game.GetInputManager();
		if (!inputManager) return;
		
		bool enable = m_bControllerOnly != inputManager.IsUsingMouseAndKeyboard();
		m_Widget.SetEnabled(enable);
		m_Widget.SetVisible(enable);
	}
	override void HandlerAttached(Widget w)
	{
		if (SCR_Global.IsEditMode()) return;
		
		m_Widget = w;
		
		ArmaReforgerScripted game = GetGame();
		if (!game) return;
		
		ScriptInvoker invoker = game.OnInputDeviceUserChangedInvoker();
		if (!invoker) return;
		
		invoker.Insert(OnInputDeviceUserChanged);
		OnInputDeviceUserChanged();
	}
	override void HandlerDeattached(Widget w)
	{
		if (SCR_Global.IsEditMode()) return;
		
		ArmaReforgerScripted game = GetGame();
		if (!game) return;
		
		ScriptInvoker invoker = game.OnInputDeviceUserChangedInvoker();
		if (!invoker) return;
		
		invoker.Remove(OnInputDeviceUserChanged);
	}
};