[ComponentEditorProps(icon: HYBRID_COMPONENT_ICON)]
class SCR_MotorExhaustEffectGeneralComponentClass: MotorExhaustEffectComponentClass
{
};

class SCR_MotorExhaustEffectGeneralComponent : MotorExhaustEffectComponent
{
	private bool								m_bParticlesInitialized;
	private bool								m_bIsUnderwater;
	private SCR_ParticleEmitter					m_pDmgParticleEmitter; //Damage particle emitter
	private VehicleWheeledSimulation			m_pVehicleWheeledSimulation;
	private SCR_CarControllerComponent			m_pCarController;
	private SignalsManagerComponent				m_pSignalsManagerComponent;
	private IEntity								m_pOwner;
	private IEntity								m_pExhaustEffect; // Particles we are working with
	private float								m_fRPMScaled; // 0 to 1 scale of RPM between current and max RPM
	private float								m_fRPMScaledOld; // RPM scale in previous frame to calculate acceleration
	private float								m_fCarSpeedKMH; // Car speed
	private float								m_fThrust; // 0 to 1. Represents how much pedal to the metal is the driver pushing.
	private float								m_fEngineLoad; // How much load is the engine being pushed through
	private float								m_fPreviousLoad = -1; // Previous load state to detect necessary change
	private float								m_fLifetimeScale; // Lifetime of the exhaust particles
	private int									m_iEngineLoadSignalIdx; // EngineLoad signal index

	// There are some heavy calculations with particles going on per frame. The following variables solve the performance impact by calling them less often when they happen far away from the camera.
	private float								m_fUpdateDelay; // Desired delay between each particle calculations in seconds. 0 means per frame.
	private float								m_fCurrentUpdateDelay; // The time of current delay
	private const float							TICK_TIME_DELAY_MAX = 2; // Max acceptable delay in seconds
	private const float							TICK_DELAY_RANGE_START = 15; // Starting range at which the delay begins to increase (from 0 to TICK_TIME_DELAY_MAX)
	private const float							TICK_DELAY_RANGE_END = 100; // End range at which delay reaches its maximum (TICK_TIME_DELAY_MAX)

	private int									m_fExhaustStagesCount; // Number of exhaust particle stages
	private int									m_fDamageStagesCount; // Number of damage particle stages
	
	private float								m_fStartupTimeLeft; // Remaining time to keep playing the damaged particle effect after startup

	[Attribute("", UIWidgets.ResourceNamePicker, desc: "Particle effect for damaged engine", params: "ptc")]
	private ResourceName						m_DamagedParticle;
	[Attribute("1", UIWidgets.Auto, desc: "Time to play damaged exhaust particle after startup")]
	private float								m_fStartupTime;

	override void OnInit(IEntity owner)
	{
		if (System.IsConsoleApp())
		{
			Deactivate(owner);
			return;
		}
		
		m_pOwner = owner;
		m_pVehicleWheeledSimulation = VehicleWheeledSimulation.Cast(m_pOwner.FindComponent(VehicleWheeledSimulation));
				
		m_pSignalsManagerComponent = SignalsManagerComponent.Cast(m_pOwner.FindComponent(SignalsManagerComponent));
		if (m_pSignalsManagerComponent)
			m_iEngineLoadSignalIdx = m_pSignalsManagerComponent.AddOrFindSignal("EngineLoad");
		
		BaseVehicleNodeComponent node = BaseVehicleNodeComponent.Cast(m_pOwner.FindComponent(BaseVehicleNodeComponent));
		if (node)
			m_pCarController = SCR_CarControllerComponent.Cast(node.FindComponent(SCR_CarControllerComponent));
	}

	//! Called once when the particle effects are spawned
	void InitializeExhaustEffect()
	{
		m_fExhaustStagesCount = SCR_ParticleAPI.GetStagesCount(m_pExhaustEffect); // We need to know the amount of stages our exhaust particle has, if any.
	}

	//! Initialize damaged exhaust particles
	void InitializeDamageEffect()
	{
		int nodeId = -1;
		auto effectPosition = GetEffectPosition();
		if (effectPosition)
			nodeId = effectPosition.GetNodeId();
		m_pDmgParticleEmitter = SCR_ParticleAPI.PlayOnObjectPTC(m_pOwner, m_DamagedParticle, vector.Zero, vector.Zero, nodeId);
		if (m_pDmgParticleEmitter)
		{
			m_pDmgParticleEmitter.Pause();
			m_fDamageStagesCount = SCR_ParticleAPI.GetStagesCount(m_pDmgParticleEmitter);
		}
	}

