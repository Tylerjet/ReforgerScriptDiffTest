[EntityEditorProps(category: "GameScripted/Weapon/Sights", description: "", color: "0 0 255 255")]
class SCR_2DPIPSightsComponentClass: SCR_2DSightsComponentClass
{
};

//! Defines different means of adjusting zeroing for PIP sights
enum SCR_EPIPZeroingType
{
	EPZ_NONE = 0,
	EPZ_RETICLE_OFFSET = 1,
	EPZ_CAMERA_TURN = 2
};

//------------------------------------------------------------------------------------------------
class SCR_2DPIPSightsComponent : SCR_2DSightsComponent
{
	// 2D -----------------------------	
	//! Is this 2D currently enabled?
	protected bool m_b2DIsEnabled;
		
	// PIP -----------------------------
	[Attribute("{EF091399D840192D}UI/layouts/Sights/PictureInPictureSightsLayout.layout", UIWidgets.ResourcePickerThumbnail, "The layout used for the PIP component", params: "layout", category: "PiPSights")]
	protected ResourceName m_sPIPLayoutResource;
	
	[Attribute("0.0", UIWidgets.Slider, "Field of view offset for PIP component", params: "-89.9 89.9 0.1", category: "PiPSights")]
	protected float m_fPIPFOVOffset;

	[Attribute("RTTexture0", UIWidgets.EditBox, "Name of RTTexture widget within provided layout", category: "PiPSights")]
	protected string m_sRTTextureWidgetName;

	[Attribute("RenderTarget0", UIWidgets.EditBox, "Name of RenderTarget widget within provided layout", category: "PiPSights")]
	protected string m_sRTargetWidgetName;

	[Attribute("1", UIWidgets.Slider, "Camera index used for this PIP component", params: "0 31 1", category: "PiPSights")]
	protected int m_iCameraIndex;
	
	[Attribute("2", UIWidgets.Slider, "TODO: fill", category: "PiPSights")]
	protected int m_iGuiIndex;

	// TODO@AS: Handle this along with other things via FovSightsInfo?
	[Attribute("40.0", UIWidgets.Slider, "Camera field of view used by this PIP component", params: "0 89.9 0.1", category: "PiPSights")]
	protected float m_fMainCameraFOV;

	[Attribute("0.2", UIWidgets.Slider, "Camera near clipping plane", params: "0 1000 0.01", category: "PiPSights")]
	protected float m_fNearPlane;

	[Attribute("1500", UIWidgets.Slider, "Camera field of view used by this PIP component", params: "0 5000 1", category: "PiPSights")]
	protected float m_fFarPlane;

	// Point info?
	[Attribute("1.0", UIWidgets.EditBox, "Scale of resolution used by the PIP", params: "0.1 1 0.1", category: "PiPSights")]
	protected float m_fResolutionScale;

	[Attribute("{972DF18CB9BFCBD4}Common/Postprocess/HDR_ScopePiP.emat", UIWidgets.EditBox, "", params: "emat", category: "PiPSights")]
	protected ResourceName m_rScopeHDRMatrial;
	
	// 0 = immediately when entering ADS camera, 1.0 = only after full blend
	[Attribute("0.25", UIWidgets.Slider, "Percentage of camera transition at which sights activate.",  params: "0.0 1.0 0.01", category: "PiPSights")]
	protected float m_fADSActivationPercentagePIP;
	
	// 0 = immediately when leaving ADS camera, 1.0 = only after full blend
	[Attribute("0.75", UIWidgets.Slider, "Percentage of camera transition at which sights deactivate.",  params: "0.0 1.0 0.01", category: "PiPSights")]
	protected float m_fADSDeactivationPercentagePIP;
	
	[Attribute("150.0", UIWidgets.Slider, "Scale of projection difference -> the more camera direction differs from scope directions, the bigger scope side parallax effect", params: "1 400 0.1", category: "PiPSights-Parallax")]
	protected float m_fProjectionDifferenceScale;

	[Attribute("0.014", UIWidgets.Slider, "Central distance = distance eye-scope with no parallax distant effect", params: "-0.2 0.2 0.001", category: "PiPSights-Parallax")]
	protected float m_fCenterDistance;

	[Attribute("0.8", UIWidgets.Slider, "How the parallax is affected when the eye moves closer to scope", params: "0 1 0.001", category: "PiPSights-Parallax")]
	protected float m_fDistanceMoveNear;

