[EntityEditorProps(category: "GameScripted/Weapon/Sights", description: "", color: "0 0 255 255")]
class SCR_2DSightsComponentClass: ScriptedSightsComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_2DSightsComponent : SCR_2DOpticsComponent
{
	
	[Attribute("1", UIWidgets.CheckBox, "Should check for character in parent and hide it when using 2D", category: "2DSights")]
	bool m_bShouldHideParentCharacter;
	
	[Attribute("0", UIWidgets.CheckBox, "Should hide parent of parent object when using 2D", category: "2DSights")]
	bool m_bShouldHideParentParentObject;
	
	[Attribute("0", UIWidgets.CheckBox, "Should reticle be scaled with current zoom", category: "2DSights")]
	protected bool m_bDynamicReticle;
	
	[Attribute("0 0 0", UIWidgets.EditBox, "Camera offset when looking through scope", category: "PiPSights", params: "inf inf 0 purposeCoords spaceEntity")]
	protected vector m_vCameraPoint;
	
	[Attribute("0 0 0", UIWidgets.EditBox, "Camera angles when looking through scope", category: "PiPSights", params: "inf inf 0 purposeAngles spaceEntity")]
	protected vector m_vCameraAngles;
	
	[Attribute("1.4", UIWidgets.Slider, "Scale mult. of scope when target recoil is applied.", category: "2DSights", params: "1 3 0.01")]
	protected float m_fRecoilScaleMax;
	
	[Attribute("0.12", UIWidgets.Slider, "The amount of linear translation of recoil in meters that is deemed as target and applies the most scale, no scale is applied beyond this value.", category: "2DSights", params: "0.001 0.5 0.001")]
	protected float m_fRecoilTranslationTarget;
	
	[Attribute("10.0", UIWidgets.Slider, "Scale mult. of scope movement when target recoil is applied.", category: "2DSights", params: "1 50 0.01")]
	protected float m_fRecoilScaleTranslation;
	
	[Attribute("1", UIWidgets.EditBox, desc: "", params: "1 2000", category: "Behavior")]
	protected bool m_bAllowRecoil;
	
	protected bool m_bIsIlluminationOn = false;

	// sway, taken from binocs
	protected vector m_vLastCameraDir;

	// sway
	protected vector m_vLastPos;

	// recoil	
	protected CharacterAimingComponent m_pCharacterAiming;
	protected TurretControllerComponent m_pTurretController;
	
	// 
	protected SCR_SightsZoomFOVInfo m_SightsFovInfo;
	
