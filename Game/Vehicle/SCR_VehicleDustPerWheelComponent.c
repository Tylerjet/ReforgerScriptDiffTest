// Spawns dust particle effect that drags with the vehicle during ride

[ComponentEditorProps(category: "GameScripted/Test", description:"SCR_VehicleDustPerWheel")]
class SCR_VehicleDustPerWheelClass : MultiEffectComponentClass
{
	[Attribute("0", UIWidgets.Auto, "Vehicle index in ParticleEffectInfo")]
	int m_iVehicleIndex;

	[Attribute("", UIWidgets.Auto, "Simulation IDs of wheels to apply particle effects to")]
	ref array<int> m_aWheels;

	[Attribute("10", UIWidgets.Slider, "Minimal speed for 0% effect intensity interpolation\n[km/h]", "0 300 1")]
	float m_fDustStartSpeed;

	[Attribute("80", UIWidgets.Slider, "Maximal speed for 100% effect intensity interpolation\n[km/h]", "0 300 1")]
	float m_fDustTopSpeed;

	[Attribute("250", UIWidgets.Slider, "Maximal distance at which the effect is visible\n[m]", "0 2000 1")]
	float m_fMaxDistanceVisible;
};

class SCR_VehicleDustPerWheel : MultiEffectComponent
{
	static const int							UPDATE_TIMEOUT = 1000; //Minimal delay between two particle swaps.

	protected VehicleWheeledSimulation			m_Simulation;
	protected SCR_VehicleDustPerWheelClass		m_ComponentData;
	protected RplComponent						m_RplComponent;
	protected ref TVectorArray					m_aLocalDustPos;
	protected ref TBoolArray 					m_aWheelHasContact;
	protected ref TIntArray						m_aLastSwap;

#ifdef ENABLE_DIAG
	ref array<string> m_aGameMatNames;
#endif

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		if (System.IsConsoleApp())
		{
			Deactivate(owner);
			return;
		}

		m_ComponentData = SCR_VehicleDustPerWheelClass.Cast(GetComponentData(owner));
		if (!m_ComponentData || m_ComponentData.m_fDustStartSpeed > m_ComponentData.m_fDustTopSpeed || m_ComponentData.m_fMaxDistanceVisible <= 0)
		{
			Deactivate(owner);
			return;
		}

		m_Simulation = VehicleWheeledSimulation.Cast(owner.FindComponent(VehicleWheeledSimulation));
		if (!m_Simulation || !m_Simulation.IsValid())
		{
			Deactivate(owner);
			return;
		}

		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));

		m_aWheelHasContact = {};
		m_aLocalDustPos = {};
		m_aLastSwap = {};

		SetEventMask(owner, EntityEvent.INIT | EntityEvent.FIXEDFRAME);
	}

	//------------------------------------------------------------------------------------------------
	bool OnTicksOnRemoteProxy()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		int count = Math.Min(m_Simulation.WheelCount(), m_ComponentData.m_aWheels.Count());

		m_aWheelHasContact.Resize(count);
		m_aLocalDustPos.Resize(count);
		m_aLastSwap.Resize(count);

		ReserveEffects(count);

#ifdef ENABLE_DIAG
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_PARTICLES_VEHICLE_DUST, "", "Show dust materials", "Vehicles");
		m_aGameMatNames = {};
		m_aGameMatNames.Resize(count);
