enum EVehicleHitZoneGroup : EHitZoneGroup
{
	HULL = 10,
	ENGINE = 20,
	DRIVE_TRAIN = 30,
	FUEL_TANKS = 40,
	WHEELS = 50,
	CARGO = 60,
	AMMO_STORAGE = 70,
	TURRET = 80,
	OPTICS = 90,
	NIGHT_OPTICS = 91,
	VIEWPORT = 100,
	ROTOR_ASSEMBLY = 110,
	TAIL_ROTOR = 111,
	LANDING_GEAR = 120,
	PROPELLER = 130,
	INSTRUMENTS = 140,
	CONTROLS = 150,
	LIGHT = 160,
}

//#define VEHICLE_DAMAGE_DEBUG
//#define VEHICLE_DEBUG_OTHER
class SCR_VehicleDamageManagerComponentClass : SCR_DamageManagerComponentClass
{
	[Attribute("1.5", desc: "Max distance of hitzone, which should receive damage, from contact point.", category: "Collision Damage")]
	protected float m_fMaxSharedDamageDistance;

	[Attribute("0.7", desc: "Damage multiplier for collisions from this side", params: "0.01 1000 0.01", category: "Collision Damage")]
	float m_fFrontMultiplier;

	[Attribute("0.7", desc: "Damage multiplier for collisions from this side", params: "0.01 1000 0.01", category: "Collision Damage")]
	float m_fBottomMultiplier;

	[Attribute("1", desc: "Damage multiplier for collisions from this side", params: "0.01 1000 0.01", category: "Collision Damage")]
	float m_fRearMultiplier;

	[Attribute("1.5", desc: "Damage multiplier for collisions from this side", params: "0.01 1000 0.01", category: "Collision Damage")]
	float m_fLeftMultiplier;

	[Attribute("1.5", desc: "Damage multiplier for collisions from this side", params: "0.01 1000 0.01", category: "Collision Damage")]
	float m_fRightMultiplier;

	[Attribute("2", desc: "Damage multiplier for collisions from this side", params: "0.01 1000 0.01", category: "Collision Damage")]
	float m_fTopMultiplier;

	[Attribute("30", "Speed in km/h over which will occupants be dealt damage in collision", category: "Collision Damage")]
	protected float m_fOccupantsDamageSpeedThreshold;

	[Attribute("100", "Speed in km/h over which will occupants die in collision", category: "Collision Damage")]
	protected float m_fOccupantsSpeedDeath;	

	[Attribute("0 0 0", "Position for frontal impact calculation", category: "Collision Damage", params: "inf inf 0 purpose=coords space=entity coordsVar=m_vFrontalImpact")]
	protected vector m_vFrontalImpact;
	
	[Attribute("1000", "Explosion damage value which, if exceeded, may cause passengers to be ejected from the vehicle", category: "Passenger Ejection")]
	protected int m_iMinExplosionEjectionDamageThreshold;
	
