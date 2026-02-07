// *************************************************************************************
// ! CharacterCameraADS - Aim down sights camera
// *************************************************************************************
class CharacterCameraADS extends CharacterCameraBase
{
	//-----------------------------------------------------------------------------
	static const float 	CONST_UD_MIN	= -89.0;		//!< down limit
	static const float 	CONST_UD_MAX	= 89.0;			//!< up limit

	static const float 	CONST_LR_MIN	= -160.0;		//!< left limit
	static const float 	CONST_LR_MAX	= 160.0;		//!< right limit

	static const float CONST_TRANSLATIONZ_MIN = -0.1; //! how far back can camera move with weapon
	
	#ifdef ENABLE_DIAG
	private static bool s_bDebugRegistered;
	#endif
	
	//-----------------------------------------------------------------------------
	void CharacterCameraADS(CameraHandlerComponent pCameraHandler)
	{
		m_iHandBoneIndex = m_OwnerCharacter.GetBoneIndex("righthandprop");
		m_fADSToFPSDeg = 45;
		m_fFreelookFOV = GetBaseFOV();
		
		m_WeaponManager = BaseWeaponManagerComponent.Cast(m_OwnerCharacter.FindComponent(BaseWeaponManagerComponent));
		m_AimingComponent = CharacterAimingComponent.Cast(m_OwnerCharacter.FindComponent(CharacterAimingComponent));
		m_GadgetManager = SCR_GadgetManagerComponent.Cast(m_OwnerCharacter.FindComponent(SCR_GadgetManagerComponent));
		m_OffsetLS = "0.0 0.03 -0.07";
		
		#ifdef ENABLE_DIAG
		if (!s_bDebugRegistered) {
			DiagMenu.RegisterRange(SCR_DebugMenuID.DEBUGUI_CHARACTER_ADS_CAMERA, "", "ADS Camera", "Character", "0, 3, 0, 1");
			s_bDebugRegistered = true;
		}
		#endif
	}
	

	//-----------------------------------------------------------------------------
	override void OnBlendIn()
	{
		m_bIsFullyBlend = true;
		if (!m_CameraHandler.IsCameraBlending())
		{
			if (m_LastWeaponComponent && !m_LastWeaponComponent.IsSightADSActive())
				m_LastWeaponComponent.SightADSActivated();
		}
	}
	
	//-----------------------------------------------------------------------------
	override void OnBlendOut()
	{
		// TODO@AS: With current API, this will always override the custom transition implemented below
		// the problem is that on immediate camera switch (e.g. 3rd->editor) this method is called,
		// but camera item is no longer updated, therefore not raising required blending calls
		if (!m_CameraHandler.IsCameraBlending())
		{
			if (m_LastWeaponComponent && m_LastWeaponComponent.IsSightADSActive())
				m_LastWeaponComponent.SightADSDeactivated();
		}
		
		GetGame().GetCameraManager().SetOverlayCamera(null);
		m_bIsFullyBlend = false;
	}
	
	//-----------------------------------------------------------------------------
	protected float GetSightsADSActivationPercentage() // percentage of blend <0,1>
	{
		if (m_LastWeaponComponent)
		{
			SCR_2DOpticsComponent optics = SCR_2DOpticsComponent.Cast(m_LastWeaponComponent.GetSights());
			if (optics)
				return optics.GetADSActivationPercentage();
		}
		
		return 1.0;
	}
	
	//-----------------------------------------------------------------------------
	protected float GetSightsADSDeactivationPercentage() // percentage of blend <0,1>
	{
		if (m_LastWeaponComponent)
		{
			SCR_2DOpticsComponent optics = SCR_2DOpticsComponent.Cast(m_LastWeaponComponent.GetSights());
			if (optics)
				return optics.GetADSDeactivationPercentage();
		}
		
		return 0.0;
	}
	
	//-----------------------------------------------------------------------------
	protected void OnBlendingIn(float blendAlpha)
	{
		m_LastWeaponComponent = m_WeaponManager.GetCurrentWeapon();
		if (m_LastWeaponComponent && blendAlpha >= GetSightsADSActivationPercentage())
		{
			if (!m_LastWeaponComponent.IsSightADSActive())
				m_LastWeaponComponent.SightADSActivated();
		}
	}
	