#endif
	}

	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		UpdateBatch(owner, timeSlice);

		if (m_Simulation.GetSpeedKmh() < m_ComponentData.m_fDustStartSpeed && !HasActiveParticles())
		{
			ClearEventMask(owner, EntityEvent.FRAME);
			SetEventMask(owner, EntityEvent.FIXEDFRAME);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void EOnFixedFrame(IEntity owner, float timeSlice)
	{
		if (m_Simulation.GetSpeedKmh() < m_ComponentData.m_fDustStartSpeed)
			return;

		SetEventMask(owner, EntityEvent.FRAME);
		ClearEventMask(owner, EntityEvent.FIXEDFRAME);
	}

	//------------------------------------------------------------------------------------------------
	void UpdateCurrent(notnull ParticleEffectHandle effect, float timeSlice, int index, float speed)
	{
		effect.Continue();

		IEntity effectEntity = effect.GetEntity();
		if (!effectEntity)
			return;

		UpdatePosition(effectEntity, index);
		UpdateVehicleDustEffect(effectEntity, speed, m_ComponentData.m_fDustTopSpeed, index);
		effect.Update(timeSlice);
	}

	//------------------------------------------------------------------------------------------------
	TraceParam TraceWheelContact(notnull IEntity owner, int index, out vector position)
	{
		int wheelIdx = m_ComponentData.m_aWheels[index];

		vector worldTransform[4];
		owner.GetWorldTransform(worldTransform);

		float radius = m_Simulation.WheelGetRadiusState(wheelIdx);

		// Physics are offset by center of mass
		Physics physics = owner.GetPhysics();
		vector centerOfMass = physics.GetCenterOfMass();

		TraceParam trace = new TraceParam();
		trace.Start = (m_Simulation.WheelGetPosition(wheelIdx, 1.0) + centerOfMass).Multiply4(worldTransform);
		trace.End = (m_Simulation.WheelGetPosition(wheelIdx, 0.0) + centerOfMass).Multiply4(worldTransform) + worldTransform[1] * -radius;
		trace.Flags = TraceFlags.WORLD;
		trace.LayerMask = EPhysicsLayerDefs.VehicleCast;

		float hit = owner.GetWorld().TraceMove(trace, null);

		vector remotePosition = trace.Start + (trace.End - trace.Start) * hit;
		position = remotePosition.InvMultiply4(worldTransform);

#ifdef ENABLE_DIAG
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_PARTICLES_VEHICLE_DUST))
		{
			vector p[2];
			p[0] = trace.Start;
			p[1] = trace.End;
			Shape.CreateLines(Color.BLUE, ShapeFlags.ONCE | ShapeFlags.NOZBUFFER, p, 2);
			Shape.CreateSphere(Color.RED, ShapeFlags.ONCE | ShapeFlags.NOZBUFFER, remotePosition, 0.05);
		}
#endif

		return trace;
	}

	//------------------------------------------------------------------------------------------------
	// Called when the effects needs to be updated. This happens per frame if the camera is close to it, but much slower if further away fr optimization purposes.
	override void UpdateEffect(notnull ParticleEffectHandle effect, IEntity owner, float timeSlice, int index)
	{
		if (!owner)
			return;

		IEntity effectEntity = effect.GetEntity();

		float speed = m_Simulation.GetSpeedKmh();

		vector camMat[4];
		owner.GetWorld().GetCurrentCamera(camMat);

		// Activate particle effect only within the desired speed and distance
		if (speed >= m_ComponentData.m_fDustStartSpeed && vector.DistanceSq(owner.GetOrigin(), camMat[3]) < m_ComponentData.m_fMaxDistanceVisible * m_ComponentData.m_fMaxDistanceVisible)
		{
			int ticks = System.GetTickCount();
			int dif = Math.AbsInt(ticks - m_aLastSwap[index]);

			if (effectEntity && dif < UPDATE_TIMEOUT)
			{
				UpdateCurrent(effect, timeSlice, index, speed);
				return;
			}

			GameMaterial newMaterial;
			bool wheelHasContact;
			if (m_RplComponent && !m_RplComponent.IsMaster())
			{
				vector position;
				TraceParam trace = TraceWheelContact(owner, index, position);
				if (trace)
				{
					newMaterial = trace.SurfaceProps;
					wheelHasContact = trace.TraceEnt != null;
					m_aLocalDustPos[index] = position;
				}
			}
			else
			{
				int wheelIdx = m_ComponentData.m_aWheels[index];
				newMaterial = m_Simulation.WheelGetContactMaterial(wheelIdx);
				wheelHasContact = m_Simulation.WheelHasContact(wheelIdx);
			}

			m_aWheelHasContact[index] = wheelHasContact;

			GameMaterial currentMaterial = effect.GetMaterial();
			ResourceName newResource;

			if (newMaterial)
			{
				if (newMaterial != currentMaterial)	//different materials -> check resource
				{
					ParticleEffectInfo effectInfo = newMaterial.GetParticleEffectInfo();
					if (effectInfo)
						newResource = effectInfo.GetVehicleDustResource(m_ComponentData.m_iVehicleIndex);
				}
			}
#ifdef ENABLE_DIAG
			else if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_PARTICLES_VEHICLE_DUST))
			{
				m_aGameMatNames[index] = "";
			}
