[EntityEditorProps(category: "GameScripted/Weapon/Sights", description: "", color: "0 0 255 255")]
class SCR_2DOpticsComponentClass : ScriptedSightsComponentClass
{
};

//------------------------------------------------------------------------------------------------
//! Base class for 2D optics
//! Unifiying binoculars and optic sight
class SCR_2DOpticsComponent : ScriptedSightsComponent
{
	// Action names
	const string ACTION_WHEEL = "WeaponChangeMagnification";
	const string ACTION_ILLUMINATION = "WeaponToggleSightsIllumination";

	// Default widget names
	const string WIDGET_LAYOUT_REAR = "HRearEyePiece";
	const string WIDGET_LAYOUT_FRONT = "HFrontObjective";
	const string WIDGET_IMAGE_COVER = "ImgCover";

	const string WIDGET_SIZE_REAR = "SizeRear";
	const string WIDGET_SIZE_OBJECTIVE = "SizeFrontObjective";

	const string WIDGET_IMAGE_PADDING_LEFT = "ImgPaddingLeft";
	const string WIDGET_IMAGE_PADDING_RIGHT = "ImgPaddingRight";
	const string WIDGET_IMAGE_PADDING_TOP = "ImgPaddingTop";
	const string WIDGET_IMAGE_PADDING_BOTTOM = "ImgPaddingBottom";

	const string WIDGET_OVERLAY_RETICLES = "OverlayReticles";
	const string WIDGET_IMAGE_RETICLE = "ImgReticle";
	const string WIDGET_IMAGE_RETICLE_GLOW = "ImgReticleGlow";
	const string WIDGET_IMAGE_VIGNETTE = "ImgVignette";
	const string WIDGET_IMAGE_SCRATCHES = "ImgScratches";
	const string WIDGET_BLUR = "Blur";
	const string WIDGET_IMAGE_FILTER = "ImgFilter";

	const float REFERENCE_FOV = 38;
	const float OPACITY_INITIAL = 0.75;

	const float NEAR_PLANE_DEFAULT = 0.05;
	const float NEAR_PLANE_ZOOMED = 0.05;

	// Optics setup
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Layout used for 2D sights HUD\n", params: "layout", category: "Resources")]
	protected ResourceName m_sLayoutResource;

	[Attribute("", UIWidgets.ResourceNamePicker, desc: "Texture of reticle\n", params: "edds imageset", category: "Resources")]
	protected ResourceName m_sReticleTexture;

	[Attribute("", UIWidgets.ResourceNamePicker, desc: "Texture of reticle under glow\n", params: "edds imageset", category: "Resources")]
	protected ResourceName m_sReticleGlowTexture;

	[Attribute("0", category: "Resources")]
	protected bool m_bHasIllumination;

	[Attribute("0 0 0", UIWidgets.ColorPicker, category: "Resources")]
	protected ref Color m_cReticleTextureIllumination;

	[Attribute("", UIWidgets.ResourceNamePicker, desc: "Texture of lens filter\n", params: "edds imageset", category: "Resources")]
	protected ResourceName m_sFilterTexture;

	[Attribute("10", UIWidgets.EditBox, desc: "Optic magnification\n", params: "0.1 200 0.001", category: "Sights", precision: 3)]
	protected float m_fMagnification;
	
	[Attribute("0", UIWidgets.Slider, "Angular size of reticle\n[ ° ]", params: "0 60 0.001", category: "Sights", precision: 3)]
	protected float m_fReticleAngularSize;

	[Attribute("1", UIWidgets.Slider, "Portion of reticle with specified angular size\n", params: "0 10 0.00001", category: "Sights", precision: 5)]
	protected float m_fReticlePortion;

	[Attribute("0", UIWidgets.Slider, "POSITIVE:\n Rear focal plane - magnification at which reticle markings are valid\n\nZERO:\n Front focal plane reticle scaling with magnification.\n\nNEGATIVE:\n Enforced base FOV in degrees\n", params: "-60 200 0.001", category: "Sights", precision: 5)]
	protected float m_fReticleBaseZoom;

	[Attribute("0", UIWidgets.CheckBox, "Check if the optic is attached to a turret\n", category: "Sights")]
	protected bool m_bIsTurretOptic;
	
	[Attribute("10", UIWidgets.EditBox, desc: "Field of view of the objective\n[ ° ]", params: "0.001 60 0.001", category: "2DSights", precision: 3)]
	protected float m_fObjectiveFov;

	[Attribute("1", UIWidgets.EditBox, desc: "Ocular texture scale\n", params: "0.001 10 0.001", category: "2DSights", precision: 3)]
	protected float m_fObjectiveScale;

	[Attribute("1.1", UIWidgets.EditBox, desc: "Vignette texture scale\n", params: "0.001 10 0.001", category: "2DSights", precision: 3)]
	protected float m_fVignetteScale;

	[Attribute("1", UIWidgets.CheckBox, "Should hide parent object when using 2D sights", category: "2DSights")]
	bool m_bShouldHideParentObject;

	[Attribute("0", UIWidgets.CheckBox, "Should hide parent character when using 2D sights", category: "2DSights")]
	bool m_bShouldHideParentCharacter;

	// Animation attributes
	[Attribute("0", UIWidgets.Slider, "Time before entering animation starts", params: "0 2000 1", category: "Animations")]
	protected int m_iHudActivationDelay;

	[Attribute("100", UIWidgets.Slider, "Animation time for canceling optic.", params: "0 2000 1", category: "Animations")]
	protected int m_iHudDeactivationDelay;

	[Attribute("5", UIWidgets.Slider, "Time before entering animation starts", params: "0 2000 1", category: "Animations")]
	protected int m_iAnimationEnterDelay;

	[Attribute("0.5", UIWidgets.Slider, "Animation time for using optic", params: "0.01 5 0.01", category: "Animations")]
	protected float m_fAnimationEnter;

