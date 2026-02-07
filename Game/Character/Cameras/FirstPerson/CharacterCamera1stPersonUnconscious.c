// *************************************************************************************
// ! CharacterCamera1stPersonUnconscious - first person only unconscious
// *************************************************************************************
class CharacterCamera1stPersonUnconscious extends CharacterCamera1stPerson
{
	void CharacterCamera1stPersonUnconscious(CameraHandlerComponent pCameraHandler)
	{
	}
	
	override void OnUpdate(float pDt, out ScriptedCameraItemResult pOutResult)
	{
		super.OnUpdate(pDt, pOutResult);

		pOutResult.m_iDirectBone 		= GetCameraBoneIndex();
		pOutResult.m_iDirectBoneMode 	= EDirectBoneMode.RelativeDirection;
		pOutResult.m_fUseHeading 		= 0.0;
		
		vector rot;
		rot[0] = 0;
		rot[1] = 90;
		rot[2] = 0;
		
		Math3D.AnglesToMatrix(rot, pOutResult.m_CameraTM);
	}
};
