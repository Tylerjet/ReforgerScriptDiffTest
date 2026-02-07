//! This class serves to unify commonly used methods in both 
//! SCR_DestructibleEntity and SCR_DestructionMultiPhaseComponent.
//! Since one is a component and the other is an entity, 
//! inheritance cannot be used to reuse code so all shared methods will be accessed here.
class SCR_DestructionUtility
{
	protected static const float SIMULATION_IMPRECISION_MULTIPLIER = 1.1;
	protected static const int MIN_MOMENTUM_RESPONSE_INDEX = 1;
	protected static const int MAX_MOMENTUM_RESPONSE_INDEX = 5;
	protected static const int MIN_DESTRUCTION_RESPONSE_INDEX = 6;
	protected static const int MAX_DESTRUCTION_RESPONSE_INDEX = 10;
	protected static const string DAMAGE_PHASE_SIGNAL_NAME = "DamagePhase";
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] contact
	//! \param[in] ownerMass
	//! \param[in] otherMass
	//! \return
	static float CalculateMomentum(notnull Contact contact, float ownerMass, float otherMass)
	{
		float dotMultiplier = vector.Dot(contact.VelocityAfter1.Normalized(), contact.VelocityBefore1.Normalized());
		float momentumBefore = ownerMass * contact.VelocityBefore1.Length() * SIMULATION_IMPRECISION_MULTIPLIER;
		float momentumAfter = ownerMass * contact.VelocityAfter1.Length() * dotMultiplier;
		float momentumA = Math.AbsFloat(momentumBefore - momentumAfter);
		
		dotMultiplier = vector.Dot(contact.VelocityAfter2.Normalized(), contact.VelocityBefore2.Normalized());
		momentumBefore = otherMass * contact.VelocityBefore2.Length() * SIMULATION_IMPRECISION_MULTIPLIER;
		momentumAfter = otherMass * contact.VelocityAfter2.Length() * dotMultiplier;
		float momentumB = Math.AbsFloat(momentumBefore - momentumAfter);
		return momentumA + momentumB;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Spawns objects that are meant to be created when the object is destroyed (particles, debris, etc)
	static void SpawnDestroyObjects(notnull IEntity owner, notnull array<ref SCR_BaseSpawnable> spawnList, notnull SCR_DestructionHitInfo destructionHitInfo)
	{		
		Physics physics = owner.GetPhysics();
		if (!physics)
			return;
		
		foreach (SCR_BaseSpawnable spawnObject : spawnList)
		{
			spawnObject.Spawn(owner, physics, destructionHitInfo);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] owner
	//! \param[in] materialSoundType
	//! \param[in] numDamagePhases
	//! \param[in] damagePhaseIndex
	static void PlaySound(notnull IEntity owner, SCR_EMaterialSoundTypeBreak materialSoundType, int numDamagePhases, int damagePhaseIndex)
	{
		SCR_SoundManagerEntity soundManagerEntity = GetGame().GetSoundManagerEntity();
		if (!soundManagerEntity)
			return;
		
		SCR_MPDestructionManager destructionManager = SCR_MPDestructionManager.GetInstance();
		if (!destructionManager)
			return;
		
		SCR_AudioSourceConfiguration audioSourceConfiguration = destructionManager.GetAudioSourceConfiguration();
		if (!audioSourceConfiguration)
			return;
		
		// Get AudioSource
		audioSourceConfiguration.m_sSoundEventName = SCR_SoundEvent.SOUND_MPD_ + owner.Type().EnumToString(SCR_EMaterialSoundTypeBreak, materialSoundType);
		
		SCR_AudioSource audioSource = soundManagerEntity.CreateAudioSource(owner, audioSourceConfiguration);
		if (!audioSource)
			return;
		
		// Set signals
		audioSource.SetSignalValue(SCR_AudioSource.PHASES_TO_DESTROYED_PHASE_SIGNAL_NAME, numDamagePhases - damagePhaseIndex - 1);
		audioSource.SetSignalValue(SCR_AudioSource.ENTITY_SIZE_SIGNAL_NAME, GetDestructibleSize(owner));
		
		// Set position
		vector mat[4];
		owner.GetTransform(mat);
		
		// Get center of bounding box
		vector mins;
		vector maxs;
		owner.GetWorldBounds(mins, maxs);
		mat[3] = vector.Lerp(mins, maxs, 0.5);
			
		// Play sound
		soundManagerEntity.PlayAudioSource(audioSource, mat);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets the model of the object
	//! \param[in] owner
	//! \param[in] modelName
	static void SetModel(notnull IEntity owner, ResourceName modelName)
	{
		Resource resource;
		VObject model = GetModel(modelName, resource);
		if (!model)
			return;

		SetModel(owner, model);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets the model of the object
	//! \param[in] owner
	//! \param[in] model
	static void SetModel(notnull IEntity owner, notnull VObject model)
	{
		owner.SetObject(model, string.Empty);
		owner.Update();
		
		Physics phys = owner.GetPhysics();
		if (!phys)
			return;
		
		// Update physics geometries after mesh change
		if (!phys.UpdateGeometries())
			phys.Destroy(); // No geoms found, destroy physics
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] modelName
	//! \param[out] resource pointer that you have to hold on to until the moment the model is set
	//! \return
	static VObject GetModel(ResourceName modelName, out Resource resource)
	{
		resource = Resource.Load(modelName);
		if (!resource.IsValid())
			return null;
		
		BaseResourceObject resourceObject = resource.GetResource();
		if (!resourceObject)
			return null;
		
		return resourceObject.ToVObject();
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] owner
	//! \return
	static float GetDestructibleSize(notnull IEntity owner)
	{
		vector mins, maxs
		owner.GetBounds(mins, maxs);
		return (maxs[0] - mins[0]) * (maxs[1] - mins[1]);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] physics
	//! \param[in] health
	//! \param[in] maxHealth
	static void UpdateResponseIndex(notnull Physics physics, float health, float maxHealth)
	{
		int responseIndex = physics.GetResponseIndex();
		if (responseIndex <= MIN_DESTRUCTION_RESPONSE_INDEX)
			return; // Cannot go lower
		
		if (maxHealth <= 0)
			maxHealth = 0;
		
		float healthPercentage = health / maxHealth;
		int minMaxDiff = MAX_DESTRUCTION_RESPONSE_INDEX - MIN_DESTRUCTION_RESPONSE_INDEX + 1;
		
		responseIndex = Math.ClampInt(MIN_DESTRUCTION_RESPONSE_INDEX - 1 + Math.Ceil(minMaxDiff * healthPercentage), MIN_DESTRUCTION_RESPONSE_INDEX, responseIndex);
		physics.SetResponseIndex(responseIndex);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] owner
	static void RegenerateNavmeshDelayed(notnull IEntity owner)
	{
		if (Replication.IsClient())
			return;
		
		SCR_AIWorld aiWorld = SCR_AIWorld.Cast(GetGame().GetAIWorld());
		
		if (!aiWorld)
			return;
		
		array<ref Tuple2<vector, vector>> areas = {};
		array<bool> redoAreas = {};
		aiWorld.GetNavmeshRebuildAreas(owner, areas, redoAreas); // Get area with current phase
		GetGame().GetCallqueue().CallLater(aiWorld.RequestNavmeshRebuildAreas, 1000, false, areas, redoAreas); // Rebuild later with new phase/when owner object is destroyed
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] owner
	//! \param[in] damagePhaseIndex
	// Update DamagePhase signal on entity that has SignalsManagerComponent
	// Used for triggering looped sounds
	static void SetDamagePhaseSignal(notnull IEntity owner, int damagePhaseIndex = 0)
	{
		SignalsManagerComponent signalsManagerComponent = SignalsManagerComponent.Cast(owner.FindComponent(SignalsManagerComponent));
		if (!signalsManagerComponent)
			return;
		
		// Pristine = 0, Destoyed > 0
		signalsManagerComponent.SetSignalValue(signalsManagerComponent.AddOrFindSignal(DAMAGE_PHASE_SIGNAL_NAME), damagePhaseIndex);
	}
}

#define ENABLE_BASE_DESTRUCTION
enum SCR_EMaterialSoundTypeDebris
{
	NONE,
	CERAMIC,
	BELL_SMALL,
	GLASS,
	HAYBALE,
	MATRESS,
	METAL_HEAVY,
	METAL_LIGHT,
	METAL_NETFENCE,	
	METAL_POLE,
	MORTAR,
	PLASTIC_HOLLOW,
	PLASTIC_SOLID,
	ROCK,
	ROCK_SMALL,
	SANDBAG,
	WOOD_PLANK_SMALL,
	WOOD_PLANK_LARGE
}

enum SCR_EMaterialSoundTypeBreak
{
	NONE,
	BREAK_CERAMIC,
	BREAK_GLASS,
	BREAK_GLASS_PANE,
	BREAK_GROUNDRADAR,
	BREAK_HAYBALE,
	BREAK_MATRESS,
	BREAK_METAL,
	BREAK_METAL_GENERATOR,
	BREAK_METAL_NETFENCE,
	BREAK_METAL_POLE,
	BREAK_PIANO,
	BREAK_PLASTIC,
	BREAK_ROCK,
	BREAK_SANDBAG,
	BREAK_TENT,
	BREAK_WATERHYDRANT,
	BREAK_WOOD_SOLID
}