	[Attribute("2", UIWidgets.Slider, "Speed of blur fade out on optic entering", params: "0.01 5 0.01", category: "Animations")]
	protected float m_fAnimationSpeedBlur;

	// Effect attributes
	[Attribute("0", UIWidgets.EditBox, desc: "Optic misalignment damping speed - intended for binoculars, the higher, the faster the misalignment gets zeroed", params: "0 60 0.1", category: "Effects")]
	protected float m_fMisalignmentDampingSpeed;

	[Attribute("0.1", UIWidgets.EditBox, desc: "Rotation effect scaling", params: "-100 100 0.01", category: "Effects")]
	protected float m_fRotationScale;

	[Attribute("4", UIWidgets.EditBox, desc: "Rotation damping speed - the higher, the faster the rotation speed gets zeroed", params: "0 60 0.1", category: "Effects")]
	protected float m_fRotationDampingSpeed;

	[Attribute("0.01", UIWidgets.EditBox, desc: "Movement effect scaling", params: "-100 100 0.01", category: "Effects")]
	protected float m_fMovementScale;

	[Attribute("2", UIWidgets.EditBox, desc: "Movement damping speed - the higher, the faster the movement speed gets zeroed", params: "0 60 0.1", category: "Effects")]
	protected float m_fMovementDampingSpeed;

	[Attribute("1", UIWidgets.EditBox, desc: "Roll effect scaling", params: "-100 100 0.01", category: "Effects")]
	protected float m_fRollScale;

	[Attribute("0", UIWidgets.EditBox, desc: "Roll damping speed - the higher, the faster the optic gets uprighted", params: "0 60 0.1", category: "Effects")]
	protected float m_fRollDampingSpeed;

	[Attribute("1", UIWidgets.EditBox, desc: "Optic misalignment scale - for turrets and binoculars", params: "-60 60 0.1", category: "Effects")]
	protected float m_fMisalignmentScale;

	[Attribute("1", UIWidgets.EditBox, desc: "Vignette adjustment speed - the faster, the less inertia there is", params: "0 60 0.1", category: "Effects")]
	protected float m_fVignetteMoveSpeed;

	[Attribute("1", UIWidgets.EditBox, desc: "Motion blur effect scaling", params: "0 100", category: "Effects")]
	protected float m_fMotionBlurScale;

	[Attribute("0.1", UIWidgets.EditBox, desc: "Max level of motion blur intensity", params: "0 1 0.01", category: "Effects")]
	protected float m_fMotionBlurMax;

	// 0 = immediately when entering ADS camera, 1.0 = only after full blend
	[Attribute("1", UIWidgets.Slider, "Percentage of camera transition at which sights activate.", params: "0.0 1.0 0.01", category: "Sights")]
	protected float m_fADSActivationPercentage;

	// 0 = immediately when leaving ADS camera, 1.0 = only after full blend
	[Attribute("0", UIWidgets.Slider, "Percentage of camera transition at which sights deactivate.", params: "0.0 1.0 0.01", category: "Sights")]
	protected float m_fADSDeactivationPercentage;

	[Attribute("0", UIWidgets.ComboBox, "Type of zeroing for this sights.", "", ParamEnumArray.FromEnum(SCR_EPIPZeroingType), category: "Sights" )]
	protected SCR_EPIPZeroingType m_eZeroingType;

	[Attribute("0", UIWidgets.Slider, "Reticle Offset of scope center in X", params: "-1 1 0.0001", precision: 5, category: "Sights")]
	protected float m_fReticleOffsetX;

	[Attribute("0", UIWidgets.Slider, "Reticle Offset of scope center in Y", params: "-1 1 0.0001", precision: 5, category: "Sights")]
	protected float m_fReticleOffsetY;

	[Attribute("25", UIWidgets.Slider, "Interpolation speed for zeroing interpolation", params: "1 100.0 0.1", category: "Sights")]
	protected float m_fReticleOffsetInterpSpeed;

	[Attribute("0 0 0", UIWidgets.EditBox, "Camera offset when looking through scope", category: "Sights", params: "inf inf 0 purpose=coords space=entity anglesVar=m_vCameraPoint")]
	protected vector m_vCameraOffset;

	[Attribute("0 0 0", UIWidgets.EditBox, "Camera angles when looking through scope", category: "Sights", params: "inf inf 0 purpose=angles space=entity coordsVar=m_vCameraAngles")]
	protected vector m_vCameraAngles;

	protected float m_fCurrentReticleOffsetY;
	protected float m_fCurrentCameraPitch;

	float GetADSActivationPercentage()
	{
		return m_fADSActivationPercentage;
	}

	float GetADSDeactivationPercentage()
	{
		return m_fADSDeactivationPercentage;
	}

	// Needed for zeroing
	SCR_EPIPZeroingType GetZeroType()
	{
		return m_eZeroingType;
	}

	float GetMagnification()
	{
		return m_fMagnification;
	}

	// Scope widgets
	protected Widget m_wRootWidget;

	protected Widget m_wLayoutRear;
	protected Widget m_wLayoutFront;
	protected ImageWidget m_wImgCover;
	protected ImageWidget m_wImgScratches;

	protected Widget m_wRearPaddingLeft;
	protected Widget m_wRearPaddingRight;
	protected Widget m_wRearPaddingTop;
	protected Widget m_wRearPaddingBottom;

	protected Widget m_wFrontPaddingLeft;
	protected Widget m_wFrontPaddingRight;
	protected Widget m_wFrontPaddingTop;
	protected Widget m_wFrontPaddingBottom;
	protected Widget m_wOverlayReticles;
	protected ImageWidget m_wImgReticle;
	protected ImageWidget m_wImgReticleGlow;
	protected BlurWidget m_wBlur;
	protected ImageWidget m_wImgFilter;

