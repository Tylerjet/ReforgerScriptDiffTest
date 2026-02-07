//#define VEHICLE_DAMAGE_DEBUG
//#define VEHICLE_DEBUG_OTHER
class SCR_VehicleDamageManagerComponentClass : ScriptedDamageManagerComponentClass
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

	[Attribute("15", "Speed in km/h over which will the vehicle be dealt damage in collision", category: "Collision Damage")]
	protected float m_fVehicleDamageSpeedThreshold;

	[Attribute("120", "Speed in km/h over which will the vehicle be destroyed in collision", category: "Collision Damage")]
	protected float m_fVehicleSpeedDestroy;

	[Attribute("0 0 0", "Position for frontal impact calculation", category: "Collision Damage")]
	protected vector m_vFrontalImpact;

	protected float m_fMinImpulse = -1;

	//------------------------------------------------------------------------------------------------
	float GetMinImpulse()
	{
		return m_fMinImpulse;
	}

	//------------------------------------------------------------------------------------------------
	float InitMinImpulse(float mass)
	{
		m_fMinImpulse = mass * 2;
		return m_fMinImpulse;
	}

	//------------------------------------------------------------------------------------------------
	float GetMaxSharedDamageDistance()
	{
		return m_fMaxSharedDamageDistance;
	}

	//------------------------------------------------------------------------------------------------
	float GetFrontMultiplier()
	{
		return m_fFrontMultiplier;
	}

	//------------------------------------------------------------------------------------------------
	float GetBottomMultiplier()
	{
		return m_fBottomMultiplier;
	}

	//------------------------------------------------------------------------------------------------
	float GetRearMultiplier()
	{
		return m_fRearMultiplier;
	}

	//------------------------------------------------------------------------------------------------
	float GetLeftMultiplier()
	{
		return m_fLeftMultiplier;
	}

	//------------------------------------------------------------------------------------------------
	float GetRightMultiplier()
	{
		return m_fRightMultiplier;
	}

	//------------------------------------------------------------------------------------------------
	float GetTopMultiplier()
	{
		return m_fTopMultiplier;
	}

	//------------------------------------------------------------------------------------------------
	float GetOccupantsDamageSpeedThreshold()
	{
		return m_fOccupantsDamageSpeedThreshold;
	}

	//------------------------------------------------------------------------------------------------
	float GetOccupantsSpeedDeath()
	{
		return m_fOccupantsSpeedDeath;
	}

	//------------------------------------------------------------------------------------------------
	float GetVehicleDamageSpeedThreshold()
	{
		return m_fVehicleDamageSpeedThreshold;
	}

	//------------------------------------------------------------------------------------------------
	float GetVehicleSpeedDestroy()
	{
		return m_fVehicleSpeedDestroy;
	}

	//------------------------------------------------------------------------------------------------
	vector GetFrontalImpact()
	{
		return m_vFrontalImpact;
	}

	//------------------------------------------------------------------------------------------------
	void SCR_VehicleDamageManagerComponentClass(BaseContainer prefab)
	{

	}
};

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
};

class SCR_VehicleDamageManagerComponent : ScriptedDamageManagerComponent
{
	protected const int MIN_RESPONSE_INDEX = SCR_EPhysicsResponseIndex.TINY_MOMENTUM;
	protected const int MAX_RESPONSE_INDEX = SCR_EPhysicsResponseIndex.HUGE_MOMENTUM;

	static ref map<SCR_EPhysicsResponseIndex, float> s_mResponseIndexMomentumMap = new map<SCR_EPhysicsResponseIndex, float>();

	[Attribute("0", desc: "Print relative force in collisions of this vehicle? Can be used to determine ideal Collision Damage Force Threshold.", category: "Debug")]
	protected bool m_bPrintRelativeForce;

	[Attribute("", UIWidgets.Hidden, "How much damage will destroy this vehicle. Takes into account current damage multipliers! Won't work precisely when these are changed.", category: "Collision Damage")]
	protected float m_fVehicleDestroyDamage;

	protected bool m_bIsInContact;
	protected float m_fMaxRelativeForce;

	// Sound
	static const float KM_PER_H_TO_M_PER_S = 0.277777;
	private static const float APPROXIMATE_CHARACTER_LETHAL_DAMAGE = 150;

#ifdef VEHICLE_DAMAGE_DEBUG
	protected ref array<ref Shape> m_aDebugShapes = {};
#endif
#ifdef VEHICLE_DEBUG_OTHER
	protected ref array<ref Shape> m_aDebugShapes = {};
#endif

