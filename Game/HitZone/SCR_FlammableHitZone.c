enum EFireState
{
	NONE,
	SMOKING_LIGHT,
	SMOKING_HEAVY,
	SMOKING_IGNITING,
	BURNING
};

// TODO: Turn flammability into destruction handler
class SCR_FlammableHitZone: SCR_DestructibleHitzone
{
	private static const float			FIRE_TERRAIN_HEIGHT_TOLERANCE = 2.2; // Prevents spawning of ground fire effect if the vehicle is too high (in meters)
	protected static const float 		LIGHT_EMISSIVITY_START = 1;
	
	private IEntity						m_pFireInstigator;
	private EFireState					m_eFireState;
	private float						m_fFireRate;
	private float						m_fLightSmokeReductionRate;
	private float						m_fHeavySmokeReductionRate;
	private float						m_fIgnitingSmokeStokeRate;
	private float						m_fFireDamageRateMin;
	private float						m_fFireDamageRateMax;
	private float						m_fUpdateFireTime;
	private bool						m_bIsUpdatingFire;
	private bool						m_bIsBurning;
	
	protected ref array<LightEntity> m_aLightEntities;
	
	[Attribute("", desc: "Data for flame lighting that is visible when vehicle burns" ,UIWidgets.Object, "", category: "Flammability")]
	protected ref array<ref SCR_BaseLightData> m_aLightData;

	[Attribute(defvalue: "8", desc: "Fire damage applied to occupants of a burning vehicle each second (hp)", params: "0 100 0.01", category: "Flammability")]
	protected float m_fFireDamageOccupants;
	[Attribute(defvalue: "600", desc: "Maximum time for light smoke to stop (s)", params: "0 10000 0.1", category: "Flammability")]
	private float m_fLightSmokeStopTime;
	[Attribute(defvalue: "1800", desc: "Maximum time for heavy smoke to become light smoke (s)", params: "0 10000 0.1", category: "Flammability")]
	private float m_fHeavySmokeStopTime;
	[Attribute(defvalue: "180", desc: "Maximum time for igniting smoke to become fire (s)", params: "0 10000 0.1", category: "Flammability")]
	private float m_fIgnitingSmokeStokeTime;
	[Attribute(defvalue: "60", desc: "Minimum time for burning hitzone down (s) \n ! Determines maximum fire damage over time", params: "0 10000 0.1", category: "Flammability")]
	private float m_fMinFireBurningTime;
	[Attribute(defvalue: "240", desc: "Maxium time for burning hitzone down (s) \n ! Determines minimum fire damage over time", params: "0 10000 0.1", category: "Flammability")]
	private float m_fMaxFireBurningTime;
	[Attribute(defvalue: "0.1", uiwidget: UIWidgets.Slider, desc: "Light smoke effect threshold \n ! % of minimum fire damage over time", params: "0 1 0.01", category: "Flammability")]
	private float m_fLightSmokeThreshold;
	[Attribute(defvalue: "0.5", uiwidget: UIWidgets.Slider, desc: "Heavy smoke effect threshold \n ! % of minimum fire damage over time", params: "0 1 0.01", category: "Flammability")]
	private float m_fHeavySmokeThreshold;
	[Attribute(defvalue: "0.8", uiwidget: UIWidgets.Slider, desc: "Igniting smoke effect threshold \n ! % of minimum fire damage over time", params: "0 1 0.01", category: "Flammability")]
	private float m_fIgnitingSmokeThreshold;
	
	[Attribute(desc: "Particle effect for light smoke", params: "ptc", category: "Effects")]
	private ResourceName m_sDamagedParticle;
	[Attribute(desc: "Particle effect for heavy smoke", params: "ptc", category: "Effects")]
	private ResourceName m_sDamagedParticleHeavy;
	[Attribute(desc: "Particle effect for destruction flame", params: "ptc", category: "Effects")]
	private ResourceName m_sDestructionParticle;
	[Attribute(desc: "Particle effect for burning down after destruction", params: "ptc", category: "Effects")]
	private ResourceName m_sBurningParticle;
	[Attribute(desc: "Fire particle on the ground under object after the wreck stops moving", params: "ptc", category: "Effects")]
	private ResourceName m_sBurningGroundParticle;

	
	[Attribute(defvalue: "120", desc: "Burning time of the object after its explosion (s)", params: "0 1000 1", category: "Effects")]
	private float m_fBurningTime;
	[Attribute(uiwidget: UIWidgets.Coords, "Position of the effect in model space", category: "Effects")]
	private vector m_vParticleOffset;
	[Attribute(defvalue: "1", desc: "Minimum water depth to stop the fire", params: "0 100 0.1", category: "Flammability")]
	protected float m_fWaterDepthThreshold;
	
