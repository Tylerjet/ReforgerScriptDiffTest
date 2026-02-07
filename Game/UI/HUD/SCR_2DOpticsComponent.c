[EntityEditorProps(category: "GameScripted/Weapon/Sights", description: "", color: "0 0 255 255")]
class SCR_2DOpticsComponentClass: ScriptedSightsComponentClass
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
		
	const float m_fReferenceFOV = 38;
	const float OPACITY_INITIAL = 0.75; 
	
	const float NEAR_PLANE_DEFAULT = 0.05;
	const float NEAR_PLANE_ZOOMED = 0.05;
	
	// Optics setup
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Layout used for 2D sights HUD", params: "layout", category: "Resources")]
	protected ResourceName m_sLayoutResource;
	
	[Attribute("", UIWidgets.ResourceNamePicker, desc: "Texture of reticle", params: "edds imageset", category: "Resources")]
	protected ResourceName m_sReticleTexture;	
	
	[Attribute("", UIWidgets.ResourceNamePicker, desc: "Texture of reticle under glow", params: "edds imageset", category: "Resources")]
	protected ResourceName m_sReticleGlowTexture;
	
	[Attribute("0", category: "Resources")]
	protected bool m_bHasIllumination;
	
	[Attribute("0 0 0", UIWidgets.ColorPicker, category: "Resources")]
	protected ref Color m_cReticleTextureIllumination;
	
	[Attribute("", UIWidgets.ResourceNamePicker, desc: "Texture of lens filter", params: "edds imageset", category: "Resources")]
	protected ResourceName m_sFilterTexture;
	
	[Attribute("10", UIWidgets.EditBox, desc: "Optic magnification. Increase of magnification reduces optic field of view proportionally", params: "0.1 50", category: "2DSights")]
	protected float m_fMagnification;
	
	[Attribute("1", UIWidgets.EditBox, desc: "Real reticle texture size in px within transparent background", params: "1 2000", category: "2DSights")]
	protected int m_iReticleTextureWidth;
	
	[Attribute("100", UIWidgets.EditBox, desc: "Expected reticle range when looking at 1000m", params: "0.1 1000", category: "2DSights")]
	protected float m_fReticleWidthRange;
	
	[Attribute("1", UIWidgets.EditBox, desc: "Manual scale manipulation mainly for improving visuals, better to keep in 1.0 or close to this number", params: "0.1 100", category: "2DSights")]
	protected float m_fVignetteTextureMultiplier;
	
	[Attribute("1", UIWidgets.CheckBox, "Should hide parent object when using 2D", category: "2DSights")]
	bool m_bShouldHideParentObject;
	
	// Behavior setup 
	[Attribute("1", UIWidgets.EditBox, desc: "", params: "1 2000", category: "Behavior")]
	protected bool m_bAllowRotation;

	// Animation attributes 
	[Attribute("0", UIWidgets.Slider, "Time before entering animation starts",  params: "0 2000 1", category: "Animations")]
	protected int m_iHudActivationDelay;
	
	[Attribute("100", UIWidgets.Slider, "Animation time for canceling optic.",  params: "0 2000 1", category: "Animations")]
	protected int m_iHudDeactivationDelay;
	
	[Attribute("5", UIWidgets.Slider, "Time before entering animation starts",  params: "0 2000 1", category: "Animations")]
	protected int m_iAnimationEnterDelay;
	
	[Attribute("0.5", UIWidgets.Slider, "Animation time for using optic",  params: "0.01 5 0.01", category: "Animations")]
	protected float m_fAnimationEnter;
	
	[Attribute("2", UIWidgets.Slider, "Speed of blur fade out on optic entering",  params: "0.01 5 0.01", category: "Animations")]
	protected float m_fAnimationSpeedBlur;
	
	// Effect attributes
	[Attribute("12", UIWidgets.EditBox, desc: "Optic misalignment scale in horizontal axis", params: "0 1000 0.1", category: "Effects")]
	protected float m_fOpticMisalignmentScaleHorizontal;
	
	[Attribute("12", UIWidgets.EditBox, desc: "Optic misalignment scale in horizontal axis", params: "0 1000 0.1", category: "Effects")]
	protected float m_fOpticMisalignmentScaleVertical;
	
	[Attribute("1", UIWidgets.EditBox, desc: "Vignette adjustment speed - the faster, the less inertia there is", params: "0 60 0.1", category: "Effects")]
	protected float m_fVignetteMoveSpeed;
	
	[Attribute("20", UIWidgets.EditBox, desc: "Objective adjustment speed - the faster, the less inertia there is", params: "0 60 0.01", category: "Effects")]
	protected float m_fObjectiveMoveSpeed;
	
	[Attribute("1", UIWidgets.EditBox, desc: "Multiplification of rotation", params: "1 50", category: "Effects")]
	protected float m_fReticleRotationScale;
	
	[Attribute("25", UIWidgets.EditBox, desc: "Multiplification of motion blur", params: "0 100", category: "Effects")]
	protected float m_fMotionBlurScale;
	
	[Attribute("0.2", UIWidgets.EditBox, desc: "Max level of motion blur intensity", params: "0 1 0.01", category: "Effects")]
	protected float m_fMotionBlurMax;
	
	// 0 = immediately when entering ADS camera, 1.0 = only after full blend
	[Attribute("1.0", UIWidgets.Slider, "Percentage of camera transition at which sights activate.",  params: "0.0 1.0 0.01", category: "Sights")]
	protected float m_fADSActivationPercentage;
	
	// 0 = immediately when leaving ADS camera, 1.0 = only after full blend
	[Attribute("0.0", UIWidgets.Slider, "Percentage of camera transition at which sights deactivate.",  params: "0.0 1.0 0.01", category: "Sights")]
	protected float m_fADSDeactivationPercentage;
	
	[Attribute("0", UIWidgets.ComboBox, "Type of zeroing for this sights.", "", ParamEnumArray.FromEnum(SCR_EPIPZeroingType), category: "Sights" )]
	protected SCR_EPIPZeroingType m_eZeroingType;
	
	[Attribute("0.0", UIWidgets.Slider, "Reticle Offset of scope center in X", params: "-1 1 0.001", category: "Sights")]
	protected float m_fReticleOffsetX;
	
	[Attribute("0.0", UIWidgets.Slider, "Reticle Offset of scope center in Y", params: "-1 1 0.001", category: "Sights")]
	protected float m_fReticleOffsetY;
	
	[Attribute("25.0", UIWidgets.Slider, "Interpolation speed for zeroing interpolation", params: "1 100.0 0.1", category: "Sights")]
	protected float m_fReticleOffsetInterpSpeed;
	
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
	
	// Scope widgets 
	protected Widget m_wRootWidget;
	
	protected Widget m_wLayoutRear;
	protected Widget m_wLayoutFront;
	protected ImageWidget m_wImgCover;
	
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
	
	protected float m_fScopeLayoutSizeRatio;
	
	protected ref array<float> m_aReticleSizes = new ref array<float>;
	protected int m_iSelectedZoomLevel = 0;
	
	// Overall movement 
	protected vector m_vObjectiveOffset; // Objective and reticle offset
	protected vector m_vVignetteOffset; // Vignette offset
	
	// Rotation movement 
	[Attribute("0.001", UIWidgets.EditBox, "Lower limit for movement rotation effect", params: "0.001 0.5 0.0001", category: "Effects")]
	protected float m_fMoveRotationMin;
	
	[Attribute("5", UIWidgets.EditBox, "Upper limit for movement rotation effect", params: "0.001 0.5 0.0001", category: "Effects")]
	protected float m_fMoveRotationMax;
	
	[Attribute("1", UIWidgets.EditBox, "Scale of movement rotation effect", params: "0.05 5 0.01", category: "Effects")]
	protected float m_fMoveRotationScale;
	
	protected vector m_vMoveRotationLast;
	protected vector m_vMoveRotation;
	
	protected vector m_vHeadInitial;
	
	protected vector m_vMoveRecoil;
	
	protected float m_fViewAngle = 0;
	
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
	protected IEntity m_Owner = null;	
	protected ChimeraCharacter m_ParentCharacter = null;
	
	static ref ScriptInvoker<bool, m_fFovZoomed> s_OnSightsADSChanged = new ScriptInvoker();
	
	//------------------------------------------------------------------------------------------------
	float GetFovZoomed() { return m_fFovZoomed; }
	
	//------------------------------------------------------------------------------------------------
	float GetNearPlane() { return  m_fNearPlaneCurrent; }
	
	//------------------------------------------------------------------------------------------------
	float GetHudActivationDelay() { return m_iHudActivationDelay; }
	
	//------------------------------------------------------------------------------------------------
	void SetViewAngle(float angle) { m_fViewAngle = angle; }
	
	//------------------------------------------------------------------------------------------------
	protected override void OnSightADSActivated()
	{
		// Setup scope widgets 
		if (!m_wRootWidget)
		{
			m_ParentCharacter = ChimeraCharacter.Cast(m_Owner.GetParent());
			
			SCR_HUDManagerComponent hudManager = SCR_HUDManagerComponent.GetHUDManager();
			if (hudManager)
				m_wRootWidget = hudManager.CreateLayout(m_sLayoutResource, EHudLayers.BACKGROUND);

			if (m_wRootWidget)
			{
				m_fFovZoomed = CalculateZoomFov(m_fReferenceFOV, m_fMagnification);
				
				// HUD and widgets setup
				m_wRootWidget.SetZOrder(-1);
				m_wRootWidget.SetVisible(false);
				
				FindWidgets();
				SetupOpticImage(m_sReticleTexture);
			}
		}
		
		super.OnSightADSActivated();
		s_OnSightsADSChanged.Invoke(true, m_fFovZoomed);
		
		// Prevent calnceling animation
		WidgetAnimator.StopAnimation(m_wRootWidget, WidgetAnimationType.Opacity);
		GetGame().GetCallqueue().Remove(DeactivateHUD);
		
		// Play cover fade in animaiton 
		m_fNearPlaneCurrent = NEAR_PLANE_DEFAULT;
		GetGame().GetCallqueue().CallLater(ActivateHUD, m_iHudActivationDelay, false);
		
		m_bZoomed = true;
		m_bInitialBlurOver = false;
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
			if (m_Owner)
				m_Owner.SetFlags(EntityFlags.VISIBLE, false);
			
			if (m_ParentCharacter)
				m_ParentCharacter.SetFlags(EntityFlags.VISIBLE, false);

			m_bWasEntityHidden = false;
		}
		
		GetGame().GetCallqueue().Remove(ActivateHUD); // in case this is called quickly after OnSightADSActivated, ActivateHUD needs to be cleared to prevent conflict
		
		// Prevent entering animation
		WidgetAnimator.StopAnimation(m_wRootWidget, WidgetAnimationType.Opacity);
		GetGame().GetCallqueue().Remove(PlayEntryAnimation);
		
		if (m_wImgCover)
			m_wImgCover.SetOpacity(0);
		
		// Leaving animation 
		m_fNearPlaneCurrent = NEAR_PLANE_DEFAULT;
		WidgetAnimator.StopAnimation(m_wRootWidget, WidgetAnimationType.Opacity);
		GetGame().GetCallqueue().CallLater(DeactivateHUD, m_iHudDeactivationDelay, false);
		
		m_bZoomed = false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnSightADSPostFrame(IEntity owner, float timeSlice)
	{		
		float interp = timeSlice * m_fReticleOffsetInterpSpeed;
		// Interpolate zeroing values
		float reticleTarget = GetReticleYOffsetTarget();
		m_fCurrentReticleOffsetY = Math.Lerp(m_fCurrentReticleOffsetY, reticleTarget, interp);		
		float pitchTarget = GetCameraPitchTarget();
		m_fCurrentCameraPitch = Math.Lerp(m_fCurrentCameraPitch, pitchTarget, interp);

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
			if (m_Owner)
				m_Owner.ClearFlags(EntityFlags.VISIBLE, false);
			
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
	protected void SetupOpticImage(ResourceName resource)
	{		
		if (resource.IsEmpty())
			return;
		
		// Setup reticle 
		m_wImgReticle.LoadImageTexture(0, resource);
		m_wImgReticleGlow.LoadImageTexture(0, m_sReticleGlowTexture);
		
		// Setup lens filter 
		if (!m_sFilterTexture.IsEmpty())
			m_wImgFilter.LoadImageTexture(0, m_sFilterTexture);
		
		m_wImgFilter.SetVisible(!m_sFilterTexture.IsEmpty());

		// Get local widgets 
		SizeLayoutWidget rearVignetteSize = SizeLayoutWidget.Cast(m_wRootWidget.FindAnyWidget(WIDGET_SIZE_REAR));
		SizeLayoutWidget frontVignetteSize = SizeLayoutWidget.Cast(m_wRootWidget.FindAnyWidget(WIDGET_SIZE_OBJECTIVE));

		// Reticle transparent borders cut of
		int imageXSize, imageYSize = 0;
		if (m_wImgReticle)
			m_wImgReticle.GetImageSize(0, imageXSize, imageYSize);
		
		float reticleTextureOffset = 0;
		if (m_iReticleTextureWidth != 0)
			reticleTextureOffset = (float)imageXSize / (float)m_iReticleTextureWidth;
		
		float yView = Math.Tan(Math.DEG2RAD * m_fFovZoomed / 2) * 1000 * 2; // Camera view height on 1000m
		if (yView == 0)
		{
			Print("yView shouldn't be 0!", LogLevel.ERROR);
			return;
		}
		
		float rRToLayout = m_fReticleWidthRange / yView; // Reticle ratio to layout size 
		
		// Texture sizing
		int w,h = 0;
		WidgetManager.GetReferenceScreenSize(w, h); 
		int r = (int)(rRToLayout * h * reticleTextureOffset);
		
		// Set texture sizes
		FrameSlot.SetSize(m_wOverlayReticles, r, r);
		
		// Save defaults
		m_fDefaultSize = r;
		m_fCurrentReticleSize = m_fDefaultSize;
		
		r = r * m_fVignetteTextureMultiplier;
		
		// Vignette size setup 
		if (rearVignetteSize)
		{
			rearVignetteSize.SetWidthOverride(r);
			rearVignetteSize.SetHeightOverride(r);
		}
		if (frontVignetteSize)
		{
			frontVignetteSize.SetWidthOverride(r);
			frontVignetteSize.SetHeightOverride(r);
		}
		
		// Get scope layout size ratio 
		m_fScopeLayoutSizeRatio = h/w;
	}
	
	//-----------------------------------------------------------------------------
	//! Return real fov from given magnification 
	//! fovBase - player current/original fov, maginicaition - power of zoom
	protected float CalculateZoomFov(float fovBase, float magnification)
	{
		return Math.RAD2DEG * 2 * Math.Atan2(Math.Tan(Math.DEG2RAD * (fovBase / 2)), magnification); 
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
	//! Frame update 
	void UpdateMovement(float timeSlice)
	{
		if (m_bAllowRotation)
			UpdateRotation(m_vMoveRotation, timeSlice);
		
		MoveScopeWidgets(timeSlice);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Update move based on character rotation  
	protected void UpdateRotation(out vector rotationPower, float timeSlice)
	{
		// Add rotation if rotation is greater than current rotation power 
		vector rot = CharacterRotation(m_vMoveRotationLast) * m_fMoveRotationScale;
	
		// Movement check 
		m_bIsMoving = rot != vector.Zero;
		m_bIsRotating = !IsMoveCloseToLimit(rot, m_fMoveRotationMin);
		
		// x  
		if (Math.AbsFloat(rot[0]) > Math.AbsFloat(rotationPower[0]) || rot[0].Sign() != rotationPower[0].Sign() && rot[0] != 0)
			rotationPower[0] = rot[0];
		// y
		if (Math.AbsFloat(rot[1]) > Math.AbsFloat(rotationPower[1]) || rot[1].Sign() != rotationPower[1].Sign() && rot[1] != 0)
			rotationPower[1] = rot[1];
		
		//limit
		rotationPower[0] = AxisLimit(rotationPower[0], m_fMoveRotationMax);
		rotationPower[1] = AxisLimit(rotationPower[1], m_fMoveRotationMax);

		// Decrease power 
		if (!m_bIsRotating)	
			rotationPower = vector.Zero;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get amout of actual character rotation
	protected vector CharacterRotation(out vector lastRotation)
	{
		vector currentRotation;
		vector camMove = CameraDirectionMove(currentRotation);
		bool changedX, changedY = false;
		
		if (currentRotation == lastRotation)
			return vector.Zero;
		
		// Results - difference between current and last move
		vector rotation = currentRotation - lastRotation;
		lastRotation = currentRotation;

		return rotation;
	}
	
	//------------------------------------------------------------------------------------------------
	float AxisLimit(float axis, float limitMax, float limitMin = 0)
	{
		// Max limit 
		if(Math.AbsFloat(axis) > limitMax)
			return limitMax * axis.Sign();
		
		// Min limit 
		if (Math.AbsFloat(axis) < limitMin)
			return 0;
		
		return axis;
	}

	//------------------------------------------------------------------------------------------------
	//! Return true if move X a Y is below limit 
	bool IsMoveCloseToLimit(vector move, float limitMin)
	{
		float x = move[0];
		float y = move[1];
		
		return (float.AlmostEqual(x, 0, limitMin) && float.AlmostEqual(y, 0, limitMin));
	}
	
#ifdef ENABLE_DEBUG
	[Attribute("0", UIWidgets.CheckBox, desc: "Debug 2D Reticles", category: "Debug")]
	static bool m_bDebug2DSights;
	static private ref array<Shape> s_aDebugShapes;
#endif
	
	//------------------------------------------------------------------------------------------------
	vector GetMisalignment()
	{
		CameraBase camera = GetGame().GetCameraManager().CurrentCamera();
		if (!camera)
			return vector.Zero;
		
		vector opticPos = GetSightsRearPosition();
		vector opticDir = GetSightsFrontPosition() - opticPos;
		
#ifdef ENABLE_DEBUG
		if (m_bDebug2DSights)
		{
			if (!s_aDebugShapes)
				s_aDebugShapes = {};
			
			foreach (Shape shape: s_aDebugShapes)
				shape = null;
			
			s_aDebugShapes.Clear();
			
			
			vector aimpoint = opticPos + opticDir * 1000;
			vector vertical[2];
			vertical[0] = aimpoint;
			vertical[1] = aimpoint + opticDir;
			
			s_aDebugShapes.Insert(Shape.CreateLines(COLOR_YELLOW, ShapeFlags.NOOUTLINE|ShapeFlags.NOZBUFFER|ShapeFlags.TRANSP, vertical, 2));
			s_aDebugShapes.Insert(Shape.CreateSphere(COLOR_YELLOW_A, ShapeFlags.NOOUTLINE|ShapeFlags.NOZBUFFER|ShapeFlags.TRANSP, aimpoint, 0.5));
			s_aDebugShapes.Insert(Shape.CreateSphere(COLOR_YELLOW_A, ShapeFlags.NOOUTLINE|ShapeFlags.NOZBUFFER|ShapeFlags.TRANSP, aimpoint, 1));
			s_aDebugShapes.Insert(Shape.CreateSphere(COLOR_YELLOW_A, ShapeFlags.NOOUTLINE|ShapeFlags.NOZBUFFER|ShapeFlags.TRANSP, aimpoint, m_fReticleWidthRange * 0.5));
		}
#endif
		vector misalignment = camera.VectorToLocal(opticDir);
		return misalignment;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Apply result movement to scope widgets 
	protected void MoveScopeWidgets(float timeSlice)
	{
		// Update vignette and reticle direction
		vector misalignment = GetMisalignment() * -m_fMagnification;
		misalignment[0] = misalignment[0] * m_fOpticMisalignmentScaleHorizontal;
		misalignment[1] = misalignment[1] * m_fOpticMisalignmentScaleVertical;
		
		m_vVignetteOffset = vector.Lerp(m_vVignetteOffset, misalignment, timeSlice * m_fVignetteMoveSpeed);
		m_vObjectiveOffset = vector.Lerp(m_vObjectiveOffset, misalignment, timeSlice * m_fObjectiveMoveSpeed);
		
		HorizontalLayoutSlot.SetFillWeight(m_wRearPaddingLeft, 1 - m_vVignetteOffset[0]);
		HorizontalLayoutSlot.SetFillWeight(m_wRearPaddingRight, 1 + m_vVignetteOffset[0]);
		VerticalLayoutSlot.SetFillWeight(m_wRearPaddingTop, 1 + m_vVignetteOffset[1] - m_fCurrentCameraPitch);
		VerticalLayoutSlot.SetFillWeight(m_wRearPaddingBottom, 1 - m_vVignetteOffset[1] - m_fCurrentCameraPitch);
		
		// Apply objective and reticle movement
		HorizontalLayoutSlot.SetFillWeight(m_wFrontPaddingLeft, 1 - m_vObjectiveOffset[0]);
		HorizontalLayoutSlot.SetFillWeight(m_wFrontPaddingRight, 1 + m_vObjectiveOffset[0]);
		VerticalLayoutSlot.SetFillWeight(m_wFrontPaddingTop, 1 + m_vObjectiveOffset[1] - m_fCurrentCameraPitch);
		VerticalLayoutSlot.SetFillWeight(m_wFrontPaddingBottom, 1 - m_vObjectiveOffset[1] - m_fCurrentCameraPitch);
		
		// Reticle rotation 
		if (m_wImgReticle)
		{
			float angle = m_fViewAngle - m_vObjectiveOffset[0] * m_fReticleRotationScale - m_vMoveRotation[0];
			
			if (angle > m_fMoveRotationMin)
				angle -= m_fMoveRotationMin;
			else if (angle < -m_fMoveRotationMin)
				angle += m_fMoveRotationMin;
			else
				angle = 0;
			
			m_wImgReticle.SetRotation(angle);
			m_wImgReticleGlow.SetRotation(angle);
		}
		
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
	protected bool IsMoving(vector move, float limit)
	{
		return (!float.AlmostEqual(move[0], 0, limit) && !float.AlmostEqual(move[1], 0, limit));
	}
	
	//------------------------------------------------------------------------------------------------
	protected vector CameraDirectionMove(out vector previousDir)
	{
		vector camera[4];
		GetGame().GetWorld().GetCurrentCamera(camera);
		vector actualDir = Math3D.MatrixToAngles(camera);
		
		bool hasXMove = !float.AlmostEqual(previousDir[0], actualDir[0], 0.1);
		bool hasYMove = !float.AlmostEqual(previousDir[1], actualDir[1], 0.1);
		
		if(hasXMove || hasYMove)
		{
			vector dif = actualDir - previousDir;
			previousDir = actualDir;
			return dif;
		}
		
		return Vector(0,0,0);
	}
	
	//-----------------------------------------------------------------------------
	//! Checking if next move is longer on inverted
	protected bool IsAxisMoveChanged(float moveNext, float moveCurrent)
	{
		if(moveNext != 0)
		{
			bool longer = Math.AbsFloat(moveNext) > Math.AbsFloat(moveCurrent);
			
			if(moveCurrent != 0)
				return longer || moveNext.Sign() != moveCurrent.Sign();
			else
				return longer;
		}
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected vector GetSightsRelPosition()
	{
		if (m_ParentCharacter)
		{
			return m_ParentCharacter.CoordToLocal(m_Owner.GetOrigin());
		}
		
		return vector.Zero;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnInit(IEntity owner)
	{
		m_Owner = owner;
	}
	
	//------------------------------------------------------------------------------------------------
	void Tick(float timeSlice)
	{
		if (!m_bZoomed)
			return;
		
		// Widget check
		if (!m_wLayoutRear || !m_wLayoutFront)
			return;
		
		UpdateMovement(timeSlice);
		LowerBlurIntensity(timeSlice, m_fAnimationSpeedBlur);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Bring down lens blur intensity in steps 
	protected void LowerBlurIntensity(float timeSlice, float speed)
	{
		if (!m_wBlur)
			return;
		
		float intensity = m_wBlur.GetIntensity();
		intensity = Math.Lerp(intensity, 0, timeSlice * speed);
		
		m_wBlur.SetIntensity(intensity);
		
		// Clear initial bluer 
		if (intensity < 0.1 && !m_bInitialBlurOver)
			m_bInitialBlurOver = true;
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
	//! Change reticle texture position with offest 
	void SetReticleOffset(int x, int y)
	{
		if (!m_wOverlayReticles)
			return;
		
		FrameSlot.SetPos(m_wOverlayReticles, x, y);
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
		{
			return GetCurrentSightsRange()[0];
		}
		
		return 0.0;
	}
	
	float GetCurrentCameraPitchOffset()
	{
		return m_fCurrentCameraPitch;
	}
};