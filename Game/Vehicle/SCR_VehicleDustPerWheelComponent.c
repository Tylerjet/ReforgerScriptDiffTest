// Spawns dust particle effect that drags with the vehicle during ride

[ComponentEditorProps(category: "GameScripted/Test", description:"SCR_VehicleDustPerWheel")]
class SCR_VehicleDustPerWheelClass: MultiEffectComponentClass
{
	[Attribute("0", UIWidgets.Auto, "Vehicle index in ParticleEffectInfo")]
	int m_aVehicleIndex;
	
	[Attribute("", UIWidgets.Auto, "Which wheel should have a particle effect")]
	ref array<int> m_aWheels;
	
	[Attribute("10", UIWidgets.Slider, "Minimal speed for 0% effect intensity interpolation [km/h]", "0 300 1")]
	float m_fDustStartSpeed;
	
	[Attribute("80", UIWidgets.Slider, "Maximal speed for 100% effect intensity interpolation [km/h]", "0 300 1")]
	float m_fDustTopSpeed;
	
	[Attribute("250", UIWidgets.Slider, "Maximal distance at which the effect is visible", "0 2000 1")]
	float m_fMaxDistanceVisible;
};

class SCR_VehicleDustPerWheel : MultiEffectComponent
{
	static const int							UPDATE_TIMEOUT = 1000; //Minimal delay between two particle swaps.
	
	VehicleWheeledSimulation					m_pVehicleWheeledSimulation;
	private SCR_VehicleDustPerWheelClass		m_pComponentData;
	private RplComponent						m_pRplComponent;
	private ref TVectorArray					m_LocalDustPos;
	private ref TBoolArray 						m_bHasWheelContact;
	ref array<int>								m_aLastSwap;
	
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
		
		m_LocalDustPos = {};
		m_bHasWheelContact = {};
		
		m_pComponentData = SCR_VehicleDustPerWheelClass.Cast(GetComponentData(owner));
		if (!m_pComponentData)
			return;
		
		if (m_pComponentData.m_fDustStartSpeed <= 0)
			return;
		
		if (m_pComponentData.m_fDustStartSpeed > m_pComponentData.m_fDustTopSpeed)
			return;
		
		m_pVehicleWheeledSimulation = VehicleWheeledSimulation.Cast(owner.FindComponent(VehicleWheeledSimulation));
		if (!m_pVehicleWheeledSimulation || !m_pVehicleWheeledSimulation.IsValid())
			return;
		
		m_pRplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		
#ifdef ENABLE_DIAG
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_PARTICLES_VEHICLE_DUST,"","Show dust materials","Vehicles");
		m_aGameMatNames = {};
		m_aGameMatNames.Resize(m_pComponentData.m_aWheels.Count());
