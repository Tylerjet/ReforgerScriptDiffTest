class SCR_ImpactEffectComponentClass : ScriptComponentClass
{	
	[Attribute()]
	ref array<ResourceName> m_aDefaultParticles;
	
	[Attribute()]
	ref array<ResourceName> m_aWaterParticles;
	
	[Attribute(defvalue: "-1", desc: "If set to -1 component will use impulse to determine effect intensity")]
	int m_iEffectMagnitude;
}

class SCR_ImpactEffectComponent : ScriptComponent
{	
	protected ref ParticleEffectEntitySpawnParams m_ParticleSpawnParams;
	protected SoundComponent m_SoundComponent;

	protected const float MIN_TINY_IMPULSE = 2500;
	protected const float MIN_SMALL_IMPULSE = 5000;
	protected const float MIN_MEDIUM_IMPULSE = 10000;
	protected const float MIN_BIG_IMPULSE = 20000;
	protected const float MIN_HUGE_IMPULSE = 40000;
	
	protected const ref array<string> WATER_SOUNDS = {SCR_SoundEvent.SOUND_VEHICLE_WATER_SMALL, SCR_SoundEvent.SOUND_VEHICLE_WATER_MEDIUM, SCR_SoundEvent.SOUND_VEHICLE_WATER_BIG, SCR_SoundEvent.SOUND_VEHICLE_WATER_BIG};
	
	protected const int MAX_CALLS_PER_CONTACT = 1;
	protected const float RESET_TIME = 1000;
	
	protected int m_iCachedContactCalls;

	//------------------------------------------------------------------------------------------------
	protected array<ResourceName> GetDefaultParticles()
	{
		return SCR_ImpactEffectComponentClass.Cast(GetComponentData(GetOwner())).m_aDefaultParticles;
	}
	
	//------------------------------------------------------------------------------------------------
	protected array<ResourceName> GetWaterParticles()
	{
		return SCR_ImpactEffectComponentClass.Cast(GetComponentData(GetOwner())).m_aWaterParticles;
	}
	