	//-----------------------------------------------------------------------------
	protected void OnBlendingOut(float blendAlpha)
	{
		if (m_LastWeaponComponent && blendAlpha >= GetSightsADSDeactivationPercentage())
		{
			if (m_LastWeaponComponent.IsSightADSActive())
				m_LastWeaponComponent.SightADSDeactivated();
			
			m_LastWeaponComponent = null;
		}
	}
	
	//-----------------------------------------------------------------------------
	override void OnActivate(ScriptedCameraItem pPrevCamera, ScriptedCameraItemResult pPrevCameraResult)
	{
		super.OnActivate(pPrevCamera, pPrevCameraResult);
		if (pPrevCamera)
		{
			vector f = pPrevCamera.GetBaseAngles();
			m_fUpDownAngle = m_ControllerComponent.GetAimingAngles()[1];
			m_fLeftRightAngle	= f[1];
		}
		// Reset roll
		m_fLastRoll = 0.0;
		
		m_fStabilizerAlpha = 0.0;
		m_fStabilizerAlphaVel = 0.0;
		m_lastStablePos = "0 0 0";
	}
	
	//-----------------------------------------------------------------------------
	protected void SolveCameraHeadAttach(ADSCameraData cameraData, out ScriptedCameraItemResult pOutResult)
	{
		//! In some cases this compensation is needed (sloped surfaces)
		float parentPitch = m_OwnerCharacter.GetLocalYawPitchRoll()[1];
		cameraData.m_vLookAngles[1] = Math.Clamp(cameraData.m_vLookAngles[1] + parentPitch, CONST_UD_MIN, CONST_UD_MAX);
		
		vector headMatrix[4];
		m_OwnerCharacter.GetBoneMatrix(GetCameraBoneIndex(), headMatrix);
		
		//! Sights transform relative to the head
		vector sightsRelativeMatrix[4];
		Math3D.MatrixInvMultiply4(headMatrix, cameraData.m_mSightsLocalMat, sightsRelativeMatrix);
		
		//! Fetch desired portion of recoil
		float recoilPortion;
		if (m_AimingComponent)
			recoilPortion = Math.Clamp(m_AimingComponent.GetRawAimingTranslation()[2] * cameraData.m_fCamRecoilAmount, -CAMERA_RECOIL_LIMIT, CAMERA_RECOIL_LIMIT);
		
		//! Local head plane offset by the recoil portion
		vector headPlane = cameraData.m_vSightsOffset;
		headPlane[2] = headPlane[2] - recoilPortion;
		
		//! Set matrix rotation
		Math3D.AnglesToMatrix(cameraData.m_vLookAngles, pOutResult.m_CameraTM);		
		
		//! Project sights onto head plane
		vector resultPosition = SCR_Math3D.IntersectPlane(sightsRelativeMatrix[3], -sightsRelativeMatrix[2], headPlane, vector.Forward);		
		
		//! Apply props
		pOutResult.m_CameraTM[3] = resultPosition;
		pOutResult.m_fFOV = cameraData.m_fFOV;
		pOutResult.m_fDistance = 0;
		pOutResult.m_fNearPlane = 0.0125;
		pOutResult.m_bAllowInterpolation = true;
		pOutResult.m_bUpdateWhenBlendOut = true;
		pOutResult.m_iDirectBone = GetCameraBoneIndex();
		pOutResult.m_iDirectBoneMode = EDirectBoneMode.RelativePosition;
	}

