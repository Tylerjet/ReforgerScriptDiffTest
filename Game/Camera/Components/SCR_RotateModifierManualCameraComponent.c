[BaseContainerProps(), SCR_BaseManualCameraComponentTitle()]
/** @ingroup ManualCamera
*/

/*!
Basic camera rotation with modifier button held (RMB by default)
*/
class SCR_RotateModifierManualCameraComponent : SCR_BaseManualCameraComponent
{
	private bool m_bRotate;
	
	protected void OnInputDeviceUserChanged(EInputDeviceType oldDevice, EInputDeviceType newDevice)
	{
		if (SCR_Global.IsChangedMouseAndKeyboard(oldDevice, newDevice))
			return;
		
		//--- Always rotate with controller, cancel rotation when switching to mouse and keyboard
		m_bRotate = !GetInputManager().IsUsingMouseAndKeyboard();
	}
	protected void ManualCameraRotateDown()
	{
		if (!IsEnabled()) return;
		
		if (GetCameraEntity().GetCameraParam().isCursorEnabled)
			m_bRotate = true;
	}
	protected void ManualCameraRotateUp()
	{
		if (!IsEnabled()) return;
		
		m_bRotate = false;
	}
	override void EOnCameraFrame(SCR_ManualCameraParam param)
	{
		if (!param.isManualInputEnabled) return;

		if (m_bRotate)
			param.flag = param.flag | EManualCameraFlag.ROTATE;
		else
			param.flag = param.flag & ~EManualCameraFlag.ROTATE;
	}
	override bool EOnCameraInit()
	{
		OnInputDeviceUserChanged(-1, -1);
		GetGame().OnInputDeviceUserChangedInvoker().Insert(OnInputDeviceUserChanged);
		GetInputManager().AddActionListener("ManualCameraRotate", EActionTrigger.DOWN, ManualCameraRotateDown);
		GetInputManager().AddActionListener("ManualCameraRotate", EActionTrigger.UP, ManualCameraRotateUp);
		return true;
	}
	override void EOnCameraExit()
	{
		GetGame().OnInputDeviceUserChangedInvoker().Remove(OnInputDeviceUserChanged);
		GetInputManager().RemoveActionListener("ManualCameraRotate", EActionTrigger.DOWN, ManualCameraRotateDown);
		GetInputManager().RemoveActionListener("ManualCameraRotate", EActionTrigger.UP, ManualCameraRotateUp);
	}
};