	override void OnFrame(IEntity owner, float timeSlice)
	{
		// Check if effect exists. If not then try to get it.
		if (!m_bParticlesInitialized)
		{
			m_bParticlesInitialized = true;
			m_pExhaustEffect = GetParticleEntity();
			if (m_pExhaustEffect)
			{
				InitializeExhaustEffect();
				InitializeDamageEffect();
			}
		}

		// Check if the effect really exists. If not then do nothing.
		if (!m_pExhaustEffect)
			return;

		// Check if delay treshold was crossed. This prevents calling intense calculations per frame for vehicles which are far away from camera as they have low priority
		m_fCurrentUpdateDelay += timeSlice;
		if (m_fCurrentUpdateDelay < m_fUpdateDelay)
			return;

		// Calculate distance between owner and camera
		vector cameraTransform[4];
		GetGame().GetWorld().GetCurrentCamera(cameraTransform);
		float distance = vector.Distance(cameraTransform[3], m_pExhaustEffect.GetOrigin());

		// Prolong update interval based on the distance between camera and effect
		if (distance > TICK_DELAY_RANGE_START)
			m_fUpdateDelay = TICK_TIME_DELAY_MAX * ( Math.Clamp( (distance - TICK_DELAY_RANGE_START)  /  TICK_DELAY_RANGE_END, 0, 1) );
		else
			m_fUpdateDelay = 0;

		// We are about to call relatively performance heavy update of particle in OnUpdateEffect(...). This will be done per frame but only if the effect is very close to camera. Otherwise the update will be separated by time intervals to save performance.
		m_fRPMScaled = GetRpmScaled();
		if (m_fRPMScaled > 0 || m_fPreviousLoad < 0)
		{
			// Prepare values which will be used for particle calculations later in OnUpdateEffect(...)
			m_fThrust = GetSignalThrust();

			if(m_pVehicleWheeledSimulation)
				m_fCarSpeedKMH = Math.AbsFloat( m_pVehicleWheeledSimulation.GetSpeedKmh() );

			float RPM_acceleration = (m_fRPMScaled - m_fRPMScaledOld) * 1000 * timeSlice;
			m_fRPMScaledOld = m_fRPMScaled;

			m_fEngineLoad = (m_fThrust - RPM_acceleration) * (1-m_fRPMScaled) * 2;
			
			if (m_fEngineLoad < 0)
				m_fEngineLoad = 0;

			if (m_fCarSpeedKMH < 100)
				m_fLifetimeScale = (100 - m_fCarSpeedKMH) / 100;
			else
				m_fLifetimeScale = 0.01;

			m_fLifetimeScale = m_fLifetimeScale + m_fEngineLoad;
			
			// Set EngineLoad signal
			if (m_pSignalsManagerComponent)
				m_pSignalsManagerComponent.SetSignalValue(m_iEngineLoadSignalIdx, m_fEngineLoad);
		}

		OnUpdateEffect(owner, m_fCurrentUpdateDelay);
		m_fCurrentUpdateDelay = 0; // reset delay counter
	}

	//! Calculates the effect according to circumstances. This is called per frame if the owner is near the camera, but otherwise it's called in slow intervals to save performance.
	void OnUpdateEffect(IEntity owner, float dt)
	{
		// Disable exhaust while exhaust is submerged
		// TODO: signal for sound
		bool isUnderwater = SCR_WorldTools.IsObjectUnderwater(m_pExhaustEffect);
		if (m_bIsUnderwater != isUnderwater)
		{
			m_bIsUnderwater = isUnderwater;
			SCR_ParticleAPI.LerpAllEmitters(m_pExhaustEffect, 0, EmitterParam.BIRTH_RATE);
			SCR_ParticleAPI.LerpAllEmitters(m_pExhaustEffect, 0, EmitterParam.BIRTH_RATE_RND);
			if (m_pDmgParticleEmitter)
				m_pDmgParticleEmitter.Pause();
		}

		if (isUnderwater)
			return;

		// Thick initial exhaust
		bool isDefective = false;
		if (m_pDmgParticleEmitter)
		{
			if (m_fStartupTimeLeft > 0)
			{
				isDefective = true;
				m_fStartupTimeLeft -= dt;
			}
			else
			{
				isDefective = m_pCarController && m_pCarController.IsEngineDefective();
			}
		}

		if (m_pDmgParticleEmitter && m_pDmgParticleEmitter.GetIsPaused() == isDefective)
		{
			// Force update emitter
			m_fPreviousLoad = -1;

			// Toggle damage effect
			if (isDefective)
				m_pDmgParticleEmitter.UnPause();
			else
				m_pDmgParticleEmitter.Pause();
		}

		// Update only when load changes
		if (float.AlmostEqual(m_fEngineLoad, m_fPreviousLoad))
			return;

		m_fPreviousLoad = m_fEngineLoad;

		if (isDefective)
			LerpEngineEffects(m_pDmgParticleEmitter, m_fDamageStagesCount);
		else
			LerpEngineEffects(m_pExhaustEffect, m_fExhaustStagesCount);
	}