	/*!
		Stable ADS camera solver that used RightHandProp as camera root.
	*/
	protected void SolveNewMethod(ADSCameraData cameraData, out ScriptedCameraItemResult pOutResult, float pDt, bool allowInterpolation)
	{
		// Right hand prop is where weapon is attached to
		vector propBoneMS[4];
		m_OwnerCharacter.GetBoneMatrix(m_iHandBoneIndex, propBoneMS);
		
		// UPDATE:
		// This is quite silly considering what is done afterwards, 
		// but the point is that we have to fight with zeroing, as it is
		// the only relevant aim modifier that we do not want to offset in
		// the camera like we do with recoil or sway
		vector zeroingLS[4];
		if (m_WeaponManager.GetCurrentWeapon() && m_WeaponManager.GetCurrentWeapon().GetCurrentSightsZeroingTransform(zeroingLS))
		{
			Math3D.MatrixMultiply4( cameraData.m_mSightsLocalMat, zeroingLS,cameraData.m_mSightsLocalMat );
		}

		// Get sights relative to right hand prop bone
		vector sightsMS[4];
		Math3D.MatrixInvMultiply4( propBoneMS, cameraData.m_mSightsLocalMat, sightsMS );

		// Get sights to character MS
		Math3D.MatrixMultiply4( propBoneMS, sightsMS, sightsMS );

		// We have sights in right hand prop hand character MS..
		vector cameraBoneMS[4];
		m_OwnerCharacter.GetBoneMatrix(GetCameraBoneIndex(), cameraBoneMS);

		// Now let us project the position onto the head in MS
		// rayOrigin:
		//	sights --> sights back (towards head)
		// planeOrigin:
		// head --> forward ( head.z )

		vector pureSightsFwd = sightsMS[2];
		
		vector projectedPosMS = SCR_Math3D.IntersectPlane( sightsMS[3], -sightsMS[2], cameraBoneMS[3], cameraBoneMS[2] );
		vector projToSightMS = projectedPosMS - sightsMS[3];
		vector projToSightsDirMS = projToSightMS.Normalized();
		
		// Let's negate translation from modifiers - this will keep our camera
		// where it ought to be, but allow weapon to move independently of camera
		vector aimingTranslationWeaponLS = m_AimingComponent.GetRawAimingTranslation();
		//float zOffset = Math.Clamp(aimingTranslationWeaponLS[2], -CAMERA_RECOIL_LIMIT, CAMERA_RECOIL_LIMIT)
		
		vector aimingTranslationMS;
		for (int i = 0; i < 3; i++)
			aimingTranslationMS += aimingTranslationWeaponLS[i] * sightsMS[i];
		
		
		// And add additional translation, this time desired amount of Z translation
		//! Fetch desired portion of recoil
		float recoilPortion;
		if (m_AimingComponent)
			recoilPortion = Math.Clamp(aimingTranslationWeaponLS[2] * cameraData.m_fCamRecoilAmount, -CAMERA_RECOIL_LIMIT, CAMERA_RECOIL_LIMIT);
		vector extraTranslation = recoilPortion * sightsMS[2];
		
		// And add that to our result
		sightsMS[3] = sightsMS[3] - aimingTranslationMS - extraTranslation;
		
		// Now we will disregard any previous rotation... (LOL and use aiming or freelook angles directly)
		
		// Take look angles directly and correct those for character pitch
		vector aimingAnglesMS = cameraData.m_vLookAngles;
		aimingAnglesMS[1] = aimingAnglesMS[1] + m_OwnerCharacter.GetLocalYawPitchRoll()[1];
		// Use as intended
		Math3D.AnglesToMatrix( aimingAnglesMS, sightsMS );
		
		const float alphaEpsilon = 0.0005;
		// Stabilize camera in certain cases
		vector resultPosition = sightsMS[3];
		bool isUnstable = false;
		if (m_CmdHandler)
		{
			isUnstable = m_ControllerComponent.IsSprinting() ||
						 m_CmdHandler.GetTargetLadder() != null ||
						 m_ControllerComponent.IsMeleeAttack() ||
						 m_CmdHandler.GetCommandModifier_Item() && m_CmdHandler.GetCommandModifier_Item().IsChangingItemTag() ||
						 m_CmdHandler.IsProneStanceTransition() && m_ControllerComponent.GetMovementVelocity().LengthSq() > 0.0 ||
						 (!m_WeaponManager || !m_WeaponManager.GetCurrentSights());
		}

		if (isUnstable)
		{
			m_fStabilizerAlpha = Math.SmoothCD(m_fStabilizerAlpha, 1.0, m_fStabilizerAlphaVel, 0.1, 1000, pDt);
			resultPosition = m_lastStablePos;
		}
		else
		{
			m_fStabilizerAlpha = Math.SmoothCD(m_fStabilizerAlpha, 0.0, m_fStabilizerAlphaVel, 0.1, 1000, pDt);
		}

		float obstructedAlpha = m_ControllerComponent.GetObstructionAlpha();
		float lerpToCameraT = Math.Max(m_fStabilizerAlpha, obstructedAlpha);
		bool shouldStabilize = lerpToCameraT > alphaEpsilon;
		if (shouldStabilize)
		{
			resultPosition = vector.Lerp(resultPosition, cameraBoneMS[3], lerpToCameraT);
		}

		if (!isUnstable && obstructedAlpha < alphaEpsilon)
		{
			m_lastStablePos = resultPosition;
		}
		
		// Last but not least, blend FOV based on aiming vs. freelook angles
		// If in freelook or not fully blended out yet, update
		if (cameraData.m_bFreeLook || m_fFreelookBlendAlpha > alphaEpsilon)
		{
			vector weaponAimingDir = m_AimingComponent.GetAimingRotation().AnglesToVector();
			vector lookAimingDir = aimingAnglesMS.AnglesToVector();
			float blendAlpha = 1.0 - ( vector.Dot( weaponAimingDir, lookAimingDir ) + 1.0) * 0.5;
			const float blendOutSpeed = 3.0; // Higher values reduce the radius range of blend alpha, lower values extend further
			
			m_fFreelookBlendAlpha =  Math.Clamp( Math.Sqrt( blendAlpha * blendOutSpeed ), 0.0, 1.0 );
			if ( m_fFreelookBlendAlpha <= alphaEpsilon )
				m_fFreelookBlendAlpha = 0.0;
			vector freeLookMat[4];
			vector additiveRotation = "0 0 0";
			m_CharacterHeadAimingComponent.GetLookTransformationLS(GetCameraBoneIndex(), EDirectBoneMode.RelativePosition, m_OffsetLS, additiveRotation, freeLookMat);
			// Blend up to 25% of head position, feels solid
			vector endPosMS = cameraBoneMS[3] + freeLookMat[3];
			// Do not blend all the way to projected position, blend up to 60%?
			resultPosition = vector.Lerp( resultPosition, endPosMS, 0.6 * m_fFreelookBlendAlpha );
		}
		
		// If the result position is behind the camera bone, the camera might collide with the chest, so we move it forward.
		float resultNegativeZ = resultPosition[2] - cameraBoneMS[3][2];
		if (resultNegativeZ < 0)
		{
			sightsMS[3] = resultPosition + (-resultNegativeZ * pureSightsFwd[2] * pureSightsFwd);
		}
		else
		{
			sightsMS[3] = resultPosition;
		}

		// Get transformation to parent
		if (!shouldStabilize)
		{
			Math3D.MatrixInvMultiply4( propBoneMS, sightsMS, pOutResult.m_CameraTM );
			pOutResult.m_iDirectBone = m_iHandBoneIndex;
			pOutResult.m_iDirectBoneMode = EDirectBoneMode.RelativeTransform;
		}
		else
		{
			pOutResult.m_bAllowInterpolation = false;
			Math3D.MatrixCopy( sightsMS, pOutResult.m_CameraTM );
		}
			
		pOutResult.m_fFOV = Math.Lerp( cameraData.m_fFOV, GetBaseFOV(), m_fFreelookBlendAlpha );
		pOutResult.m_fDistance = 0;
		pOutResult.m_fNearPlane = 0.0125;
		pOutResult.m_bAllowInterpolation = allowInterpolation && (shouldStabilize == m_bWasStabilizedLastFrame);
		pOutResult.m_fUseHeading = 1.0;
		pOutResult.m_bUpdateWhenBlendOut = true;
		pOutResult.m_fPositionModelSpace = 0.0;
		
		m_bWasStabilizedLastFrame = shouldStabilize;
		
		return;
	}