	[Attribute("0.4", UIWidgets.Slider, "How the parallax is affected when the eye moves farther from scope", params: "0 1 0.001", category: "PiPSights-Parallax")]
	protected float m_fDistanceMoveFar;

	[Attribute("0.5", UIWidgets.Slider, "Basic value for simulating parallax effect", params: "0 2 0.001", category: "PiPSights-Parallax")]
	protected float m_fBasicParallax;
	
	[Attribute("1.75", UIWidgets.Slider, "Max parallax value for simulating parallax effect (should be bigger than BasicParallax)", params: "0 2 0.001", category: "PiPSights-Parallax")]
	protected float m_fMaxParallax;

	[Attribute("0.0", UIWidgets.Slider, "Offset of scope center in X", params: "-1 1 0.001", category: "PiPSights-Parallax")]
	protected float m_fCenterOffsetX;

	[Attribute("0.0", UIWidgets.Slider, "Offset of scope center in Y", params: "-1 1 0.001", category: "PiPSights-Parallax")]
	protected float m_fCenterOffsetY;
	
	[Attribute("0.0", UIWidgets.Slider, "Radius of PIP scope from the center", category: "PiPSights", precision: 5)]
	protected float m_fScopeRadius;
	
	protected int m_iLastProjectionFrame = -1;
	protected vector m_vScreenScopeCenter;
	protected float m_fScreenScopeRadiusSq;
	
	protected IEntity m_CurrentPlayable;
	
	//! Current PIP reticle color
	protected ref Color m_cReticleColor = Color.Black;
	
	/*!
		Returns whether screen position is in located within the sights radius.
		\param screenPosition Point on screen
		\return Returns true if point is in picture in picture sights, false otherwise.
	*/
	bool IsScreenPositionInSights(vector screenPosition)
	{
		if (!m_PIPCamera || !IsSightADSActive()) // With no active camera or disabled sights, the point is never in PIP
			return false;
		
		BaseWorld world = GetGame().GetWorld();
		int index = world.GetFrameNumber();
		
		// Recalculate the projection data only once per frame, so we save some perf on huge queries
		if (m_iLastProjectionFrame != index)
		{
			WorkspaceWidget workspaceWidget = GetGame().GetWorkspace();
			
			vector sightsRear = GetSightsRearPosition();
			m_vScreenScopeCenter = workspaceWidget.ProjWorldToScreen(sightsRear, world);
			m_vScreenScopeCenter[2] = 0.0;
			
			vector extent = workspaceWidget.ProjWorldToScreen(sightsRear + m_pOwner.GetWorldTransformAxis(0) * m_fScopeRadius, world);
			extent[2] = 0.0;
			
			const float invSqrt2 = 0.70710678118; // 1.0 / Math.Sqrt(2.0);
			m_fScreenScopeRadiusSq = invSqrt2 * vector.DistanceSq(extent, m_vScreenScopeCenter);	
			m_iLastProjectionFrame = index;
		}
		
		float screenDistanceSq = vector.DistanceSq(screenPosition, m_vScreenScopeCenter);	
		return screenDistanceSq < m_fScreenScopeRadiusSq;
	}
		
	protected int m_iDisableVignetteFrames;
		
	//! The root layout hierarchy widget for PIP
	protected ref Widget m_wPIPRoot;
	
	//! Is PIP currently enabled?
	protected bool m_bPIPIsEnabled;
	protected static int s_bIsPIPActive;

	//! The render target texture found within our layout
	protected RTTextureWidget m_wRenderTargetTextureWidget;

	//! The render target found within our layout
	protected RenderTargetWidget m_wRenderTargetWidget;

	//! The camera to be used by this pip component
	protected CameraBase m_PIPCamera;
	
	IEntity 	m_pOwner;	
	Material 	m_pMaterial;
	int				m_iVignetteCenterXIndex;
	int 			m_iVignetteCenterYIndex;
	int   			m_iVignettePowerIndex;
	int 			m_iLensDistortIndex;
	int				m_iReticleOffsetXIndex;
	int 			m_iReticleOffsetYIndex;
	int				m_iReticleColorIndex;
	