#endif
		m_aLastSwap = {};
		m_aLastSwap.Resize(m_pComponentData.m_aWheels.Count());
		
		for(int i = 0; i < m_aLastSwap.Count(); i++)
		{
			m_aLastSwap[i] = 0;
#ifdef ENABLE_DIAG
			m_aGameMatNames[i] = "";
#endif
		}
		
		SetEventMask(owner, EntityEvent.INIT | EntityEvent.FRAME);
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
		
		int count = Math.Min(m_pVehicleWheeledSimulation.WheelCount(), m_pComponentData.m_aWheels.Count());
		ReserveEffects(count);
		m_LocalDustPos.Resize(count);
		m_bHasWheelContact.Resize(count);
	}
	
	//------------------------------------------------------------------------------------------------ 
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		UpdateBatch(owner, timeSlice);
		
		/*if (!HasActiveParticles())
		{
			ClearEventMask(owner, EntityEvent.FRAME);
			return;
		}*/
	}
	
	//------------------------------------------------------------------------------------------------ 
	void UpdateCurrent(ParticleEffectHandle effect, IEntity owner, float timeSlice, int index, float speed)
	{
		IEntity effectEntity = effect.GetEntity();
		effect.Continue();
		
		if (!effectEntity)
			return;
		
		UpdatePosition(effectEntity, index, owner);
		UpdateVehicleDustEffect(effectEntity, speed, m_pComponentData.m_fDustTopSpeed, index, owner);
		effect.Update(timeSlice);
	}
	
    //------------------------------------------------------------------------------------------------ 
	// Called when the effects needs to be updated. This happens per frame if the camera is close to it, but much slower if further away fr optimization purposes.
	override void UpdateEffect(ParticleEffectHandle effect, IEntity owner, float timeSlice, int index)
	{
		IEntity effectEntity = effect.GetEntity();
		
		float speed = m_pVehicleWheeledSimulation.GetSpeedKmh();
		
		vector camMat[4];
		owner.GetWorld().GetCurrentCamera(camMat);
		vector ownerMat[4];
		owner.GetWorldTransform(ownerMat);
		
		// Activate particle effect only within the desired speed and distance
		if (speed >= m_pComponentData.m_fDustStartSpeed && vector.DistanceSq(ownerMat[3], camMat[3]) < m_pComponentData.m_fMaxDistanceVisible * m_pComponentData.m_fMaxDistanceVisible)
		{		
			int ticks = System.GetTickCount();
			int dif = Math.AbsInt(ticks - m_aLastSwap[index]);
			
			if (effectEntity && dif < UPDATE_TIMEOUT)
			{
				UpdateCurrent(effect, owner, timeSlice, index, speed);
				return;
			}
			
			int wheelIdx = m_pComponentData.m_aWheels[index];
			GameMaterial newMaterial;
			
			if (m_pRplComponent && m_pRplComponent.IsRemoteProxy())
			{
				vector worldTransform[4];
				owner.GetPhysics().GetDirectWorldTransform(worldTransform);
				vector worldDir = -worldTransform[1];
				
				TraceParam trace = new TraceParam();
				trace.Start = m_pVehicleWheeledSimulation.WheelGetPosition(wheelIdx, 1.0).Multiply4(worldTransform);
				trace.End = m_pVehicleWheeledSimulation.WheelGetPosition(wheelIdx, 0.0).Multiply4(worldTransform) + worldDir * m_pVehicleWheeledSimulation.WheelGetRadiusState(wheelIdx);
				trace.Flags = TraceFlags.WORLD;
				trace.LayerMask = EPhysicsLayerDefs.Terrain;
				
				float hit = owner.GetWorld().TraceMove(trace, null);
				
				newMaterial = trace.SurfaceProps;
				vector remotePosition = vector.Direction(trace.Start, trace.End) * hit + trace.Start;
				m_LocalDustPos[index] = remotePosition.InvMultiply4(worldTransform);
				m_bHasWheelContact[index] = trace.TraceEnt != null;
						
#ifdef ENABLE_DIAG					
				if(DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_PARTICLES_VEHICLE_DUST))
				{
					vector p[2];
					p[0] = trace.Start;
					p[1] = trace.End;
					Shape.CreateLines(Color.BLUE, ShapeFlags.ONCE | ShapeFlags.NOZBUFFER, p, 2);
					Shape.CreateSphere(Color.RED, ShapeFlags.ONCE | ShapeFlags.NOZBUFFER, remotePosition, 0.05);
				}
#endif
			}
			else
			{
				newMaterial = m_pVehicleWheeledSimulation.WheelGetContactMaterial(wheelIdx);
			}
			
			GameMaterial currentMaterial = effect.GetMaterial();
						
			ResourceName newResource;
			
			if (newMaterial)
			{
				if (newMaterial != currentMaterial)	//different materials -> check resource
				{
					ParticleEffectInfo effectInfo = newMaterial.GetParticleEffectInfo();		 
					if (effectInfo)
						newResource = effectInfo.GetVehicleDustResource(m_pComponentData.m_aVehicleIndex);
				}
			}
#ifdef ENABLE_DIAG
			else if(DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_PARTICLES_VEHICLE_DUST))
			{
				m_aGameMatNames[index] = "";
			}