	//-----------------------------------------------------------------------------
	protected void SolveCameraHandAttach(ADSCameraData cameraData, out ScriptedCameraItemResult pOutResult, float pDt, bool allowInterpolation)
	{
		//! In some cases this compensation is needed (sloped surfaces)
		float parentPitch = m_OwnerCharacter.GetLocalYawPitchRoll()[1];
		cameraData.m_vLookAngles[1] = Math.Clamp(cameraData.m_vLookAngles[1] + parentPitch, CONST_UD_MIN, CONST_UD_MAX);
		
		vector headMatrix[4];
		m_OwnerCharacter.GetBoneMatrix(GetCameraBoneIndex(), headMatrix);
		
		//! Sights transform relative to the head
		vector sightsRelativeMatrix[4];
		Math3D.MatrixInvMultiply4(headMatrix, cameraData.m_mSightsLocalMat, sightsRelativeMatrix);
		
		//! Fetch desired portion of recoil
		float recoilPortion;
		if (m_AimingComponent)
			recoilPortion = Math.Clamp(m_AimingComponent.GetRawAimingTranslation()[2] * cameraData.m_fCamRecoilAmount, -CAMERA_RECOIL_LIMIT, CAMERA_RECOIL_LIMIT);
		
		//! Local head plane offset by the recoil portion
		vector headPlane = cameraData.m_vSightsOffset;
		headPlane[2] = headPlane[2] - recoilPortion;
		
		//! Project sights onto head plane
		vector resultPosition = SCR_Math3D.IntersectPlane(sightsRelativeMatrix[3], -sightsRelativeMatrix[2], headPlane, vector.Forward);		
		
		// transform relative to hand
		vector handMatrix[4];
		m_OwnerCharacter.GetBoneMatrix(m_iHandBoneIndex, handMatrix);
		
		// Get sights relative to anchor
		vector relativeSightsMatrix[4];
		// Sight transform relative to head bone
		Math3D.MatrixInvMultiply4(handMatrix, cameraData.m_mSightsLocalMat, relativeSightsMatrix);
		
		// Prepare and apply rotation matrix
		Math3D.AnglesToMatrix(cameraData.m_vLookAngles, pOutResult.m_CameraTM);	
		
		bool isUnstable = false;
		if (m_CmdHandler)
		{
			isUnstable = m_ControllerComponent.IsSprinting() ||
						 m_CmdHandler.GetTargetLadder() != null ||
						 m_ControllerComponent.IsMeleeAttack() ||
						 m_CmdHandler.GetCommandModifier_Item() && m_CmdHandler.GetCommandModifier_Item().IsChangingItemTag();
		}
		
		if (isUnstable)
		{
			m_fStabilizerAlpha = Math.SmoothCD(m_fStabilizerAlpha, 1.0, m_fStabilizerAlphaVel, 0.14, 1000, pDt);
			resultPosition = m_lastStablePos;
		}
		else
		{
			m_fStabilizerAlpha = Math.SmoothCD(m_fStabilizerAlpha, 0.0, m_fStabilizerAlphaVel, 0.14, 1000, pDt);
		}
		
		float obstructedAlpha = m_ControllerComponent.GetObstructionAlpha();
		
		if (m_fStabilizerAlpha > 0 || obstructedAlpha > 0)
		{
			float t = Math.Max(m_fStabilizerAlpha, obstructedAlpha);
			vector headRelativePosition = "0 0 0";
			resultPosition = vector.Lerp(resultPosition, headRelativePosition, t);
		}
		
		if (!isUnstable && obstructedAlpha < 0.001)
		{
			m_lastStablePos = resultPosition;
		}

		//! Get end position rel. to hand
		resultPosition = resultPosition.Multiply4(headMatrix);
		resultPosition = resultPosition.InvMultiply4(handMatrix);
		
		
		//! Apply props
		pOutResult.m_CameraTM[3] = resultPosition;
		pOutResult.m_fFOV = cameraData.m_fFOV;
		pOutResult.m_fDistance = 0;
		pOutResult.m_fNearPlane = 0.0125;
		pOutResult.m_bAllowInterpolation = allowInterpolation;
		pOutResult.m_bUpdateWhenBlendOut = true;
		pOutResult.m_iDirectBone = m_iHandBoneIndex;
		pOutResult.m_iDirectBoneMode = EDirectBoneMode.RelativePosition;
		pOutResult.m_fPositionModelSpace = 0.0;
	}
	
