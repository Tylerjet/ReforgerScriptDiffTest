[ComponentEditorProps(category: "GameScripted/Character", description: "Scripted character camera handler (new)", color: "0 0 255 255")]
class SCR_CharacterCameraHandlerComponentClass: CameraHandlerComponentClass
{
};

class SCR_CharacterCameraHandlerComponent : CameraHandlerComponent
{
	[Attribute(category: "Camera Shake")]
	ref SCR_RecoilCameraShakeParams m_pRecoilShakeParams;
	
	//! Progress of recoil based camera shake
	ref SCR_RecoilCameraShakeProgress m_pRecoilShake = new SCR_RecoilCameraShakeProgress();
	
	//------------------------------------------------------------------------------------------------
	void SCR_CharacterCameraHandlerComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		#ifdef ENABLE_DIAG 
		DiagMenu.RegisterRange(SCR_DebugMenuID.DEBUGUI_CHARACTER_DEBUG_VIEW, "", "Cycle Debug View", "Character", "0, 8, 0, 1");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_DRAW_CAMERA_COLLISION_SOLVER, "", "Draw Camera Collision Solver", "Character");
		#endif
	}
	
	//------------------------------------------------------------------------------------------------
	override void Init()
	{
		m_OwnerCharacter = SCR_ChimeraCharacter.Cast(GetOwner());
		m_ControllerComponent = SCR_CharacterControllerComponent.Cast(m_OwnerCharacter.FindComponent(SCR_CharacterControllerComponent));
		m_AnimationComponent = CharacterAnimationComponent.Cast(m_OwnerCharacter.FindComponent(CharacterAnimationComponent));
		m_LoadoutManager = BaseLoadoutManagerComponent.Cast(m_OwnerCharacter.FindComponent(BaseLoadoutManagerComponent));
		m_InputManager = GetGame().GetInputManager();
		m_IdentityComponent = CharacterIdentityComponent.Cast(m_OwnerCharacter.FindComponent(CharacterIdentityComponent));
			
		m_iHeadBoneIndex = m_OwnerCharacter.GetBoneIndex("Head");
	}
	//------------------------------------------------------------------------------------------------
	override void OnCameraActivate()
	{
		if (m_OwnerCharacter && m_AnimationComponent)
		{
			OnThirdPersonSwitch(IsInThirdPerson());
		}
		
		if (m_InputManager)
			m_InputManager.AddActionListener("SwitchCameraType", EActionTrigger.PRESSED, OnCameraSwitchPressed);	
		
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (playerController)
		{
			if (playerController.m_bRetain3PV)
				SetThirdPerson(true);
			playerController.m_bRetain3PV = false;
		}
	}
	//------------------------------------------------------------------------------------------------
	void OnAlphatestChange(int a)
	{
		m_OwnerCharacter.m_fFaceAlphaTest = a;
		
		if (m_IdentityComponent)
		{
			m_IdentityComponent.SetHeadAlpha(a);
		}
		
		if (m_LoadoutManager)
		{
			auto ent = m_LoadoutManager.GetClothByArea(ELoadoutArea.ELA_HeadCover);
			if (ent)
			{
				auto par = ParametricMaterialInstanceComponent.Cast(ent.FindComponent(ParametricMaterialInstanceComponent));
				if (par)
					par.SetUserAlphaTestParam(a);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnCameraDeactivate()
	{
		if (m_OwnerCharacter)
			OnAlphatestChange(0);
		
		if (m_InputManager)
			m_InputManager.RemoveActionListener("SwitchCameraType", EActionTrigger.PRESSED, OnCameraSwitchPressed);
		m_fFocusTargetValue = 0.0;
		m_fFocusValue = 0.0;
		SetFocusMode(m_fFocusValue);
	}
	
	bool IsDebugView()
	{
		#ifdef ENABLE_DIAG
		return DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_CHARACTER_DEBUG_VIEW) != CharacterCameraSet.DEBUGVIEW_NONE;
		#else
		return false;
		#endif
	}

	//------------------------------------------------------------------------------------------------
	private bool m_bWasVehicleADS = false;
	bool CheckVehicleADS()
	{
		CompartmentAccessComponent compartmentAccess = m_OwnerCharacter.GetCompartmentAccessComponent();
		if (compartmentAccess && compartmentAccess.IsInCompartment())
		{
			BaseCompartmentSlot compartment = compartmentAccess.GetCompartment();
			if (compartment)
			{
				BaseControllerComponent controller = compartment.GetController();
				if (controller)
				{
					TurretControllerComponent turretController = TurretControllerComponent.Cast(controller);
					if(turretController && turretController.IsWeaponADS())
					{
						if(!m_bWasVehicleADS)
						{
							m_bWasVehicleADS = true;

						}
						return true;
					}
				}
			}
		}
		m_bWasVehicleADS = false;
		return false;
	}
	bool CheckIsInTurret(out bool isInAds)
	{
		isInAds = false;
		CompartmentAccessComponent compartmentAccess = m_OwnerCharacter.GetCompartmentAccessComponent();
		// Fallback to default VehicleCamera behavior if getting out
		if (compartmentAccess && compartmentAccess.IsInCompartment() && !compartmentAccess.IsGettingOut())
		{
			BaseCompartmentSlot compartment = compartmentAccess.GetCompartment();
			if (compartment)
			{
				BaseControllerComponent controller = compartment.GetController();
				if (controller)
				{
					TurretControllerComponent turretController = TurretControllerComponent.Cast(controller);
					if(turretController)
					{
						isInAds = turretController.IsWeaponADS();
						return true;
					}
				}
			}
		}
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override int CameraSelector()
	{
		m_AnimationComponent.GetMovementState(m_CharMovementState);
		
		#ifdef ENABLE_DIAG 
		if (IsDebugView())
		{
			return CharacterCameraSet.CHARACTERCAMERA_DEBUG;
		}
		#endif
		
		if (SCR_BinocularsComponent.IsZoomedView())
		{
			return CharacterCameraSet.CHARACTERCAMERA_BINOCULARS;
		}
		
		//! game camera selection
		if (m_ControllerComponent.IsWeaponADS() || (m_ControllerComponent.GetInputContext().IsWeaponADS() && m_ControllerComponent.IsChangingStance()))
		{
			return CharacterCameraSet.CHARACTERCAMERA_ADS;
		}
		
		if (m_ControllerComponent.IsGadgetRaisedModeWanted() && !m_OwnerCharacter.IsInVehicle())
			return CharacterCameraSet.CHARACTERCAMERA_1ST;

		CompartmentAccessComponent compartmentAccess = m_OwnerCharacter.GetCompartmentAccessComponent();
		if (IsInThirdPerson())
		{
			
			if( m_OwnerCharacter.IsInVehicle() && compartmentAccess && !compartmentAccess.IsGettingOut() )
			{
				bool isTurretAds = false;
				bool inTurret = CheckIsInTurret(isTurretAds);
				if (inTurret)
				{
					if (isTurretAds)
					{
						return CharacterCameraSet.CHARACTERCAMERA_ADS_VEHICLE;
					}
					return CharacterCameraSet.CHARACTERCAMERA_3RD_TURRET;
				}

				return CharacterCameraSet.CHARACTERCAMERA_3RD_VEHICLE;
			}
			
			if( m_CharMovementState.m_CommandTypeId == ECharacterCommandIDs.CLIMB )
				return CharacterCameraSet.CHARACTERCAMERA_3RD_CLIMB;
			
			if( m_CharMovementState.m_iStanceIdx == ECharacterStance.PRONE )
				return CharacterCameraSet.CHARACTERCAMERA_3RD_PRO;
			else if( m_CharMovementState.m_iStanceIdx == ECharacterStance.CROUCH )
				return CharacterCameraSet.CHARACTERCAMERA_3RD_CRO;
			
			return CharacterCameraSet.CHARACTERCAMERA_3RD_ERC;
		}
		else if (compartmentAccess && (compartmentAccess.IsGettingIn() || compartmentAccess.IsGettingOut()))
		{
			return CharacterCameraSet.CHARACTERCAMERA_1ST_VEHICLE_TRANSITION;
		}
		else if( m_OwnerCharacter.IsInVehicle() )
		{
			bool isTurretAds = false;
			bool inTurret = CheckIsInTurret(isTurretAds);
			if (inTurret)
			{
				if (isTurretAds)
				{
					return CharacterCameraSet.CHARACTERCAMERA_ADS_VEHICLE;
				}
				return CharacterCameraSet.CHARACTERCAMERA_1ST_TURRET;
			}
			else if( CheckVehicleADS() )
					return CharacterCameraSet.CHARACTERCAMERA_ADS_VEHICLE;

			return CharacterCameraSet.CHARACTERCAMERA_1ST_VEHICLE;
		}
		return CharacterCameraSet.CHARACTERCAMERA_1ST;
	}
	
	protected float m_fADSProgress;
	protected float m_fADSTime;
	override float GetCameraTransitionTime(int pFrom, int pTo)
	{
		float transTime = 0.4;
		
		if (pFrom == CharacterCameraSet.CHARACTERCAMERA_DEBUG || pTo == CharacterCameraSet.CHARACTERCAMERA_DEBUG)
			transTime = 0.0;
		else if (pTo == CharacterCameraSet.CHARACTERCAMERA_ADS || pTo == CharacterCameraSet.CHARACTERCAMERA_1ST_READY)
		{
			auto controller = CharacterControllerComponent.Cast(GetOwner().FindComponent(CharacterControllerComponent));
			if (controller)
			{
				m_fADSTime = controller.GetADSTime();
				if (m_fADSProgress > 0)
					transTime = m_fADSTime - m_fADSProgress;
				else
					transTime = m_fADSTime;
			}
		}
		else if (pFrom == CharacterCameraSet.CHARACTERCAMERA_ADS)
		{
			transTime = m_fADSProgress;
		}
		else if (pTo == CharacterCameraSet.CHARACTERCAMERA_ADS_VEHICLE)
		{
			auto compAcc = CompartmentAccessComponent.Cast(GetOwner().FindComponent(CompartmentAccessComponent));
			if (compAcc && compAcc.IsInCompartment())
			{
				auto compartment = compAcc.GetCompartment();
				if (compartment)
				{
					BaseControllerComponent controller = compartment.GetController();
					if (controller)
					{
						TurretControllerComponent turretController = TurretControllerComponent.Cast(controller);
						if (turretController)
						{
							m_fADSTime = turretController.GetADSTime();
							if (m_fADSProgress > 0)
								transTime = m_fADSTime - m_fADSProgress;
							else
								transTime = m_fADSTime;
						}
					}
				}
			}
		}
		else if (pFrom == CharacterCameraSet.CHARACTERCAMERA_1ST_VEHICLE_TRANSITION || pTo == CharacterCameraSet.CHARACTERCAMERA_1ST_VEHICLE_TRANSITION)
		{
			transTime = 0.8;
		}
		else
			transTime = GetCameraSet().GetTransitionTime(pFrom, pTo);

		return transTime;
	}	
	
	//------------------------------------------------------------------------------------------------
	override void OnThirdPersonSwitch(bool isInThirdPerson)
	{
		if (m_AnimationComponent)
		{
			if (isInThirdPerson)
				m_AnimationComponent.SetAnimationLayerTPP();
			else
				m_AnimationComponent.SetAnimationLayerFPP();
		}
	}
	
	// Empirical multiplier for focus deceleration
	private const float FOCUS_DECELERATION_RATIO = 4.5;
	
	[Attribute("0.5", UIWidgets.Slider, "Focus interpolation time", "0 10 0.05")]
	protected float m_fFocusTime;
	[Attribute("0.9", UIWidgets.Slider, "Focus interpolation deceleration", "0 1 0.05")]
	protected float m_fFocusDeceleration;

	private bool m_bDoInterpolateFocus;
	private float m_fFocusTargetValue;
	private float m_fFocusValue;
	
	//------------------------------------------------------------------------------------------------
	override void OnBeforeCameraUpdate(float pDt, bool pIsKeyframe)
	{
		if (!pIsKeyframe)
		{
			m_InputManager.ActivateContext("PlayerCameraContext");
			float cameraFocus;
			bool isWeaponADS = m_ControllerComponent.IsWeaponADS();
			SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
			
			if (m_ControllerComponent.GetDisableWeaponControls())
				cameraFocus = 0;
			else if ( isWeaponADS && m_ControllerComponent.GetMaxZoomInADS() )
				cameraFocus = 1;
			else if ( playerController )
				cameraFocus = playerController.GetFocusValue();
			
			if (!float.AlmostEqual(m_fFocusTargetValue, cameraFocus))
			{
				m_fFocusTargetValue = cameraFocus;
				m_bDoInterpolateFocus = true;
			}
			
			// Update ADS transition offset so that camera transform is faster when ADS stance is ready
			if (isWeaponADS)
			{
				if (m_fADSProgress < m_fADSTime)
					m_fADSProgress = Math.Min(m_fADSProgress + pDt, m_fADSTime);
			}
			else if (m_fADSProgress > 0)
			{
				m_fADSProgress = Math.Max(0, m_fADSProgress - pDt);
			}
		}
		else
		{
			UpdateViewBob(pDt);
		}
		
		if (!m_bDoInterpolateFocus)
			return;
		
		float absDelta = 0;
		
		// Only care about transition if it takes time
		if (m_fFocusTime > 0)
		{
			absDelta = Math.AbsFloat(m_fFocusTargetValue - m_fFocusValue);
			
			// Scale the focusChange by absDelta to smooth out the transition
			float focusDeceleration = FOCUS_DECELERATION_RATIO * absDelta * m_fFocusDeceleration;
			
			// Linear component needs to be squared so that focus is 99% correct after m_fFocusTime
			float focusSpeed = (1 - m_fFocusDeceleration) * (1 - m_fFocusDeceleration) + focusDeceleration;
			float focusChange = pDt * focusSpeed / m_fFocusTime;
			
			if (m_fFocusTargetValue > m_fFocusValue)
				m_fFocusValue += focusChange;
			else
				m_fFocusValue -= focusChange;
			
			m_fFocusValue = Math.Clamp(m_fFocusValue, 0, 1);
		}
		
		if (float.AlmostEqual(absDelta, 0))
		{
			m_fFocusValue = m_fFocusTargetValue;
			m_bDoInterpolateFocus = false;
		}
		
		SetFocusMode(m_fFocusValue);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnAfterCameraUpdate(float pDt, bool pIsKeyframe, inout vector transformMS[4])
	{
		//! update head visibility
		vector headBoneMat[4];
		m_OwnerCharacter.GetBoneMatrix(m_iHeadBoneIndex, headBoneMat);
		OnAlphatestChange(255 - Math.Clamp((vector.Distance(transformMS[3], headBoneMat[3]) - 0.2) / 0.15, 0.0, 1.0)*255);
		
		//! aiming update
		if( pIsKeyframe )
			UpdateAiming(transformMS);
		
		if (GetCurrentCamera())
			GetCurrentCamera().OnAfterCameraUpdate(pDt, pIsKeyframe, transformMS);
		
		//! update recoil and generic camera shake
		UpdateShake(pDt);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateShake(float pDt)
	{
		// Update and apply recoil based camera shake
		if (m_pRecoilShake && !m_pRecoilShake.IsFinished())
			m_pRecoilShake.Update(m_OwnerCharacter, pDt);
	}
	
	//------------------------------------------------------------------------------------------------
	void AddShakeToToTransform(inout vector transform[4], inout float fieldOfView)
	{
		bool applyRecoilShake = true;
		#ifdef ENABLE_DIAG
		applyRecoilShake = !DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_CHARACTER_RECOIL_CAMERASHAKE_DISABLE);
		#endif
		
		// Apply recoil shake
		if (applyRecoilShake && m_pRecoilShake)
			m_pRecoilShake.Apply(transform, fieldOfView);
		
		// Apply generic shakes from shake manager to out transform
		SCR_CameraShakeManagerComponent.ApplyCameraShake(transform, fieldOfView);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnCameraSwitchPressed()
	{
		bool current = IsInThirdPerson();
		SetThirdPerson(!current);
	}
	
	//------------------------------------------------------------------------------------------------
	override void CollisionSolver(float pDt, inout ScriptedCameraItemResult pOutResult)
	{
		if( pOutResult.m_fDistance == 0)
		{
			m_fCameraDistanceFilter = 0;
			ResetSlide();
			return;
		}
		vector cameraTransformWS[4];
		vector charTransformWS[4];
		vector camTransformLS[4];
		camTransformLS[0] = pOutResult.m_CameraTM[0];
		camTransformLS[1] = pOutResult.m_CameraTM[1];
		camTransformLS[2] = pOutResult.m_CameraTM[2];
		camTransformLS[3] = pOutResult.m_CameraTM[3];

		m_OwnerCharacter.GetWorldTransform(charTransformWS);
		
		if (pOutResult.m_fPositionModelSpace == 2)
		{
			cameraTransformWS[0] = pOutResult.m_CameraTM[0];
			cameraTransformWS[1] = pOutResult.m_CameraTM[1];
			cameraTransformWS[2] = pOutResult.m_CameraTM[2];
			cameraTransformWS[3] = pOutResult.m_CameraTM[3];
		}
		else
		{			
			Math3D.MatrixMultiply4(charTransformWS, camTransformLS, cameraTransformWS);
		}
		
		vector charRot[4];
		Math3D.MatrixIdentity4(charRot);
		if(pOutResult.m_fUseHeading > 0)
		{
			float s = Math.Sin(pOutResult.m_fHeading);
			float c = Math.Cos(pOutResult.m_fHeading);
			vector heading[4];
			heading[0] = Vector(c, 0, s);
			heading[1] = Vector(0, 1, 0);
			heading[2] = Vector(-s, 0, c);
			heading[3] = Vector(0, 0, 0);

			m_OwnerCharacter.GetLocalTransform(charRot);
			charRot[3] = vector.Zero;

			Math3D.MatrixInvMultiply4(charRot, heading, charRot);
			float charRotQ[4];
			Math3D.MatrixToQuat(charRot, charRotQ);
			float quatF[4] = {0, 0, 0, 1};
			Math3D.QuatLerp(charRotQ, quatF, charRotQ, pOutResult.m_fUseHeading);
			vector charRotQAngles = Math3D.QuatToAngles(charRotQ);
			Math3D.AnglesToMatrix(charRotQAngles, charRot);
		}
		
		// Get the base position for the camera
		vector basePos = vector.Zero;
		// TODO: Conversion from world to local can cause floating point error, but in 3rd person this is not that bad
		basePos[1] = m_OwnerCharacter.EyePosition()[1] - charTransformWS[3][1];
		basePos = basePos.Multiply4(charTransformWS);
		
		float traceDir = pOutResult.m_CameraTM[3][0];
		if (traceDir > 0)
		{
			traceDir = 1;
		}
		else
		{
			traceDir = -1;
		}
		vector sideTraceDir = cameraTransformWS[3] - basePos;
		vector sideTraceStart = basePos + cameraTransformWS[0] * 0.25 * traceDir;
		vector sideTraceEnd = sideTraceStart + sideTraceDir;
		
		array<IEntity> excludeArray = {m_OwnerCharacter};
		IEntity parent = m_OwnerCharacter.GetParent();
		IEntity sibling = m_OwnerCharacter.GetSibling();
		while (parent != null)
		{
			excludeArray.Insert(parent);
			while (sibling != null)
			{
				excludeArray.Insert(sibling);
				sibling = sibling.GetSibling();
			}
			sibling = parent.GetSibling();
			parent = parent.GetParent();
		}
		
		BaseWorld world = m_OwnerCharacter.GetWorld();
		autoptr TraceSphere param = new TraceSphere;
		param.Radius = 0.09;
		param.ExcludeArray = excludeArray;
		param.Flags = TraceFlags.WORLD | TraceFlags.ENTS;
		param.LayerMask = TRACE_LAYER_CAMERA;
		
		if (pOutResult.m_bNoParent)
		{
			param.Start = cameraTransformWS[3];
			param.End = cameraTransformWS[3] + cameraTransformWS[0];
		}
		else
		{
			vector charTransformLS[4];
			m_OwnerCharacter.GetLocalTransform(charTransformLS);
			
			sideTraceStart = sideTraceStart.InvMultiply4(charTransformWS);
			sideTraceEnd = sideTraceEnd.InvMultiply4(charTransformWS);
			sideTraceStart = sideTraceStart.Multiply4(charRot);
			sideTraceEnd = sideTraceEnd.Multiply4(charRot);
			sideTraceStart = sideTraceStart.Multiply4(charTransformWS);
			sideTraceEnd = sideTraceEnd.Multiply4(charTransformWS);
			
			param.Start = sideTraceStart;
			param.End = sideTraceEnd;
		}
		
		float rightTrace = world.TraceMove(param, null);
		vector slide = vector.Zero;
		if (rightTrace < 1)
		{
			float sideTraceDistance = sideTraceDir.Length();
			sideTraceEnd = sideTraceStart + sideTraceDir * rightTrace - sideTraceDir * param.Radius * 0.5;
			slide = -1 * camTransformLS[0] * (1 - rightTrace) * sideTraceDistance * traceDir;
		}
		
		// Not a pretty way how to limit slide from pushing the camera position too far (so that it could pass through things)
		for (int i = 0; i < 3; i++)
		{
			if (Math.AbsFloat(slide[i]) > Math.AbsFloat(pOutResult.m_CameraTM[3][i]))
			{
				m_vCameraSlideFilterVelocity[i] = 0;
				float slideSign;
				if (slide[i] > 0) slideSign = 1;
				else slideSign = -1;
				
				m_vCameraSlideFilter[i] = slideSign * Math.AbsFloat(pOutResult.m_CameraTM[3][i]);
				slide[i] = slideSign * Math.AbsFloat(pOutResult.m_CameraTM[3][i]);
			}
		}	
		
		vector cameraFinalLS[4];
		cameraFinalLS[0] = camTransformLS[0];
		cameraFinalLS[1] = camTransformLS[1];
		cameraFinalLS[2] = camTransformLS[2];
		cameraFinalLS[3] = camTransformLS[3];
		if (SlideSmooth(pDt, slide, camTransformLS, cameraFinalLS))
		{
			Math3D.MatrixMultiply4(charTransformWS, cameraFinalLS, cameraTransformWS);
		}
		else
		{
			Math3D.MatrixMultiply4(charTransformWS, camTransformLS, cameraTransformWS);
		}

		pOutResult.m_CameraTM[3] = camTransformLS[3];
		
		// Default final position
		cameraFinalLS[3] = camTransformLS[3] - camTransformLS[2] * pOutResult.m_fDistance;
		vector cameraTransformLSHeading[4];
		cameraTransformLSHeading[0] = camTransformLS[0];
		cameraTransformLSHeading[1] = camTransformLS[1];
		cameraTransformLSHeading[2] = camTransformLS[2];
		cameraTransformLSHeading[3] = camTransformLS[3];
		
		// Use same calculations as in CameraHandlerComponent::ComputeCameraMatrixLS
		if(pOutResult.m_fUseHeading > 0)
		{
			Math3D.MatrixMultiply4(charRot, cameraFinalLS, cameraFinalLS);
			Math3D.MatrixMultiply4(charRot, cameraTransformLSHeading, cameraTransformLSHeading);
			
			if (pOutResult.m_fDistance == 0)
			{
				cameraFinalLS[3] = camTransformLS[3];
			}
		}

		if (pOutResult.m_fPositionModelSpace > 0)
		{
			vector msPos = cameraFinalLS[3];
			vector lsPos = camTransformLS[3] - cameraFinalLS[2] * pOutResult.m_fDistance;
			msPos = vector.Lerp(lsPos, msPos, pOutResult.m_fPositionModelSpace);
			cameraFinalLS[3] = msPos;
		}

		// Convert to World Space
		vector cameraFinalWS[4];
		Math3D.MatrixMultiply4(charTransformWS, cameraFinalLS, cameraFinalWS);
		vector cameraTransformWSHeading[4];
		Math3D.MatrixMultiply4(charTransformWS, cameraTransformLSHeading, cameraTransformWSHeading);

		// Ugh, ugly hack because of what
		// m_bNoParent is actually doing..
		// TODO@AS: Refactor on cpp and scr
		CameraManager manager = GetGame().GetCameraManager();
		if (pOutResult.m_bNoParent && manager)
		{
			float magnitude = (cameraTransformWS[3] - cameraFinalWS[3]).Length();
			vector direction = manager.CurrentCamera().GetOrigin() - cameraTransformWS[3];
			direction.Normalize();
			param.Start = cameraTransformWS[3];
			param.End = cameraTransformWS[3] + direction * magnitude;
			param.Radius = param.Radius * 7;
		}
		else
		{
			param.Start = cameraTransformWSHeading[3];
			param.End = cameraFinalWS[3];
			param.End = 2.0 * (param.End - param.Start) + param.Start;
		}
		float tracePct = 1.0;
		if (vector.Distance(param.End, param.Start) > 0.05)
		{
			tracePct = world.TraceMove(param, null);
		}
		float newDistance = pOutResult.m_fDistance;
		
		if (tracePct < 1)
		{
			vector linePoint = param.Start;
			vector lineDirection = param.End - param.Start;
			vector planeNormal = param.TraceNorm;
			vector planePointOrig = param.Start + (tracePct * lineDirection) + (planeNormal * param.Radius);
			vector planePointOrigButThisOneIsFucked = param.Start + tracePct * lineDirection + planeNormal * param.Radius;
			
			float t = (vector.Dot(planeNormal, planePointOrig) - vector.Dot(planeNormal, linePoint)) / vector.Dot(planeNormal, lineDirection.Normalized());
			vector newPt = linePoint + lineDirection.Normalized() * t * tracePct;
			
			if (t < 0)
				t = float.MAX;
			
			newDistance = Math.Min(t, pOutResult.m_fDistance * tracePct);
		}
		
		if (newDistance < sm_fDistanceEpsilon)
			newDistance = 0.0;
		
		if (Math.AbsFloat(m_fCameraDistanceFilter - newDistance) < sm_fDistanceEpsilon && m_fCameraDistanceFilterVel != 0.0)
			m_fCameraDistanceFilterVel = 0.0;
		
		// smooth it
		if(newDistance < m_fCameraDistanceFilter)
			m_fCameraDistanceFilter = newDistance;
		else
			m_fCameraDistanceFilter = Math.SmoothCD(m_fCameraDistanceFilter, newDistance, m_fCameraDistanceFilterVel, 0.3, 1000, pDt);
		
		pOutResult.m_fDistance = m_fCameraDistanceFilter;

		if (m_fCameraDistanceFilter > 0.0)
			m_fCameraDistanceFilter = m_fCameraDistanceFilter;
		
		#ifdef ENABLE_DIAG
		if (DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_DRAW_CAMERA_COLLISION_SOLVER))
		{
			int baseColor = ARGB(255, 0, 110, 255);
			ShapeFlags shFlags = ShapeFlags.WIREFRAME|ShapeFlags.ONCE;

			Shape.CreateSphere(ARGB(255, 0, 255, 0), shFlags, sideTraceStart, param.Radius);									// GREEN
			Shape.CreateSphere(ARGB(255, 255, 255 * rightTrace, 0), shFlags, sideTraceEnd, param.Radius);						// YELLOW - RED

			Shape.CreateSphere(ARGB(255, 150, 0, 255), shFlags, param.Start, param.Radius);										// PURPLE
			Shape.CreateSphere(ARGB(255, 0, 255, 255), shFlags, (param.Start + param.End) * 0.5, param.Radius * 0.35);			// TEAL
			if (tracePct < 1)
				Shape.CreateSphere(ARGB(255, 255, 255 * tracePct, 0), shFlags, param.Start - cameraTransformWS[2] * (newDistance - param.Radius), param.Radius * (1 - tracePct));
			
		}
		#endif
	}

	//------------------------------------------------------------------------------------------------
	private void UpdateViewBob(float pDt)
	{
		int stance = m_ControllerComponent.GetStance();
		
		// Adjust bob from freelook
		float freeLookOffsetScale = 0;
		if (m_ControllerComponent.IsFreeLookEnabled() || m_ControllerComponent.IsTrackIREnabled())
			freeLookOffsetScale = 1;
		m_fBobFreelokFilter = Math.SmoothCD(m_fBobFreelokFilter, 1 - Math.Pow(1 - freeLookOffsetScale, 3), m_fBobFreelokFilterVel, 0.3, 1000, pDt);
		freeLookOffsetScale = m_fBobFreelokFilter;
		
		// Add bobbing based on speed
		m_fBob_ScaleFast = Math.Clamp(m_ControllerComponent.GetVelocity().Length() / Math.Max(m_AnimationComponent.GetTopSpeed(-1, false), 0.1), 0, 1);
		m_fBob_ScaleSlow += (m_fBob_ScaleFast - m_fBob_ScaleSlow) * (pDt * 4);
		
		m_fBob_ScaleFast = Math.Pow(m_fBob_ScaleSlow, 3);
		m_fBob_ScaleFast = Math.Clamp(m_fBob_ScaleFast + freeLookOffsetScale * m_fBob_ScaleFast, 0, 1);
		if (m_fBob_ScaleFast < 0.01)
		{
			m_fBob_Up *= 1 - pDt;
			m_fBob_Right *= 1 - pDt;
		}
		else
		{
			float stanceBobScale = 0.5;
			if (stance == ECharacterStance.PRONE)
				stanceBobScale = 0.25;
			else if (stance == ECharacterStance.CROUCH)
				stanceBobScale = 0.55;
			m_fBob_Up += Math.Clamp(pDt * 2 * stanceBobScale * m_fBob_ScaleFast, 0, 1);
			if (m_fBob_Up >= 1)
				m_fBob_Up -= 1;
			m_fBob_Right += Math.Clamp(pDt * 2.5 * stanceBobScale * m_fBob_ScaleFast, 0, 1);
			if (m_fBob_Right >= 1)
				m_fBob_Right -= 1;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	float AddViewBobToTransform(inout vector pOutTransform[4], float pBobScale, bool pAllowTranslation)
	{
		// Add bob
		float bobMoveScale = (m_fBob_ScaleFast * 0.8 + 0.2) * pBobScale;
		if( pAllowTranslation )
		{
			float freeLookOffsetScale = m_fBobFreelokFilter;
			float bobFw = m_fBob_ScaleSlow * -0.5 * m_fBob_ScaleFast * (1 - freeLookOffsetScale);
			float bobUp = Math.Sin(m_fBob_Up * 360 * Math.DEG2RAD) * 0.02 * bobMoveScale;
			float bobRt = Math.Sin(m_fBob_Right * 360 * Math.DEG2RAD) * 0.015 * bobMoveScale;
			bobRt += m_fBob_ScaleSlow * -0.2 * m_fBob_ScaleFast;
			vector bobTranslation = pOutTransform[0] * bobRt + pOutTransform[3];
			bobTranslation = pOutTransform[1] * bobUp + bobTranslation;
			pOutTransform[3] = pOutTransform[2] * bobFw + bobTranslation;
		}
		vector angBob = vector.Zero;
		angBob[0] = Math.Sin(m_fBob_Up * 360 * Math.DEG2RAD) * 0.3 * bobMoveScale;		// YAW		< >
		angBob[1] = Math.Sin(m_fBob_Right * 360 * Math.DEG2RAD) * 0.3 * bobMoveScale;	// PITCH	^ v
		// ROLL REMOVED, AS ITS ONLY PERCEIVABLE EFFECT WAS THAT IT CAUSES STRONG HEAD BOB WHEN LOOKING DIRECTLY AT GROUND
		vector angBobMat[3], endBobmat[3];
		Math3D.AnglesToMatrix(angBob * pBobScale, angBobMat);
		Math3D.MatrixMultiply3(angBobMat, pOutTransform, endBobmat);
		pOutTransform[0] = endBobmat[0];
		pOutTransform[1] = endBobmat[1];
		pOutTransform[2] = endBobmat[2];
		
		return m_fBob_ScaleFast;
	}
	
	//------------------------------------------------------------------------------------------------
	private void UpdateAiming(vector transformMS[4])
	{
		return;
		/*
		vector characterMat[4];
		m_OwnerCharacter.GetWorldTransform(characterMat);
		
		if( Is3rdPersonView() )
		{
			vector dirLS = transformMS[2];
			vector dirWS = dirLS.Multiply3(characterMat);
			vector aimposLS = transformMS[3];
			vector aimposWS = aimposLS.Multiply4(characterMat);
			
			// Trace aiming for 3rd person
			autoptr TraceParam param = new TraceParam;
			param.Start = aimposWS + dirWS * 3;
			param.End = param.Start + dirWS * 100;
			param.Flags = TraceFlags.WORLD | TraceFlags.ENTS;
			param.Exclude = m_OwnerCharacter;
			param.LayerMask = TRACE_LAYER_CAMERA;
			vector aimpos = param.End;
			
			float traced = m_OwnerCharacter.GetWorld().TraceMove(param, null);
			if (traced < 1)
				aimpos = (param.End - param.Start) * traced + param.Start;
			
			m_ControllerComponent.SetLookAtPosition(aimpos);
			if (m_ControllerComponent.IsFreeLookEnabled() || m_ControllerComponent.IsWeaponObstructed())
				m_ControllerComponent.SetAimPosition(vector.Zero);
			else
				m_ControllerComponent.SetAimPosition(aimpos);
		}
		else
		{
			m_ControllerComponent.SetAimPosition(vector.Zero);
			m_ControllerComponent.SetLookAtPosition(vector.Zero);
		}
		*/
	}

	//------------------------------------------------------------------------------------------------
	bool Is3rdPersonView()
	{
		return !IsDebugView() && IsInThirdPerson() && !(m_ControllerComponent && m_ControllerComponent.IsWeaponADS());
	}
	

	//------------------------------------------------------------------------------------------------
	private SCR_ChimeraCharacter m_OwnerCharacter;
	private SCR_CharacterControllerComponent m_ControllerComponent;
	private CharacterAnimationComponent m_AnimationComponent;
	private InputManager m_InputManager;
	private BaseLoadoutManagerComponent m_LoadoutManager;	
	private CharacterIdentityComponent m_IdentityComponent;
	
	static private float sm_fDistanceEpsilon = 0.0001;
	
	private vector 	m_vCameraSlideFilter = vector.Zero;
	private vector 	m_vCameraSlideFilterVelocity = vector.Zero;
	private float 	m_fChangedTimer = 0;
	private float 	m_fSlideTimeout = 0.4;
	private float	m_fBob_Up = 0;
	private float	m_fBob_Right = 0;
	private float	m_fBob_ScaleSlow = 0;
	private float	m_fBob_ScaleFast = 0;
	private int		m_iHeadBoneIndex = -1;
	private float	m_fCameraDistanceFilter;	
	private float	m_fCameraDistanceFilterVel;
	private float	m_fBobFreelokFilter;	
	private float	m_fBobFreelokFilterVel;	
	private bool 	m_bPrevSlideDirectionInside = false;
	private bool	m_b3rdCollision_UseLeftShoulder = false;

	private ref CharacterMovementState m_CharMovementState = new CharacterMovementState();

	private void ResetSlide()
	{
		m_vCameraSlideFilter = vector.Zero;
		m_vCameraSlideFilterVelocity = vector.Zero;
		m_fChangedTimer = 0;
	}
	
	private bool SlideSmooth(float pDt, vector slide, inout vector outInterpolatedMat[4], inout vector outFinalMat[4])
	{
		bool slideInside = slide.LengthSq() > m_vCameraSlideFilter.LengthSq();
		// Apply timer only when direction of slide is changed
		if (m_bPrevSlideDirectionInside != slideInside)
		{
			m_fChangedTimer = m_fSlideTimeout;
		}
		m_bPrevSlideDirectionInside = slideInside;
		
		if (m_fChangedTimer > 0)
		{
			m_fChangedTimer = m_fChangedTimer - pDt;
		}
		if (IsCameraBlending() || slideInside)
		{
			m_fChangedTimer = 0;
		}
		if ((m_fChangedTimer <= 0 || slideInside) && m_vCameraSlideFilter != slide)
		{
			float slideTime = m_fSlideTimeout;
			if (slideInside)
			{
				slideTime = 0.05;
			}
			for (int i = 0; i < 3; i++)
			{
				float vel = m_vCameraSlideFilterVelocity[i];
				m_vCameraSlideFilter[i] = Math.SmoothCD(m_vCameraSlideFilter[i], slide[i], vel, slideTime, 1000, pDt);
				m_vCameraSlideFilterVelocity[i] = vel;
				float currDiff = Math.AbsFloat(slide[i] - m_vCameraSlideFilter[i]);
				if (currDiff <= sm_fDistanceEpsilon)
				{
					m_vCameraSlideFilter[i] = slide[i];
				}
			}
		}
		
		TranslateKeepLookAt(m_vCameraSlideFilter, outInterpolatedMat);
		// If slide target is inside - we want to use final mat for further (distance) estimation
 		// Otherwise we want current state so don't even bother to make calculations
		if (slideInside)
		{
			TranslateKeepLookAt(slide, outFinalMat);
		}
		return slideInside;
	}
	
	private void TranslateKeepLookAt(vector translation, inout vector transform[4])
	{
		vector isLookingAt = transform[3] + transform[2] * 100000;
		transform[3] = transform[3] + translation;
			
		vector newForward = (isLookingAt - transform[3]).Normalized();
		vector newUp = "0.0 1.0 0.0"; // temp Up vector;
		vector newRight = (newUp * newForward).Normalized();
		newUp = (newForward * newRight).Normalized(); // precise Up Vector
		transform[0] = newRight;
		transform[1] = newUp;
		transform[2] = newForward;
	}
	
	//------------------------------------------------------------------------------------------------
	override float GetOverlayCameraFOVScalarWeight()
	{
		if (!m_ControllerComponent)
			return 0.0;
		
		if (m_ControllerComponent.IsFreeLookEnabled())
			return 0.0;
		
		return 1.0;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_RecoilCameraShakeParams GetRecoilShakeParams()
	{
		return m_pRecoilShakeParams;
	}
};