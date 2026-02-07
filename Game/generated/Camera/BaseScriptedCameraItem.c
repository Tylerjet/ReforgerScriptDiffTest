/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Camera
* @{
*/

class BaseScriptedCameraItem
{
	//! virtual callback - called when camera is created
	event void OnActivate (ScriptedCameraItem pPrevCamera, ScriptedCameraItemResult pPrevCameraResult);
	event void OnBlendIn();
	event void OnBlendOut();
	//!	virtual callback - called each frame
	event void OnUpdate(float pDt, out ScriptedCameraItemResult pOutResult);
	event void SetBaseAngles(vector angles);
};

/** @}*/