	//------------------------------------------------------------------------------------------------
	SCR_VehicleDamageManagerComponentClass GetPrefabData()
	{
		return SCR_VehicleDamageManagerComponentClass.Cast(GetComponentData(GetOwner()));
	}

	//------------------------------------------------------------------------------------------------
	float GetTopMultiplier()
	{
		SCR_VehicleDamageManagerComponentClass prefabData = GetPrefabData();
		if (!prefabData)
			return 2;

		return prefabData.GetTopMultiplier();
	}

	//------------------------------------------------------------------------------------------------
	float GetRightMultiplier()
	{
		SCR_VehicleDamageManagerComponentClass prefabData = GetPrefabData();
		if (!prefabData)
			return 1.5;

		return prefabData.GetRightMultiplier();
	}

	//------------------------------------------------------------------------------------------------
	float GetLeftMultiplier()
	{
		SCR_VehicleDamageManagerComponentClass prefabData = GetPrefabData();
		if (!prefabData)
			return 1.5;

		return prefabData.GetLeftMultiplier();
	}

	//------------------------------------------------------------------------------------------------
	float GetRearMultiplier()
	{
		SCR_VehicleDamageManagerComponentClass prefabData = GetPrefabData();
		if (!prefabData)
			return 1;

		return prefabData.GetRearMultiplier();
	}

	//------------------------------------------------------------------------------------------------
	float GetBottomMultiplier()
	{
		SCR_VehicleDamageManagerComponentClass prefabData = GetPrefabData();
		if (!prefabData)
			return 0.7;

		return prefabData.GetBottomMultiplier();
	}

	//------------------------------------------------------------------------------------------------
	float GetFrontMultiplier()
	{
		SCR_VehicleDamageManagerComponentClass prefabData = GetPrefabData();
		if (!prefabData)
			return 0.7;

		return prefabData.GetFrontMultiplier();
	}

	//------------------------------------------------------------------------------------------------
	float GetMaxSharedDamageDistance()
	{
		SCR_VehicleDamageManagerComponentClass prefabData = GetPrefabData();
		if (!prefabData)
			return 1.5;

		return prefabData.GetMaxSharedDamageDistance();
	}

	//------------------------------------------------------------------------------------------------
	float GetVehicleSpeedDestroy()
	{
		SCR_VehicleDamageManagerComponentClass prefabData = GetPrefabData();
		if (!prefabData)
			return 100;

		return prefabData.GetVehicleSpeedDestroy();
	}

	//------------------------------------------------------------------------------------------------
	float GetVehicleDamageSpeedThreshold()
	{
		SCR_VehicleDamageManagerComponentClass prefabData = GetPrefabData();
		if (!prefabData)
			return 30;

		return prefabData.GetVehicleDamageSpeedThreshold();
	}

	//------------------------------------------------------------------------------------------------
	float GetOccupantsDamageSpeedThreshold()
	{
		SCR_VehicleDamageManagerComponentClass prefabData = GetPrefabData();
		if (!prefabData)
			return 30;

		return prefabData.GetOccupantsDamageSpeedThreshold();
	}

	//------------------------------------------------------------------------------------------------
	float GetOccupantsSpeedDeath()
	{
		SCR_VehicleDamageManagerComponentClass prefabData = GetPrefabData();
		if (!prefabData)
			return 80;

		return prefabData.GetOccupantsSpeedDeath();
	}

	//------------------------------------------------------------------------------------------------
	vector GetFrontalImpact()
	{
		SCR_VehicleDamageManagerComponentClass prefabData = GetPrefabData();
		if (!prefabData)
			return "0 1 1";

		return prefabData.GetFrontalImpact();
	}

	//------------------------------------------------------------------------------------------------
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

		RplComponent rpl = vehicle.GetRplComponent();
		if (rpl && rpl.IsProxy())
			return;

		SetEventMask(owner, EntityEvent.CONTACT);