	//------------------------------------------------------------------------------------------------
	override protected vector GetSightsRelPosition()
	{
		if (m_ParentCharacter)
		{
			return m_ParentCharacter.CoordToLocal(m_Owner.GetOrigin());
		}
		
		return vector.Zero;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnSightADSActivated()
	{
		super.OnSightADSActivated();
		
		// Set fov 
		if (!m_SightsFovInfo)
		{
			m_SightsFovInfo = SCR_SightsZoomFOVInfo.Cast(GetFOVInfo());
			SetupFovInfo();
		}
		
		SelectZoomLevel(m_iSelectedZoomLevel);
		
		// Set parent 
		if (m_Owner)
		{
			IEntity parent = m_Owner.GetParent();
			if ( m_bShouldHideParentObject )
			{
				m_Owner.ClearFlags(EntityFlags.VISIBLE, false);
				if (parent && m_bShouldHideParentParentObject)
				{
					parent.ClearFlags(EntityFlags.VISIBLE, false);
				}
			}
			
			m_pTurretController = TurretControllerComponent.Cast(m_Owner.FindComponent(TurretControllerComponent));
			if (m_pTurretController)
			{
				BaseCompartmentSlot slot = m_pTurretController.GetCompartmentSlot();
				if (slot)
				{
					ChimeraCharacter character = ChimeraCharacter.Cast(slot.GetOccupant());
					if (character)
					{
						m_ParentCharacter = character;
					}
				}
			}
			else
			{
				IEntity parentParent = parent;
				while (parentParent)
				{
					ChimeraCharacter character = ChimeraCharacter.Cast(parentParent);
					if (character)
					{
						m_ParentCharacter = character;
						m_pCharacterAiming = CharacterAimingComponent.Cast(character.FindComponent(CharacterAimingComponent));
						break;
					}
					
					parentParent = parentParent.GetParent();
				}
			}
			
			
			if (m_bShouldHideParentCharacter && m_ParentCharacter)
			{
				m_ParentCharacter.ClearFlags(EntityFlags.VISIBLE, false);
			}
		}
		
		if (m_pCharacterAiming)
		{
			// due to sway
			vector m[4];
			GetGame().GetWorld().GetCurrentCamera(m);
			m_vLastCameraDir = Math3D.MatrixToAngles(m);
			m_vLastPos = GetSightsRelPosition();
		}
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

	//------------------------------------------------------------------------------------------------
	protected override void OnSightADSDeactivated()
	{
		if (m_Owner && m_bShouldHideParentObject)
		{
			m_Owner.SetFlags(EntityFlags.VISIBLE, true);
			IEntity parent = m_Owner.GetParent();
			if (parent && m_bShouldHideParentParentObject)
			{
				parent.SetFlags(EntityFlags.VISIBLE, true);
			}
		}
		
		if (m_bShouldHideParentCharacter && m_ParentCharacter)
		{
			m_ParentCharacter.SetFlags(EntityFlags.VISIBLE, false);
		}

		m_ParentCharacter = null;
		
		super.OnSightADSDeactivated();
		
		// Removing switching input
		GetGame().GetInputManager().RemoveActionListener(ACTION_WHEEL, EActionTrigger.VALUE, SelectNextZoomLevel);
		GetGame().GetInputManager().RemoveActionListener(ACTION_ILLUMINATION, EActionTrigger.DOWN, ToggleIllumination);
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnInit(IEntity owner)
	{
		m_Owner = owner;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanFreelook() 
	{ 
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void ApplyRecoilToCamera(inout vector pOutCameraTransform[4], vector aimModAngles)
	{
		vector weaponAnglesMat[3];
		Math3D.AnglesToMatrix(aimModAngles, weaponAnglesMat);				
		Math3D.MatrixMultiply3(pOutCameraTransform, weaponAnglesMat, pOutCameraTransform);
	}

	//------------------------------------------------------------------------------------------------
	//! Constructor
	void SCR_2DSightsComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! Destructor
	void ~SCR_2DSightsComponent()
	{
		if (m_wRootWidget)
		{
			m_wRootWidget.RemoveFromHierarchy();
		}
	}

	//-----------------------------------------------------------------------------
	//! Update vignettes scale based on weapon recoil
	void UpdateRecoil(float timeSlice)
	{
		float zScaleRear = 1;
		float zScaleReticle = 1;
		
		if (m_ParentCharacter)
		{
			AimingComponent aimingComponent;
			if (m_ParentCharacter.IsInVehicle() && m_pTurretController)
			{
				aimingComponent = m_pTurretController.GetTurretComponent();
			}
			else
			{
				aimingComponent = m_pCharacterAiming;
			}
			
			if (aimingComponent)
			{
				vector aimingTranslation = aimingComponent.GetRawAimingTranslation();
				float zAmount = aimingTranslation[2];
				float zMag = Math.AbsFloat(zAmount / m_fRecoilTranslationTarget);
				// kick back only, TODO@AS: make sure we work both directions
				zScaleRear = Math.Clamp(1+zMag, 1, m_fRecoilScaleMax);
				zScaleReticle = Math.Clamp(1+zMag/m_fMagnification, 1, m_fRecoilScaleMax);
				
				m_vMoveRecoil[0] = aimingTranslation[0] * m_fRecoilScaleTranslation;
				m_vMoveRecoil[1] = aimingTranslation[1] * m_fRecoilScaleTranslation;
			}
		}
		
		// Scale rear 
		if (m_wSizeLayoutRear)
		{
			float rearSize = m_fDefaultSize * zScaleRear * m_fVignetteTextureMultiplier;
			
			m_wSizeLayoutRear.EnableWidthOverride(true);
			m_wSizeLayoutRear.SetWidthOverride(rearSize);
			
			m_wSizeLayoutRear.EnableHeightOverride(true);
			m_wSizeLayoutRear.SetHeightOverride(rearSize);
		}
		
		// Scale reticle 
		if (m_wOverlayReticles)
		{
			float recticleSize = m_fCurrentReticleSize * zScaleReticle;
			FrameSlot.SetSize(m_wOverlayReticles, recticleSize, recticleSize);
		}
	}
	
	//-----------------------------------------------------------------------------
	//! Checking if next move is longer on inverted
	/*protected bool IsAxisMoveChanged(float moveNext, float moveCurrent)
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
	}*/
			
	//------------------------------------------------------------------------------------------------
	override void Tick(float timeSlice)
	{				
		if (!m_bZoomed)
			return;
		
		super.Tick(timeSlice);
		
		if (m_bAllowRecoil)
			UpdateRecoil(timeSlice);
		
		// This value is handpicked and can vary between different sights,
		// but more or less should be equal to the pixel size of the
		// visible radius on screen, i.e. if vignette covers 300 pixels
		// from the center, this value should be around ~600
		const float reticleWidth = 800;
		// Magic is a hand picked constant that is the median of all fractions 
		// of target zeroing values of 3d and 2d optics, giving a rough approximation
		// of world PIP -> screen space 2D reticle position
		const float magic = 0.5772357724;
		// TODO@Wernerjak: pls uncomment and fix, it's complaining about widgets, ty
		SetReticleOffset(m_fReticleOffsetX * reticleWidth, m_fCurrentReticleOffsetY * reticleWidth * magic);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetupFovInfo()
	{
		if (!m_SightsFovInfo)
			return;
		
		// Get reference 1x FOV
		float referenceFOV;
		PlayerController pc = GetGame().GetPlayerController();
		if (pc && pc.GetPlayerCamera())
			referenceFOV = pc.GetPlayerCamera().GetFocusFOV();
		else
			referenceFOV = REFERENCE_FOV;
		
		// Set zooms 
		//m_SightsFovInfo.SetupZooms();
		array<float> zooms = m_SightsFovInfo.ZoomsToArray();
		
		// Get zooms 
		//int zoomCount = m_SightsFovInfo.GetStepsCount();
		int zoomCount = zooms.Count();
		
		for (int i = 0; i < zoomCount; i++)
		{
			// Calculate fov for each zoom
			float zoom = zooms[i];
			float fov = CalculateZoomFov(referenceFOV, zoom);
			m_SightsFovInfo.InsertFov(fov);	
			
			// Reticle sizes
			float initalZoom = zooms[0];
			
			if (i == 0)
			{
				// Initial setup
				m_aReticleSizes.Insert(m_fDefaultSize); 
				m_SightsFovInfo.SetCurrentFov(fov);
			}
			else
			{
				// Calulate size
				float zoomProgress = zooms[i] / initalZoom;
				float size = m_fDefaultSize * zoomProgress;
				m_aReticleSizes.Insert(size);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Switch betweem zoom levels from sights 
	protected void SelectZoomLevel(int id)
	{
		// Is zoomed 
		if (!m_bZoomed)
			return;
		
		// Check sights and id
		if (!m_SightsFovInfo)
			return;
		
		if (id < 0 || id > m_SightsFovInfo.GetStepsCount() - 1)
			return;
		
		// Set zoom 
		m_SightsFovInfo.SetIndex(id);
		m_fMagnification = m_SightsFovInfo.GetCurrentZoom();
		
		// Reticle size - dynamic reticle size 
		if (m_bDynamicReticle)
			m_fCurrentReticleSize = m_aReticleSizes[id];
	}
	
	//------------------------------------------------------------------------------------------------
	//! Move zoom level up and down based on mouse wheel value
	protected void SelectNextZoomLevel(float value) 
	{
		if (!m_SightsFovInfo || value == 0)
			return;

		// Down
		if (value > 0 && m_iSelectedZoomLevel < m_SightsFovInfo.GetStepsCount())
		{
			m_iSelectedZoomLevel++;
			SelectZoomLevel(m_iSelectedZoomLevel);
			
			// Play zoom change sound
			if (m_WeaponSoundComp)
				m_WeaponSoundComp.SoundEvent("SOUND_SCOPE_ZOOM_IN");
		}
		
		// Up 
		if (value < 0 && m_iSelectedZoomLevel > 0)
		{
			m_iSelectedZoomLevel--;
			SelectZoomLevel(m_iSelectedZoomLevel);
			
			// Play zoom change sound
			if (m_WeaponSoundComp)
				m_WeaponSoundComp.SoundEvent("SOUND_SCOPE_ZOOM_OUT");
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Action for toggling illumination 
	protected void ToggleIllumination(float value, EActionTrigger trigger)
	{
		m_bIsIlluminationOn = !m_bIsIlluminationOn;
		EnableReticleIllumination(m_bIsIlluminationOn);
		
		// Play toggle sound
		if (!m_WeaponSoundComp)
			return;
		
		if (m_bIsIlluminationOn)
			m_WeaponSoundComp.SoundEvent("SOUND_SCOPE_ILLUM_ON");
		else 
			m_WeaponSoundComp.SoundEvent("SOUND_SCOPE_ILLUM_OFF");
		
	}
	
	//------------------------------------------------------------------------------------------------
	//! Toggle between illumination modes 
	protected void EnableReticleIllumination(bool enable)
	{
		if (!m_SightsFovInfo || !m_wImgReticle)
			return;
		
		Color reticleTint = Color.Black;
		
		if (m_bHasIllumination && enable)
		{
			reticleTint = m_cReticleTextureIllumination;
		}
		
		m_wImgReticle.SetColor(reticleTint);
		
		if (m_wImgReticleGlow)
			m_wImgReticleGlow.SetColor(reticleTint);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override float GetReticleYOffsetTarget()
	{
		if (m_eZeroingType == SCR_EPIPZeroingType.EPZ_RETICLE_OFFSET)
		{
			float zoom = 1;
			if (m_SightsFovInfo)
				zoom = m_SightsFovInfo.GetCurrentZoom() / m_SightsFovInfo.GetBaseZoom();
			
			return m_fReticleOffsetY + (zoom * GetCurrentSightsRange()[0]);
		}
		
		return m_fReticleOffsetY;
	}
	
	
	//------------------------------------------------------------------------------------------------
	// Get Set API
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	SCR_SightsZoomFOVInfo GetSightsFovInfo()
	{
		return m_SightsFovInfo;
	}
};