	[Attribute("50", "Collision damage value which, if exceeded, may cause passengers to be ejected from the vehicle", category: "Passenger Ejection")]
	protected int m_iMinCollisionEjectionDamageThreshold;

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetMaxSharedDamageDistance()
	{
		return m_fMaxSharedDamageDistance;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetFrontMultiplier()
	{
		return m_fFrontMultiplier;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetBottomMultiplier()
	{
		return m_fBottomMultiplier;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetRearMultiplier()
	{
		return m_fRearMultiplier;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetLeftMultiplier()
	{
		return m_fLeftMultiplier;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetRightMultiplier()
	{
		return m_fRightMultiplier;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetTopMultiplier()
	{
		return m_fTopMultiplier;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetOccupantsDamageSpeedThreshold()
	{
		return m_fOccupantsDamageSpeedThreshold;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetOccupantsSpeedDeath()
	{
		return m_fOccupantsSpeedDeath;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	vector GetFrontalImpact()
	{
		return m_vFrontalImpact;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	float GetMinExplosionEjectionDamageThreshold()
	{
		return m_iMinExplosionEjectionDamageThreshold;
	}	
	
	//------------------------------------------------------------------------------------------------
	//! \return
	float GetMinCollisionEjectionDamageThreshold()
	{
		return m_iMinCollisionEjectionDamageThreshold;
	}
}

enum SCR_EPhysicsResponseIndex
{
	NORMAL = 0,
	TINY_MOMENTUM = 1,
	SMALL_MOMENTUM = 2,
	MEDIUM_MOMENTUM = 3,
	LARGE_MOMENTUM = 4,
	HUGE_MOMENTUM = 5,
	TINY_DESTRUCTIBLE = 6,
	SMALL_DESTRUCTIBLE = 7,
	MEDIUM_DESTRUCTIBLE = 8,
	LARGE_DESTRUCTIBLE = 9,
	HUGE_DESTRUCTIBLE = 10
}

class SCR_VehicleDamageManagerComponent : SCR_DamageManagerComponent
{
	protected const float MIN_IMPULSE_THRESHOLD = 5;
	protected const int MIN_RESPONSE_INDEX = SCR_EPhysicsResponseIndex.TINY_MOMENTUM;
	protected const int MAX_RESPONSE_INDEX = SCR_EPhysicsResponseIndex.HUGE_MOMENTUM;
	
	protected float m_fVelocityMultiplierOnWaterEnter = 0.5;
	protected int m_fMinWaterFallDamageVelocity = 5;

	static ref map<SCR_EPhysicsResponseIndex, float> s_mResponseIndexMomentumMap = new map<SCR_EPhysicsResponseIndex, float>();

	[Attribute("0", desc: "Print relative force in collisions of this vehicle? Can be used to determine ideal Collision Damage Force Threshold.", category: "Debug")]
	protected bool m_bPrintRelativeForce;

	[Attribute("0", "Frontal impact damage needed to destroy this vehicle\nTakes into account current damage multipliers\nUse context menu on Damage Manager component to compute this value.\n[hp]", category: "Collision Damage")]
	protected float m_fVehicleDestroyDamage;

	[Attribute("15", "Speed of collision that damages the vehicle\n[km/h]", category: "Collision Damage")]
	protected float m_fVehicleDamageSpeedThreshold;

	[Attribute("120", "Speed of collision that destroys the vehicle\n[km/h]", category: "Collision Damage")]
	protected float m_fVehicleSpeedDestroy;

	[Attribute("0.7", "Engine efficiency at which it is considered to be malfunctioning\n[x * 100%]", category: "Vehicle Damage")]
	protected float m_fEngineMalfunctioningThreshold;

	[Attribute(defvalue: "VehicleFireState", desc: "Vehicle parts fire state signal name", category: "Secondary damage")]
	protected string m_sVehicleFireStateSignal;

	[Attribute(defvalue: "FuelTankFireState", desc: "Fuel tank fire signal name", category: "Secondary damage")]
	protected string m_sFuelTankFireStateSignal;

	[Attribute(defvalue: "SuppliesFireState", desc: "Supplies fire signal name", category: "Secondary damage")]
	protected string m_sSuppliesFireStateSignal;

	[Attribute(defvalue: "0.02", desc: "Fuel tank fire damage rate\n[x * 100%]", params: "0 100 0.001", category: "Secondary damage")]
	protected float m_fFuelTankFireDamageRate;

	[Attribute(defvalue: "0.02", desc: "Supplies fire damage rate\n[x * 100%]", params: "0 100 0.001", category: "Secondary damage")]
	protected float m_fSuppliesFireDamageRate;

	[Attribute(defvalue: "1", desc: "Delay between secondary fire damage\n[s]", params: "0 10000 0.1", category: "Secondary damage")]
	protected float m_fSecondaryFireDamageDelay;

	protected bool m_bIsInContact;
	protected float m_fMaxRelativeForce;
	protected float m_fMinImpulse;
	protected SCR_BaseCompartmentManagerComponent m_CompartmentManager;

	protected static ref ScriptInvokerInt s_OnVehicleDestroyed;
	protected static ref ScriptInvoker s_OnVehicleDamageStateChanged;

	//! Common vehicle features that will influence its simulation
	protected ref array<HitZone> m_aVehicleHitZones = {};
	protected CompartmentControllerComponent m_Controller;
	protected VehicleBaseSimulation m_Simulation;

	protected float m_fEngineEfficiency = 1;
	protected bool m_bEngineFunctional = true;

	protected float m_fGearboxEfficiency = 1;
	protected bool m_bGearboxFunctional = true;

	// The vehicle damage manager needs to know about all the burning hitzones that it consists of
	protected ref array<SCR_FlammableHitZone>			m_aFlammableHitZones;
	protected SignalsManagerComponent					m_SignalsManager;

	// Vehicle fire
	protected float									m_fVehicleFireDamageTimeout;

	// Fuel tanks fire
	protected ParticleEffectEntity					m_FuelTankFireParticle; // Burning fuel particle
	protected FuelManagerComponent					m_FuelManager;
	protected float									m_fFuelTankFireDamageTimeout;

	// Supplies fire
	protected ParticleEffectEntity					m_SuppliesFireParticle; // Burning supplies particle
	protected float									m_fSuppliesFireDamageTimeout;

	// Audio features
	protected int									m_iVehicleFireStateSignalIdx;
	protected int									m_iFuelTankFireStateSignalIdx;
	protected int									m_iSuppliesFireStateSignalIdx;
	
	// Impact FX
	protected SCR_ImpactEffectComponent m_ImpactEffectComponent;

	[RplProp(onRplName: "OnVehicleFireStateChanged")]
	SCR_ESecondaryExplosionScale					m_eVehicleFireState;

	[RplProp(onRplName: "OnFuelTankFireStateChanged")]
	SCR_ESecondaryExplosionScale					m_eFuelTankFireState;

	[RplProp(onRplName: "OnSuppliesFireStateChanged")]
	SCR_ESecondaryExplosionScale					m_eSuppliesFireState;

	[RplProp()]
	protected vector								m_vVehicleFireOrigin;

	[RplProp()]
	protected vector								m_vFuelTankFireOrigin;

	[RplProp()]
	protected vector								m_vSuppliesFireOrigin;

	// Sound
	protected static const float APPROXIMATE_CHARACTER_LETHAL_DAMAGE = 150;

#ifdef VEHICLE_DAMAGE_DEBUG
	protected ref array<ref Shape> m_aDebugShapes = {};
#endif
#ifdef VEHICLE_DEBUG_OTHER
	protected ref array<ref Shape> m_aDebugShapes = {};
#endif

	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_VehicleDamageManagerComponentClass GetPrefabData()
	{
		return SCR_VehicleDamageManagerComponentClass.Cast(GetComponentData(GetOwner()));
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetTopMultiplier()
	{
		SCR_VehicleDamageManagerComponentClass prefabData = GetPrefabData();
		if (!prefabData)
			return 2;

		return prefabData.GetTopMultiplier();
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetRightMultiplier()
	{
		SCR_VehicleDamageManagerComponentClass prefabData = GetPrefabData();
		if (!prefabData)
			return 1.5;

		return prefabData.GetRightMultiplier();
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetLeftMultiplier()
	{
		SCR_VehicleDamageManagerComponentClass prefabData = GetPrefabData();
		if (!prefabData)
			return 1.5;

		return prefabData.GetLeftMultiplier();
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetRearMultiplier()
	{
		SCR_VehicleDamageManagerComponentClass prefabData = GetPrefabData();
		if (!prefabData)
			return 1;

		return prefabData.GetRearMultiplier();
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetBottomMultiplier()
	{
		SCR_VehicleDamageManagerComponentClass prefabData = GetPrefabData();
		if (!prefabData)
			return 0.7;

		return prefabData.GetBottomMultiplier();
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetFrontMultiplier()
	{
		SCR_VehicleDamageManagerComponentClass prefabData = GetPrefabData();
		if (!prefabData)
			return 0.7;

		return prefabData.GetFrontMultiplier();
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetMaxSharedDamageDistance()
	{
		SCR_VehicleDamageManagerComponentClass prefabData = GetPrefabData();
		if (!prefabData)
			return 1.5;

		return prefabData.GetMaxSharedDamageDistance();
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetOccupantsDamageSpeedThreshold()
	{
		SCR_VehicleDamageManagerComponentClass prefabData = GetPrefabData();
		if (!prefabData)
			return 30;

		return prefabData.GetOccupantsDamageSpeedThreshold();
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetOccupantsSpeedDeath()
	{
		SCR_VehicleDamageManagerComponentClass prefabData = GetPrefabData();
		if (!prefabData)
			return 80;

		return prefabData.GetOccupantsSpeedDeath();
	}	
	
	//------------------------------------------------------------------------------------------------
	//! \return
	int GetMinCollisionDamageEjectionThreshold()
	{
		SCR_VehicleDamageManagerComponentClass prefabData = GetPrefabData();
		if (!prefabData)
			return 50;

		return prefabData.GetMinCollisionEjectionDamageThreshold();
	}	
	
	//------------------------------------------------------------------------------------------------
	//! \return
	int GetMinExplosionDamageEjectionThreshold()
	{
		SCR_VehicleDamageManagerComponentClass prefabData = GetPrefabData();
		if (!prefabData)
			return 1000;

		return prefabData.GetMinExplosionEjectionDamageThreshold();
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	vector GetFrontalImpact()
	{
		SCR_VehicleDamageManagerComponentClass prefabData = GetPrefabData();
		if (!prefabData)
			return "0 1 1";

		return prefabData.GetFrontalImpact();
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] side
	//! \return
	float GetSideDamageMultiplier(SCR_EBoxSide side)
	{
		switch (side)
		{
			case SCR_EBoxSide.FRONT:
				return GetFrontMultiplier();
			case SCR_EBoxSide.REAR:
				return GetRearMultiplier();
			case SCR_EBoxSide.BOTTOM:
				return GetBottomMultiplier();
			case SCR_EBoxSide.TOP:
				return GetTopMultiplier();
			case SCR_EBoxSide.LEFT:
				return GetLeftMultiplier();
			case SCR_EBoxSide.RIGHT:
				return GetRightMultiplier();

		}

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	bool IsInContact()
	{
		return m_bIsInContact;
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		Vehicle vehicle = Vehicle.Cast(owner);
		if (!vehicle)
			return;

		m_CompartmentManager = SCR_BaseCompartmentManagerComponent.Cast(vehicle.FindComponent(SCR_BaseCompartmentManagerComponent));
		m_Controller = CompartmentControllerComponent.Cast(owner.FindComponent(CompartmentControllerComponent));
		m_Simulation = VehicleBaseSimulation.Cast(owner.FindComponent(VehicleBaseSimulation));
		m_FuelManager = FuelManagerComponent.Cast(owner.FindComponent(FuelManagerComponent));
		m_SignalsManager = SignalsManagerComponent.Cast(owner.FindComponent(SignalsManagerComponent));
		m_ImpactEffectComponent = SCR_ImpactEffectComponent.Cast(owner.FindComponent(SCR_ImpactEffectComponent));

		if (m_SignalsManager)
		{
			m_iVehicleFireStateSignalIdx = m_SignalsManager.AddOrFindSignal(m_sVehicleFireStateSignal);
			m_iFuelTankFireStateSignalIdx = m_SignalsManager.AddOrFindSignal(m_sFuelTankFireStateSignal);
			m_iSuppliesFireStateSignalIdx = m_SignalsManager.AddOrFindSignal(m_sSuppliesFireStateSignal);
		}
		
		SCR_VehicleBuoyancyComponent vehicleBuoyancy = SCR_VehicleBuoyancyComponent.Cast(vehicle.FindComponent(SCR_VehicleBuoyancyComponent));
		if (vehicleBuoyancy)
			vehicleBuoyancy.GetOnWaterEnter().Insert(OnWaterEnter);

		UpdateVehicleState();

		RplComponent rpl = vehicle.GetRplComponent();
		if (rpl && rpl.IsProxy())
			return;

		Physics physics = owner.GetPhysics();
		if (physics)
			m_fMinImpulse = physics.GetMass() * 2;

		if (m_fMinImpulse > 0)
			SetEventMask(owner, EntityEvent.CONTACT);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] hitZone
	// Register simulation feature hit zone
	void RegisterVehicleHitZone(notnull HitZone hitZone)
	{
		// Remove the hitzone if it was already registered
		if (!m_aVehicleHitZones.Contains(hitZone))
			m_aVehicleHitZones.Insert(hitZone);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] hitZone
	// Register simulation feature hit zone
	void UnregisterVehicleHitZone(HitZone hitZone)
	{
		m_aVehicleHitZones.RemoveItem(hitZone);
	}

	//------------------------------------------------------------------------------------------------
	//! Compute current simulation state of vehicle
	//! Called when hit zone damage states change
	void UpdateVehicleState()
	{
		int engineCount;
		float engineEfficiency;
		bool engineFunctional;

		int gearboxCount;
		float gearboxEfficiency;
		bool gearboxFunctional;

		SCR_EngineHitZone engineHitZone;
		SCR_GearboxHitZone gearboxHitZone;

		foreach (HitZone hitZone : m_aVehicleHitZones)
		{
			engineHitZone = SCR_EngineHitZone.Cast(hitZone);
			if (engineHitZone)
			{
				engineCount++;
				engineEfficiency += engineHitZone.GetEfficiency();

				if (engineHitZone.GetDamageState() != EDamageState.DESTROYED)
					engineFunctional = true;

				continue;
			}

			gearboxHitZone = SCR_GearboxHitZone.Cast(hitZone);
			if (gearboxHitZone)
			{
				gearboxCount++;
				gearboxEfficiency += gearboxHitZone.GetEfficiency();

				if (gearboxHitZone.GetDamageState() != EDamageState.DESTROYED)
					gearboxFunctional = true;

				continue;
			}
		}

		if (engineCount > 0)
		{
			SetEngineFunctional(engineFunctional);
			SetEngineEfficiency(engineEfficiency / engineCount);
		}

		if (gearboxCount > 0)
		{
			SetGearboxFunctional(gearboxFunctional);
			SetGearboxEfficiency(gearboxEfficiency / gearboxCount);
		}

		UpdateMovementDamage();
	}

	//------------------------------------------------------------------------------------------------
	//!
	void UpdateMovementDamage()
	{
		float movementDamage;

		if (!m_bEngineFunctional)
			movementDamage = 1;
		else
			movementDamage = 1 - (m_fGearboxEfficiency * m_fEngineEfficiency);

		SetMovementDamage(movementDamage);
		
		VehicleControllerComponent controller = VehicleControllerComponent.Cast(m_Controller);
		if (controller)
			controller.SetCanMove(movementDamage < 1);
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	bool GetEngineFunctional()
	{
		return m_bEngineFunctional;
	}

	//------------------------------------------------------------------------------------------------
	protected void SetEngineFunctional(bool functional)
	{
		m_bEngineFunctional = functional;

		if (functional)
			return;

		VehicleControllerComponent controller = VehicleControllerComponent.Cast(m_Controller);
		if (controller && controller.IsEngineOn())
			controller.StopEngine(false);
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetEngineMalfunctionThreshold()
	{
		return m_fEngineMalfunctioningThreshold;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetEngineEfficiency()
	{
		return m_fEngineEfficiency;
	}

	//------------------------------------------------------------------------------------------------
	protected void SetEngineEfficiency(float efficiency)
	{
		m_fEngineEfficiency = efficiency;

		VehicleWheeledSimulation simulation = VehicleWheeledSimulation.Cast(m_Simulation);
		if (simulation && simulation.IsValid())
		{
			simulation.EngineSetPeakTorqueState(efficiency * simulation.EngineGetPeakTorque());
			simulation.EngineSetPeakPowerState(efficiency * simulation.EngineGetPeakPower());
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void SetGearboxFunctional(bool functional)
	{
		m_bGearboxFunctional = functional;

		VehicleControllerComponent controller = VehicleControllerComponent.Cast(m_Controller);
		if (controller)
			controller.SetCanMove(functional);
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	bool GetGearboxFunctional()
	{
		return m_bGearboxFunctional;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetGearboxEfficiency()
	{
		return m_fGearboxEfficiency;
	}

	//------------------------------------------------------------------------------------------------
	protected void SetGearboxEfficiency(float efficiency)
	{
		m_fGearboxEfficiency = efficiency;

		VehicleWheeledSimulation simulation = VehicleWheeledSimulation.Cast(m_Simulation);
		if (simulation && simulation.IsValid())
			simulation.GearboxSetEfficiencyState(efficiency * simulation.GearboxGetEfficiency());
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] position
	//! \param[in] damage
	//! \param[in] damageType
	//! \return
	float DamageSurroundingHitzones(vector position, float damage, EDamageType damageType)
	{
		Physics physics = GetOwner().GetPhysics();
		if (!physics)
			return damage;

		array<HitZone> hitzones = {};

		int count = GetAllHitZonesInHierarchy(hitzones);
		float maxSharedDamageDistance = GetMaxSharedDamageDistance();
		float maxDistanceSq = maxSharedDamageDistance * maxSharedDamageDistance; //SQUARE it for faster calculations of distance

		array<string> hitzoneColliderNames = {};
		vector closestPosition;
		int colliderCount;
		float distancePercent;
		float currentDistance;
		float minDistance;
		int geomIndex;
		vector mat[4];
		HitZone hitzone;
		int hitzonesInRangeCount;
		float hitzonesDistancePercentSum;
		HitZoneContainerComponent hitzoneContainer;
		IEntity hitzoneParentEntity;
		vector mins, maxs;
		vector center;

		map<HitZone, float> hitzoneDistancePercentMap = new map<HitZone, float>();
		for (int i = count - 1; i >= 0; i--)
		{
			hitzone = hitzones[i];
			minDistance = float.MAX;
			colliderCount = hitzone.GetAllColliderNames(hitzoneColliderNames); //The array is cleared inside the GetAllColliderNames method

			if (colliderCount == 0)
			{
				hitzoneContainer = hitzone.GetHitZoneContainer();
				if (!hitzoneContainer || hitzoneContainer == this)
					continue;

				hitzoneParentEntity = hitzoneContainer.GetOwner();
				hitzoneParentEntity.GetBounds(mins, maxs);

				for (int j = 0; j < 3; j++)
					center[j] = mins[j] + Math.AbsFloat(((maxs[j] - mins[j]) * 0.5));

				minDistance = vector.DistanceSq(position, hitzoneParentEntity.CoordToParent(center));
			}
			else
			{
				for (int y = colliderCount - 1; y >= 0; y--)
				{
					geomIndex = physics.GetGeom(hitzoneColliderNames[y]);
					if (geomIndex == -1)
						continue;

					physics.GetGeomWorldTransform(geomIndex, mat);
					currentDistance = vector.DistanceSq(position, mat[3]);

					if (currentDistance < minDistance)
					{
						minDistance = currentDistance;
						closestPosition = mat[3];
					}
				}
			}

			if (minDistance > maxDistanceSq)
				continue;

			minDistance = Math.Sqrt(minDistance);
			distancePercent = 1 - minDistance / GetMaxSharedDamageDistance();

			hitzonesInRangeCount++;
			hitzonesDistancePercentSum += distancePercent;
			hitzoneDistancePercentMap.Insert(hitzone, distancePercent);
		}

		float leftoverDamage = damage;
		float currentDamage;
		float currentDamagePercent;
		float hitzoneHealth;
		float damageMultiplier;

		DamageManagerComponent damageManager;

		vector empty[3];
		empty[0] = vector.Zero;
		empty[1] = vector.Zero;
		empty[2] = vector.Zero;

		for (int i = hitzonesInRangeCount - 1; i >= 0; i--)
		{
			hitzone = hitzoneDistancePercentMap.GetKey(i);
			distancePercent = hitzoneDistancePercentMap.Get(hitzone);
			currentDamagePercent = distancePercent / hitzonesDistancePercentSum;

			currentDamage = currentDamagePercent * damage;

			//TODO @Vojta LEVEL2: This currentDamage calculation that is being done here is automatically done by the HandleDamage.
			//This means that we are applying damage reduction twice.
			//Also you are dividing with the damage multiplier, making it the opposite effect.

			hitzoneHealth = hitzone.GetHealth();

			damageMultiplier = hitzone.GetDamageMultiplier(damageType) * hitzone.GetBaseDamageMultiplier();
			if (damageMultiplier != 0)
				currentDamage = Math.Clamp(currentDamage, 0, (hitzoneHealth + hitzone.GetDamageReduction()) / damageMultiplier);

			damageManager = DamageManagerComponent.Cast(hitzone.GetHitZoneContainer());
			if (!damageManager)
				continue;

			SCR_DamageContext damageContext = new SCR_DamageContext(damageType, currentDamage, empty, GetOwner(), hitzone, null, null, -1, -1);
			damageManager.HandleDamage(damageContext);

			leftoverDamage -= currentDamage;
		}

		leftoverDamage = Math.Clamp(leftoverDamage, 0, float.MAX);

		return leftoverDamage;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] position
	//! \return
	// TODO: This won't work properly for all the types of objects - E.G. flat boats with huge towers on them
	// e.g x = collision point, this will say it got hit from the back, because it's inside the rear-side pyramid
	//       ||
	// __x___||_______
	// \             /
	//  \           /
	// /TODO
	// To trigger this, just crash a vehicle into something
	// Make sure the vehicle has SCR_VehicleDamageManagerComponent with attribute "Print Relative Force" set to true.
	SCR_EBoxSide GetHitDirection(vector position)
	{
		IEntity owner = GetOwner();
		//Get local space contact position
		vector contact = owner.CoordToLocal(position);

		//Get local space mins and maxs
		vector mins, maxs;
		owner.GetBounds(mins, maxs);

		//Calculate bbox center
		vector bboxCenter = (maxs - mins) * 0.5 + mins;

		//Get contact position relative to the bbox center, for correct dot product calculations later
		contact = contact - bboxCenter;

		//Here we calculate all the normals of our 6 planes as well as their distances, but these broke the code completely, so I commented them out.
		//Naming: XZA = plane A perpendicular to the X-Z plane and so on...
		vector normalXZA, normalXZB, normalYZA, normalYZB, normalXYA, normalXYB;
		float distanceXZA, distanceXZB, distanceYZA, distanceYZB, distanceXYA, distanceXYB;

		normalYZA = Vector(0, mins[2] - maxs[2], maxs[1] - mins[1]).Normalized();
		distanceYZA = vector.Dot(normalYZA, Vector(0, mins[1], mins[2]));

		normalYZB = Vector(0, mins[2] - maxs[2], mins[1] - maxs[1]).Normalized();
		distanceYZB = vector.Dot(normalYZB, Vector(0, mins[1], mins[2]));

		normalXZA = Vector(maxs[2] - mins[2], 0, mins[0] - maxs[0]).Normalized();
		distanceXZA = vector.Dot(normalXZA, Vector(mins[0], 0, mins[2]));

		normalXZB = Vector(mins[2] - maxs[2], 0, mins[0] - maxs[0]).Normalized();
		distanceXZB = vector.Dot(normalXZB, Vector(mins[0], 0, mins[2]));

		normalXYA = Vector(maxs[1] - mins[1], mins[0] - maxs[0], 0).Normalized();
		distanceXYA = vector.Dot(normalXYA, Vector(mins[0], mins[1], 0));

		normalXYB = Vector(mins[1] - maxs[1], mins[0] - maxs[0], 0).Normalized();
		distanceXYB = vector.Dot(normalXYB, Vector(mins[0], mins[1], 0));

#ifdef VEHICLE_DAMAGE_DEBUG
		//Debug shapes
		//Here we just draw all the fancy stuff
		m_aDebugShapes.Clear();

		m_aDebugShapes.Insert(Shape.Create(ShapeType.BBOX, ARGB(255, 0, 255, 255), ShapeFlags.NOZBUFFER | ShapeFlags.WIREFRAME, owner.CoordToParent(mins), owner.CoordToParent(maxs)));
		m_aDebugShapes.Insert(Shape.CreateSphere(ARGB(255, 255, 0, 0), ShapeFlags.NOZBUFFER | ShapeFlags.WIREFRAME, position, 0.2));

		vector p[2];
		p[0] = owner.CoordToParent(bboxCenter);
		p[1] = owner.CoordToParent(normalXZA);
		m_aDebugShapes.Insert(Shape.CreateLines(ARGB(255, 255, 0, 0), ShapeFlags.NOZBUFFER, p, 2));

		p[1] = owner.CoordToParent(normalXZB);
		m_aDebugShapes.Insert(Shape.CreateLines(ARGB(255, 0, 255, 0), ShapeFlags.NOZBUFFER, p, 2));

		p[1] = owner.CoordToParent(normalYZA);
		m_aDebugShapes.Insert(Shape.CreateLines(ARGB(255, 0, 0, 255), ShapeFlags.NOZBUFFER, p, 2));

		p[1] = owner.CoordToParent(normalYZB);
		m_aDebugShapes.Insert(Shape.CreateLines(ARGB(255, 255, 0, 255), ShapeFlags.NOZBUFFER, p, 2));

		p[1] = owner.CoordToParent(normalXYA);
		m_aDebugShapes.Insert(Shape.CreateLines(ARGB(255, 0, 255, 255), ShapeFlags.NOZBUFFER, p, 2));

		p[1] = owner.CoordToParent(normalXYB);
		m_aDebugShapes.Insert(Shape.CreateLines(ARGB(255, 0, 0, 0), ShapeFlags.NOZBUFFER, p, 2));

		p[1] = owner.CoordToParent(contact);
		m_aDebugShapes.Insert(Shape.CreateLines(ARGB(255, 255, 155, 255), ShapeFlags.NOZBUFFER, p, 2));
#endif
#ifdef VEHICLE_DEBUG_OTHER
		//This is a temporary solution, TODO: PROPER DEBUG
		//Do not remove the commented scripts!
		m_aDebugShapes.Clear();
		vector p[2];

		vector globalMins, globalMaxs;
		globalMins = owner.CoordToParent(mins);
		globalMaxs = owner.CoordToParent(maxs);

		m_aDebugShapes.Insert(Shape.CreateSphere(ARGB(255, 255, 0, 0), ShapeFlags.NOZBUFFER | ShapeFlags.WIREFRAME, position, 0.2));

		/*p[0] = globalMins;
		p[1] = globalMaxs;
		m_aDebugShapes.Insert(Shape.CreateLines(ARGB(255, 255, 0, 0), ShapeFlags.NOZBUFFER, p, 2));

		p[0][0] = globalMaxs[0];
		p[1][0] = globalMins[0];
		m_aDebugShapes.Insert(Shape.CreateLines(ARGB(255, 255, 0, 0), ShapeFlags.NOZBUFFER, p, 2));

		p[0] = globalMins;
		p[0][1] = globalMaxs[1];
		p[1] = globalMaxs;
		p[1][1] = globalMins[1];
		m_aDebugShapes.Insert(Shape.CreateLines(ARGB(255, 255, 0, 0), ShapeFlags.NOZBUFFER, p, 2));

		p[0] = globalMins;
		p[0][2] = globalMaxs[2];
		p[1] = globalMaxs;
		p[1][2] = globalMins[2];
		m_aDebugShapes.Insert(Shape.CreateLines(ARGB(255, 255, 0, 0), ShapeFlags.NOZBUFFER, p, 2)); */

		//Corner lines
		p[0] = globalMins;
		p[1] = p[0];
		p[1][1] = globalMaxs[1];
		m_aDebugShapes.Insert(Shape.CreateLines(ARGB(255, 255, 0, 0), ShapeFlags.NOZBUFFER, p, 2));

		p[0] = globalMins;
		p[0][0] = globalMaxs[0];
		p[1] = p[0];
		p[1][1] = globalMaxs[1];
		m_aDebugShapes.Insert(Shape.CreateLines(ARGB(255, 255, 0, 0), ShapeFlags.NOZBUFFER, p, 2));

		p[0] = globalMaxs;
		p[1] = p[0];
		p[1][1] = globalMins[1];
		m_aDebugShapes.Insert(Shape.CreateLines(ARGB(255, 255, 0, 0), ShapeFlags.NOZBUFFER, p, 2));

		p[0] = globalMaxs;
		p[0][0] = globalMins[0];
		p[1] = p[0];
		p[1][1] = globalMins[1];
		m_aDebugShapes.Insert(Shape.CreateLines(ARGB(255, 255, 0, 0), ShapeFlags.NOZBUFFER, p, 2));

		vector mat[4];
		Math3D.MatrixIdentity4(mat);
		//Top pyramid
		p[0] = mins;
		p[0][1] = maxs[1] * 0.5;
		p[1] = maxs;
		Shape shape = Shape.Create(ShapeType.PYRAMID, ARGB(255, 0, 255, 0), ShapeFlags.NOZBUFFER, p[1], p[0]);
		m_aDebugShapes.Insert(shape);
		mat[3] = owner.GetOrigin();
		shape.SetMatrix(mat);

		//Bottom pyramid
		p[0] = mins;
		p[1] = maxs;
		p[1][1] = p[1][1] * 0.5;
		shape = Shape.Create(ShapeType.PYRAMID, ARGB(255, 0, 255, 0), ShapeFlags.NOZBUFFER, p[0], p[1]);
		m_aDebugShapes.Insert(shape);
		shape.SetMatrix(mat);

		//Right pyramid
		/*p[0][0] = maxs[1];
		p[0][1] = mins[0];
		p[0][2] = mins[2];
		p[1][0] = mins[1];
		p[1][1] = maxs[0];
		p[1][2] = maxs[2];
		p[0] = maxs;
		p[1] = mins;
		p[1][0] = mins[0]; */
		/*shape = Shape.Create(ShapeType.PYRAMID, ARGB(255, 0, 255, 0), ShapeFlags.NOZBUFFER, p[1], p[0]);
		m_aDebugShapes.Insert(shape);
		mat[0] = -vector.Up;
		mat[1] = vector.Right;
		mat[2] = vector.Forward;
		mat[3] = owner.GetOrigin() + maxs;
		shape.SetMatrix(mat);
		Math3D.MatrixIdentity4(mat); */

		/*mat[0] = vector.Up;
		mat[1] = -vector.Right;
		mat[2] = vector.Forward; */
#endif

		//Calculating where the contact point is relative to the plane by calculating the dot product of the contact point and the planes normal
		bool XZA, XZB, YZA, YZB, XYA, XYB;
		XZA = (vector.Dot(contact, normalXZA) > 0) - distanceXZA;
		XZB = (vector.Dot(contact, normalXZB) > 0) - distanceXZB;
		YZA = (vector.Dot(contact, normalYZA) > 0) - distanceYZA;
		YZB = (vector.Dot(contact, normalYZB) > 0) - distanceYZB;
		XYA = (vector.Dot(contact, normalXYA) > 0) - distanceXYA;
		XYB = (vector.Dot(contact, normalXYB) > 0) - distanceXYB;

		//XZA = Front, right, bottom, top
		//XZB = Rear, right, bottom, top
		//YZA = Front, bottom, right, left
		//YZB = Rear, bottom, right, left
		//XYA = Front, Rear, top, left
		//XYB = Front, Rear, top, right
		//			XZA	XZB	YZA	YZB	XYA	XYB
		//Top	=	 - 	 -	 0	 0	 0	 0
		//Bottom=	 -	 -	 1	 1	 1	 1
		//Left	=	 0	 1	 -	 -	 0	 1
		//Right	=	 1	 0	 -	 -	 1	 0
		//Front =	 0	 0	 1	 0	 -	 -
		//Rear	=	 1	 1 	 0	 1	 -	 -

		//Right
		if (XZA && !XZB && XYA && !XYB)
			return SCR_EBoxSide.RIGHT;

		//Left
		if (!XZA && XZB && !XYA && XYB)
			return SCR_EBoxSide.LEFT;

		//Bottom
		if (YZA && YZB && XYA && XYB)
			return SCR_EBoxSide.BOTTOM;

		//Top
		if (!YZA && !YZB && !XYA && !XYB)
			return SCR_EBoxSide.TOP;

		//Front
		if (!XZA && !XZB && YZA && !YZB)
			return SCR_EBoxSide.FRONT;

		//Rear
		if (XZA && XZB && !YZA && YZB)
			return SCR_EBoxSide.REAR;

		return SCR_EBoxSide.FRONT;
	}

#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	override array<ref WB_UIMenuItem> _WB_GetContextMenuItems(IEntity owner)
	{
		array<ref WB_UIMenuItem> items = { new WB_UIMenuItem("Compute collision damage", 0) };

		return items;
	}

	//------------------------------------------------------------------------------------------------
	override void _WB_OnContextMenu(IEntity owner, int id)
	{
		switch (id)
		{
			case 0:
			{
				GenericEntity entity = GenericEntity.Cast(owner);
				if (!entity)
					return;

				WorldEditorAPI api = entity._WB_GetEditorAPI();
				if (!api)
					return;

				array<HitZone> hitzones;
				int count = GetSurroundingHitzones(GetOwner().CoordToParent(GetFrontalImpact()), GetOwner().GetPhysics(), GetMaxSharedDamageDistance(), hitzones);
				hitzones.Insert(GetDefaultHitZone());
				count++;
				float damage = GetMinDestroyDamage(EDamageType.COLLISION, hitzones, count);

				// Damage is not valid - warn the user!
				if (damage < 0)
				{
					Print("Cannot destroy selected vehicle on collision", LogLevel.WARNING);
					return;
				}

				float newFrontMultiplier = GetFrontMultiplier();
				float targetFrontalDamage = Math.Ceil(damage / newFrontMultiplier);

				api.BeginEntityAction();

				IEntitySource ownerSource = api.EntityToSource(owner);
				IEntityComponentSource componentSource = SCR_BaseContainerTools.FindComponentSource(ownerSource, Type().ToString());

				if (componentSource && componentSource.Set("m_fVehicleDestroyDamage", targetFrontalDamage.ToString()))
					Print("Entity instance's m_fVehicleDestroyDamage set to " + targetFrontalDamage.ToString(), LogLevel.WARNING);
				else
					Print("Error setting m_fVehicleDestroyDamage set to " + targetFrontalDamage.ToString(), LogLevel.ERROR);

				api.EndEntityAction();
			}
		}
	}
#endif
	
	//------------------------------------------------------------------------------------------------
	//! Determine whether collision into water is severe enough to apply damage, then get velocities an manually call CollisionDamage
	protected void OnWaterEnter()
	{
		Physics physics = GetOwner().GetPhysics();
		if (!physics)
			return;

		// Don't try to apply waterFallDamage when going less than X m/s
		float impactSpeed = Math.AbsFloat(physics.GetVelocity()[1]);
		if (impactSpeed < m_fMinWaterFallDamageVelocity)
			return;
		
		vector ownerTransform = GetOwner().GetOrigin();
		vector velocityBefore = Vector(0, GetOwner().GetPhysics().GetVelocity()[1], 0);
		vector velocityAfter = velocityBefore * m_fVelocityMultiplierOnWaterEnter;
		CollisionDamage(GetGame().GetWorldEntity().GetTerrain(0, 0), 0, velocityBefore, velocityAfter, vector.Zero, vector.Zero, ownerTransform, vector.Zero);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Filter out any contacts under a reasonable speed for damage
	override bool FilterContact(IEntity owner, IEntity other, Contact contact)
	{
		// Returning when impulse is less than 5 doesn't impact gameplay at all, but saves a lot of OnFilteredContact calls
		if (contact.Impulse < MIN_IMPULSE_THRESHOLD)
			return false;
		
		return super.FilterContact(owner, other, contact);;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Handle damage and collision effects of contact
	//! Contact must NEVER be passed as variable to external functions
	override void OnFilteredContact(IEntity owner, IEntity other, Contact contact)
	{
		if (m_ImpactEffectComponent && other)
			m_ImpactEffectComponent.OnImpact(other, contact.Impulse, contact.Position, contact.Normal, contact.Material2, contact.VelocityBefore1, contact.VelocityAfter1);
		
		if (contact.Impulse > m_fMinImpulse)
			CollisionDamage(other, contact.Impulse, contact.VelocityBefore1, contact.VelocityAfter1, contact.VelocityBefore2, contact.VelocityAfter2, contact.Position, contact.Normal);
	}

	//------------------------------------------------------------------------------------------------
	//! Collisiondamage is exposed to be used by other scripts as well.
	//! \param other Other is other entity involved in collision with GetOwner()
	//! \param impulse Impulse applied to resolve the collision
	//! \param velocityBefore1 Velocity of Object1 before collision (world space)
	//! \param velocityAfter1 Velocity of Object1 after collision (world space)
	//! \param velocityBefore2 Velocity of Object2 before collision (world space)
	//! \param velocityAfter2 Velocity of Object2 after collision (world space)
	//! \param impactNormal Collision axis at the contact point
	//! \param impactPosition Position of the contact point (world space)
	void CollisionDamage(notnull IEntity other, float impulse, vector velocityBefore1, vector velocityAfter1, vector velocityBefore2, vector velocityAfter2, vector impactPosition, vector impactNormal)
	{
		// This data can be moved back to component
		SCR_VehicleDamageManagerComponentClass prefabData = GetPrefabData();
		if (!prefabData)
			return;

		// Get the physics of the dynamic object (if owner is static we ignore the collision)
		Physics ownerPhysics = GetOwner().GetPhysics();
		if (!ownerPhysics.IsDynamic())
		{
			m_bIsInContact = false;
			return;
		}

		Physics otherPhysics = other.GetPhysics();
		if (!otherPhysics)
			return;

		// Now get the relative force, which is the impulse divided by the mass of the dynamic object
		float relativeForce = impulse / ownerPhysics.GetMass();
		// We keep relative force temporarily, until we re-calculate it to momentum

		// We hit a destructible that will break, static object -> deal no damage to vehicle or occupants
		int ownerResponseIndex = ownerPhysics.GetResponseIndex();
		int otherResponseIndex = otherPhysics.GetResponseIndex();
		if (!otherPhysics.IsDynamic() && other.FindComponent(SCR_DestructionDamageManagerComponent) && otherResponseIndex - MIN_DESTRUCTION_RESPONSE_INDEX <= ownerResponseIndex - MIN_MOMENTUM_RESPONSE_INDEX)
			return;

		float ownerMass = GetOwner().GetPhysics().GetMass();
		float otherMass = otherPhysics.GetMass();

		// Store information about being in contact - so we don't delete physics objects
		m_bIsInContact = true;

#ifdef DISABLE_VEHICLE_COLLISION_DAMAGE
		m_bIsInContact = false;
		return;
#endif

		//TODOv Pre-calculate these values into prefab data, when callback of prefab data cretion is added.
		float momentumVehicleThreshold = ownerMass * m_fVehicleDamageSpeedThreshold * Physics.KMH2MS;
		float momentumVehicleDestroy = ownerMass * m_fVehicleSpeedDestroy * Physics.KMH2MS;
		float damageScaleToVehicle = m_fVehicleDestroyDamage / (momentumVehicleDestroy - momentumVehicleThreshold);
		//TODO^

		// Debug stuff
		if (m_bPrintRelativeForce && relativeForce > m_fMaxRelativeForce)
		{
			m_fMaxRelativeForce = relativeForce;
			Print(impulse, LogLevel.DEBUG);
			Print(velocityBefore1, LogLevel.DEBUG);
			Print(m_fMaxRelativeForce, LogLevel.DEBUG);
		}

		float damageShare = 1;
		if (otherMass > 0) // mass == 0 is probably a static object
			damageShare -= ownerMass / (ownerMass + otherMass);

		float DotMultiplier = vector.Dot(velocityAfter1.Normalized(), velocityBefore1.Normalized());
		float MomentumBefore = ownerMass * velocityBefore1.Length();
		float MomentumAfter = ownerMass * velocityAfter1.Length() * DotMultiplier;
		float momentumA = Math.AbsFloat(MomentumBefore - MomentumAfter);

		DotMultiplier = vector.Dot(velocityAfter2.Normalized(), velocityBefore2.Normalized());
		MomentumBefore = otherMass * velocityBefore2.Length();
		MomentumAfter = otherMass * velocityAfter2.Length() * DotMultiplier;
		float momentumB = Math.AbsFloat(MomentumBefore - MomentumAfter);

		float collisionDamage = damageScaleToVehicle * (momentumA + momentumB - momentumVehicleThreshold);
		IEntity instigatorEntity;

		// Find compartment manager to damage occupants
		if (m_CompartmentManager)
		{
			//TODOv Pre-calculate these values into prefab data, when callback of prefab data cretion is added.
			float momentumOccupantsThreshold = ownerMass * GetOccupantsDamageSpeedThreshold() * Physics.KMH2MS;
			float momentumOccupantsDeath = ownerMass * GetOccupantsSpeedDeath() * Physics.KMH2MS;
			float damageScaleToCharacter = APPROXIMATE_CHARACTER_LETHAL_DAMAGE / (momentumOccupantsDeath - momentumOccupantsThreshold);
			//TODO^

			// This is the momentum, which will be transferred to damage
			float momentumOverOccupantsThreshold = (momentumA + momentumB) * damageShare - momentumOccupantsThreshold;

			// Deal damage if it's more that 0
			if (momentumOverOccupantsThreshold > 0)
			{
				//If the entity is dynamic, we need to look if the entity has a driver or not.
				//If the vehicle's speed is bigger than m_fVehicleDamageSpeedThreshold
				if (otherPhysics.IsDynamic() && velocityBefore1.LengthSq() > m_fVehicleDamageSpeedThreshold * m_fVehicleDamageSpeedThreshold)
				{
					IEntity otherPilot;
					Vehicle otherVehicle = Vehicle.Cast(other);
					if (otherVehicle)
						otherPilot = otherVehicle.GetPilot();

					//If the entity has a driver:
					if (otherPilot)
					{
						//If their speed is bigger than GetVehicleDamageSpeedThreshold and they are not running away from the vehicle
						//(their dot product indicates that they are within 60° cone):
						//their driver is the instigator of the damage the occupants of this vehicle's compartments receive.
						//If not, the driver of this vehicle is the instigator.
						vector directionToOther = (other.GetOrigin() - GetOwner().GetOrigin()).Normalized();
						if (vector.Dot(velocityBefore1.Normalized(), directionToOther) < 0.5)
							instigatorEntity = otherPilot;
					}
				}

				// If there was no other pilot to blame, Blame last person to damage the vehicle, if there is nobody, blame the pilot.
				if (!instigatorEntity)
				{
					instigatorEntity = GetInstigator().GetInstigatorEntity();
					if (!instigatorEntity && Vehicle.Cast(GetOwner()))
						instigatorEntity = Vehicle.Cast(GetOwner()).GetPilot();
				}
				
				// Apply damage to passengers in vehicle
				HandlePassengerDamage(EDamageType.COLLISION, momentumOverOccupantsThreshold * damageScaleToCharacter, Instigator.CreateInstigator(instigatorEntity));
			}
		}

		// Deal damage if collision damage is over threshold
		if (collisionDamage > 0)
		{
			// Get hit side multiplier (e. g. front is stronger than the left/right side)
			float damageSideMultiplier = GetSideDamageMultiplier(GetHitDirection(impactPosition));

			collisionDamage *= damageSideMultiplier * damageShare;
			// Handle damage requires a matrix, so we create an empty one
			vector empty[3];
			empty[0] = vector.Zero;
			empty[1] = vector.Zero;
			empty[2] = vector.Zero;

			SCR_DamageContext damageContext = new SCR_DamageContext(EDamageType.COLLISION, DamageSurroundingHitzones(impactPosition, collisionDamage, EDamageType.COLLISION), empty, GetOwner(), GetDefaultHitZone(), Instigator.CreateInstigator(instigatorEntity), null, -1, -1);
			damageContext.damageEffect = new SCR_CollisionDamageEffect();
			// finally we deal damage
			HandleDamage(damageContext);
		}

		// Reset is in contact
		m_bIsInContact = false;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	static ScriptInvokerInt GetOnVehicleDestroyed()
	{
		if (!s_OnVehicleDestroyed)
			s_OnVehicleDestroyed = new ScriptInvokerInt();

		return s_OnVehicleDestroyed;
	}	
	
	//------------------------------------------------------------------------------------------------
	//! \return
	static ScriptInvoker GetOnVehicleDamageStateChanged()
	{
		if (!s_OnVehicleDamageStateChanged)
			s_OnVehicleDamageStateChanged = new ScriptInvoker();

		return s_OnVehicleDamageStateChanged;
	}

	//------------------------------------------------------------------------------------------------
	//! Update buoyancy loss rate
	override void OnDamageStateChanged(EDamageState state)
	{
		super.OnDamageStateChanged(state);
		
		if (s_OnVehicleDamageStateChanged)
			s_OnVehicleDamageStateChanged.Invoke(this);
		
		HitZone defaultHitZone = GetDefaultHitZone();
		if (!defaultHitZone)
			return;

		SCR_VehicleBuoyancyComponent vehicleBuoyancy = SCR_VehicleBuoyancyComponent.Cast(GetOwner().FindComponent(SCR_VehicleBuoyancyComponent));
		if (vehicleBuoyancy)
		{
			vehicleBuoyancy.SetHealth(defaultHitZone.GetDamageStateThreshold(state));
			
			if (state == EDamageState.DESTROYED)
				vehicleBuoyancy.GetOnWaterEnter().Remove(OnWaterEnter);
		}
		
		if (s_OnVehicleDestroyed && state == EDamageState.DESTROYED)
			s_OnVehicleDestroyed.Invoke(GetInstigator().GetInstigatorPlayerID());
	}
	
	//------------------------------------------------------------------------------------------------
	//! On Vehicle being blown up, there is a chance the passengers are ejected
	override protected void OnDamage(notnull BaseDamageContext damageContext)
	{
		if (damageContext.damageType == EDamageType.EXPLOSIVE)
			HandlePassengerDamage(EDamageType.EXPLOSIVE, damageContext.damageValue, damageContext.instigator);
	}

	//------------------------------------------------------------------------------------------------
	//! Return true if there is damage that can be repaired
	override bool CanBeHealed(bool ignoreHealingDOT = true)
	{
		// Get drowned engine
		BaseVehicleNodeComponent vehicleNode = BaseVehicleNodeComponent.Cast(GetOwner().FindComponent(BaseVehicleNodeComponent));
		if (vehicleNode)
		{
			VehicleControllerComponent vehicleController = VehicleControllerComponent.Cast(vehicleNode.FindComponent(VehicleControllerComponent));
			if (vehicleController && vehicleController.GetEngineDrowned())
				return true;
		}

		return super.CanBeHealed();
	}

	//------------------------------------------------------------------------------------------------
	//! Fix all the damage
	//! \param[in] ignoreHealingDOT
	override void FullHeal(bool ignoreHealingDOT = true)
	{
		// Fix drowned engine
		BaseVehicleNodeComponent vehicleNode = BaseVehicleNodeComponent.Cast(GetOwner().FindComponent(BaseVehicleNodeComponent));
		if (vehicleNode)
		{
			VehicleControllerComponent vehicleController = VehicleControllerComponent.Cast(vehicleNode.FindComponent(VehicleControllerComponent));
			if (vehicleController && vehicleController.GetEngineDrowned())
				vehicleController.SetEngineDrowned(false);
		}

		// Repair everything else that can be repaired
		super.FullHeal(ignoreHealingDOT);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Damage and eject passengers based on vehicle taking damage
	void HandlePassengerDamage(EDamageType damageType, float damage, notnull Instigator instigator)
	{
		if (damageType == EDamageType.COLLISION)
		{
			// Damage occupants of the car that experienced a collision
			m_CompartmentManager.DamageOccupants(damage, EDamageType.COLLISION, instigator);
			// if a collision was capable of causing more than minCollisionEjectionDamageThreshold damage, try to eject passengers
			if (damage > GetMinCollisionDamageEjectionThreshold())
				m_CompartmentManager.EjectRandomOccupants(-1, true);
			
			return;
		}
		
		if (damageType == EDamageType.EXPLOSIVE)
		{
			// An explosion of minExplosionEjectionDamageThreshold or larger is capable of ejecting occupants.
			if (damage < GetMinExplosionDamageEjectionThreshold())
				m_CompartmentManager.EjectRandomOccupants(-1, true);
			
			return;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	// Takes care of updating the reponse index
	void TickResponseIndexCheck()
	{
		Physics physics = GetOwner().GetPhysics();
		if (!physics)
			return;

		float momentum = physics.GetMass() * physics.GetVelocity().Length(); // Calculate current vehicle's momentum
		float currentIndexMinMomentum = s_mResponseIndexMomentumMap.Get(physics.GetResponseIndex());
		int currentIndex = physics.GetResponseIndex();

		while (momentum > currentIndexMinMomentum && currentIndex < MAX_RESPONSE_INDEX)
		{
			currentIndex++;
			currentIndexMinMomentum = s_mResponseIndexMomentumMap.Get(currentIndex);
			if (momentum < currentIndexMinMomentum) // We are at a point, where current momentum isn't big enough for this index
			{
				// We go one index down and break from the loop
				currentIndex--;
				currentIndexMinMomentum = s_mResponseIndexMomentumMap.Get(currentIndex);
				break;
			}
		}

		while (momentum < currentIndexMinMomentum && currentIndex > MIN_RESPONSE_INDEX)
		{
			currentIndex--;
			currentIndexMinMomentum = s_mResponseIndexMomentumMap.Get(currentIndex);
			if (momentum > currentIndexMinMomentum) // We are at a point, where current momentum is big enough for this index
				break;
		}

		physics.SetResponseIndex(currentIndex);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RPC_OnPhysicsActive(bool activeState)
	{
		ToggleResponseIndexTicking(GetOwner(), activeState);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] owner
	//! \param[in] activeState
	void ToggleResponseIndexTicking(IEntity owner, bool activeState)
	{
		// Always remove first so we don't end up double-registering
		GetGame().GetCallqueue().Remove(TickResponseIndexCheck);
		if (activeState)
			GetGame().GetCallqueue().CallLater(TickResponseIndexCheck, 500, true);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] owner
	//! \param[in] activeState
	void EOnPhysicsActive(IEntity owner, bool activeState)
	{
		Vehicle vehicle = Vehicle.Cast(owner);
		if (!vehicle)
			return;

		RplComponent rplComponent = vehicle.GetRplComponent();
		if (!rplComponent)
			return;

		if (rplComponent.IsProxy() && !rplComponent.IsOwner())
		{
			// Make sure to always deactivate ticking on remote proxies
			if (!activeState)
				RPC_OnPhysicsActive(activeState);
		}
		else if (rplComponent.IsOwner() || !rplComponent.IsProxy())
		{
			RPC_OnPhysicsActive(activeState);
		}
	}

	//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
	// TODO: Flammability should become damage effect
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] hitZone
	void RegisterFlammableHitZone(notnull SCR_FlammableHitZone hitZone)
	{
		if (m_aFlammableHitZones)
			m_aFlammableHitZones.Insert(hitZone);
		else
			m_aFlammableHitZones = {hitZone};
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] hitZone
	void UnregisterFlammableHitZone(notnull SCR_FlammableHitZone hitZone)
	{
		if (m_aFlammableHitZones)
			m_aFlammableHitZones.RemoveItem(hitZone);

		if (m_aFlammableHitZones.IsEmpty())
			m_aFlammableHitZones = null;
	}
	//---- REFACTOR NOTE END ----

	//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
	// TODO: It should be possible to work without explicit flammable hitzones, perhaps supply crates could have this ability though simplified
	//------------------------------------------------------------------------------------------------
	override void UpdateFireDamage(float timeSlice)
	{
		if (!m_aFlammableHitZones)
			return;

		float fireRate;
		UpdateVehicleFireState(fireRate, timeSlice);
		UpdateFuelTankFireState(fireRate, timeSlice);
		UpdateSuppliesFireState(fireRate, timeSlice);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnVehicleFireStateChanged()
	{
		if (m_SignalsManager)
			m_SignalsManager.SetSignalValue(m_iVehicleFireStateSignalIdx, m_eVehicleFireState);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetVehicleFireState(SCR_ESecondaryExplosionScale state, vector origin = vector.Zero)
	{
		if (m_eVehicleFireState == state)
			return;

		m_eVehicleFireState = state;
		m_vVehicleFireOrigin = origin;
		Replication.BumpMe();

		OnVehicleFireStateChanged();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnFuelTankFireStateChanged()
	{
		if (m_SignalsManager)
			m_SignalsManager.SetSignalValue(m_iFuelTankFireStateSignalIdx, m_eFuelTankFireState);

		UpdateFireParticles(m_vFuelTankFireOrigin, m_FuelTankFireParticle, m_eFuelTankFireState, SCR_ESecondaryExplosionType.FUEL);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetFuelTankFireState(SCR_ESecondaryExplosionScale state, vector origin = vector.Zero)
	{
		if (m_eFuelTankFireState == state)
			return;

		m_eFuelTankFireState = state;
		m_vFuelTankFireOrigin = origin;
		Replication.BumpMe();

		OnFuelTankFireStateChanged();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSuppliesFireStateChanged()
	{
		if (m_SignalsManager)
			m_SignalsManager.SetSignalValue(m_iSuppliesFireStateSignalIdx, m_eSuppliesFireState);

		UpdateFireParticles(m_vSuppliesFireOrigin, m_SuppliesFireParticle, m_eSuppliesFireState, SCR_ESecondaryExplosionType.RESOURCE, EResourceType.SUPPLIES);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetSuppliesFireState(SCR_ESecondaryExplosionScale state, vector origin = vector.Zero)
	{
		if (m_eSuppliesFireState == state)
			return;

		m_eSuppliesFireState = state;
		m_vSuppliesFireOrigin = origin;
		Replication.BumpMe();

		OnSuppliesFireStateChanged();
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateVehicleFireState(out float fireRate, float timeSlice)
	{
		vector averagePosition = GetSecondaryExplosionPosition(SCR_FlammableHitZone, fireRate);

		// Fire area damage
		m_fVehicleFireDamageTimeout -= timeSlice;
		if (m_fVehicleFireDamageTimeout < 0)
		{
			EntitySpawnParams spawnParams();
			spawnParams.Transform[3] = averagePosition;
			ResourceName fireDamage = GetSecondaryExplosion(fireRate, SCR_ESecondaryExplosionType.FUEL, fire: true);
			SecondaryExplosion(fireDamage, GetInstigator(), spawnParams);

			// Constant intervals for secondary damage
			m_fVehicleFireDamageTimeout = m_fSecondaryFireDamageDelay;
		}

		SCR_ESecondaryExplosionScale vehicleFireState = GetSecondaryExplosionScale(fireRate, SCR_ESecondaryExplosionType.FUEL);
		SetVehicleFireState(vehicleFireState, averagePosition);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateFuelTankFireState(float fireRate, float timeSlice)
	{
		if (!m_FuelManager)
		{
			SetFuelTankFireState(SCR_ESecondaryExplosionScale.NONE);
			return;
		}

		if (fireRate <= 0)
		{
			SetFuelTankFireState(SCR_ESecondaryExplosionScale.NONE);
			return;
		}

		if (GetState() == EDamageState.DESTROYED)
		{
			SetFuelTankFireState(SCR_ESecondaryExplosionScale.NONE);
			return;
		}

		float burningFuel;
		vector averagePosition = GetSecondaryExplosionPosition(SCR_FuelHitZone, burningFuel);
		if (float.AlmostEqual(burningFuel, 0))
		{
			SetFuelTankFireState(SCR_ESecondaryExplosionScale.NONE);
			return;
		}

		// Fire area damage
		float fuelBurnRate;
		m_fFuelTankFireDamageTimeout -= timeSlice;
		if (m_fFuelTankFireDamageTimeout < 0)
		{
			EntitySpawnParams spawnParams();
			spawnParams.Transform[3] = averagePosition;
			ResourceName fireDamage = GetSecondaryExplosion(burningFuel / fireRate, SCR_ESecondaryExplosionType.FUEL, fire: true);
			SecondaryExplosion(fireDamage, GetInstigator(), spawnParams);

			// Constant intervals for secondary damage
			m_fFuelTankFireDamageTimeout = m_fSecondaryFireDamageDelay;
			fuelBurnRate = fireRate * m_fSecondaryFireDamageDelay * m_fFuelTankFireDamageRate;
		}

		// Consume fuel
		if (fuelBurnRate > 0)
		{
			SCR_FuelNode fuelTank;
			float fuelLoss;
			array<BaseFuelNode> fuelTanks = {};
			m_FuelManager.GetFuelNodesList(fuelTanks);
			foreach (BaseFuelNode fuelNode : fuelTanks)
			{
				fuelTank = SCR_FuelNode.Cast(fuelNode);
				if (!fuelTank)
					continue;

				// Burn fuel based on fuel tank health and amount of fuel vs burning fuel amount
				fuelLoss = fuelBurnRate * (1 - fuelTank.GetHealth()) * fuelNode.GetFuel() / burningFuel;
				if (fuelLoss > 0)
					fuelTank.SetFuel(fuelTank.GetFuel() - fuelLoss);
			}
		}

		SCR_ESecondaryExplosionScale fuelTankFireState = GetSecondaryExplosionScale(fireRate, SCR_ESecondaryExplosionType.FUEL);
		SetFuelTankFireState(fuelTankFireState, averagePosition);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateSuppliesFireState(float fireRate, float timeSlice)
	{
		if (fireRate <= 0)
		{
			SetSuppliesFireState(SCR_ESecondaryExplosionScale.NONE);
			return;
		}

		if (GetState() == EDamageState.DESTROYED)
		{
			SetSuppliesFireState(SCR_ESecondaryExplosionScale.NONE);
			return;
		}

		SCR_ResourceEncapsulator encapsulator = GetResourceEncapsulator();
		if (!encapsulator)
		{
			SetSuppliesFireState(SCR_ESecondaryExplosionScale.NONE);
			return;
		}

		float totalResources = encapsulator.GetAggregatedResourceValue();
		if (totalResources <= 0)
		{
			SetSuppliesFireState(SCR_ESecondaryExplosionScale.NONE);
			return;
		}

		SCR_ResourceContainerQueueBase containerQueue = encapsulator.GetContainerQueue();
		int containerCount = encapsulator.GetContainerCount();

		SCR_ResourceContainer container;
		float weight;
		vector position;

		vector averagePosition = GetOwner().CoordToLocal(encapsulator.GetOwnerOrigin());

		// Get the weighed average position of explosion relative to encapsulator
		for (int i; i < containerCount; i++)
		{
			container = containerQueue.GetContainerAt(i);
			if (!container)
				continue;

			// Determine secondary explosion position
			weight = container.GetResourceValue() / totalResources;
			if (weight <= 0)
				continue;

			position = GetOwner().CoordToLocal(container.GetOwnerOrigin());
			averagePosition += position * weight;
		}

		SCR_ESecondaryExplosionScale suppliesFireState = GetSecondaryExplosionScale(fireRate, SCR_ESecondaryExplosionType.RESOURCE);
		SetSuppliesFireState(suppliesFireState, averagePosition);

		// Fire area damage
		float suppliesDamage;
		m_fSuppliesFireDamageTimeout -= timeSlice;
		if (m_fSuppliesFireDamageTimeout < 0)
		{

			EntitySpawnParams spawnParams();
			spawnParams.Transform[3] = averagePosition;
			ResourceName fireDamage = GetSecondaryExplosion(fireRate, SCR_ESecondaryExplosionType.RESOURCE, fire: true);
			SecondaryExplosion(fireDamage, GetInstigator(), spawnParams);

			// Constant intervals for secondary damage
			m_fSuppliesFireDamageTimeout = m_fSecondaryFireDamageDelay;
			suppliesDamage = Math.Ceil(fireRate * m_fSecondaryFireDamageDelay * m_fSuppliesFireDamageRate);
		}

		if (containerCount > 1)
			suppliesDamage /= containerCount;

		for (int i; i < containerCount; i++)
		{
			container = containerQueue.GetContainerAt(i);
			if (container)
				container.DecreaseResourceValue(suppliesDamage);
		}
	}
	//---- REFACTOR NOTE END ----

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetMinImpulse()
	{
		return m_fMinImpulse;
	}

	//------------------------------------------------------------------------------------------------
	//!
	void InitStaticMapForIndices()
	{
		s_mResponseIndexMomentumMap.Insert(SCR_EPhysicsResponseIndex.TINY_MOMENTUM, 0);
		s_mResponseIndexMomentumMap.Insert(SCR_EPhysicsResponseIndex.SMALL_MOMENTUM, 6666.6); // Approx UAZ (1600kg) at 15 km/h in kJ
		s_mResponseIndexMomentumMap.Insert(SCR_EPhysicsResponseIndex.MEDIUM_MOMENTUM, 22222.2); // 50 km/h
		s_mResponseIndexMomentumMap.Insert(SCR_EPhysicsResponseIndex.LARGE_MOMENTUM, 44444.4); // 100 km/h
		s_mResponseIndexMomentumMap.Insert(SCR_EPhysicsResponseIndex.HUGE_MOMENTUM, 133333.3); // 300 km/h
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Called whenever an instigator is going to be set.
	//! \param[in] currentInstigator: This damage manager's last instigator
	//! \param[in] newInstigator: The new instigator for this damage manager
	//! \return If it returns true, newInstigator will become the new current instigator for the damage manager and it will receive kill credit.
	*/
	protected override bool ShouldOverrideInstigator(notnull Instigator currentInstigator, notnull Instigator newInstigator)
	{
		// if the vehicle is on fire and we have a valid fireInstigator, do not allow any new instigators to override it
		SCR_FlammableHitZone flammableHitZone = SCR_FlammableHitZone.Cast(GetDefaultHitZone());
		Instigator fireInstigator = flammableHitZone.GetFireInstigator();
		if (fireInstigator && flammableHitZone && IsOnFire(flammableHitZone) && flammableHitZone.GetFireInstigator() != newInstigator)
			return false;
		
		// Don't override valid instigators with invalid instigators
		if (newInstigator.GetInstigatorType() == InstigatorType.INSTIGATOR_NONE)
			return false;
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_VehicleDamageManagerComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		//Can be removed when this event is called on components
		Vehicle ownerVehicle = Vehicle.Cast(ent);
		if (ownerVehicle)
			ownerVehicle.GetOnPhysicsActive().Insert(EOnPhysicsActive);

		if (!s_mResponseIndexMomentumMap.Find(SCR_EPhysicsResponseIndex.TINY_MOMENTUM, null))
			InitStaticMapForIndices();
	}

	void ~SCR_VehicleDamageManagerComponent()
	{
		if (m_FuelTankFireParticle)
			SCR_ParticleHelper.StopParticleEmissionAndLights(m_FuelTankFireParticle);

		if (m_SuppliesFireParticle)
			SCR_ParticleHelper.StopParticleEmissionAndLights(m_SuppliesFireParticle);
	}
}