		SCR_VehicleDamageManagerComponentClass prefabData = GetPrefabData();
		if (prefabData && prefabData.GetMinImpulse() == -1)
		{
			Physics physics = owner.GetPhysics();
			if (physics)
				prefabData.InitMinImpulse(physics.GetMass());
		}
	}

	//------------------------------------------------------------------------------------------------
	float DamageSurroundingHitzones(vector position, float damage, EDamageType damageType)
	{
		Physics physics = GetOwner().GetPhysics();
		if (!physics)
			return damage;

		array<HitZone> hitzones = {};

		int count = GetAllHitZones(hitzones);
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

			HandleDamage(damageType, currentDamage, empty, GetOwner(), hitzone, null, null, -1, -1);
			leftoverDamage -= currentDamage;
		}

		leftoverDamage = Math.Clamp(leftoverDamage, 0, float.MAX);

		return leftoverDamage;
	}

	//------------------------------------------------------------------------------------------------
	//TODO: This won't work properly for all the types of objects - E.G. flat boats with huge towers on them
	//E.g. x = collision point, this will say it got hit from the back, because it's inside the rear-side pyramid
	//      ||
	//__x___||_______
	//\             /
	// \___________/
	//end of TODO
	//To trigger this, just crash a vehicle into something
	//Make sure the vehicle has SCR_VehicleDamageManagerComponent with attribute "Print Relative Force" set to true.
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
		m_aDebugShapes.Insert(Shape.CreateLines(ARGB(255, 255, 0, 0), ShapeFlags.NOZBUFFER, p, 2));*/

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
		/*p[0] = maxs;
		p[1] = mins;
		p[1][0] = mins[0];*/
		/*shape = Shape.Create(ShapeType.PYRAMID, ARGB(255, 0, 255, 0), ShapeFlags.NOZBUFFER, p[1], p[0]);
		m_aDebugShapes.Insert(shape);
		mat[0] = -vector.Up;
		mat[1] = vector.Right;
		mat[2] = vector.Forward;
		mat[3] = owner.GetOrigin() + maxs;
		shape.SetMatrix(mat);
		Math3D.MatrixIdentity4(mat);*/

		/*mat[0] = vector.Up;
		mat[1] = -vector.Right;
		mat[2] = vector.Forward;*/
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

	//------------------------------------------------------------------------------------------------
	override bool _WB_OnKeyChanged(IEntity owner, BaseContainer src, string key, BaseContainerList ownerContainers, IEntity parent)
	{
		if (key == "m_fVehicleDestroyDamage")
			return true;

		WorldEditorAPI api = GenericEntity.Cast(GetOwner())._WB_GetEditorAPI();
		if (!api)
			return false;

		if (api.UndoOrRedoIsRestoring())
			return false;

		array<HitZone> hitzones;
		int count = GetSurroundingHitzones(GetOwner().CoordToParent(GetFrontalImpact()), GetOwner().GetPhysics(), GetMaxSharedDamageDistance(), hitzones);
		hitzones.Insert(GetDefaultHitZone());
		count++;
		float damage = GetMinDestroyDamage(EDamageType.COLLISION, hitzones, count);

		// Damage is not valid - warn the user!
		if (damage < 0)
		{
			Print("SCR_VehicleDamageManagerComponent._WB_OnKeyChanged(): Can't destroy current vehicle on collision", LogLevel.WARNING);
			return false;
		}

		float newFrontMultiplier = GetFrontMultiplier();
		if (key == "m_fFrontMultiplier")
			src.Get("m_fFrontMultiplier", newFrontMultiplier);

		float targetFrontalDamage = damage / newFrontMultiplier;

		IEntitySource entitySrc = api.EntityToSource(owner);

		array<ref ContainerIdPathEntry> entryPath = {ContainerIdPathEntry("SCR_VehicleDamageManagerComponent")};

		api.SetVariableValue(entitySrc, entryPath, "m_fVehicleDestroyDamage", targetFrontalDamage.ToString());

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Must be first enabled with event mask
	override bool OnContact(IEntity owner, IEntity other, Contact contact)
	{
		SCR_VehicleDamageManagerComponentClass prefabData = GetPrefabData();
		if (prefabData)
		{
			if (contact.Impulse < prefabData.GetMinImpulse())
				return false;
		}

		// Get the physics of the dynamic object (if owner is static, then we use the other object instead)
		Physics physics = owner.GetPhysics();
		if (!physics.IsDynamic())
		{
			m_bIsInContact = false;
			return false;
		}

		Physics otherPhysics = other.GetPhysics();

		// Now get the relative force, which is the impulse divided by the mass of the dynamic object
		float relativeForce = contact.Impulse / physics.GetMass();
		// We keep relative force temporarily, until we re-calculate it to momentum

		// We hit a destructible that will break, static object -> deal no damage to vehicle or occupants
		int ownerResponseIndex = physics.GetResponseIndex();
		int otherResponseIndex = otherPhysics.GetResponseIndex();
		if (otherPhysics && !otherPhysics.IsDynamic() && other.FindComponent(SCR_DestructionBaseComponent) && otherResponseIndex - MIN_DESTRUCTION_RESPONSE_INDEX <= ownerResponseIndex - MIN_MOMENTUM_RESPONSE_INDEX)
			return false;

		float ownerMass = owner.GetPhysics().GetMass();
		float otherMass = otherPhysics.GetMass();

		// Store information about being in contact - so we don't delete physics objects
		m_bIsInContact = true;

#ifndef VEHICLE_COLLISION_DAMAGE
		m_bIsInContact = false;
		return false;
#endif

		//TODOv Pre-calculate these values into prefab data, when callback of prefab data cretion is added.
		float momentumVehicleThreshold = ownerMass * GetVehicleDamageSpeedThreshold() * KM_PER_H_TO_M_PER_S;
		float momentumVehicleDestroy = ownerMass * GetVehicleSpeedDestroy() * KM_PER_H_TO_M_PER_S;
		float damageScaleToVehicle = m_fVehicleDestroyDamage / (momentumVehicleDestroy - momentumVehicleThreshold);
		//TODO^

		// Debug stuff
		if (m_bPrintRelativeForce && relativeForce > m_fMaxRelativeForce)
		{
			m_fMaxRelativeForce = relativeForce;
			Print(contact.Impulse);
			Print(contact.VelocityBefore1);
			Print(m_fMaxRelativeForce);
		}

		float damageShare = 1;
		if (otherMass > 0) // mass == 0 is probably a static object
			damageShare -= ownerMass / (ownerMass + otherMass);

		float DotMultiplier = vector.Dot(contact.VelocityAfter1.Normalized(), contact.VelocityBefore1.Normalized());
		float MomentumBefore = ownerMass * contact.VelocityBefore1.Length();
		float MomentumAfter = ownerMass * contact.VelocityAfter1.Length() * DotMultiplier;
		float momentumA = Math.AbsFloat(MomentumBefore - MomentumAfter);

		DotMultiplier = vector.Dot(contact.VelocityAfter2.Normalized(), contact.VelocityBefore2.Normalized());
		MomentumBefore = otherMass * contact.VelocityBefore2.Length();
		MomentumAfter = otherMass * contact.VelocityAfter2.Length() * DotMultiplier;
		float momentumB = Math.AbsFloat(MomentumBefore - MomentumAfter);

		float collisionDamage = damageScaleToVehicle * (momentumA + momentumB - momentumVehicleThreshold);

		// Find compartment manager to damage occupants
		SCR_BaseCompartmentManagerComponent compartmentManager = SCR_BaseCompartmentManagerComponent.Cast(owner.FindComponent(SCR_BaseCompartmentManagerComponent));
		if (compartmentManager)
		{
			//TODOv Pre-calculate these values into prefab data, when callback of prefab data cretion is added.
			float momentumOccupantsThreshold = ownerMass * GetOccupantsDamageSpeedThreshold() * KM_PER_H_TO_M_PER_S;
			float momentumOccupantsDeath = ownerMass * GetOccupantsSpeedDeath() * KM_PER_H_TO_M_PER_S;
			float damageScaleToCharacter = APPROXIMATE_CHARACTER_LETHAL_DAMAGE / (momentumOccupantsDeath - momentumOccupantsThreshold);
			//TODO^

			// This is the momentum, which will be transferred to damage
			float momentumOverOccupantsThreshold = (momentumA + momentumB) * damageShare - momentumOccupantsThreshold;

			// Deal damage if it's more that 0
			if (momentumOverOccupantsThreshold > 0)
				compartmentManager.DamageOccupants(momentumOverOccupantsThreshold * damageScaleToCharacter, EDamageType.COLLISION, other, true, true);
		}

		// Deal damage if collision damage is over threshold
		if (collisionDamage > 0)
		{
			// Get hit side multiplier (e. g. front is stronger than the left/right side)
			float damageSideMultiplier = GetSideDamageMultiplier(GetHitDirection(contact.Position));

			collisionDamage *= damageSideMultiplier * damageShare;
			// Handle damage requires a matrix, so we create an empty one
			vector empty[3];
			empty[0] = vector.Zero;
			empty[1] = vector.Zero;
			empty[2] = vector.Zero;

			// finally we deal damage
			HandleDamage(EDamageType.COLLISION, DamageSurroundingHitzones(contact.Position, collisionDamage, EDamageType.COLLISION), empty, GetOwner(), GetDefaultHitZone(), other, null, -1, -1);
		}

		// Reset is in contact
		m_bIsInContact = false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Update buoyancy loss rate
	override void OnDamageStateChanged(EDamageState state)
	{
		super.OnDamageStateChanged(state);
		HitZone defaultHitZone = GetDefaultHitZone();
		if (!defaultHitZone)
			return;

		SCR_VehicleWaterPhysicsComponent waterPhysics = SCR_VehicleWaterPhysicsComponent.Cast(GetOwner().FindComponent(SCR_VehicleWaterPhysicsComponent));
		if (waterPhysics)
			waterPhysics.SetHealth(defaultHitZone.GetDamageStateThreshold(state));
	}

	//------------------------------------------------------------------------------------------------
	//! Return true if there is damage that can be repaired
	override bool CanBeHealed()
	{
		// Get drowned engine
		BaseVehicleNodeComponent vehicleNode = BaseVehicleNodeComponent.Cast(GetOwner().FindComponent(BaseVehicleNodeComponent));
		if (vehicleNode)
		{
			if(GetGame().GetIsClientAuthority())
			{
				VehicleControllerComponent vehicleController = VehicleControllerComponent.Cast(vehicleNode.FindComponent(VehicleControllerComponent));
				if (vehicleController && vehicleController.GetEngineDrowned())
					return true;
			}
			else
			{
				VehicleControllerComponent_SA vehicleController = VehicleControllerComponent_SA.Cast(vehicleNode.FindComponent(VehicleControllerComponent_SA));
				if (vehicleController && vehicleController.GetEngineDrowned())
					return true;
			}
		}

		return super.CanBeHealed();
	}

	//------------------------------------------------------------------------------------------------
	//! Fix all the damage
	override void FullHeal(bool ignoreHealingDOT = true)
	{
		// Fix drowned engine
		BaseVehicleNodeComponent vehicleNode = BaseVehicleNodeComponent.Cast(GetOwner().FindComponent(BaseVehicleNodeComponent));
		if (vehicleNode)
		{
			if(GetGame().GetIsClientAuthority())
			{
				VehicleControllerComponent vehicleController = VehicleControllerComponent.Cast(vehicleNode.FindComponent(VehicleControllerComponent));
				if (vehicleController && vehicleController.GetEngineDrowned())
					vehicleController.SetEngineDrowned(false);
			}
			else
			{
				VehicleControllerComponent_SA vehicleController = VehicleControllerComponent_SA.Cast(vehicleNode.FindComponent(VehicleControllerComponent_SA));
				if (vehicleController && vehicleController.GetEngineDrowned())
					vehicleController.SetEngineDrowned(false);
			}
			
		}

		// Repair everything else that can be repaired
		super.FullHeal(ignoreHealingDOT);
	}

	//------------------------------------------------------------------------------------------------
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
	void ToggleResponseIndexTicking(IEntity owner, bool activeState)
	{
		// Always remove first so we don't end up double-registering
		GetGame().GetCallqueue().Remove(TickResponseIndexCheck);
		if (activeState)
			GetGame().GetCallqueue().CallLater(TickResponseIndexCheck, 500, true);
	}

	//------------------------------------------------------------------------------------------------
	void EOnPhysicsActive(IEntity owner, bool activeState)
	{
		RplComponent rplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		if (!rplComponent)
			return;

		if(!System.IsCLIParam("clientVehicles"))
		{
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
		else
		{
			if (rplComponent.IsProxy())
			{
				// Make sure to always deactivate ticking on remote proxies
				if (!activeState)
					RPC_OnPhysicsActive(activeState);
				// Only enable ticking on owner proxies
				else if (rplComponent.IsOwner())
					RPC_OnPhysicsActive(activeState);
			}
	
			Rpc(RPC_OnPhysicsActive, activeState);
		}
	}

	//------------------------------------------------------------------------------------------------
	void InitStaticMapForIndices()
	{
		s_mResponseIndexMomentumMap.Insert(SCR_EPhysicsResponseIndex.TINY_MOMENTUM, 0);
		s_mResponseIndexMomentumMap.Insert(SCR_EPhysicsResponseIndex.SMALL_MOMENTUM, 13333.3); // Approx UAZ (1600kg) at 30 km/h in kJ
		s_mResponseIndexMomentumMap.Insert(SCR_EPhysicsResponseIndex.MEDIUM_MOMENTUM, 26666.6); // 60 km/h
		s_mResponseIndexMomentumMap.Insert(SCR_EPhysicsResponseIndex.LARGE_MOMENTUM, 37777.7); // 85 km/h
		s_mResponseIndexMomentumMap.Insert(SCR_EPhysicsResponseIndex.HUGE_MOMENTUM, 62222.2); // 140 km/h
	}

	//------------------------------------------------------------------------------------------------
	void SCR_VehicleDamageManagerComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		//Can be removed when this event is called on components
		Vehicle ownerVehicle = Vehicle.Cast(ent);
		if (ownerVehicle)
			ownerVehicle.GetOnPhysicsActive().Insert(EOnPhysicsActive);

		if (!s_mResponseIndexMomentumMap.Find(SCR_EPhysicsResponseIndex.TINY_MOMENTUM, null))
			InitStaticMapForIndices();
	}
};