	//------------------------------------------------------------------------------------------------
	/*!
		Returns the camera used for picture in picture mode or null if none.
	*/
	CameraBase GetPIPCamera()
	{
		return m_PIPCamera;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
		Returns the camera index used by this component when enabled.
	*/
	int GetPIPCameraIndex()
	{
		return m_iCameraIndex;
	}
	
	//------------------------------------------------------------------------------------------------
	override float GetADSActivationPercentage()
	{
		if (SCR_Global.IsScope2DEnabled())
			return super.GetADSActivationPercentage();
		
		return m_fADSActivationPercentagePIP;
	}
	
	//------------------------------------------------------------------------------------------------
	override float GetADSDeactivationPercentage()
	{
		if (SCR_Global.IsScope2DEnabled())
			return super.GetADSDeactivationPercentage();
		
		return m_fADSDeactivationPercentagePIP;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsPIPEnabled()
	{
		return m_bPIPIsEnabled;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
		Returns true if any sights PIP is active.
	*/
	static bool IsPIPActive()
	{
		return s_bIsPIPActive;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetMainCameraFOV()
	{
		if (!SCR_Global.IsScope2DEnabled())
			return m_fMainCameraFOV;
		
		return GetFOV();
	}
	
	//------------------------------------------------------------------------------------------------
	private bool IsLocalControlledEntity(IEntity pEntity, bool checkHierarchy = true)
	{
		if (!pEntity)
			return false;
		
		// We make sure that we only perform any local items on our local controlled entity
		auto pGame = GetGame();
		if (!pGame)
			return false;
		
		auto pPlayerController = pGame.GetPlayerController();
		if (!pPlayerController)
			return false;		
		
		auto pControlledEntity = pPlayerController.GetControlledEntity();
		if (!pControlledEntity)
			return false;
		
		// Straight hit
		if (pControlledEntity == pEntity)
			return true;
		
		// No can do
		if (!checkHierarchy)
			return false;
		
		// Check up in hierarchy?
		m_CurrentPlayable = pEntity.GetParent();

		while (m_CurrentPlayable)
		{
			if (pControlledEntity == m_CurrentPlayable)
				return true;
			
			m_CurrentPlayable = m_CurrentPlayable.GetParent();
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	/*! 
		Return actual parent (weapon) with calculated transformation in relation to it.
		\paran parent Direct parent entity which coords are provided in relation to
		\param position offset in relation to original parent
		\param angles rotation in degrees in relation to original parent
		\param matrix Out rotation in relation to new parent
		\return Returns new parent to use, usually the weapon
	*/
	IEntity GetPIPCameraLocalTransform(IEntity parent, vector position, vector angles, out vector matrix[4])
	{
		// Construct the local matrix in relation to original parent
		vector yawPitchRoll = Vector(angles[1], angles[0], angles[2]);
		Math3D.AnglesToMatrix(yawPitchRoll, matrix);
		matrix[3] = position;
		
		// Now additionally we will browse the hierarchy until we hit our weapon
		vector temp[4];
		Math3D.MatrixCopy(matrix, temp);
		IEntity lastNode = parent;
		
		BaseWeaponComponent weaponRoot;
		while (lastNode)
		{
			// Stop on weapon, that is final node
			weaponRoot = BaseWeaponComponent.Cast(lastNode.FindComponent(BaseWeaponComponent));
			if (weaponRoot)
				break;
			
			/// These multiplications will run even if we have no weapon,
			/// but it shouldn't be that big of a deal for now, weapon should exist regardless
			vector localTransform[4];
			lastNode.GetLocalTransform(localTransform);
			
			// If using a pivot, we need to apply the pivot transformation
			TNodeId pivotIndex = lastNode.GetPivot();
			if (pivotIndex > 0)
			{
				// Multiply pivot * local TM = model TM
				IEntity parentNode = lastNode.GetParent();
				if (parentNode)
				{
					vector pivotTM[4];
					parentNode.GetBoneMatrix(pivotIndex, pivotTM);
					
					// This should not be happening - there should
					// rather be no pivot, yet it triggers at times.
					// ??? TODO@AS: See if we can have better API for pivoting like this
					if (!SCR_Math3D.IsMatrixEmpty(pivotTM))	
						Math3D.MatrixMultiply4(pivotTM, localTransform, localTransform);
				}
			}
			Math3D.MatrixMultiply4( temp, localTransform, temp );
			lastNode = lastNode.GetParent();
		}
		
		// Only use calculated matrix+parent if its valid
		if (weaponRoot != null)
		{
			Math3D.MatrixCopy( temp, matrix );
			parent = lastNode;
		}
		
		return parent;
	}

	//------------------------------------------------------------------------------------------------
	//! \param parent Parent entity this camera should be attached to
	//! \param position Local offset from parent entity
	//! \param cameraIndex The index created camera should use, 0-31
	//! \param fov The field of view of camera 0-89
	//! \param nearPlane Near clipping plane of the camera in metres
	//! \param farPlane Far clipping plane of the camera in metres
	protected CameraBase CreateCamera(IEntity parent, vector position, vector angles, int cameraIndex, float fov, float nearPlane, float farPlane)
	{
		// Spawn camera
		BaseWorld baseWorld = parent.GetWorld();
		
		// Spawn it close
		EntitySpawnParams params = new EntitySpawnParams();
		parent.GetWorldTransform(params.Transform);
		SCR_PIPCamera pipCamera = SCR_PIPCamera.Cast(GetGame().SpawnEntity(SCR_PIPCamera, baseWorld, params));
		
		vector mat[4];
		parent = GetPIPCameraLocalTransform(parent, position, angles, mat);
		

		
		pipCamera.SetVerticalFOV(fov);
		pipCamera.SetNearPlane(nearPlane);
		pipCamera.SetFarPlane(farPlane);		
		pipCamera.ApplyProps(m_iCameraIndex);
		baseWorld.SetCameraLensFlareSet(cameraIndex, CameraLensFlareSetType.FirstPerson, string.Empty);

		// Set camera to hierarchy
		parent.AddChild(pipCamera, -1, EAddChildFlags.AUTO_TRANSFORM);
		pipCamera.SetLocalTransform(mat);
		pipCamera.UpdatePIPCamera(1.0, m_iCameraIndex);
		return pipCamera;
	}
	
	//------------------------------------------------------------------------------------------------
	protected Widget CreateUI(string layout, string rtTextureName, string rtName, out RTTextureWidget RTTexture, out RenderTargetWidget RTWidget)
	{
		// Empty layout, cannot create any widget
		if (layout == string.Empty)
			return null;

		// Create layout
		Widget root = GetGame().GetWorkspace().CreateWidgets(layout);

		// Layout was not created successfully
		if (!root)
			return null;

		// We dont have required RT widgets, delete layout and terminate
		RTTexture = RTTextureWidget.Cast(root.FindAnyWidget(rtTextureName));
		RTWidget = RenderTargetWidget.Cast(root.FindAnyWidget(rtName));
		if (!RTTexture || !RTWidget)
		{
			root.RemoveFromHierarchy();
			return null;
		}

		return root;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetPIPEnabled(bool enabled)
	{
		// disabled->enabled
		// Create neccessary items
		if (enabled && !m_bPIPIsEnabled)
		{
			m_iDisableVignetteFrames = 5;
			
			// Try to create UI for PIP,
			// output params are either set to valid ones,
			// or root itself is set to null and destroyed
			if (!m_wPIPRoot || !m_wRenderTargetTextureWidget || !m_wRenderTargetWidget)
				m_wPIPRoot = CreateUI(m_sPIPLayoutResource, m_sRTTextureWidgetName, m_sRTargetWidgetName, m_wRenderTargetTextureWidget, m_wRenderTargetWidget);

			if (!m_wPIPRoot)
			{
				Print("Could not create PIP layouts!", LogLevel.ERROR);
				return;
			}
	
			// Create PIP camera
			if (!m_PIPCamera)
				m_PIPCamera = CreateCamera(m_Owner, m_vCameraPoint, m_vCameraAngles, m_iCameraIndex, GetFOV(), m_fNearPlane, m_fFarPlane);
			
			if (!m_PIPCamera)
			{
				Print("Could not create PIP camera!", LogLevel.ERROR);
				return;
			}
	
			// Set camera index of render target widget
			BaseWorld baseWorld = m_Owner.GetWorld();
			m_wRenderTargetWidget.SetWorld(baseWorld, m_iCameraIndex);
	
			// Set resolution scale
			m_wRenderTargetWidget.SetResolutionScale(m_fResolutionScale, m_fResolutionScale);
	
			if (!m_Owner.IsDeleted())
				m_wRenderTargetTextureWidget.SetGUIWidget(m_Owner, m_iGuiIndex);

			if (m_pMaterial)
			{
				GetGame().GetWorld().SetCameraPostProcessEffect(m_iCameraIndex, 10, PostProcessEffectType.HDR, m_rScopeHDRMatrial);
			}
			
			s_bIsPIPActive = true;
			m_bPIPIsEnabled = true;
			return;
		}

		// enabled -> disabled
		if (!enabled && m_bPIPIsEnabled)
		{
			Destroy();
			s_bIsPIPActive = false;
			m_bPIPIsEnabled = false;
			return;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DestroyCamera(CameraBase camera)
	{
		if (camera)
		{
			IEntity cameraParent = camera.GetParent();
			if (cameraParent)
				cameraParent.RemoveChild(camera);
			
			camera.GetWorld().SetCameraLensFlareSet(m_iCameraIndex, CameraLensFlareSetType.None, string.Empty);

			delete camera;
		}
	}

	//------------------------------------------------------------------------------------------------
	// Destroy everything this component created during its lifetime
	protected void Destroy()
	{
		if (m_wRenderTargetTextureWidget && m_Owner && !m_Owner.IsDeleted())
			m_wRenderTargetTextureWidget.SetGUIWidget(m_Owner, -1);
		
		if (m_wPIPRoot) 
		{
			m_wPIPRoot.RemoveFromHierarchy();
			m_wPIPRoot = null;
		}
		
		DestroyCamera(m_PIPCamera);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Toggle between illumination modes 
	protected override void EnableReticleIllumination(bool enable)
	{
		if (m_b2DIsEnabled)
		{
			super.EnableReticleIllumination(enable);
			return;
		}
		
		if (enable)
		{
			m_cReticleColor = GetReticleTextureIllumination();
		}
		else
		{
			m_cReticleColor = Color.Black;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnSightADSActivated()
	{
		// A component without an owner shouldnt exist
		if (!m_Owner)
			return;
		
		// We make sure that we only perform any local items on our local controlled entity
		if (!IsLocalControlledEntity(m_Owner))
			return;
		
		// Set fov TODO@AS: Copied from base?
		if (!m_SightsFovInfo)
		{
			m_SightsFovInfo = SCR_SightsZoomFOVInfo.Cast(GetFOVInfo());
			SetupFovInfo();
		}
		
		// Initialize to current zero value
		m_fCurrentReticleOffsetY = GetReticleYOffsetTarget();
		
		bool scope2d = SCR_Global.IsScope2DEnabled();
		if(!scope2d)
		{
			m_iDisableVignetteFrames = 15;
			SetPIPEnabled(true);
			UpdateVignette();
			
			// Switching input
			if (m_SightsFovInfo.GetStepsCount() > 1)
				GetGame().GetInputManager().AddActionListener(ACTION_WHEEL, EActionTrigger.VALUE, SelectNextZoomLevel);
			
			// Setup illumination 
			if (m_bHasIllumination)
			{
				GetGame().GetInputManager().AddActionListener(ACTION_ILLUMINATION, EActionTrigger.DOWN, ToggleIllumination);
				EnableReticleIllumination(m_bIsIlluminationOn);
			}
		}
		
		s_OnSightsADSChanged.Invoke(true, m_fMainCameraFOV);
		
		if(scope2d)
		{
			if(m_bPIPIsEnabled)
			{
				SetPIPEnabled(false);
			}
			
			m_b2DIsEnabled = true;
			super.OnSightADSActivated();
		}
		else if(!scope2d && m_b2DIsEnabled)
		{
			m_b2DIsEnabled = false;
			super.OnSightADSDeactivated();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnSightADSDeactivated()
	{
		// A component without an owner shouldnt exist
		if (!m_Owner)
			return;
		
		if (!m_CurrentPlayable)
		{
			if (IsPIPEnabled())
			{
				// Make sure to revert overlay camera to default
				if (m_PIPCamera)
				{
					auto camMgr = GetGame().GetCameraManager();
					if (camMgr.GetOverlayCamera() == m_PIPCamera)
						camMgr.SetOverlayCamera(null);
				}
				
				SetPIPEnabled(false);
			}
			
			return;
		}
		
		s_OnSightsADSChanged.Invoke(false, 0);
		
		
		bool scope2d = SCR_Global.IsScope2DEnabled();
		if(scope2d)
		{
			super.OnSightADSDeactivated();
			m_b2DIsEnabled = false;
		}
		else 
		{
			SetPIPEnabled(false);
			GetGame().GetInputManager().RemoveActionListener(ACTION_ILLUMINATION, EActionTrigger.DOWN, ToggleIllumination);
		}
		
		// Removing switching input
		GetGame().GetInputManager().RemoveActionListener(ACTION_WHEEL, EActionTrigger.VALUE, SelectNextZoomLevel);
		GetGame().GetInputManager().RemoveActionListener(ACTION_ILLUMINATION, EActionTrigger.DOWN, ToggleIllumination);
	}
	
	
	//------------------------------------------------------------------------------------------------
	void UpdateCamera(float timeSlice)
	{
		SCR_PIPCamera pipCamera = SCR_PIPCamera.Cast(m_PIPCamera);
		if (pipCamera)
		{
			// Set proper FOV for camera, zoom levels can vary
			SightsFOVInfo fovInfo = GetFOVInfo();
			if (fovInfo)
				pipCamera.SetVerticalFOV(fovInfo.GetFOV());
			
			// Get local tm in relation to actual parent
			vector mat[4];
			vector angles = m_vCameraAngles + Vector(m_fCurrentCameraPitch, 0.0, 0.0);
			GetPIPCameraLocalTransform(m_pOwner, m_vCameraPoint, angles, mat);
			
			// Apply zero angles to default rot
			pipCamera.SetLocalTransform(mat);
			
			// Finally update camera props
			pipCamera.UpdatePIPCamera(timeSlice, m_iCameraIndex);
		}
	}
	
	//------------------------------------------------------------------------------------------------	
	void UpdateHDR()
	{
		int mainCameraIndex = 0;
		CameraManager manager = GetGame().GetCameraManager();
		if (manager)
		{
			CameraBase cam = manager.CurrentCamera();
			if (cam)
				mainCameraIndex = cam.GetCameraIndex();
		}
		//don't forget to disable preexposure on scope material !
		float hdrBrightness = m_pOwner.GetWorld().GetCameraHDRBrightness(mainCameraIndex);
		m_pOwner.GetWorld().SetCameraHDRBrightness(m_iCameraIndex, hdrBrightness);
	}

	
	//------------------------------------------------------------------------------------------------
	void UpdateVignette()
	{
		if (m_iDisableVignetteFrames > 0)
		{
			m_pMaterial.SetParamByIndex(m_iVignetteCenterXIndex, 0);
			m_pMaterial.SetParamByIndex(m_iVignetteCenterYIndex, 0);
			m_pMaterial.SetParamByIndex(m_iVignettePowerIndex, 2);
			
			m_iDisableVignetteFrames--;
			return;
		}
		
		IEntity character = GetGame().GetPlayerController().GetControlledEntity();
		if (!character)
			return;
				
		BaseWeaponManagerComponent weaponManager = BaseWeaponManagerComponent.Cast(character.FindComponent(BaseWeaponManagerComponent));
		if (!weaponManager)
			return;
		
		auto cameraManager = GetGame().GetCameraManager();
		if (!cameraManager)
			return;
		
		CameraBase mainCamera = cameraManager.CurrentCamera();
		if (!mainCamera)
			return;
			
		//main camera matrix in local coordinates
		vector mainCam[4];
		mainCamera.GetLocalTransform(mainCam);

		
		//sights matrix in local coordinates
		vector sightsLSCam[4];
		float fov;
		weaponManager.GetCurrentSightsCameraTransform(sightsLSCam, fov);
		
		//camera pos (in world pos but need just direction)
		vector pipCam[4];
		m_PIPCamera.GetWorldTransform(pipCam);

				
		//position of scope in main camera space
		vector scopePosCam = sightsLSCam[3] - mainCam[3];
		
		//vectors 
		vector scopeFwd = sightsLSCam[2];//m_pOwner.GetWorldTransformAxis(2);
		vector camFwd   = mainCam[2];//m_PIPCamera.GetWorldTransformAxis(2);
		
		//Print(scopeFwd);
		//Print(camFwd);
		
		//project to reticle plane at scopePosCam, plane = n*P + d = 0 -> scopeFwd dot scopePosLS + d = 0
		//ray = C + s*t = camPos + camFwd*t
		// t = (n*P - n*C) / (n*s)
		//camPos is zero in this case
		float ns = vector.Dot(camFwd, scopeFwd);
		float nP = vector.Dot(scopePosCam, scopeFwd);
		
		float t = nP/ns;
		vector projP = camFwd*t;

		vector locPos = projP - scopePosCam;


		//final  move of vignette
		float centerX = locPos[0]*m_fProjectionDifferenceScale + m_fCenterOffsetX;
		float centerY = locPos[1]*m_fProjectionDifferenceScale + m_fCenterOffsetY;
	
	
		//distance from center distance - base of the eye pos from the projection plane
		//TODO: t is for ARTII weapon negative !
		float distance = Math.AbsFloat(t) - m_fCenterDistance;
		
		//change some material setting based on parameters
		float vignettePower = 0;
		//float lensPower     = 0;
		
		 //bigger power = much faster change with distance
		float distancePower;
		if (distance > 0)
		{
			//when we are farther to scope 
			distancePower = m_fDistanceMoveFar;
		} else 
		{
			//when we are closer to scope 
			distancePower	= m_fDistanceMoveNear;
			//lensPower 		= -5.5;
		}

		//stronger at beginning then slower -> some power function, log2 as Math from some reason doesn't contain power function
		float fc = 1 - 1/(Math.Log2(distancePower*Math.AbsFloat(distance) + 2));
			
		//max vignette power is 2, let a little opened
		vignettePower = Math.Clamp(fc*100, m_fBasicParallax, m_fMaxParallax);
			
		//how is it with lens distortion? now at zero is the -5.5
		//lensPower = lensPower*Math.Clamp(-distance/m_fCenterDistance, 0, 5);

		
		//modify vignete power also by distance from scope center
		//locPos[2] = 0;
		//const float ProjectionCenterDistanceModif = 140.0;
		//vignettePower += locPos.Length()*ProjectionCenterDistanceModif;

		//Print(centerX);
		//Print(centerY);
		//Print(vignettePower);

		//centerX = Math.Clamp(centerX, -4, 4);
		//centerY = Math.Clamp(centerY, -4, 4);
		
		m_pMaterial.SetParamByIndex(m_iVignetteCenterXIndex, centerX);
		m_pMaterial.SetParamByIndex(m_iVignetteCenterYIndex, centerY);
		m_pMaterial.SetParamByIndex(m_iVignettePowerIndex, vignettePower);
		m_pMaterial.SetParamByIndex(m_iReticleOffsetXIndex, m_fReticleOffsetX);
		m_pMaterial.SetParamByIndex(m_iReticleOffsetYIndex, m_fCurrentReticleOffsetY);
		float reticleRGBA[] = { m_cReticleColor.R(), m_cReticleColor.G(), m_cReticleColor.B(), m_cReticleColor.A() };
		m_pMaterial.SetParamByIndex(m_iReticleColorIndex, reticleRGBA);
		
		//m_pMaterial.SetParamByIndex(m_iLensDistortIndex, lensPower); // disabled for now, set in material directly
	}

	
	//------------------------------------------------------------------------------------------------
	protected override void OnSightADSPostFrame(IEntity owner, float timeSlice)
	{
		super.OnSightADSPostFrame(owner, timeSlice);
		
		
		// Before any updates make sure that we are in target state
		bool use2D = SCR_Global.IsScope2DEnabled();
		if ((m_bPIPIsEnabled && use2D) || (!m_bPIPIsEnabled && !use2D))
		{
			// Reactivate on change
			OnSightADSDeactivated();
			OnSightADSActivated();
		}

		if (m_bPIPIsEnabled)
		{
			UpdateCamera(timeSlice);
			UpdateHDR();
			
			if (m_pMaterial && m_PIPCamera)
			{
				UpdateVignette();
			}
		}
		
		#ifdef ENABLE_DIAG
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_WEAPONS_PIP_SIGHTS))
			DrawDiagWindow();
		#endif
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnInit(IEntity owner)
	{
		super.OnInit(owner);
		
		m_pOwner = owner;
		
		if (m_rScopeHDRMatrial.IsEmpty())
			Print("Scope HDR material is empty!", LogLevel.WARNING);
		
		ResourceName mat = m_rScopeHDRMatrial;
		m_pMaterial = Material.GetOrLoadMaterial(mat, 0);

		if (m_pMaterial)
		{
			m_iVignetteCenterXIndex = m_pMaterial.GetParamIndex("VignetteCenterX");
			m_iVignetteCenterYIndex = m_pMaterial.GetParamIndex("VignetteCenterY");
			m_iVignettePowerIndex   = m_pMaterial.GetParamIndex("Vignette");
			m_iLensDistortIndex     = m_pMaterial.GetParamIndex("LensDistort");
			m_iReticleOffsetXIndex = m_pMaterial.GetParamIndex("ReticleOffsetX");
			m_iReticleOffsetYIndex = m_pMaterial.GetParamIndex("ReticleOffsetY");
			m_iReticleColorIndex = m_pMaterial.GetParamIndex("ReticleColor");
		}
		else
		{
			Print("Cannot initialize PiP HDR PP !", LogLevel.ERROR);
		}
	}

	
	//------------------------------------------------------------------------------------------------
	protected override void ApplyRecoilToCamera(inout vector pOutCameraTransform[4], vector aimModAngles)
	{	
		if(m_bPIPIsEnabled)
			return;
		
		vector weaponAnglesMat[3];
		Math3D.AnglesToMatrix(aimModAngles, weaponAnglesMat);				
		Math3D.MatrixMultiply3(pOutCameraTransform, weaponAnglesMat, pOutCameraTransform);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override bool CanFreelook()
	{
		if (m_bPIPIsEnabled)
			return true;
		
		return super.CanFreelook();
	}
	
	#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	protected override void _WB_AfterWorldUpdate(IEntity owner, float timeSlice)
	{
		const float axisLength = 0.1;

		vector origin = owner.GetOrigin() + owner.VectorToParent(m_vCameraPoint);
		vector right = origin + owner.GetTransformAxis(0) * axisLength;
		vector up = origin + owner.GetTransformAxis(1) * axisLength;
		vector fwd = origin + owner.GetTransformAxis(2) * axisLength;

		ref auto rightArrow = Shape.CreateArrow(origin, right, 0.01, ARGB(240,255,0,0), ShapeFlags.ONCE);
		ref auto upArrow = Shape.CreateArrow(origin, up, 0.01, ARGB(240,0,255,0), ShapeFlags.ONCE);
		ref auto fwdArrow = Shape.CreateArrow(origin, fwd, 0.01, ARGB(240,0,0,255), ShapeFlags.ONCE);
		
		vector scopeCenter = GetSightsRearPosition();
		vector scopeExtent = scopeCenter + owner.GetTransformAxis(0) * m_fScopeRadius;
		Shape.CreateArrow(scopeCenter, scopeExtent, 0.01, ARGB(255,255,128,128), ShapeFlags.ONCE);
	}
	#endif	
	
	#ifdef ENABLE_DIAG
	private static bool s_DiagRegistered;
	
	//------------------------------------------------------------------------------------------------
	protected void InputFloatClamped(inout float value, string label, float min, float max, int pxWidth = 100)
	{
		DbgUI.InputFloat(label, value, pxWidth);
		value = Math.Clamp( value, min, max );
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DrawDiagWindow()
	{
		DbgUI.Begin("2DPIPSights");
		{
			InputFloatClamped(m_fProjectionDifferenceScale, "m_fProjectionDifferenceScale", 1.0, 400.0);
			InputFloatClamped(m_fCenterDistance, "m_fCenterDistance", -0.2, 0.2);
			InputFloatClamped(m_fDistanceMoveNear, "m_fDistanceMoveNear", 0.0, 1.0);
			InputFloatClamped(m_fDistanceMoveFar, "m_fDistanceMoveFar", 0.0, 1.0);
			InputFloatClamped(m_fBasicParallax, "m_fBasicParallax", 0.0, 2.0);
			InputFloatClamped(m_fMaxParallax, "m_fMaxParallax", 0.0, 2.0);
			InputFloatClamped(m_fCenterOffsetX, "m_fCenterOffsetX", -1.0, 1.0);
			InputFloatClamped(m_fCenterOffsetY, "m_fCenterOffsetY", -1.0, 1.0);
			InputFloatClamped(m_fReticleOffsetX, "m_fReticleOffsetX", -1.0, 1.0);
			InputFloatClamped(m_fReticleOffsetY, "m_fReticleOffsetY", -1.0, 1.0);
			InputFloatClamped(m_fCurrentCameraPitch, "m_fCurrentCameraPitch", -90.0, 90.0);
			
		}
		DbgUI.End();
	}
	#endif

	//------------------------------------------------------------------------------------------------
	//! Constructor
	void SCR_2DPIPSightsComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		#ifdef ENABLE_DIAG
		if (!s_DiagRegistered)
		{
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_WEAPONS_PIP_SIGHTS,"","Show PIP settings diag","Weapons");
			s_DiagRegistered = true;
		}
		#endif
	}

	//------------------------------------------------------------------------------------------------
	//! Destructor
	void ~SCR_2DPIPSightsComponent()
	{		
		Destroy();
		if (m_pMaterial)
		{
			m_pMaterial.Release();
			m_pMaterial = null;
		}
		
		/*
			This should prevent some static leakness.
		*/
		if (IsPIPEnabled())
			SetPIPEnabled(false);
	}
};