#endif

			//update old effect
			if (effectEntity && (newMaterial == currentMaterial || effect.GetResource() == newResource))
			{
				if (newMaterial != currentMaterial)	//different GameMats, but resources are the same
				{
					effect.SetMaterial(newMaterial);
#ifdef ENABLE_DIAG
					if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_PARTICLES_VEHICLE_DUST))
						m_aGameMatNames[index] = newMaterial.GetName();
#endif
				}
				UpdateCurrent(effect, timeSlice, index, speed);
				return;
			}
			else
			{
				if (newResource && newResource.Length() > 0)	//create new effect
				{
					if (effectEntity) // stop old effect
					{
						effect.StopEmit();
						effectEntity = null;
					}
					effectEntity = effect.CreateEffect(newResource, GenericEntity, owner.GetWorld());
#ifdef ENABLE_DIAG
					if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_PARTICLES_VEHICLE_DUST))
						m_aGameMatNames[index] = newMaterial.GetName();
#endif
					m_aLastSwap[index] = ticks;
					effect.SetMaterial(newMaterial);
				}
				else if (effectEntity)	//only pause old effect
				{
					effect.Pause();
				}

				if (effectEntity)	//update current effect
				{
					UpdatePosition(effectEntity, index);
					UpdateVehicleDustEffect(effectEntity, speed, m_ComponentData.m_fDustTopSpeed, index);
					effect.Update(timeSlice);
				}
			}
		}
		else	//moving too slow to generate more dust
		{
			effect.Pause();
			if (effect.HasActiveParticles())
			{
				UpdatePosition(effectEntity, index);
				effect.Update(timeSlice);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	void UpdatePosition(notnull IEntity effectEntity, int index)
	{
		vector position;
		if (m_RplComponent && !m_RplComponent.IsMaster())
		{
			vector worldTransform[4];
			GetOwner().GetWorldTransform(worldTransform);
			position = m_aLocalDustPos[index].Multiply4(worldTransform);
		}
		else
		{
			position = m_Simulation.WheelGetContactPosition(m_ComponentData.m_aWheels[index]);
		}

		vector mat[4];
		Math3D.MatrixIdentity4(mat);
		mat[3] = position;
		effectEntity.SetWorldTransform(mat);
	}

	//------------------------------------------------------------------------------------------------
	void UpdateVehicleDustEffect(notnull IEntity effectEntity, float speed, float endSpeed, int index)
	{
		float speedCoef = 0;
		float birthCoef = 0;
		float gravityCoef = 0;

		int wheelIdx = m_ComponentData.m_aWheels[index];
		float longitudinalSlip = m_Simulation.WheelGetLongitudinalSlip(wheelIdx);
		float lateralSlip = m_Simulation.WheelGetLateralSlip(wheelIdx);
		float slip = Math.AbsFloat(longitudinalSlip) + Math.AbsFloat(lateralSlip);

		bool wheelHasContact = m_aWheelHasContact[index];
		if (wheelHasContact)
		{
			speedCoef = Math.AbsFloat(0.2 + speed * 0.8 / endSpeed + slip);
			birthCoef = Math.AbsFloat(0.8 + speed * 0.2 / endSpeed + slip * 2);
			gravityCoef = Math.AbsFloat(0.8 + speed * 0.2 / endSpeed);
		}

		Particles particles = effectEntity.GetParticles();
		particles.MultParam(-1, EmitterParam.BIRTH_RATE, birthCoef);
		particles.MultParam(-1, EmitterParam.GRAVITY_SCALE_RND, gravityCoef);
		particles.MultParam(-1, EmitterParam.VELOCITY, speedCoef);
		particles.MultParam(-1, EmitterParam.VELOCITY_RND, speedCoef);

#ifdef ENABLE_DIAG
		if (!DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_PARTICLES_VEHICLE_DUST))
			return;

		if (!wheelHasContact)
			return;

		if (m_aGameMatNames[index].IsEmpty())
			return;

		vector effectPosition = effectEntity.GetWorldTransformAxis(3);
		DebugTextWorldSpace.Create(GetOwner().GetWorld(), m_aGameMatNames[index] + "\nlongSlip: " + longitudinalSlip.ToString(-1, 3) + " latSlip: " + lateralSlip.ToString(-1, 3), DebugTextFlags.CENTER | DebugTextFlags.ONCE, effectPosition[0], effectPosition[1] - 1, effectPosition[2]);
#endif
	}
};