	//------------------------------------------------------------------------------------------------
	protected int GetEffectMagnitude()
	{
		return SCR_ImpactEffectComponentClass.Cast(GetComponentData(GetOwner())).m_iEffectMagnitude;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ResetContactsDelayed()
	{
		m_iCachedContactCalls = 0;
	}
	
	//------------------------------------------------------------------------------------------------
	void OnImpact(notnull IEntity other, float impulse, vector impactPosition, vector impactNormal, GameMaterial mat)
	{
		if (m_iCachedContactCalls >= MAX_CALLS_PER_CONTACT)
			return;
		
		// all contacts will be ignored until cached contacts are reset
		if (m_iCachedContactCalls == MAX_CALLS_PER_CONTACT - 1)
			GetGame().GetCallqueue().CallLater(ResetContactsDelayed, RESET_TIME);
		
		m_iCachedContactCalls++;
		
		int magnitude = GetEffectMagnitude();
		
		if (magnitude <= -1)
		{
			int responseIndex = other.GetPhysics().GetResponseIndex();
		
			// Exclude collisions with physics objects with tiny response index e.g. bushes, fences etc.
			if (responseIndex == SCR_EPhysicsResponseIndex.TINY_DESTRUCTIBLE || responseIndex == SCR_EPhysicsResponseIndex.SMALL_DESTRUCTIBLE)
				return;
		
			// Exclude collisions with other vehicle parts
			if (SCR_VehicleDamageManagerComponent.Cast(other.GetRootParent().FindComponent(SCR_VehicleDamageManagerComponent)))
				return;
			
			if (impulse < MIN_TINY_IMPULSE)
				return;
		
			if (impulse < MIN_SMALL_IMPULSE)
				magnitude = 0;
			else if (impulse < MIN_MEDIUM_IMPULSE)
				magnitude = 1;
			else
				magnitude = 2;
		}
		
		vector transform[4];
		Math3D.MatrixIdentity4(transform);
		transform[3] = impactPosition;

		GameMaterial material = mat;
		ParticleEffectInfo effectInfo = material.GetParticleEffectInfo();		
		ResourceName resourceName = effectInfo.GetBlastResource(magnitude);

		if (resourceName.IsEmpty())
			resourceName = GetDefaultParticles()[magnitude];
		
		EmitParticles(transform, resourceName);
		
		if (magnitude > -1)
			Rpc(RPC_OnImpactBroadcast, impactPosition, impactNormal, magnitude);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnWaterEnter()
	{
		if (GetWaterParticles().IsEmpty())
			return;
		
		Physics physics = GetOwner().GetPhysics();
		if (!physics)
			return;

		// Only vertical axis is measured since water particles only go up
		float impulse = Math.AbsFloat(physics.GetVelocity()[1]) * physics.GetMass();
		
		if (impulse < MIN_TINY_IMPULSE)
			return;
		
		int magnitude = -1;	
		if (impulse < MIN_SMALL_IMPULSE)
			magnitude = 0;
		else if (impulse < MIN_MEDIUM_IMPULSE)
			magnitude = 1;
		else if (impulse < MIN_BIG_IMPULSE)
			magnitude = 2;
		else
			magnitude = 3;

		vector ownerTransform[4];
		GetOwner().GetTransform(ownerTransform);

		EWaterSurfaceType surfaceType;
		float lakeArea;
		float waterHeight = SCR_WorldTools.GetWaterSurfaceY(GetGame().GetWorld(), ownerTransform[3], surfaceType, lakeArea);

		vector transform[4];
		Math3D.MatrixIdentity3(transform);
		vector particleOrigin = {ownerTransform[3][0], waterHeight, ownerTransform[3][2]};
		transform[3] = particleOrigin;
		
		ResourceName resourceName = GetWaterParticles()[magnitude];
		string soundEvent = WATER_SOUNDS[magnitude];	

		if (resourceName.IsEmpty())
			resourceName = GetWaterParticles()[0];

		EmitParticles(transform, resourceName);
		PlaySound(soundEvent);
		
		if (magnitude > -1)
			Rpc(RPC_OnWaterEnterBroadcast, transform, magnitude);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void EmitParticles(vector transform[4], ResourceName particleResource)
	{
		if (!m_ParticleSpawnParams || particleResource.IsEmpty())
			return;
		
		m_ParticleSpawnParams.Transform = transform;
		ParticleEffectEntity.SpawnParticleEffect(particleResource, m_ParticleSpawnParams);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void PlaySound(string soundEvent)
	{
		if (!m_SoundComponent || soundEvent.IsEmpty())
			return;
		
		m_SoundComponent.SoundEvent(soundEvent);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Unreliable, RplRcver.Broadcast)]
	protected void RPC_OnImpactBroadcast(vector contactPos, vector contactNormal, int magnitude)
	{
		vector transform[4];
		Math3D.MatrixIdentity4(transform);
		transform[3] = contactPos;
		
		TraceParam trace = new TraceParam();
		trace.Start = contactPos + contactNormal;
		trace.End = contactPos - contactNormal;
		trace.Flags = TraceFlags.WORLD | TraceFlags.ENTS;
		
		GetOwner().GetWorld().TraceMove(trace, TraceFilter);
		
		GameMaterial contactMat = trace.SurfaceProps;
		ParticleEffectInfo effectInfo = contactMat.GetParticleEffectInfo();		
		ResourceName resourceName = effectInfo.GetBlastResource(magnitude);
		
		if (resourceName.IsEmpty())
			resourceName = GetDefaultParticles()[magnitude];
		
		EmitParticles(transform, resourceName);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Unreliable, RplRcver.Broadcast)]
	protected void RPC_OnWaterEnterBroadcast(vector transform[4], int magnitude)
	{
		ResourceName resourceName = GetWaterParticles()[magnitude];
		string soundEvent = WATER_SOUNDS[magnitude];
		
		if (resourceName.IsEmpty())
			resourceName = GetWaterParticles()[0];
		
		EmitParticles(transform, resourceName);
		PlaySound(soundEvent);
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool TraceFilter(notnull IEntity e)
	{
		return e != GetOwner() && e.GetRootParent() != GetOwner(); // ignore if traced entity is vehicle or vehicle part
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		m_ParticleSpawnParams = new ParticleEffectEntitySpawnParams();
		m_ParticleSpawnParams.TransformMode = ETransformMode.WORLD;
		m_ParticleSpawnParams.UseFrameEvent = true;
		
		m_SoundComponent = SoundComponent.Cast(GetOwner().FindComponent(SoundComponent));
		
		RplComponent rpl = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		if (!rpl || rpl.IsProxy())
			return;
		
		SCR_VehicleBuoyancyComponent buoyancyComp = SCR_VehicleBuoyancyComponent.Cast(GetOwner().FindComponent(SCR_VehicleBuoyancyComponent));
		if (!buoyancyComp)
			return;
		
		buoyancyComp.GetOnWaterEnter().Insert(OnWaterEnter);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(GetOwner(), EntityEvent.INIT);
	}
}

