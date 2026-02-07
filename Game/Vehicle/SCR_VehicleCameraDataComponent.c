[ComponentEditorProps(category: "GameScripted/Vehicle", description:"Vehicle camera data", color: "0 0 255 255")]
class SCR_VehicleCameraDataComponentClass: ScriptComponentClass
{
};


class SCR_VehicleCameraDataComponent : ScriptComponent
{
	[Attribute("1", UIWidgets.EditBox, "Height the camera should stay above the vehicle's bounding box center (in m)")]
	float m_fHeight;
	[Attribute("1", UIWidgets.EditBox, "Speed-based view bobbing multiplier")]
	float m_fBobScale;
	[Attribute("1", UIWidgets.EditBox, "Acceleration-based view shaking multiplier")]
	float m_fShakeScale;
	[Attribute("150", UIWidgets.EditBox, "Maximum speed at which camera distance and FOV adjustments are at their maximum (in km/h)")]
	float m_fSpeedMax;
	[Attribute("8", UIWidgets.EditBox, "Maximum distance the camera should keep from the vehicle (in m)")]
	float m_fDist_Max;
	[Attribute("3", UIWidgets.EditBox, "Minimum distance the camera should keep from the vehicle (in m)")]
	float m_fDist_Min;
	[Attribute("5", UIWidgets.EditBox, "Desired distance the camera should keep from the vehicle (in m)")]
	float m_fDist_Desired;
	[Attribute("70", UIWidgets.EditBox, "Field Of View of the camera (in deg)")]
	float m_fFOV;
	[Attribute("10", UIWidgets.EditBox, "Maximum Field Of View adjustment based on speed (in deg)")]
	float m_fFOV_SpeedAdjustMax;
	[Attribute("0.3", UIWidgets.Slider, "How much roll (0=0%, 1=100%, ...5=500% of original) is applied of the parent vehicle roll.", params: "0 5 0.001")]
	float m_fRollFactor;
	[Attribute("0.1", UIWidgets.Slider, "How much pitch (0=0%, 1=100%, ...5=500% of original) is applied of the parent vehicle pitch.", params: "0 5 0.001")]
	float m_fPitchFactor;
	[Attribute("20", UIWidgets.EditBox, "Angle in third person added to the camera on the vehicle (in deg)")]
	float m_fAngleThirdPerson;
	[Attribute(uiwidget: UIWidgets.Object)]
	ref SCR_VehicleCameraAimpoint m_pCameraAimpointData;
	[Attribute(uiwidget: UIWidgets.Object)]
	ref SCR_VehicleCameraAlignment m_pCameraAlignData;
};