	// Audio features
	private int						m_iFireStateSignalIdx;
	
	// Damage particles
	private SCR_ParticleEmitter		m_pDmgParticleLight; // Lighter damage particle emitter
	private SCR_ParticleEmitter		m_pDmgParticleHeavy; // Darker damage particle emitter
	
	// Destruction particles
	private SCR_ParticleEmitter		m_pDstParticle; // Destruction particle
	private SCR_ParticleEmitter		m_pBurningGroundParticle; // Burning fuel on ground particle
	
	// Burning particles
	private SCR_ParticleEmitter		m_pBurningParticle; // Rapid fire damage particle emitter
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.OnInit(pOwnerEntity, pManagerComponent);
		
		InitFireRates();
		
		SignalsManagerComponent signalsManager = SignalsManagerComponent.Cast(pOwnerEntity.FindComponent(SignalsManagerComponent));
		if (signalsManager)
			m_iFireStateSignalIdx = signalsManager.AddOrFindSignal("FireState");
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMaxHealthChanged()
	{
		super.OnMaxHealthChanged();
		
		InitFireRates();
		
		if (GetGame().GetWorld())
			UpdateFireRate(true);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Compute smoke and  fire damage thresholds and rates
	void InitFireRates()
	{
		float maxDamage = GetMaxHealth();
		
		if (m_fMinFireBurningTime > 0)
			m_fFireDamageRateMax = maxDamage / m_fMinFireBurningTime;
		
		if (m_fMaxFireBurningTime > 0)
			m_fFireDamageRateMin = maxDamage / m_fMaxFireBurningTime;
		
		if (m_fLightSmokeStopTime > 0)
			m_fLightSmokeReductionRate = m_fFireDamageRateMin * (m_fHeavySmokeThreshold - m_fLightSmokeThreshold) / m_fLightSmokeStopTime;
		
		if (m_fHeavySmokeStopTime > 0)
			m_fHeavySmokeReductionRate = m_fFireDamageRateMin * (m_fIgnitingSmokeThreshold - m_fHeavySmokeThreshold) / m_fHeavySmokeStopTime;
		
		if (m_fIgnitingSmokeStokeTime > 0)
			m_fIgnitingSmokeStokeRate = m_fFireDamageRateMin * (1 - m_fIgnitingSmokeThreshold) / m_fIgnitingSmokeStokeTime;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Called after damage multipliers and thresholds are applied to received impacts and damage is applied to hitzone.
	This is also called when transmitting the damage to parent hitzones!
	\param type Type of damage
	\param damage Amount of damage received
	\param pOriginalHitzone Original hitzone that got dealt damage, as this might be transmitted damage.
	\param instigator Damage source parent entity (soldier, vehicle, ...)
	\param hitTransform [hitPosition, hitDirection, hitNormal]
	\param speed Projectile speed in time of impact
	\param colliderID ID of the collider receiving damage
	\param nodeID ID of the node of the collider receiving damage
	*/
	override void OnDamage(EDamageType type, float damage, HitZone pOriginalHitzone, IEntity instigator, inout vector hitTransform[3], float speed, int colliderID, int nodeID)
	{
		super.OnDamage(type, damage, pOriginalHitzone, instigator, hitTransform, speed, colliderID, nodeID);

		if (this != pOriginalHitzone)
			return;
		
		if (IsProxy())
			return;
		
		if (GetDamageState() == EDamageState.DESTROYED)
			return;
		
		// Incendiary or explosive ammo can set this kind of hitzone on fire
		if (type != EDamageType.INCENDIARY)
			return;
		
		SetFireRate(m_fFireRate + damage);
		
		// Last shot that sets the vehicle on fire is going to be remembered as instigator of fire
		if (m_eFireState == EFireState.SMOKING_IGNITING || (!m_pFireInstigator && m_eFireState == EFireState.BURNING))
			m_pFireInstigator = instigator;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Destruction logic
	override void OnDamageStateChanged()
	{
		super.OnDamageStateChanged();
		
		if (!GetGame().GetWorld())
			return;
		
		if (GetDamageState() == EDamageState.DESTROYED)
		{
			StartDestructionFire();
			if (m_pFireInstigator && m_eFireState == EFireState.BURNING)
				m_pLastInstigatorMap.Set(this, m_pFireInstigator);
			
			m_pFireInstigator = null;
		}
		else if (GetPreviousDamageState() == EDamageState.DESTROYED)
		{
			StopDestructionFire();
			m_pFireInstigator = null;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get rate of fire (dps). If it is below fire damage threshold, no damage will be dealt.
	float GetFireRate()
	{
		return m_fFireRate;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set rate of fire (dps). If it is below fire damage threshold, no damage will be dealt.
	void SetFireRate(float fireRate)
	{
		m_fFireRate = fireRate;
		UpdateFireRate(true);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns current fire state
	EFireState GetFireState()
	{
		return m_eFireState;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get fire rate that will achieve specified fire state
	\param fireState New fire state to be set
	\param weight Part of fire state zone to aim for. Default: -1 (becomes 0.5)
	*/
	float GetFireRateForState(EFireState fireState, float weight = -1)
	{
		if (weight < 0 || weight > 1)
			weight = 0.5;
		
		float top;
		float bottom;
		switch (fireState)
		{
			case EFireState.BURNING:
			{
				top = m_fFireDamageRateMax;
				bottom = m_fFireDamageRateMin;
				break;
			}
			case EFireState.SMOKING_IGNITING:
			{
				top = m_fFireDamageRateMin;
				bottom = m_fFireDamageRateMin * m_fIgnitingSmokeThreshold;
				break;
			}
			case EFireState.SMOKING_HEAVY:
			{
				top = m_fFireDamageRateMin * m_fIgnitingSmokeThreshold;
				bottom = m_fFireDamageRateMin * m_fHeavySmokeThreshold;
				break;
			}
			case EFireState.SMOKING_LIGHT:
			{
				top = m_fFireDamageRateMin * m_fHeavySmokeThreshold;
				bottom = m_fFireDamageRateMin * m_fLightSmokeThreshold;
				break;
			}
			default:
			{
				top = m_fFireDamageRateMin * m_fLightSmokeThreshold;
				bottom = 0;
			}
		}
		
		float fireRate = Math.Lerp(bottom, top, weight);
		return fireRate;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Set fire rate that will achieve specified fire state
	\param fireState New fire state to be set
	\param weight Part of fire state zone to aim for. Default: -1 (if state is different, becomes 0.5, otherwise no change)
	\param rate Allow changing fire rate. Default: true
	*/
	void SetFireState(EFireState fireState, float weight = -1, bool changeRate = true)
	{
		// Continue only if weight is defined or fir state is changed
		bool stateChanged = fireState != m_eFireState;
		m_eFireState = fireState;
		
		if (stateChanged)
			UpdateFireEffects(fireState);
		else if (weight < 0)
			return;
		
		if (IsProxy())
			return;
		
		// Update fire rate
		if (changeRate)
			SetFireRate(GetFireRateForState(fireState, weight));
		
		if (!stateChanged)
			return;
		
		// Clean the fire instigator if the fire is extinguished
		if (m_eFireState < EFireState.SMOKING_IGNITING)
			m_pFireInstigator = null;
		
		// Send update to remote clients
		array<HitZone> hitZones = {};
		SCR_HitZoneContainerComponent hitZoneContainer = GetHitZoneContainer();
		hitZoneContainer.GetAllHitZones(hitZones);
		int hitZoneIndex = hitZones.Find(this);
		if (hitZoneIndex >= 0)
			hitZoneContainer.Rpc(hitZoneContainer.RpcDo_SetFireState, hitZoneIndex, m_eFireState);
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get fire state that will be achieved for specified fire rate
	\param fireRate Fire rate to test for
	\return fireState New fire state
	*/
	EFireState GetFireStateForRate(float fireRate)
	{
		EFireState fireState;
		if (fireRate >= m_fFireDamageRateMin)
			fireState = EFireState.BURNING;
		else if (fireRate >= m_fFireDamageRateMin * m_fIgnitingSmokeThreshold)
			fireState = EFireState.SMOKING_IGNITING;
		else if (fireRate >= m_fFireDamageRateMin * m_fHeavySmokeThreshold)
			fireState =  EFireState.SMOKING_HEAVY;
		else if (fireRate >= m_fFireDamageRateMin * m_fLightSmokeThreshold)
			fireState = EFireState.SMOKING_LIGHT;
		else
			fireState = EFireState.NONE;
		
		return fireState;
	}
	
	//------------------------------------------------------------------------------------------------
	/*! Determine new fire rate
	\param fireRate Current rate of fire
	\param deltaTime Time passed in seconds since last update
	\return fireRate
	*/
	private float CalculateNewFireRate(float fireRate, float deltaTime)
	{
		float depth;
		if (fireRate > 0 && SCR_WorldTools.IsObjectUnderwater(GetOwner(), m_vParticleOffset, -1, depth) && depth > m_fWaterDepthThreshold)
		{
			fireRate = 0;
		}
		else if (deltaTime > 0 && fireRate > 0 && fireRate < m_fFireDamageRateMin)
		{
			// Update smoking virtual fire rate, that can get 
			if (fireRate < m_fHeavySmokeThreshold * m_fFireDamageRateMin)
				fireRate -= deltaTime * m_fLightSmokeReductionRate;
			else if (fireRate < m_fIgnitingSmokeThreshold * m_fFireDamageRateMin)
				fireRate -= deltaTime * m_fHeavySmokeReductionRate;
			else
				fireRate += deltaTime * m_fIgnitingSmokeStokeRate;
			
			fireRate = Math.Clamp(fireRate, 0, m_fFireDamageRateMin);
		}
		else
		{
			// On fire
			fireRate = Math.Clamp(fireRate, 0, m_fFireDamageRateMax);
		}
		
		return fireRate;
	}
	
	//------------------------------------------------------------------------------------------------
	/*! Apply fire damage to hitzone and to compartment occupants
	\param fireRate Current rate of fire
	\param deltaTime Time passed in seconds since last update
	*/
	private void ApplyFireDamage(float fireRate, float deltaTime)
	{
		if (fireRate < m_fFireDamageRateMin || GetDamageState() == EDamageState.DESTROYED)
		{
			SetDamageOverTime(EDamageType.FIRE, 0);
			return;
		}
		
		SetDamageOverTime(EDamageType.FIRE, fireRate);
		
		/*
		// Damage surrounding hitzones
		// TODO: Optimize and exclude current hitzone
		SCR_VehicleDamageManagerComponent damageManager = SCR_VehicleDamageManagerComponent.Cast(GetHitZoneContainer());
		if (damageManager)
			damageManager.DamageSurroundingHitzones(GetOwner().CoordToParent(m_vParticleOffset), fireRate, EDamageType.INCENDIARY);
		*/
			
		// Deal damage to crew depending on how much the fire is stoked
		// TODO: Deal damage based on distance from fire
		if (!m_pCompartmentManager)
			return;
		
		float damageOccupants = m_fFireDamageOccupants * deltaTime;
		if (m_fFireDamageRateMin > 0)
			damageOccupants *= fireRate / m_fFireDamageRateMin;
		
		if (damageOccupants > 0)
			m_pCompartmentManager.DamageOccupants(damageOccupants, EDamageType.FIRE, m_pFireInstigator, false, false);
	}

	//------------------------------------------------------------------------------------------------
	//! Flammable hitzone is not immediately set on fire.
	//! At first it generates smoke which can spontaneously extinguish or ignite itself.
	//! Damage is applied once fire rate exceeds damage threshold.
	private void UpdateFireRate(bool forceUpdate = false)
	{
		if (IsProxy())
			return;
		
		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return;
		
		IEntity owner = GetOwner();
		if (!owner)
		{
			m_bIsUpdatingFire = false;
			GetGame().GetCallqueue().Remove(UpdateFireRate);
			return;
		}
		
		if (!forceUpdate && m_fUpdateFireTime == 0)
			return;
		
		// Scale damage by time passed
		float deltaTime = 0;
		float currentTime = world.GetWorldTime();
		
		if (m_fUpdateFireTime > 0)
			deltaTime = (currentTime - m_fUpdateFireTime) * 0.001; // Convert to seconds
		
		float previousFireRate = m_fFireRate;
		m_fFireRate = CalculateNewFireRate(previousFireRate, deltaTime);
		
		// Extinguishing should force RPC and removal of queued call so that clients stop updating fire rate
		if (m_fFireRate == 0 && previousFireRate > 0)
			forceUpdate = true;
				
		ApplyFireDamage(m_fFireRate, deltaTime);

		// Clear any scheduled call in the queue
		if (forceUpdate && m_bIsUpdatingFire)
		{
			m_bIsUpdatingFire = false;
			GetGame().GetCallqueue().Remove(UpdateFireRate);
		}
		
		// Schedule future update
		if (m_fFireRate > 0)
		{
			m_fUpdateFireTime = currentTime;
			if (!m_bIsUpdatingFire)
			{
				// Schedule delayed update every second
				m_bIsUpdatingFire = true;
				GetGame().GetCallqueue().CallLater(UpdateFireRate, 1000, true, false);
			}
		}
		else
		{
			m_fUpdateFireTime = 0;
		}
		
		// Update fire state
		EFireState newFireState = GetFireStateForRate(m_fFireRate);
		if (newFireState != m_eFireState)
			SetFireState(newFireState, -1, false);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------
	void StartDestructionFire()
	{
		IEntity owner = GetOwner();
		if (!owner)
			return;
		
		if (IsProxy() && !GetHitZoneContainer().IsRplReady())
			return;
		
		float depth;
		if (SCR_WorldTools.IsObjectUnderwater(owner, m_vParticleOffset, -1, depth) && depth > m_fWaterDepthThreshold)
		{
			StopDestructionFire();
			return;
		}
		
		SetFireState(EFireState.BURNING);
		
		ScriptCallQueue queue = GetGame().GetCallqueue();
		queue.CallLater(StopDestructionFire, m_fBurningTime * 1000);
		queue.CallLater(StartDestructionGroundFire, 1000, true);
		
		// Particles are only relevant for non-dedicated clients
		if (System.IsConsoleApp())
			return;
		
		if (!m_pDstParticle && !m_sDestructionParticle.IsEmpty())
			m_pDstParticle = SCR_ParticleAPI.PlayOnObjectPTC( owner, m_sDestructionParticle, m_vParticleOffset, vector.Zero );
	}
	
	//------------------------------------------------------------------------------------------------
	//! Attempts to spawn fire effect on the ground under the burning wreck.
	private void StartDestructionGroundFire()
	{
		IEntity owner = GetOwner();
		if (!owner)
			return;
		
		float depth;
		if (SCR_WorldTools.IsObjectUnderwater(owner, m_vParticleOffset, -1, depth) && depth > m_fWaterDepthThreshold)
		{
			StopDestructionFire();
			return;
		}
		else if (SCR_WorldTools.IsObjectUnderwater(owner))
		{
			// Do not create surface fire effect
			return;
		}
		
		// Make sure the wreck is not moving
		Physics physics = owner.GetPhysics();
		if (physics && physics.GetVelocity().LengthSq() > 0.01)
			return;
		
		// Measure the height difference between owner and the surface
		vector position = owner.GetOrigin();
		float surfaceY = GetGame().GetWorld().GetSurfaceY(position[0], position[2]);
		
		// If the difference is within tolerance then proceed to spawn fire effect and stop the loop
		if (position[1] - surfaceY > FIRE_TERRAIN_HEIGHT_TOLERANCE)
			return;
		
		// Stop the loop
		GetGame().GetCallqueue().Remove(StartDestructionGroundFire);
		
		// No need to play particles on headless client
		if (System.IsConsoleApp())
			return;
		
		// Stop existing effect so that it gets removed and does not have to be tracked again
		if (m_pBurningGroundParticle)
			m_pBurningGroundParticle.Pause();
		
		position[1] = surfaceY;
		
		if (!m_sDestructionParticle.IsEmpty())
			m_pBurningGroundParticle = SCR_ParticleAPI.PlayOnPositionPTC(m_sBurningGroundParticle, position);
	}
	
	// Stops fire on vehicle and on the ground
	void StopDestructionFire()
	{
		SetFireState(EFireState.NONE);
		
		ScriptCallQueue queue = GetGame().GetCallqueue();
		queue.Remove(StartDestructionGroundFire);
		queue.Remove(StopDestructionFire);
		
		if (m_pBurningGroundParticle)
			m_pBurningGroundParticle.Pause();
	}

	//------------------------------------------------------------------------------------------------
	//! Logic when vehicle is damaged
	private void UpdateFireEffects(EFireState fireState)
	{
		// No need to play particles on headless client
		if (System.IsConsoleApp())
			return;
		
		// Update sound effects
		IEntity owner = GetOwner();
		if (!owner)
			return;
		
		SignalsManagerComponent signalsManager = SignalsManagerComponent.Cast(owner.FindComponent(SignalsManagerComponent));
		if (signalsManager)
			signalsManager.SetSignalValue(m_iFireStateSignalIdx, fireState);
		
		// Requiring all the particles to be defined simplifies the logic.
		if (m_sDamagedParticle.IsEmpty() || m_sDamagedParticleHeavy.IsEmpty() || m_sBurningParticle.IsEmpty())
			return;
		
		// This part is terrible, it should be refactored
		if (!m_pDmgParticleLight || !m_pDmgParticleHeavy || !m_pBurningParticle)
		{
			// Spawn particles on object
			if (!m_pDmgParticleLight)
			{
				m_pDmgParticleLight = SCR_ParticleAPI.PlayOnObjectPTC(owner, m_sDamagedParticle, m_vParticleOffset, vector.Zero);
				m_pDmgParticleLight.Pause();
			}
			
			if (!m_pDmgParticleHeavy)
			{
				m_pDmgParticleHeavy = SCR_ParticleAPI.PlayOnObjectPTC(owner, m_sDamagedParticleHeavy, m_vParticleOffset, vector.Zero);
				m_pDmgParticleHeavy.Pause();
			}
			
			if (!m_pBurningParticle)
			{
				m_pBurningParticle = SCR_ParticleAPI.PlayOnObjectPTC(owner, m_sBurningParticle, m_vParticleOffset, vector.Zero);
				m_pBurningParticle.Pause();
			}
		}
		
		// Switch particle states
		// start fire
		if (fireState == EFireState.BURNING)
		{
			if (!m_bIsBurning)
			{
				m_pBurningParticle.RestartParticle();
				m_bIsBurning = true;	
			}
			
			m_pDmgParticleLight.Pause();
			m_pDmgParticleHeavy.Pause();
			m_pBurningParticle.UnPause();
			FireLightOn();
		}
		else
		{
			m_bIsBurning = false;
			
			// igniting smoke
			if (fireState == EFireState.SMOKING_IGNITING)
			{
				m_pDmgParticleLight.Pause();
				m_pDmgParticleHeavy.UnPause();
				m_pBurningParticle.Pause();
				FireLightOff();
			}
			// dark smoke
			else if (fireState == EFireState.SMOKING_HEAVY)
			{
				m_pDmgParticleLight.Pause();
				m_pDmgParticleHeavy.UnPause();
				m_pBurningParticle.Pause();
				FireLightOff();
			}
			// light smoke
			else if (fireState == EFireState.SMOKING_LIGHT)
			{
				m_pDmgParticleLight.UnPause();
				m_pDmgParticleHeavy.Pause();
				m_pBurningParticle.Pause();
				FireLightOff();
			}
			else
			{
				m_pDmgParticleLight.Pause();
				m_pDmgParticleHeavy.Pause();
				m_pBurningParticle.Pause();
				FireLightOff();
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void FireLightOn()
	{
		if (m_aLightEntities)
			return;
		
		IEntity owner = GetOwner();
		if (!owner)
			return;
		
		vector ownerOrigin = owner.GetOrigin();
		
		m_aLightEntities = {};
		
		foreach (SCR_BaseLightData lightdata: m_aLightData)
		{
			LightEntity lightEntity = LightEntity.CreateLight(lightdata.GetLightType(), lightdata.GetLightFlag(), lightdata.GetEffectRadius(), Color.FromVector(lightdata.GetLightColor()), LIGHT_EMISSIVITY_START, ownerOrigin);
			owner.AddChild(lightEntity, -1, EAddChildFlags.AUTO_TRANSFORM);
			
			// Calculate desired position
			lightEntity.SetOrigin(ownerOrigin + lightdata.GetLightOffset());
			lightEntity.SetLensFlareType(LightLensFlareType.Disabled);
			lightEntity.SetIntensityEVClip(lightdata.GetIntensityClipEV());
			
			m_aLightEntities.Insert(lightEntity);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void FireLightOff()
	{
		if (!m_aLightEntities)
			return;
		
		foreach (LightEntity lightEntity: m_aLightEntities)
		{
			if (lightEntity)
				delete lightEntity;
		}
		
		m_aLightEntities = null;
	}
};