	protected SizeLayoutWidget m_wSizeLayoutRear;
	protected SizeLayoutWidget m_wSizeLayoutObjective;

	protected ref array<float> m_aReticleSizes = {};
	protected int m_iSelectedZoomLevel = 0;

	// Overall movement
	protected vector m_vObjectiveOffset; // Objective and reticle offset
	protected vector m_vVignetteOffset; // Vignette offset
	protected vector m_vMisalignmentOffset;

	protected vector m_vRotation;
	protected vector m_vRotationSpeed;
	protected vector m_vRotationOffset;

	protected vector m_vMovement;
	protected vector m_vMovementSpeed;
	protected vector m_vMovementOffset;

	protected float m_fRollOffset;
	protected float m_fScratchesRoll;

	// FOVs
	protected float m_fFovZoomed;
	protected float m_fDefaultSize;
	protected float m_fCurrentReticleSize;
	protected float m_fNearPlaneCurrent;

	// Movement check
	protected bool m_bIsMoving;
	protected bool m_bIsRotating;
	protected bool m_bZoomed = false;
	protected bool m_bInitialBlurOver;
	bool m_bWasEntityHidden = false;
	protected int m_iHeadBoneId = -1;

	// Owner and character references
	protected ChimeraCharacter m_ParentCharacter = null;

	static ref ScriptInvoker<bool, m_fFovZoomed> s_OnSightsADSChanged = new ScriptInvoker();

	//------------------------------------------------------------------------------------------------
	float GetFovZoomed() { return m_fFovZoomed; }

	//------------------------------------------------------------------------------------------------
	float GetNearPlane() { return m_fNearPlaneCurrent; }

	//------------------------------------------------------------------------------------------------
	float GetHudActivationDelay() { return m_iHudActivationDelay; }