	//-----------------------------------------------------------------------------
	protected void SolveCamera2DSight(ADSCameraData cameraData, out ScriptedCameraItemResult pOutResult)
	{

		float targetFOV;
		float zeroingPitch;
		m_WeaponManager.GetCurrentSightsCameraTransform(cameraData.m_mSightsLocalMat, targetFOV);
		
		// TODO@AS: refactor
		SCR_2DPIPSightsComponent pipSights = SCR_2DPIPSightsComponent.Cast(m_WeaponManager.GetCurrentSights());
		if (pipSights)
		{
			targetFOV = pipSights.GetMainCameraFOV();
			zeroingPitch = pipSights.GetCurrentCameraPitchOffset();
		}
		
		// we start clean
		vector lookRot[4];
		vector adjustedLookAngles = cameraData.m_vLookAngles;
		adjustedLookAngles[1] = adjustedLookAngles[1] + zeroingPitch; // Add extra zeroing pitch for sights that allow such functionality
		Math3D.AnglesToMatrix(adjustedLookAngles, lookRot);
		
		// snap to bone
		vector handBoneTM[4];
		m_OwnerCharacter.GetBoneMatrix(m_iHandBoneIndex, handBoneTM);
		
		//! clamp fov so sights FOV is never greater than current fov?
		pOutResult.m_fFOV = Math.Min(GetBaseFOV(), targetFOV);
		
		//! sights transform relative to hand bone
		Math3D.MatrixInvMultiply4(handBoneTM, cameraData.m_mSightsLocalMat, pOutResult.m_CameraTM);
		vector finalPos = pOutResult.m_CameraTM[3];
		
		//! sights in relation to hand
		vector viewMatHandRel[4];
		Math3D.MatrixInvMultiply4(handBoneTM, lookRot, pOutResult.m_CameraTM);
		
		//! apply position
		pOutResult.m_CameraTM[3] = finalPos;
		
		//! apply roll to camera (if any)
		SCR_2DOpticsComponent optics = SCR_2DOpticsComponent.Cast(m_WeaponManager.GetCurrentSights());
		if (optics)
		{
			// Fetch desired roll
			vector lookAngles = Math3D.MatrixToAngles(pOutResult.m_CameraTM);
			float roll = ROLL_AMOUNT * -lookAngles[2];
			// Interpolate value
			m_fLastRoll = Math.Lerp(m_fLastRoll, roll, ROLL_INTERP * cameraData.m_fDeltaTime);
			
			optics.SetViewAngle(m_fLastRoll);
		}
		
		
		//! setup camera props
		pOutResult.m_iDirectBone 			= m_iHandBoneIndex;
		pOutResult.m_iDirectBoneMode 		= EDirectBoneMode.RelativeTransform;
		pOutResult.m_bUpdateWhenBlendOut	= true; // otherwise camera stops blending out properly
		pOutResult.m_fDistance 				= 0;
		pOutResult.m_fUseHeading 			= 0;
		pOutResult.m_fNearPlane				= 0.025;
		pOutResult.m_bBlendFOV 				= true; // otherwise FOV blend transitions awkwardly
	}
	ref ADSCameraData cameraData = new ADSCameraData();
	//-----------------------------------------------------------------------------
	override void OnUpdate(float pDt, out ScriptedCameraItemResult pOutResult)
	{
		if (m_CameraHandler && m_CameraHandler.IsCameraBlending())
		{
			if (m_CameraHandler.GetCurrentCamera() == this)
				OnBlendingIn(m_CameraHandler.GetBlendAlpha(this));
			else
				OnBlendingOut(m_CameraHandler.GetBlendAlpha(m_CameraHandler.GetCurrentCamera()));
		}
		
		//! update angles
		auto sights = m_WeaponManager.GetCurrentSights();
		bool canFreelook = sights && sights.CanFreelook();
		//! update fov
		m_fFreelookFOV = GetBaseFOV();

		pOutResult.m_vBaseAngles = GetBaseAngles();
		

		//! yaw pitch roll vector
		vector lookAngles = m_CharacterHeadAimingComponent.GetLookAngles();
		if (!canFreelook)
		{
			lookAngles[0] = m_CommandWeapons.GetAimAngleLR();
			lookAngles[1] = m_CommandWeapons.GetAimAngleUD();
		}
		//! Prepare data
		
		cameraData.m_fDeltaTime = pDt;
		cameraData.m_vLookAngles = lookAngles;
		cameraData.m_fFOV = GetBaseFOV();
		
		//! Fetch sights transformation
		if (sights)
		{
			m_WeaponManager.GetCurrentSightsCameraTransform(cameraData.m_mSightsLocalMat, cameraData.m_fFOV);
			cameraData.m_vSightsOffset = sights.GetSightsOffset();
			cameraData.m_fCamRecoilAmount = sights.GetCameraRecoilAmount();
		}
		else
		{
			Math3D.MatrixIdentity4(cameraData.m_mSightsLocalMat);
		}
		
		//! Recalculate FOV
		cameraData.m_fFOV = Math.Min(GetBaseFOV(), cameraData.m_fFOV);
		
		//! Fetch zeroing data
		// Apparently in rare cases like bandaging, weapon can be missing
		BaseWeaponComponent currentWeapon = m_WeaponManager.GetCurrent();
		if (currentWeapon)
		{
			vector zeroingMatrix[4];
			if (currentWeapon.GetCurrentSightsZeroingTransform(zeroingMatrix))
			{
				// add zeroing to our initial local sights
				zeroingMatrix[3] = -zeroingMatrix[3];
				Math3D.MatrixMultiply4(cameraData.m_mSightsLocalMat, zeroingMatrix, cameraData.m_mSightsLocalMat);
			}
		}

		// 2nd camera -> provided by sights if any
		CameraBase overlayCamera;
		
		//special handling for FOV blending for 2d sights
		//special handling for eye snapping too
		auto sights2d = SCR_2DOpticsComponent.Cast(sights);
		if (sights2d)
		{
			sights2d.Tick(pDt);
			
			auto pip = SCR_2DPIPSightsComponent.Cast(sights2d);
			// 2D sights
			if (!pip || SCR_Global.IsScope2DEnabled())
			{
				SolveCamera2DSight(cameraData, pOutResult);
				pOutResult.m_pOwner 				= m_OwnerCharacter;
				pOutResult.m_pWSAttachmentReference = null;
				return;
			}
			
			// Camera FOV to be used is different from the sights FOV
			cameraData.m_fFOV = pip.GetMainCameraFOV();
			overlayCamera  = pip.GetPIPCamera();
		}
		
		if (!canFreelook)
		{
			cameraData.m_vLookAngles[0] = 0.0;
		}
		
		// Store freelook state
		cameraData.m_bFreeLook = canFreelook && (m_ControllerComponent.IsFreeLookEnabled() || m_bForceFreeLook);
	
		int solveMethod = 0;
		#ifdef ENABLE_DIAG
			solveMethod = DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_CHARACTER_ADS_CAMERA);
		#endif		
		
