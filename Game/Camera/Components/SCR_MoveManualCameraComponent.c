[BaseContainerProps(), SCR_BaseManualCameraComponentTitle()]
/** @ingroup ManualCamera
*/

/*!
Camera movement above sea level.
*/
class SCR_MoveManualCameraComponent : SCR_BaseManualCameraComponent
{
	[Attribute(defvalue: "27", desc: "Speed coefficient.")]
	private float m_fSpeed;	
	
	override void EOnCameraSave(SCR_ManualCameraComponentSave data)
	{
		vector transform[4];
		GetCameraEntity().GetWorldTransform(transform);
		vector angles = Math3D.MatrixToAngles(transform);
		
		data.m_aValues = {
			transform[3][0], transform[3][1], transform[3][2], //--- Pos
			angles[0], angles[1] //--- Yaw, pitch (don't save roll, this component cannot change)
		};
	}
	override void EOnCameraLoad(SCR_ManualCameraComponentSave data)
	{
		if (!data.m_aValues || data.m_aValues.Count() < 5)
			return;
		
		vector transform[4];
		transform[3] = Vector(data.m_aValues[0], data.m_aValues[1], data.m_aValues[2]);
		Math3D.AnglesToMatrix(Vector(data.m_aValues[3], data.m_aValues[4], 0), transform);
		
		GetCameraEntity().SetWorldTransform(transform);
	}
	override void EOnCameraFrame(SCR_ManualCameraParam param)
	{
		if (!param.isManualInputEnabled) return;
		
		float lateral = GetInputManager().GetActionValue("ManualCameraMoveLateral");
		float vertical = GetInputManager().GetActionValue("ManualCameraMoveVertical");
		float longitudinal = GetInputManager().GetActionValue("ManualCameraMoveLongitudinal");
		if (lateral == 0 && vertical == 0 && longitudinal == 0) return;
		
		//--- Get horizontal vector
		vector dir = param.transform[2];
		dir[1] = 0;
		dir.Normalize();
		
		float horizontalSpeedCoef = param.multiplier[0] * m_fSpeed;
		param.transform[3] = param.transform[3] + Vector(
			(dir[0] * longitudinal + dir[2] * lateral) * horizontalSpeedCoef,
			vertical * param.multiplier[1] * m_fSpeed,
			(dir[2] * longitudinal - dir[0] * lateral) * horizontalSpeedCoef,
		);
		param.isManualInput = true;
		param.isDirty = true;
	}
	override bool EOnCameraInit()
	{
		return true;
	}
};






















