// *************************************************************************************
// ! CharacterCamera1stPersonVehicle - 1st person camera when character is in vehicle
// *************************************************************************************
class CharacterCamera1stPersonTurret extends CharacterCamera1stPerson
{

	private TurretControllerComponent m_pTurretController;
	private TurretComponent m_pControlledTurret;
	private TurretCompartmentSlot m_pCompartment;
	private vector m_vVertAimLimits;
	private vector m_vHorAimLimits;
	private float m_fLRAngleAdd;
	private vector m_vPrevEyePosition;
	private IEntity m_OwnerVehicle;

	//-----------------------------------------------------------------------------
	void CharacterCamera1stPersonTurret(CameraHandlerComponent pCameraHandler)
	{
		m_ApplyHeadBob = false;
		m_vPrevEyePosition = m_OwnerCharacter.EyePositionModel();
		m_OffsetLS = "0.0 0.1 0.0";
		m_bLRAngleNoLimit = true;
	}
		
	//-----------------------------------------------------------------------------
	override void OnActivate(ScriptedCameraItem pPrevCamera, ScriptedCameraItemResult pPrevCameraResult)
	{
		super.OnActivate(pPrevCamera, pPrevCameraResult);

		if (m_pCompartmentAccess && m_pCompartmentAccess.IsInCompartment())
		{
			BaseCompartmentSlot compartment = m_pCompartmentAccess.GetCompartment();
			if (compartment)
			{
				ForceFreelook(compartment.GetForceFreeLook());
				
				m_OwnerVehicle = compartment.GetVehicle();
				
				SCR_VehicleCameraDataComponent vehicleCamData = SCR_VehicleCameraDataComponent.Cast(m_OwnerVehicle.FindComponent(SCR_VehicleCameraDataComponent));
				if (vehicleCamData)
				{
					m_fRollFactor = vehicleCamData.m_fRollFactor;
					m_fPitchFactor = vehicleCamData.m_fPitchFactor;
				}
				
				BaseControllerComponent controller = compartment.GetController();
				if (controller)
				{
					m_pTurretController = TurretControllerComponent.Cast(controller);
				}
				// If we'll have multiple turrets, don't cache turret
				m_pControlledTurret = m_pTurretController.GetTurretComponent();
				if (!m_pControlledTurret)
				{
					return;
				}

				m_pControlledTurret.GetAimingLimits(m_vHorAimLimits, m_vVertAimLimits);
				m_pCompartment = TurretCompartmentSlot.Cast(compartment);
			}
		}
		
		m_fLRAngleAdd = 0;
	}
	//-----------------------------------------------------------------------------
	static float UpdateAngleWithTarget(out float pAngle, out float pAngleAdd, out float addVelocity, vector limits, float pDt, float target, float change, bool freeLook)
	{
		//! update it in degrees
		pAngle = target;
		pAngle = Math.Clamp(pAngle, limits[0], limits[1]);
		// remove freelook when switched back to controlled state
		pAngleAdd = Math.SmoothCD(pAngleAdd, 0, addVelocity, 0.14, 1000, pDt);
		
		return Math.Clamp(pAngle + pAngleAdd, limits[0], limits[1]);
	}
	//-----------------------------------------------------------------------------
	override void OnUpdate(float pDt, out ScriptedCameraItemResult pOutResult)
	{
		// no super for now, will calculate angle based on turret aiming (HeadAngles not supported when turret compartment attachment)
		if (!m_pControlledTurret)
			return;
		
		pOutResult.m_pWSAttachmentReference = null;
		pOutResult.m_pOwner 				= m_OwnerCharacter;
		pOutResult.m_fPositionModelSpace 	= 1.0;
		pOutResult.m_fFOV 					= m_fFOV;
		pOutResult.m_fNearPlane 			= 0.04;
		pOutResult.m_bAllowInterpolation 	= true;
		pOutResult.m_bUpdateWhenBlendOut 	= true;
		pOutResult.m_fUseHeading 			= 0.0;
		pOutResult.m_iDirectBoneMode 		= EDirectBoneMode.None;
		pOutResult.m_iDirectBone 			= -1;

		vector aimingAngles = m_pControlledTurret.GetAimingRotation();
		vector offset = m_OffsetLS;
		if (m_pCompartment && m_pCompartment.IsDirectAimMode() && m_pTurretController.GetCanAimOnlyInADS())
		{
			offset = "0 0 0";
		}
		vector aimChange = m_Input.GetAimChange();
		bool freeLook = m_bForceFreeLook || m_pTurretController.IsFreeLookEnabled() || m_ControllerComponent.IsTrackIREnabled();

		float lrAngle = 0.0;
		float udAngle = 0.0;
		
		// Freelook is using specific angles and is not very compatible with the UpdateAngleWithTarget method
		if (freeLook)
		{
			lrAngle = UpdateLRAngle(m_fLeftRightAngle, CONST_LR_MIN, CONST_LR_MAX, pDt);
			udAngle = UpdateUDAngle(m_fUpDownAngle, CONST_UD_MIN, CONST_UD_MAX, pDt);
		}
		else
		{
			lrAngle = UpdateAngleWithTarget(m_fLeftRightAngle, m_fLRAngleAdd, m_fLRAngleVel, m_vHorAimLimits, pDt, aimingAngles[0], aimChange[0], freeLook);
			udAngle = UpdateAngleWithTarget(m_fUpDownAngle, m_fUpDownAngleAdd, m_fUDAngleVel, m_vVertAimLimits, pDt, aimingAngles[1], aimChange[1], freeLook);
		}

		if(m_pTurretController.GetCanAimOnlyInADS() || freeLook)
		{
			aimingAngles[0] = lrAngle;
			aimingAngles[1] = udAngle;
		}
		//! apply to rotation matrix
		Math3D.AnglesToMatrix(aimingAngles, pOutResult.m_CameraTM);
		//! lerp eye position to prevent nosiating shake when character walks around deployed turret
		m_vPrevEyePosition = vector.Lerp(m_vPrevEyePosition, m_OwnerCharacter.EyePositionModel(), 0.25);
		pOutResult.m_CameraTM[3] = m_vPrevEyePosition + offset;
		
		// Apply shake
		if (m_CharacterCameraHandler)
			m_CharacterCameraHandler.AddShakeToToTransform(pOutResult.m_CameraTM, pOutResult.m_fFOV);
	}
	
	//-----------------------------------------------------------------------------
	override void OnAfterCameraUpdate(float pDt, bool pIsKeyframe, inout vector transformMS[4])
	{
		AddVehiclePitchRoll(m_OwnerVehicle, pDt, transformMS);
	}
	
	//-----------------------------------------------------------------------------	
	override float GetBaseFOV()
	{
		return GetGame().GetCameraManager().GetVehicleFOV();
	}
};