		if (solveMethod == 1)
			SolveCameraHandAttach(cameraData, pOutResult, pDt, false);
		else if (solveMethod == 2)
			SolveCameraHeadAttach(cameraData, pOutResult);
		else if (solveMethod == 3)
			SolveCameraHandAttach(cameraData, pOutResult, pDt, true);
		else // :-)
			SolveNewMethod(cameraData, pOutResult, pDt, true);
		
		
		// Update overlay camera
		if (cameraData.m_bFreeLook)
		{
			// Supress overlay cam
			GetGame().GetCameraManager().SetOverlayCamera(null);
		}
		else if (overlayCamera)
		{
			// Override overlay comera if any is used
			GetGame().GetCameraManager().SetOverlayCamera(overlayCamera);
		}
		
		pOutResult.m_pOwner 				= m_OwnerCharacter;
		pOutResult.m_pWSAttachmentReference = null;
		
		// Apply shake
		if (m_CharacterCameraHandler)
			m_CharacterCameraHandler.AddShakeToToTransform(pOutResult.m_CameraTM, pOutResult.m_fFOV);
	}
	
	//-----------------------------------------------------------------------------	
	protected BaseWeaponManagerComponent m_WeaponManager;
	protected BaseWeaponComponent m_LastWeaponComponent;
	protected CharacterAimingComponent m_AimingComponent;
	protected SCR_GadgetManagerComponent m_GadgetManager;
	protected BaseSightsComponent m_BinocularSight;
	
	protected	bool	m_bIsFullyBlend = false;
	protected	int 	m_iHandBoneIndex;	//!< hand bone
	protected	int 	m_iHeadBoneIndex;	//!< head bone
	protected	vector	m_OffsetLS;			//!< position offset 
	protected	float	m_fADSToFPSDeg;		//!< freelook degrees for transitioning into fps pos
	protected	float	m_fFreelookFOV;
	protected	vector	m_lastStablePos;

	protected	float	m_fStabilizerAlpha = 0.0;
	protected	float	m_fStabilizerAlphaVel = 0.0;
	
	protected	bool	m_bWasStabilizedLastFrame = false;
	
	protected 	float	m_fFreelookBlendAlpha;
	
	private		vector	m_vLastEndPos;		//!< last position used for interpolation
	private		float	m_fLastRoll;		//!< last roll value used for interpolation
	
	protected const float ROLL_AMOUNT = 1; // 0-1
	protected const float ROLL_INTERP = 8.75; // speed of interpolation, greater value = faster
	protected const float CAMERA_INTERP = 0.6;
	protected const float CAMERA_RECOIL_LIMIT = 0.25; //!< Maximum amount of recoil applied to camera from weapon in meters.
};