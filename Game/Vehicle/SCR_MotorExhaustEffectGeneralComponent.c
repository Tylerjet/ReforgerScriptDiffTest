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
	private VehicleWheeledSimulation_SA			m_pVehicleWheeledSimulation_SA;
	private SCR_CarControllerComponent			m_pCarController;
	private SCR_CarControllerComponent_SA		m_pCarController_SA;
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
	private int									m_iIsExhaustUnderWaterSignalIdx; // IsExhaustUnderWater signal index

	// There are some heavy calculations with particles going on per frame. The following variables solve the performance impact by calling them less often when they happen far away from the camera.
	private float								m_fUpdateDelay; // Desired delay between each particle calculations in seconds. 0 means per frame.
	private float								m_fCurrentUpdateDelay; // The time of current delay
	private const float							TICK_TIME_DELAY_MAX = 2; // Max acceptable delay in seconds
	private const float							TICK_DELAY_RANGE_START = 15; // Starting range at which the delay begins to increase (from 0 to TICK_TIME_DELAY_MAX)
	private const float							TICK_DELAY_RANGE_END = 100; // End range at which delay reaches its maximum (TICK_TIME_DELAY_MAX)

	// Arrays of arrays of emitter indices (one array of indices per stage).
	// E.g. m_aExhaustStageEmitters[1][0] gets emitter index of the emitter #0 of stage #1
	private ref array<ref array<int>>			m_aExhaustStagesEmitters;
	private ref array<ref array<int>>			m_aDamageStagesEmitters;
	
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
		if(GetGame().GetIsClientAuthority())
			m_pVehicleWheeledSimulation = VehicleWheeledSimulation.Cast(m_pOwner.FindComponent(VehicleWheeledSimulation));
		else
			m_pVehicleWheeledSimulation_SA = VehicleWheeledSimulation_SA.Cast(m_pOwner.FindComponent(VehicleWheeledSimulation_SA)); 
				
		m_pSignalsManagerComponent = SignalsManagerComponent.Cast(m_pOwner.FindComponent(SignalsManagerComponent));
		if (m_pSignalsManagerComponent)
		{
			m_iIsExhaustUnderWaterSignalIdx = m_pSignalsManagerComponent.AddOrFindSignal("IsExhaustUnderWater");
		}
		
		BaseVehicleNodeComponent node = BaseVehicleNodeComponent.Cast(m_pOwner.FindComponent(BaseVehicleNodeComponent));
		if(GetGame().GetIsClientAuthority())
		{
			if (node)
				m_pCarController = SCR_CarControllerComponent.Cast(node.FindComponent(SCR_CarControllerComponent));
		}
		else
		{
			if (node)
				m_pCarController_SA = SCR_CarControllerComponent_SA.Cast(node.FindComponent(SCR_CarControllerComponent_SA));
		}
		
	}
	
	// Parses an emitter name and returns a number of a stage the emitter belongs to.
	// Returns -1 if the emitter's name does not meet the stage naming WeatherWindPattern
	// (s<X>_<name> where <X> is a number of a stage and <name> is any string, e.g. s0_smoke, s1_smoke, s10_debris etc.)
	private static int GetStageNumber(string emitterName)
	{
		static const string EMITTER_STAGE_PREFIX = "s";
		static const string EMITTER_STAGE_SUFIX  = "_";
		
		int offset = EMITTER_STAGE_PREFIX.Length();
		if (!emitterName.StartsWith(EMITTER_STAGE_PREFIX) || !emitterName.IsDigitAt(offset))
			return -1;
		
		int numberLen;
		int stageNo = emitterName.ToInt(offset: offset, parsed: numberLen);
		offset += numberLen;
		if (!emitterName.ContainsAt(EMITTER_STAGE_SUFIX, offset))
			return -1;
		
		return stageNo;
	}

	// Creates an index of emitters in effect stages,
	// i.e. array of arrays of emitter indices - one array of emitter indices for each stage.
	// E.g. for stageEmitters = CreateStageIndex(effect),
	// at stageEmitters[2] there is a an array of indices of emitters belonging to the the stage #2.
	private array<ref array<int>> CreateStageIndex(Particles effect)
	{		
		array<ref array<int>> ret = new array<ref array<int>>;

		array<string> emitterNames = new array<string>;
		effect.GetEmitterNames(emitterNames);
				
		for (int i=0; i<emitterNames.Count(); i++)
		{
			int stageNo = GetStageNumber(emitterNames[i]);
			if (stageNo < 0)
				continue;
			if (stageNo >= ret.Count())
				ret.Resize(stageNo+1);
			if (ret[stageNo] == null)
				ret[stageNo] = new array<int>;
			ret[stageNo].Insert(i);
		}
		
		return ret;
	}
	
	//! Called once when the particle effects are spawned
	void InitializeExhaustEffect()
	{
		m_aExhaustStagesEmitters = CreateStageIndex(m_pExhaustEffect.GetParticles());
	}

	//! Initialize damaged exhaust particles
	void InitializeDamageEffect()
	{
		int nodeId = -1;
		auto effectPosition = GetEffectPosition();
		if (effectPosition)
			nodeId = effectPosition.GetNodeId();
		m_pDmgParticleEmitter = SCR_ParticleEmitter.CreateAsChild(m_DamagedParticle, m_pOwner, vector.Zero, vector.Zero, nodeId);
		if (m_pDmgParticleEmitter)
		{
			m_pDmgParticleEmitter.Pause();
			m_aDamageStagesEmitters = CreateStageIndex(m_pDmgParticleEmitter.GetParticles());
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

			if(GetGame().GetIsClientAuthority())
			{
				if(m_pVehicleWheeledSimulation)
					m_fCarSpeedKMH = Math.AbsFloat( m_pVehicleWheeledSimulation.GetSpeedKmh() );
			}
			else
			{
				if(m_pVehicleWheeledSimulation_SA)
					m_fCarSpeedKMH = Math.AbsFloat( m_pVehicleWheeledSimulation_SA.GetSpeedKmh() );
			}

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
			Particles particles = m_pExhaustEffect.GetParticles();
			particles.SetParam(-1, EmitterParam.BIRTH_RATE, 0);
			particles.SetParam(-1, EmitterParam.BIRTH_RATE_RND, 0);
			if (m_pDmgParticleEmitter)
				m_pDmgParticleEmitter.Pause();
			
			if (m_pSignalsManagerComponent)
				m_pSignalsManagerComponent.SetSignalValue(m_iIsExhaustUnderWaterSignalIdx, isUnderwater);
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
				if(GetGame().GetIsClientAuthority())
					isDefective = m_pCarController && m_pCarController.IsEngineDefective();
				else
					isDefective = m_pCarController_SA && m_pCarController_SA.IsEngineDefective();
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
			AdjustEngineEffects(m_pDmgParticleEmitter, m_aDamageStagesEmitters);
		else
			AdjustEngineEffects(m_pExhaustEffect, m_aExhaustStagesEmitters);
	}

	void AdjustEngineEffects(IEntity particleEffectEnt, array<ref array<int>> stageIndex)
	{
		if (!particleEffectEnt) return;
		
		Particles particles = particleEffectEnt.GetParticles();
		
		int iMaxStage = stageIndex.Count() - 1;

		if (stageIndex && stageIndex.Count()) // Check if the assigned particle effect supports staging (Staging divides effect's emittors into groups which are selectively enabled/disabled according to our needs)
		{
			// Staging is supported, so disable all emittors...
			particles.SetParam(-1, EmitterParam.BIRTH_RATE, 0);
			particles.SetParam(-1, EmitterParam.BIRTH_RATE_RND, 0);

			// ... now enable only the relevant emittors and work with them
			int current_stage = Math.ClampInt(Math.Round(iMaxStage * m_fEngineLoad), 0, iMaxStage);
			array<int> relevant_emitters_ids = stageIndex[current_stage];

			// Now we work only with the relevant stage (group) of emitters
			for (int i = 0; i < relevant_emitters_ids.Count(); i++)
			{
				particles.MultParam(relevant_emitters_ids[i], EmitterParam.BIRTH_RATE,         m_fRPMScaled*0.5 + 0.5);
				particles.MultParam(relevant_emitters_ids[i], EmitterParam.BIRTH_RATE_RND,     m_fRPMScaled*0.5 + 0.5);
				particles.MultParam(relevant_emitters_ids[i], EmitterParam.LIFETIME,           m_fLifetimeScale);
				particles.MultParam(relevant_emitters_ids[i], EmitterParam.LIFETIME_RND,       m_fLifetimeScale);
				particles.MultParam(relevant_emitters_ids[i], EmitterParam.VELOCITY,           ( m_fRPMScaled*3 ));
				particles.MultParam(relevant_emitters_ids[i], EmitterParam.VELOCITY_RND,       ( m_fRPMScaled*3 ));
				particles.MultParam(relevant_emitters_ids[i], EmitterParam.AIR_RESISTANCE,     m_fRPMScaled);
				particles.MultParam(relevant_emitters_ids[i], EmitterParam.AIR_RESISTANCE_RND, m_fRPMScaled); 
			}
		}
		else
		{
			// Staging is not supported so apply changes to all emittors.
			particles.MultParam(-1, EmitterParam.BIRTH_RATE,         m_fRPMScaled*1);
			particles.MultParam(-1, EmitterParam.BIRTH_RATE_RND,     m_fRPMScaled*1);
			particles.MultParam(-1, EmitterParam.LIFETIME,           m_fLifetimeScale);
			particles.MultParam(-1, EmitterParam.LIFETIME_RND,       m_fLifetimeScale);
			particles.MultParam(-1, EmitterParam.VELOCITY,           ( m_fRPMScaled*7 ));
			particles.MultParam(-1, EmitterParam.VELOCITY_RND,       ( m_fRPMScaled*7 ));
			particles.MultParam(-1, EmitterParam.AIR_RESISTANCE,     m_fRPMScaled);
			particles.MultParam(-1, EmitterParam.AIR_RESISTANCE_RND, m_fRPMScaled);
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