	void LerpEngineEffects(IEntity particleEffect, int stageCount = 0)
	{
		if (!particleEffect) return;

		if (stageCount > 0) // Check if the assigned particle effect supports staging (Staging divides effect's emittors into groups which are selectively enabled/disabled according to our needs)
		{
			// Staging is supported, so disable all emittors...
			SCR_ParticleAPI.LerpAllEmitters(particleEffect, 0, EmitterParam.BIRTH_RATE );
			SCR_ParticleAPI.LerpAllEmitters(particleEffect, 0, EmitterParam.BIRTH_RATE_RND );

			// ... now enable only the relevant emittors and work with them
			int current_stage = Math.ClampInt( Math.Round(stageCount * m_fEngineLoad)  , 0, stageCount);
			array<int> relevant_emitters_ids = SCR_ParticleAPI.FindEmittersByString(particleEffect, SCR_ParticleAPI.EMITTER_STAGE_PREFIX + current_stage.ToString() + SCR_ParticleAPI.EMITTER_STAGE_SUFIX);

			// Now we work only with the relevant stage (group) of emitters
			for (int i = 0; i < relevant_emitters_ids.Count(); i++)
			{
				SCR_ParticleAPI.LerpEmitter(particleEffect, relevant_emitters_ids[i], m_fRPMScaled*0.5 + 0.5, EmitterParam.BIRTH_RATE );
				SCR_ParticleAPI.LerpEmitter(particleEffect, relevant_emitters_ids[i], m_fRPMScaled*0.5 + 0.5, EmitterParam.BIRTH_RATE_RND );
				SCR_ParticleAPI.LerpEmitter(particleEffect, relevant_emitters_ids[i],  m_fLifetimeScale, EmitterParam.LIFETIME );
				SCR_ParticleAPI.LerpEmitter(particleEffect, relevant_emitters_ids[i],  m_fLifetimeScale, EmitterParam.LIFETIME_RND );
				SCR_ParticleAPI.LerpEmitter(particleEffect, relevant_emitters_ids[i],  ( m_fRPMScaled*3 ), EmitterParam.VELOCITY );
				SCR_ParticleAPI.LerpEmitter(particleEffect, relevant_emitters_ids[i],  ( m_fRPMScaled*3 ), EmitterParam.VELOCITY_RND );
				SCR_ParticleAPI.LerpEmitter(particleEffect, relevant_emitters_ids[i],  m_fRPMScaled, EmitterParam.AIR_RESISTANCE );
				SCR_ParticleAPI.LerpEmitter(particleEffect, relevant_emitters_ids[i],  m_fRPMScaled, EmitterParam.AIR_RESISTANCE_RND );
			}
		}
		else
		{
			// Staging is not supported so apply changes to all emittors.
			SCR_ParticleAPI.LerpAllEmitters(particleEffect,  m_fRPMScaled*1, EmitterParam.BIRTH_RATE );
			SCR_ParticleAPI.LerpAllEmitters(particleEffect,  m_fRPMScaled*1, EmitterParam.BIRTH_RATE_RND );
			SCR_ParticleAPI.LerpAllEmitters(particleEffect, m_fLifetimeScale, EmitterParam.LIFETIME );
			SCR_ParticleAPI.LerpAllEmitters(particleEffect, m_fLifetimeScale, EmitterParam.LIFETIME_RND );
			SCR_ParticleAPI.LerpAllEmitters(particleEffect,  ( m_fRPMScaled*7 ), EmitterParam.VELOCITY );
			SCR_ParticleAPI.LerpAllEmitters(particleEffect,  ( m_fRPMScaled*7 ), EmitterParam.VELOCITY_RND );
			SCR_ParticleAPI.LerpAllEmitters(particleEffect,  m_fRPMScaled, EmitterParam.AIR_RESISTANCE );
			SCR_ParticleAPI.LerpAllEmitters(particleEffect,  m_fRPMScaled, EmitterParam.AIR_RESISTANCE_RND );
		}
	}
	
	//! Turn on the effect for engine start and reset flow variables
	void OnEngineStart(bool startupSmoke)
	{
		m_fCurrentUpdateDelay = 0;
		m_fPreviousLoad = -1;
		m_bIsUnderwater = false;
		
		if (startupSmoke)
			m_fStartupTimeLeft = m_fStartupTime;
		else
			m_fStartupTimeLeft = 0;
		
		TurnOn(m_pOwner);
	}

	//! Turn off the effect and pause the damaged exhaust effect
	void OnEngineStop()
	{
		TurnOff();
		if (m_pDmgParticleEmitter && !m_pDmgParticleEmitter.GetIsPaused())
			m_pDmgParticleEmitter.Pause();
	}
};