#endif
			

			//update old effect
			if (effectEntity &&  ( newMaterial == currentMaterial || effect.GetResource() == newResource ))
			{
				if (newMaterial !=  currentMaterial )	//different GameMats, but resources are the same
				{
					effect.SetMaterial(newMaterial);
#ifdef ENABLE_DIAG					
					if(DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_PARTICLES_VEHICLE_DUST))
						m_aGameMatNames[index] = newMaterial.GetName();
#endif
				}	
				UpdateCurrent(effect, owner, timeSlice, index, speed);
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
					if(DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_PARTICLES_VEHICLE_DUST))
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
					UpdatePosition(effectEntity, index, owner);
					UpdateVehicleDustEffect(effectEntity, speed, m_pComponentData.m_fDustTopSpeed, index, owner);
					effect.Update(timeSlice);
				}									
			}
		}
		else	//moving too slow to generate more dust
		{
			effect.Pause();
			if (effect.HasActiveParticles())
			{
				UpdatePosition(effectEntity, index, owner);
				effect.Update(timeSlice);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------ 
	void UpdatePosition(IEntity effectEntity, int index, IEntity owner)
	{
		if (!effectEntity)
			return;
		
		vector position;
		if (m_pRplComponent && m_pRplComponent.IsRemoteProxy())
		{
			vector worldTransform[4];
			owner.GetPhysics().GetDirectWorldTransform(worldTransform);
			position = m_LocalDustPos[index].Multiply4(worldTransform);
		}
		else
		{
			position = m_pVehicleWheeledSimulation.WheelGetContactPosition(m_pComponentData.m_aWheels[index]);
		}
		
		vector mat[4];
		Math3D.MatrixIdentity4(mat);
		mat[3] = position;
		effectEntity.SetWorldTransform(mat);
		
#ifdef ENABLE_DIAG		
		if(DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_PARTICLES_VEHICLE_DUST))
		{
			string currentGM = m_aGameMatNames[index];
			if (!currentGM.IsEmpty())
			{	
				DebugTextWorldSpace.Create(owner.GetWorld(), currentGM, DebugTextFlags.CENTER | DebugTextFlags.ONCE, position[0], position[1], position[2]); 
			}
		}
#endif
	}
	
	//------------------------------------------------------------------------------------------------ 
	void UpdateVehicleDustEffect(IEntity effectEntity, float speed, float end_speed, int index, IEntity owner)
	{
		float speed_coef = 0;
		float birth_coef = 0;
		float gravity_coef = 0;
		float slip = m_pVehicleWheeledSimulation.WheelGetLongitudinalSlip(index);
		
		bool wheelHasContact;
		if (m_pRplComponent && m_pRplComponent.IsRemoteProxy())
		{
			wheelHasContact = m_bHasWheelContact[index];
		}
		else
		{
			wheelHasContact = m_pVehicleWheeledSimulation.WheelHasContact(index);
		}
		
		if (wheelHasContact)
		{
			speed_coef = Math.AbsFloat( 0.2 + speed * 0.8  / end_speed + slip);
			birth_coef = Math.AbsFloat( 0.8 + speed * 0.2  / end_speed + slip * 2);
			gravity_coef = Math.AbsFloat( 0.8 + speed * 0.2 / end_speed );
		}

#ifdef ENABLE_DIAG	
		if(DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_PARTICLES_VEHICLE_DUST))
		{
			vector mat[4];
			owner.GetWorldTransform(mat);
			vector position = mat[3];
			DebugTextWorldSpace.Create(owner.GetWorld(), "slip : " + slip + " wheelHasContact : " + wheelHasContact, DebugTextFlags.CENTER | DebugTextFlags.ONCE, position[0], position[1], position[2]); 
		}
#endif
		
		Particles particles = effectEntity.GetParticles();
		particles.MultParam(-1, EmitterParam.BIRTH_RATE,        birth_coef);	
		particles.MultParam(-1, EmitterParam.GRAVITY_SCALE_RND, gravity_coef);
		particles.MultParam(-1, EmitterParam.VELOCITY,          speed_coef);
		particles.MultParam(-1, EmitterParam.VELOCITY_RND,      speed_coef);
	}
};