// *************************************************************************************
// ! CharacterCamera1stPerson - 1st person camera
// *************************************************************************************
class CharacterCamera1stPerson extends CharacterCameraBase
{
	//-----------------------------------------------------------------------------
	static const float 	CONST_UD_MIN	= -89.0;		//!< down limit
	static const float 	CONST_UD_MAX	= 89.0;			//!< up limit

	static const float 	CONST_LR_MIN	= -160.0;		//!< left limit
	static const float 	CONST_LR_MAX	= 160.0;		//!< right limit
	
	//-----------------------------------------------------------------------------
	void CharacterCamera1stPerson(CameraHandlerComponent pCameraHandler)
	{
		m_pCompartmentAccess = m_OwnerCharacter.GetCompartmentAccessComponent();
		// in head bone space
		m_OffsetLS = "0.0 0.03 -0.07";
		m_ApplyHeadBob = true;
	}
	
	//-----------------------------------------------------------------------------
	override void OnActivate(ScriptedCameraItem pPrevCamera, ScriptedCameraItemResult pPrevCameraResult)
	{
		super.OnActivate(pPrevCamera, pPrevCameraResult);
		
		if (pPrevCamera)
		{
			//vector f = pPrevCamera.GetBaseAngles();
			//m_fUpDownAngle = m_ControllerComponent.GetAimingAngles()[1];
			//m_fLeftRightAngle = f[1];
		}
		
		m_bCameraTransition = false;
	}
	
	//-----------------------------------------------------------------------------
	override void OnUpdate(float pDt, out ScriptedCameraItemResult pOutResult)
	{
		//! update angles
		float udAngle = UpdateUDAngle(m_fUpDownAngle, CONST_UD_MIN, CONST_UD_MAX, pDt);
		m_fLeftRightAngle = UpdateLRAngle(m_fLeftRightAngle, CONST_LR_MIN, CONST_LR_MAX, pDt);
		
		//Print("m_fLeftRightAngle : " + m_fLeftRightAngle);
		
		pOutResult.m_vBaseAngles 		= GetBaseAngles();
		pOutResult.m_fUseHeading 		= 1.0;
		pOutResult.m_iDirectBoneMode 	= EDirectBoneMode.RelativePosition;
					
		//! yaw pitch roll vector
		vector lookAngles;
		lookAngles[0] = m_fLeftRightAngle;
		lookAngles[1] = Math.Clamp(udAngle + GetInterpolatedUDTransformAngle(pDt), CONST_UD_MIN, CONST_UD_MAX);
		lookAngles[2] = 0;
		
		//! update fov
		m_fFOV = GetBaseFOV();
		if (m_bCameraTransition)
		{
			pOutResult.m_fUseHeading = 0.0;
			lookAngles[0] = lookAngles[0] + 180.0;
			pOutResult.m_iDirectBoneMode = EDirectBoneMode.RelativeDirection;
		}
		//! apply to rotation matrix
		Math3D.AnglesToMatrix(lookAngles, pOutResult.m_CameraTM);
		
		// position
		pOutResult.m_CameraTM[3]		= m_OffsetLS;
		if( m_ApplyHeadBob )
			m_CharacterCameraHandler.AddViewBobToTransform(pOutResult.m_CameraTM, 1, false);

		pOutResult.m_iDirectBone 			= GetCameraBoneIndex();
		pOutResult.m_fFOV		 			= m_fFOV;
		pOutResult.m_fNearPlane	 			= 0.0125;
		pOutResult.m_bUpdateWhenBlendOut 	= true;
		pOutResult.m_fPositionModelSpace 	= 0.0;
		pOutResult.m_bAllowInterpolation 	= true;
		pOutResult.m_pOwner 				= m_OwnerCharacter;
		pOutResult.m_pWSAttachmentReference = null;
		
		// Apply shake
		if (m_CharacterCameraHandler)
			m_CharacterCameraHandler.AddShakeToToTransform(pOutResult.m_CameraTM, pOutResult.m_fFOV);
	}

	//-----------------------------------------------------------------------------
	override float GetBaseFOV()
	{
		return GetGame().GetCameraManager().GetFirstPersonFOV();
	}
	
	//-----------------------------------------------------------------------------
	protected	vector	m_OffsetLS;			//!< position offset 

	protected	bool	m_ApplyHeadBob;

	protected bool m_bCameraTransition = false;
	protected CompartmentAccessComponent m_pCompartmentAccess;
};


class CharacterCamera1stPersonDeath extends CharacterCamera1stPerson
{
	override void OnUpdate(float pDt, out ScriptedCameraItemResult pOutResult)
	{
		vector lookAngles;
		lookAngles[0] = 180.0;
		lookAngles[1] = 0;
		lookAngles[2] = 0;

		//! apply to rotation matrix
		Math3D.AnglesToMatrix(lookAngles, pOutResult.m_CameraTM);
		// position
		pOutResult.m_CameraTM[3]			= m_OffsetLS;
		pOutResult.m_iDirectBone 			= GetCameraBoneIndex();
		pOutResult.m_fUseHeading 			= 0.0;
		pOutResult.m_fDistance 				= 0;
		pOutResult.m_iDirectBoneMode 		= EDirectBoneMode.RelativeTransform;
		pOutResult.m_fFOV		 			= m_fFOV;
		pOutResult.m_fNearPlane	 			= 0.0125;
		pOutResult.m_bUpdateWhenBlendOut 	= true;
		pOutResult.m_fPositionModelSpace 	= 0.0;
		pOutResult.m_bAllowInterpolation 	= true;
		pOutResult.m_pOwner 				= m_OwnerCharacter;
		pOutResult.m_pWSAttachmentReference = null;
	}
};
