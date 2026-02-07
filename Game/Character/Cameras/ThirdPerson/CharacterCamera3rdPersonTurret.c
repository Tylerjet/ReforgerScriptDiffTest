// *************************************************************************************
// ! CharacterCamera3rdPersonTurret - 3rd person camera when character is controlling turret
// *************************************************************************************
class CharacterCamera3rdPersonTurret extends CharacterCameraBase
{

	static const float 	CONST_UD_MIN = -45.0; //!< down limit
	static const float 	CONST_UD_MAX = 30.0; //!< up limit
	static const float 	CONST_UD_CAMERA_ANGLE_OFFSET = -10.0; //!< camera angle offset

	private TurretControllerComponent m_pTurretController;
	private TurretComponent m_pControlledTurret;
	private vector m_vVertAimLimits;
	private vector m_vHorAimLimits;
	private vector m_vCameraCenter;
	private float m_fLRAngleAdd;
	private float m_fHeight;
	private float m_fDist_Min;
	private IEntity m_OwnerVehicle;
	//-----------------------------------------------------------------------------
	void CharacterCamera3rdPersonTurret(CameraHandlerComponent pCameraHandler)
	{
		m_bLRAngleNoLimit = true;
	}
	//-----------------------------------------------------------------------------
	override void OnActivate(ScriptedCameraItem pPrevCamera, ScriptedCameraItemResult pPrevCameraResult)
	{
		super.OnActivate(pPrevCamera, pPrevCameraResult);
		
		if (pPrevCamera)
		{
			vector f = pPrevCamera.GetBaseAngles();
			m_fUpDownAngle		= f[0]; 
			m_fLeftRightAngle	= f[1]; 
		}
		
		CompartmentAccessComponent compartmentAccess = m_OwnerCharacter.GetCompartmentAccessComponent();
		if (compartmentAccess && compartmentAccess.IsInCompartment())
		{
			auto compartment = compartmentAccess.GetCompartment();
			
			ForceFreelook(compartment.GetForceFreeLook());
			
			m_OwnerVehicle = compartment.GetVehicle();
			IEntity turret = compartment.GetOwner();
			if (turret)
			{
				BaseControllerComponent controller = compartment.GetController();
				if (controller)
					m_pTurretController = TurretControllerComponent.Cast(controller);
				
				vector mins, maxs;
				turret.GetBounds(mins, maxs);
				m_vCameraCenter = (maxs - mins) * 0.5 + mins;
				
				SCR_VehicleCameraDataComponent vehicleCamData = SCR_VehicleCameraDataComponent.Cast(turret.FindComponent(SCR_VehicleCameraDataComponent));
				if (vehicleCamData)
				{
					m_fHeight = vehicleCamData.m_fHeight;
					m_fDist_Min = vehicleCamData.m_fDist_Min;
					m_fFOV = vehicleCamData.m_fFOV;
					m_vCameraCenter[1] = m_vCameraCenter[1] + m_fHeight;
					m_vCameraCenter[2] = m_vCameraCenter[2] - m_fDist_Min;
					m_fRollFactor = vehicleCamData.m_fRollFactor;
					m_fPitchFactor = vehicleCamData.m_fPitchFactor;
				}
				
				// If we'll have multiple turrets, don't cache turret
				m_pControlledTurret = m_pTurretController.GetTurretComponent();
				if (!m_pControlledTurret)
					return;
				
				m_pControlledTurret.GetAimingLimits(m_vHorAimLimits, m_vVertAimLimits);
				m_vVertAimLimits[0] = CONST_UD_MIN;
				m_vVertAimLimits[1] = CONST_UD_MAX;
			}
		}
	}
	//-----------------------------------------------------------------------------
	override void OnUpdate(float pDt, out ScriptedCameraItemResult pOutResult)
	{
		if (!m_pControlledTurret)
			return;
		
		// character matrix
		vector charMat[4];
		m_OwnerCharacter.GetWorldTransform(charMat);

		// parent transform
		vector turretMat[4];
		IEntity parentEntity = m_OwnerCharacter.GetParent();
		if (!parentEntity)
			parentEntity = m_OwnerCharacter;

		parentEntity.GetWorldTransform(turretMat);
		
		// TODO@AS:
		// TODO@ZGUBA:
		// For now this seems to handle most of our typical cases,
		// but some unification, generalization and refactor (there is new api that can help)
		// would be advised
		vector aiming = m_pControlledTurret.GetAimingRotation();
		vector aimChange = m_Input.GetAimChange();
		bool freeLook = m_bForceFreeLook || m_pTurretController.IsFreeLookEnabled() || m_ControllerComponent.IsTrackIREnabled();
		
		// Freelook is using specific angles and is not very compatible with the UpdateAngleWithTarget method
		if (freeLook)
		{
			m_fLeftRightAngle = UpdateLRAngle(m_fLeftRightAngle, m_vHorAimLimits[0], m_vHorAimLimits[1], pDt);
			m_fUpDownAngle = UpdateUDAngle(m_fUpDownAngle, m_vVertAimLimits[0], m_vVertAimLimits[1], pDt);
		}
		else
		{
			m_fLeftRightAngle = CharacterCamera1stPersonTurret.UpdateAngleWithTarget(m_fLeftRightAngle, m_fLRAngleAdd, m_fLRAngleVel, m_vHorAimLimits, pDt, aiming[0], aimChange[0], freeLook);
			m_fUpDownAngle = CharacterCamera1stPersonTurret.UpdateAngleWithTarget(m_fUpDownAngle, m_fUpDownAngleAdd, m_fUDAngleVel, m_vVertAimLimits, pDt, aiming[1], aimChange[1], freeLook);
		}
		
		pOutResult.m_vBaseAngles = GetBaseAngles();
		
		//! yaw pitch roll vector
		vector lookAngles;
		lookAngles[0] = m_fLeftRightAngle;
		lookAngles[1] = m_fUpDownAngle;
		lookAngles[2] = 0;
		
		//! apply to rotation matrix
		Math3D.AnglesToMatrix(lookAngles, pOutResult.m_CameraTM);

		// position
		vector cameraPositionLS = turretMat[3].InvMultiply4(turretMat) + Vector(0, m_vCameraCenter[1], 0);
		
		CompartmentAccessComponent cac = m_OwnerCharacter.GetCompartmentAccessComponent();
		if (!(cac && (cac.IsGettingOut() || cac.IsGettingIn())))
		{
			// Offset by character height, height is not the same when entities are rotated, this is why we revert position without any rotation. Then we can do the height diff.
			float charHeight = charMat[3].InvMultiply3(charMat)[1] - turretMat[3].InvMultiply3(turretMat)[1];
			cameraPositionLS[1] = cameraPositionLS[1] - charHeight;
		}
		
		// This is our pivot point
		pOutResult.m_CameraTM[3] = cameraPositionLS;
		
		const float camDist = 2.0;
		
		// other parameters
		pOutResult.m_fUseHeading 			= 0.0;
		pOutResult.m_fFOV 					= m_fFOV;
		pOutResult.m_fDistance 				= camDist;
		pOutResult.m_bAllowInterpolation 	= true;
		pOutResult.m_pWSAttachmentReference = null;
		pOutResult.m_pOwner 				= m_OwnerCharacter;
		
		// Apply shake
		if (m_CharacterCameraHandler)
			m_CharacterCameraHandler.AddShakeToToTransform(pOutResult.m_CameraTM, pOutResult.m_fFOV);
	}
	
	//-----------------------------------------------------------------------------
	override void OnAfterCameraUpdate(float pDt, bool pIsKeyframe, inout vector transformMS[4])
	{
		AddVehiclePitchRoll(m_OwnerVehicle, pDt, transformMS);
	}
};