	//------------------------------------------------------------------------------------------------
	/*!
	Called during EOnInit.
	\param owner Entity this component is attached to.
	*/
	override void OnInit(IEntity owner)
	{
		m_fScratchesRoll = Math.RandomFloat(0, 360);
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnSightADSActivated()
	{
		// Setup scope widgets
		if (!m_wRootWidget)
		{
			m_ParentCharacter = ChimeraCharacter.Cast(GetOwner().GetParent());

			SCR_HUDManagerComponent hudManager = SCR_HUDManagerComponent.GetHUDManager();
			if (hudManager)
				m_wRootWidget = hudManager.CreateLayout(m_sLayoutResource, EHudLayers.BACKGROUND);

			if (m_wRootWidget)
			{
				m_fFovZoomed = CalculateZoomFOV(m_fMagnification);

				// HUD and widgets setup
				m_wRootWidget.SetZOrder(-1);
				m_wRootWidget.SetVisible(false);

				FindWidgets();
				SetupOpticImage();
			}
		}

		super.OnSightADSActivated();
		s_OnSightsADSChanged.Invoke(true, m_fFovZoomed);

		// Prevent calnceling animation
		AnimateWidget.StopAnimation(m_wRootWidget, WidgetAnimationOpacity);
		GetGame().GetCallqueue().Remove(DeactivateHUD);

		// Play cover fade in animaiton
		m_fNearPlaneCurrent = NEAR_PLANE_DEFAULT;
		GetGame().GetCallqueue().CallLater(ActivateHUD, m_iHudActivationDelay, false);

		m_bZoomed = true;
		m_bInitialBlurOver = false;

		m_vMisalignmentOffset = vector.Zero;

		GetRotation(m_vRotation, 1);
		m_vRotationSpeed = vector.Zero;
		m_vRotationOffset = vector.Zero;

		GetMovement(m_vMovement, 1);
		m_vMovementSpeed = vector.Zero;
		m_vMovementOffset = vector.Zero;
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnSightADSDeactivated()
	{
		super.OnSightADSDeactivated();
		s_OnSightsADSChanged.Invoke(false, 0);

		// Initialize to current zero value
		m_fCurrentReticleOffsetY = GetReticleYOffsetTarget();

		if (m_bWasEntityHidden)
		{
			IEntity owner = GetOwner();
			if (owner)
				owner.SetFlags(EntityFlags.VISIBLE, false);

			if (m_ParentCharacter)
				m_ParentCharacter.SetFlags(EntityFlags.VISIBLE, false);

			m_bWasEntityHidden = false;
		}

		GetGame().GetCallqueue().Remove(ActivateHUD); // in case this is called quickly after OnSightADSActivated, ActivateHUD needs to be cleared to prevent conflict

		// Prevent entering animation
		AnimateWidget.StopAnimation(m_wRootWidget, WidgetAnimationOpacity);
		GetGame().GetCallqueue().Remove(PlayEntryAnimation);

		if (m_wImgCover)
			m_wImgCover.SetOpacity(0);

		// Leaving animation
		m_fNearPlaneCurrent = NEAR_PLANE_DEFAULT;
		GetGame().GetCallqueue().CallLater(DeactivateHUD, m_iHudDeactivationDelay, false);

		m_bZoomed = false;
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnSightADSPostFrame(IEntity owner, float timeSlice)
	{
		float interp = Math.Min(1, timeSlice * m_fReticleOffsetInterpSpeed);
		// Interpolate zeroing values
		float reticleTarget = GetReticleYOffsetTarget();
		m_fCurrentReticleOffsetY = Math.Lerp(m_fCurrentReticleOffsetY, reticleTarget, interp);
		float pitchTarget = GetCameraPitchTarget();
		m_fCurrentCameraPitch = Math.Lerp(m_fCurrentCameraPitch, pitchTarget, interp);

		Tick(timeSlice);

#ifdef ENABLE_DIAG
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_WEAPONS_OPTICS))
			DebugOptics();
#endif
	}

	//------------------------------------------------------------------------------------------------
	//! Open optics hud with animation delay
	protected void ActivateHUD()
	{
		// Activate and setup hud
		m_wRootWidget.SetVisible(true);
		m_wImgCover.SetVisible(false);
		m_wRootWidget.SetOpacity(0);

		// Blur
		m_wBlur.SetIntensity(OPACITY_INITIAL);
		m_wBlur.SetOpacity(1);

		// Near plane
		m_fNearPlaneCurrent = NEAR_PLANE_ZOOMED;

		GetGame().GetCallqueue().CallLater(PlayEntryAnimation, m_iAnimationEnterDelay, false, m_fAnimationEnter);

		// Hide rendering
		if (m_bShouldHideParentObject)
		{
			IEntity owner = GetOwner();
			if (owner)
				owner.ClearFlags(EntityFlags.VISIBLE, false);

			// Set flag as otherwise we can make visible something that we didn't make invisible
			m_bWasEntityHidden = true;
		}

		// Hide parent character
		if (m_bShouldHideParentCharacter)
		{
			if (m_ParentCharacter)
				m_ParentCharacter.ClearFlags(EntityFlags.VISIBLE, false);

			// Set flag as otherwise we can make visible something that we didn't make invisible
			m_bWasEntityHidden = true;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Open optics hud with animation delay
	protected void DeactivateHUD()
	{
		if (m_wRootWidget)
			m_wRootWidget.SetVisible(false);
	}

	//------------------------------------------------------------------------------------------------
	//! Start optics entring animation
	protected void PlayEntryAnimation(float animationSpeed)
	{
		m_wRootWidget.SetOpacity(1);
	}

	//------------------------------------------------------------------------------------------------
	//! Find widget refences
	protected void FindWidgets()
	{
		// Parts
		m_wLayoutRear = m_wRootWidget.FindAnyWidget(WIDGET_LAYOUT_REAR);
		m_wLayoutFront = m_wRootWidget.FindAnyWidget(WIDGET_LAYOUT_FRONT);
		m_wImgCover = ImageWidget.Cast(m_wRootWidget.FindAnyWidget(WIDGET_IMAGE_COVER));
		m_wImgScratches = ImageWidget.Cast(m_wRootWidget.FindAnyWidget(WIDGET_IMAGE_SCRATCHES));

		if (!m_wLayoutRear || !m_wLayoutFront)
		{
			Print("Scope vignette movement is not possible due to missing widget references!", LogLevel.WARNING);
			return;
		}

		// Rear part - eye piece
		m_wSizeLayoutRear = SizeLayoutWidget.Cast(m_wLayoutRear.FindAnyWidget(WIDGET_SIZE_REAR));
		m_wRearPaddingLeft = m_wLayoutRear.FindAnyWidget(WIDGET_IMAGE_PADDING_LEFT);
		m_wRearPaddingRight = m_wLayoutRear.FindAnyWidget(WIDGET_IMAGE_PADDING_RIGHT);
		m_wRearPaddingTop = m_wLayoutRear.FindAnyWidget(WIDGET_IMAGE_PADDING_TOP);
		m_wRearPaddingBottom = m_wLayoutRear.FindAnyWidget(WIDGET_IMAGE_PADDING_BOTTOM);

		// Front part - objective
		m_wSizeLayoutObjective = SizeLayoutWidget.Cast(m_wLayoutFront.FindAnyWidget(WIDGET_SIZE_OBJECTIVE));
		m_wFrontPaddingLeft = m_wLayoutFront.FindAnyWidget(WIDGET_IMAGE_PADDING_LEFT);
		m_wFrontPaddingRight = m_wLayoutFront.FindAnyWidget(WIDGET_IMAGE_PADDING_RIGHT);
		m_wFrontPaddingTop = m_wLayoutFront.FindAnyWidget(WIDGET_IMAGE_PADDING_TOP);
		m_wFrontPaddingBottom = m_wLayoutFront.FindAnyWidget(WIDGET_IMAGE_PADDING_BOTTOM);

		m_wOverlayReticles = m_wLayoutFront.FindAnyWidget(WIDGET_OVERLAY_RETICLES);
		m_wImgReticle = ImageWidget.Cast(m_wLayoutFront.FindAnyWidget(WIDGET_IMAGE_RETICLE));
		m_wImgReticleGlow = ImageWidget.Cast(m_wLayoutFront.FindAnyWidget(WIDGET_IMAGE_RETICLE_GLOW));
		m_wBlur = BlurWidget.Cast(m_wRootWidget.FindAnyWidget(WIDGET_BLUR));
		m_wImgFilter = ImageWidget.Cast(m_wLayoutFront.FindAnyWidget(WIDGET_IMAGE_FILTER));
	}

	//------------------------------------------------------------------------------------------------
	//! Setup images and right texture sizing
	protected void SetupOpticImage()
	{
		// Setup reticle
		if (!m_sReticleTexture.IsEmpty())
			m_wImgReticle.LoadImageTexture(0, m_sReticleTexture);

		m_wImgReticleGlow.SetVisible(!m_sReticleTexture.IsEmpty());

		if (!m_sReticleGlowTexture.IsEmpty())
			m_wImgReticleGlow.LoadImageTexture(0, m_sReticleGlowTexture);

		m_wImgReticleGlow.SetVisible(!m_sReticleGlowTexture.IsEmpty());

		// Setup lens filter
		if (!m_sFilterTexture.IsEmpty())
			m_wImgFilter.LoadImageTexture(0, m_sFilterTexture);

		m_wImgFilter.SetVisible(!m_sFilterTexture.IsEmpty());

		// Compute reticle base FOV once
		if (m_fReticleBaseZoom > 0)
			m_fReticleBaseZoom = -CalculateZoomFOV(m_fReticleBaseZoom);

		float fovReticle;
		if (m_fReticleBaseZoom == 0)
			fovReticle = m_fFovZoomed;
		else if (m_fReticleBaseZoom < 0)
			fovReticle = -m_fReticleBaseZoom;

		// Account for part that represents the measurable reticle
		float reticleAngularSize;
		if (m_fReticlePortion > 0)
			reticleAngularSize = m_fReticleAngularSize / m_fReticlePortion;

		float reticleSize;
		if (fovReticle > 0)
			reticleSize = reticleAngularSize / fovReticle;
		else if (m_fFovZoomed > 0)
			reticleSize = reticleAngularSize / m_fFovZoomed;

		// Save basic data
		m_fDefaultSize = reticleSize;
		m_fCurrentReticleSize = m_fDefaultSize;

		UpdateScale(1, 1, 1);
	}

	//------------------------------------------------------------------------------------------------
	// Update widget sizes according to the screen height and DPI scale
	protected void UpdateScale(float reticleScale, float vignetteScale, float objectiveScale)
	{
		float uiScale = 1;
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (workspace)
			uiScale = workspace.DPIUnscale(workspace.GetHeight());

		// Reticle size setup
		if (m_wOverlayReticles)
		{
			float reticleSize = m_fCurrentReticleSize * uiScale * reticleScale;
			FrameSlot.SetSize(m_wOverlayReticles, reticleSize, reticleSize);
		}

		// Vignette size setup
		if (m_wSizeLayoutRear)
		{
			float vignetteSize = m_fObjectiveFov * m_fVignetteScale * uiScale * vignetteScale / m_fFovZoomed;

			m_wSizeLayoutRear.SetWidthOverride(vignetteSize);
			m_wSizeLayoutRear.SetHeightOverride(vignetteSize);
		}

		// Ocular size setup
		if (m_wSizeLayoutObjective)
		{
			float objectiveSize = m_fObjectiveFov * m_fObjectiveScale * uiScale * objectiveScale / m_fFovZoomed;

			m_wSizeLayoutObjective.SetWidthOverride(objectiveSize);
			m_wSizeLayoutObjective.SetHeightOverride(objectiveSize);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Return camera FOV for given magnification. Focus FOV is used as reference 1x magnification
	static float CalculateZoomFOV(float magnification)
	{
		CameraManager cameraManager = GetGame().GetCameraManager();
		if (!cameraManager)
			return 74;

		float referenceFOV;
		PlayerCamera camera = PlayerCamera.Cast(cameraManager.CurrentCamera());
		if (camera)
			referenceFOV = camera.GetFocusFOV();
		else
			referenceFOV = cameraManager.GetFirstPersonFOV();

		float referenceTan = Math.Tan(Math.DEG2RAD * (referenceFOV * 0.5));
		return Math.RAD2DEG * 2 * Math.Atan2(referenceTan, magnification);
	}

	//------------------------------------------------------------------------------------------------
	//! Activate and deactivate optics HUD
	void SetOpticsActive(bool activate)
	{
		if (activate)
		{
			OnSightADSActivated();
		}
		else
		{
			OnSightADSDeactivated();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Return true if move X a Y is below limit
	bool IsMoveCloseToLimit(vector move, float limitMin)
	{
		float x = move[0];
		float y = move[1];

		return (float.AlmostEqual(x, 0, limitMin) && float.AlmostEqual(y, 0, limitMin));
	}

	//------------------------------------------------------------------------------------------------
	/*! Get sight angles in camera space and vertical FOV of the camera for further use
	\param fov field of view of main camera
	\param sightM sight orientation in camera space
	\return Math3D.MatrixToAngles(cameraMat)
	*/
	vector GetMisalignment(out vector sightMat[4], out float fov)
	{
		CameraManager cameraManager = GetGame().GetCameraManager();
		if (!cameraManager)
			return vector.Zero;

		CameraBase camera = cameraManager.CurrentCamera();
		if (!camera)
			return vector.Zero;

		// Get FOV since we are already having camera
		fov = camera.GetVerticalFOV();

		// Get sight orientation
		// Turrets require local sight transform for reliability
		vector sightMat[4];
		GetSightsTransform(sightMat, true);

		// Add optic orientation
		vector opticMat[4];
		GetCameraLocalTransform(opticMat);
		Math3D.MatrixMultiply3(opticMat, sightMat, sightMat);

		// Convert to world orientation
		vector ownerMat[3];
		GetOwner().GetWorldTransform(ownerMat);
		Math3D.MatrixMultiply3(ownerMat, sightMat, sightMat);

		vector cameraMat[3];
		camera.GetWorldTransform(cameraMat);
		Math3D.MatrixInvMultiply3(cameraMat, sightMat, cameraMat);

		//! Fix for PGO-7, however it breaks other types of optics.
		//! Different solution is needed, because this does not handle leaning gracefully.
		if (m_eZeroingType == SCR_EPIPZeroingType.EPZ_NONE)
			Math3D.MatrixInvMultiply3(opticMat, cameraMat, cameraMat);

		vector angles = Math3D.MatrixToAngles(cameraMat);

		return angles;
	}

	//------------------------------------------------------------------------------------------------
	//! Apply result movement to scope widgets
	protected void MoveScopeWidgets(float timeSlice)
	{
		// Update vignette and reticle direction and roll
		vector sightMat[4];
		float fov;
		vector misalignment = GetMisalignment(sightMat, fov);

		// Stabilize roll
		float roll = misalignment[2] * m_fRollScale;
		if (m_fRollDampingSpeed > 0)
		{
			float rollSmooth = Math.Min(1, timeSlice * m_fRollDampingSpeed);
			m_fRollOffset = Math.Lerp(m_fRollOffset, roll, rollSmooth);
			roll -= m_fRollOffset;
		}

		misalignment[0] = fixAngle_180_180(misalignment[0]);
		misalignment[1] = fixAngle_180_180(misalignment[1]);

		// Stabilize binocular reticle over time
		if (m_fMisalignmentDampingSpeed > 0)
		{
			float offsetDamping = Math.Min(timeSlice * m_fMisalignmentDampingSpeed, 1);
			m_vMisalignmentOffset = vector.Lerp(m_vMisalignmentOffset, misalignment, offsetDamping);
			misalignment = misalignment - m_vMisalignmentOffset;
		}

		// Stabilize rotation speed over time
		vector rotation = GetRotation(m_vRotation, timeSlice);
		if (m_fRotationDampingSpeed > 0)
		{
			float rotationDamping = Math.Min(timeSlice * m_fRotationDampingSpeed, 1);
			m_vRotationOffset = vector.Lerp(m_vRotationOffset, rotation, rotationDamping);
			rotation = rotation - m_vRotationOffset;
		}

		// Movement is also stabilizable
		vector movement = GetMovement(m_vMovement, timeSlice);
		if (m_fMovementDampingSpeed > 0)
		{
			float movementDamping = Math.Min(timeSlice * m_fMovementDampingSpeed, 1);
			m_vMovementOffset = vector.Lerp(m_vMovementOffset, movement, movementDamping);
			movement = movement - m_vMovementOffset;
		}

		// Make sure lerp doesn't go out of bounds
		float vignetteMove = Math.Min(timeSlice * m_fVignetteMoveSpeed, 1);

		m_vVignetteOffset = vector.Lerp(m_vVignetteOffset, m_fMisalignmentScale * misalignment, vignetteMove);

		// Objective must be set instantly
		m_vObjectiveOffset = m_fMisalignmentScale * misalignment;

		if (m_fRotationScale > 0)
			m_vVignetteOffset = m_vVignetteOffset - rotation * m_fRotationScale;

		if (m_fMovementScale > 0)
			m_vVignetteOffset = m_vVignetteOffset + movement * m_fMovementScale;

		WorkspaceWidget workspace = GetGame().GetWorkspace();

		float screenW, screenH;
		workspace.GetScreenSize(screenW, screenH);

		float rearW, rearH;
		m_wLayoutRear.GetScreenSize(rearW, rearH);
		rearW *= 0.5;
		rearH *= 0.5;

		float frontW, frontH;
		m_wLayoutFront.GetScreenSize(frontW, frontH);
		frontW *= 0.5;
		frontH *= 0.5;

		float pixelsPerDegree = screenH / fov;

		// Vignette movement
		float rearPaddingLeft = frontW + pixelsPerDegree * m_vVignetteOffset[0];
		float rearPaddingRight = frontW - pixelsPerDegree * m_vVignetteOffset[0];
		float rearPaddingTop = frontH - pixelsPerDegree * m_vVignetteOffset[1];
		float rearPaddingBottom = frontH + pixelsPerDegree * m_vVignetteOffset[1];

		HorizontalLayoutSlot.SetFillWeight(m_wRearPaddingLeft, rearPaddingLeft);
		HorizontalLayoutSlot.SetFillWeight(m_wRearPaddingRight, rearPaddingRight);
		VerticalLayoutSlot.SetFillWeight(m_wRearPaddingTop, rearPaddingTop);
		VerticalLayoutSlot.SetFillWeight(m_wRearPaddingBottom, rearPaddingBottom);

		// Objective and reticle movement
		float frontPaddingLeft = frontW + pixelsPerDegree * m_vObjectiveOffset[0];
		float frontPaddingRight = frontW - pixelsPerDegree * m_vObjectiveOffset[0];
		float frontPaddingTop = frontH - pixelsPerDegree * m_vObjectiveOffset[1];
		float frontPaddingBottom = frontH + pixelsPerDegree * m_vObjectiveOffset[1];

		HorizontalLayoutSlot.SetFillWeight(m_wFrontPaddingLeft, frontPaddingLeft);
		HorizontalLayoutSlot.SetFillWeight(m_wFrontPaddingRight, frontPaddingRight);
		VerticalLayoutSlot.SetFillWeight(m_wFrontPaddingTop, frontPaddingTop);
		VerticalLayoutSlot.SetFillWeight(m_wFrontPaddingBottom, frontPaddingBottom);

		// Reticle rotation
		if (m_wImgReticle)
			m_wImgReticle.SetRotation(roll);

		if (m_wImgReticleGlow)
			m_wImgReticleGlow.SetRotation(roll);

		if (m_wImgFilter)
			m_wImgFilter.SetRotation(roll);

		if (m_wImgCover)
			m_wImgCover.SetRotation(roll);

		if (m_wImgScratches)
			m_wImgScratches.SetRotation(roll + m_fScratchesRoll);

		// Motion offset blur
		if (m_bInitialBlurOver)
			MotionBlur(m_vVignetteOffset[0], m_vVignetteOffset[1]);
	}

	//------------------------------------------------------------------------------------------------
	protected void MotionBlur(float posX, float posY)
	{
		if (!m_wBlur)
			return;

		// Check movement
		if (!m_bIsMoving || !m_bIsRotating)
			return;

		// Pick axis
		float intensity = Math.AbsFloat(posX);
		if (intensity > Math.AbsFloat(posY))
			intensity = Math.AbsFloat(posY);

		// Scale and limit
		intensity = intensity * m_fMotionBlurScale;
		if (intensity > m_fMotionBlurMax)
			intensity = m_fMotionBlurMax;

		m_wBlur.SetIntensity(intensity);
	}

	//------------------------------------------------------------------------------------------------
	protected vector GetRotation(out vector previousDir, float timeSlice)
	{
		if (timeSlice <= 0)
			return vector.Zero;

		CameraManager cameraManager = GetGame().GetCameraManager();
		if (!cameraManager)
			return vector.Zero;

		CameraBase camera = cameraManager.CurrentCamera();
		if (!camera)
			return vector.Zero;

		vector currentDir = camera.GetWorldTransformAxis(2);

		bool rotated = !float.AlmostEqual(previousDir[0], currentDir[0]);
		rotated |= !float.AlmostEqual(previousDir[1], currentDir[1]);

		if (rotated)
		{
			m_bIsRotating = true;
			vector rotation = (currentDir - previousDir) / timeSlice;
			previousDir = currentDir;
			return camera.VectorToLocal(rotation);
		}

		m_bIsRotating = false;

		return vector.Zero;
	}

	//------------------------------------------------------------------------------------------------
	protected vector GetMovement(out vector previousPos, float timeSlice)
	{
		if (timeSlice <= 0)
			return vector.Zero;

		CameraManager cameraManager = GetGame().GetCameraManager();
		if (!cameraManager)
			return vector.Zero;

		CameraBase camera = cameraManager.CurrentCamera();
		if (!camera)
			return vector.Zero;

		vector currentPos = camera.GetWorldTransformAxis(3);

		bool moved = !float.AlmostEqual(previousPos[0], currentPos[0]);
		moved &= !float.AlmostEqual(previousPos[1], currentPos[1]);
		moved &= !float.AlmostEqual(previousPos[2], currentPos[2]);

		if (moved)
		{
			m_bIsMoving = true;
			vector movement = (currentPos - previousPos) / timeSlice;
			previousPos = currentPos;
			return camera.VectorToLocal(movement);
		}

		m_bIsMoving = false;
		return vector.Zero;
	}

	//------------------------------------------------------------------------------------------------
	protected vector GetSightsRelPosition()
	{
		IEntity owner = GetOwner();
		if (m_ParentCharacter && owner)
			return m_ParentCharacter.CoordToLocal(owner.GetOrigin());

		return vector.Zero;
	}

	//------------------------------------------------------------------------------------------------
	void Tick(float timeSlice)
	{
		if (!m_bZoomed)
			return;

		// Widget check
		if (!m_wLayoutRear || !m_wLayoutFront)
			return;

		MoveScopeWidgets(timeSlice);
		LowerBlurIntensity(timeSlice, m_fAnimationSpeedBlur);

#ifdef ENABLE_DIAG
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_WEAPONS_OPTICS))
			DebugOptics();
#endif
	}

	//------------------------------------------------------------------------------------------------
	//! Bring down lens blur intensity in steps
	protected void LowerBlurIntensity(float timeSlice, float speed)
	{
		if (!m_wBlur)
			return;

		float intensity = m_wBlur.GetIntensity();
		intensity = Math.Lerp(intensity, 0, timeSlice * speed);

		// Clear initial bluer
		if (intensity < 0.1 && !m_bInitialBlurOver)
		{
			m_bInitialBlurOver = true;
			intensity = 0;
		}

		m_wBlur.SetIntensity(intensity);
	}

	//------------------------------------------------------------------------------------------------
	bool HasIllumination()
	{
		return m_bHasIllumination;
	}

	//------------------------------------------------------------------------------------------------
	Color GetReticleTextureIllumination()
	{
		return m_cReticleTextureIllumination;
	}

	//------------------------------------------------------------------------------------------------
	//! Change reticle texture position with angular offset
	void SetReticleOffset(float x, float y)
	{
		if (!m_wOverlayReticles)
			return;

		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace)
			return;

		float fov = m_fFovZoomed;
		CameraManager cameraManager = GetGame().GetCameraManager();
		if (cameraManager)
		{
			CameraBase camera = cameraManager.CurrentCamera();
			if (camera)
				fov = camera.GetVerticalFOV();
		}

		float uiHeight = workspace.DPIUnscale(workspace.GetHeight());
		float offsetX = x * uiHeight / fov;
		float offsetY = y * uiHeight / fov;

		FrameSlot.SetPos(m_wOverlayReticles, offsetX, offsetY);

		vector size = FrameSlot.GetSize(m_wOverlayReticles);
		if (m_wImgReticle)
		{
			float pivotX = 0.5 - (offsetX / size[0]);
			float pivotY = 0.5 - (offsetY / size[1]);
			m_wImgReticle.SetPivot(pivotX, pivotY);
		}

		if (m_wImgReticleGlow)
		{
			float pivotX = 0.5 - (offsetX / size[0]);
			float pivotY = 0.5 - (offsetY / size[1]);
			m_wImgReticleGlow.SetPivot(pivotX, pivotY);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected float GetReticleYOffsetTarget()
	{
		return 0.0;
	}

	//------------------------------------------------------------------------------------------------
	float GetCurrentReticleYOffset()
	{
		return m_fReticleOffsetY;
	}

	//------------------------------------------------------------------------------------------------
	protected float GetCameraPitchTarget()
	{
		if (m_eZeroingType == SCR_EPIPZeroingType.EPZ_CAMERA_TURN)
			return -GetCurrentSightsRange()[0];

		return 0.0;
	}

	float GetCurrentCameraPitchOffset()
	{
		return m_fCurrentCameraPitch;
	}

	//------------------------------------------------------------------------------------------------
	vector GetCameraOffset()
	{
		return m_vCameraOffset;
	}

	//------------------------------------------------------------------------------------------------
	vector GetCameraAngles()
	{
		return m_vCameraAngles;
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Return actual parent (weapon) with calculated transformation in relation to it.
		\param matrix Out rotation in relation to new parent
		\return Returns new parent to use, usually the weapon
	*/
	IEntity GetCameraLocalTransform(out vector matrix[4])
	{
		// Apply offset and angles as configured against front sight position
		vector position = m_vCameraOffset + GetSightsFrontPosition(true);
		vector yawPitchRoll = Vector(m_vCameraAngles[1], m_vCameraAngles[0] + m_fCurrentCameraPitch, m_vCameraAngles[2]);

		// Construct the local matrix in relation to original parent
		Math3D.AnglesToMatrix(yawPitchRoll, matrix);
		matrix[3] = position;

		// Now additionally we will browse the hierarchy until we hit our weapon
		vector temp[4];
		Math3D.MatrixCopy(matrix, temp);

		IEntity parent = GetOwner();
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
			if (pivotIndex != -1)
			{
				// Multiply pivot * local TM = model TM
				IEntity parentNode = lastNode.GetParent();
				if (parentNode)
				{
					vector pivotTM[4];
					parentNode.GetAnimation().GetBoneMatrix(pivotIndex, pivotTM);

					// This should not be happening - there should
					// rather be no pivot, yet it triggers at times.
					// ??? TODO@AS: See if we can have better API for pivoting like this
					if (!SCR_Math3D.IsMatrixEmpty(pivotTM))
						Math3D.MatrixMultiply4(pivotTM, localTransform, localTransform);
				}
			}
			Math3D.MatrixMultiply4(temp, localTransform, temp);
			lastNode = lastNode.GetParent();
		}

		// Only use calculated matrix+parent if its valid
		if (weaponRoot)
		{
			Math3D.MatrixCopy(temp, matrix);
			parent = lastNode;
		}

		return parent;
	}

#ifdef ENABLE_DIAG
	protected static bool s_bOpticDiagRegistered;
	protected float m_fDebugAngle = 6;
	protected float m_fDebugUnit = 1;

	//------------------------------------------------------------------------------------------------
	//! Constructor
	void SCR_2DOpticsComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		if (!s_bOpticDiagRegistered)
		{
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_WEAPONS_OPTICS, "", "Show optics diag", "Weapons");
			s_bOpticDiagRegistered = true;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void InputFloatClamped(inout float value, string label, float min, float max, int pxWidth = 100)
	{
		DbgUI.InputFloat(label, value, pxWidth);
		value = Math.Clamp(value, min, max);
	}

	//------------------------------------------------------------------------------------------------
	protected void DebugOptics()
	{
		DbgUI.Begin("Optics");
		{
			InputFloatClamped(m_fDebugUnit, "m_fDebugUnit", -100, 100);
			InputFloatClamped(m_fDebugAngle, "m_fDebugAngle", -1000, 1000);
			InputFloatClamped(m_fReticleAngularSize, "m_fReticleAngularSize", 0, 100);
			InputFloatClamped(m_fReticlePortion, "m_fReticlePortion", 0, 100);
			InputFloatClamped(m_fObjectiveFov, "m_fObjectiveFov", 0, 100);
			InputFloatClamped(m_fObjectiveScale, "m_fObjectiveScale", -100, 100);
			InputFloatClamped(m_fVignetteScale, "m_fVignetteScale", -100, 100.0);
			InputFloatClamped(m_fVignetteMoveSpeed, "m_fVignetteMoveSpeed", -100, 100.0);
			InputFloatClamped(m_fMisalignmentDampingSpeed, "m_fMisalignmentDampingSpeed", -100, 100);
			InputFloatClamped(m_fRotationScale, "m_fRotationScale", -100, 100);
			InputFloatClamped(m_fRotationDampingSpeed, "m_fRotationDampingSpeed", -100, 100);
			InputFloatClamped(m_fMovementScale, "m_fMovementScale", -100, 100);
			InputFloatClamped(m_fMovementDampingSpeed, "m_fMovementDampingSpeed", -100, 100);
			InputFloatClamped(m_fRollScale, "m_fRollScale", -100, 100);
			InputFloatClamped(m_fRollDampingSpeed, "m_fRollDampingSpeed", -100, 100);
			InputFloatClamped(m_fMisalignmentScale, "m_fMisalignmentScale", -100, 100);
		}
		DbgUI.End();

		SCR_2DPIPSightsComponent pip = SCR_2DPIPSightsComponent.Cast(this);
		if (!pip || !pip.IsPIPActive())
			SetupOpticImage();

		float targetSize = 0.5 * Math.Tan(Math.DEG2RAD * m_fDebugAngle * m_fDebugUnit);
		float range = 1000;

		vector sightMat[4];
		GetSightsTransform(sightMat, true);
		vector ownerMat[4];
		GetOwner().GetWorldTransform(ownerMat);
		Math3D.MatrixMultiply4(ownerMat, sightMat, sightMat);

		vector opticSide = sightMat[0];
		vector opticUp = sightMat[1];
		vector opticDir = sightMat[2];
		vector opticPos = sightMat[3];

		vector targetPos = opticPos + opticDir * range;
		vector targetSide = opticSide * targetSize * range;
		vector targetUp = opticUp * targetSize * range;

		vector target[8];
		target[0] = targetPos - targetSide + targetUp;
		target[1] = targetPos + targetSide + targetUp;
		target[2] = targetPos + targetSide - targetUp;
		target[3] = targetPos - targetSide - targetUp;
		target[4] = targetPos - targetSide + targetUp;
		target[5] = targetPos + targetSide - targetUp;
		target[6] = targetPos + targetSide + targetUp;
		target[7] = targetPos - targetSide - targetUp;

		vector target2[6];
		target2[0] = targetPos + targetSide;
		target2[1] = targetPos - targetSide;
		target2[2] = targetPos - targetUp;
		target2[3] = targetPos + targetSide;
		target2[4] = targetPos + targetUp;
		target2[5] = targetPos - targetSide;

		Shape.CreateLines(COLOR_RED, ShapeFlags.ONCE | ShapeFlags.NOZBUFFER, target, 8);
		Shape.CreateLines(COLOR_RED, ShapeFlags.ONCE | ShapeFlags.NOZBUFFER, target2, 6);
		Shape.CreateSphere(COLOR_YELLOW_A, ShapeFlags.ONCE | ShapeFlags.NOZBUFFER | ShapeFlags.TRANSP | ShapeFlags.NOOUTLINE | ShapeFlags.NOZWRITE, targetPos, targetSize * range);
	}
#endif
};
