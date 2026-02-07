class SCR_EntityHelper
{
	//------------------------------------------------------------------------------------------------
	//! Returns number of children the input entity has
	//! \param parent
	//! \param recursive checks children's children if set to true, the number of direct children otherwise
	//! \return 0 if the provided entity is null
	static int GetChildrenCount(IEntity parent, bool recursive = false)
	{
		if (!parent)
			return 0;

		int num = 0;
		IEntity child = parent.GetChildren();
		while (child)
		{
			num++;
			if (recursive)
				num += GetChildrenCount(child);
			child = child.GetSibling();
		}

		return num;
	}

	//------------------------------------------------------------------------------------------------
	//! Deletes all children of the input parent entity
	// used by DeleteEntityAndChildren only
	protected static void DeleteChildren(IEntity parent)
	{
		if (!parent)
			return;

		IEntity child = parent.GetChildren();
		while (child)
		{
			DeleteChildren(child);

			parent.RemoveChild(child);
			delete child;
			child = parent.GetChildren();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Deletes input parent entity and all children
	static void DeleteEntityAndChildren(IEntity entity)
	{
		if (entity)
			RplComponent.DeleteRplEntity(entity, false);
	}

	//------------------------------------------------------------------------------------------------
	//! Returns the size of an entity from its bounding box
	static vector GetEntitySize(notnull IEntity entity)
	{
		vector entMins, entMaxs;
		entity.GetBounds(entMins, entMaxs);

		return entMaxs - entMins;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns the center of the entity from its bounding box in world coordinates
	static vector GetEntityCenterWorld(notnull IEntity entity)
	{
		vector entMins, entMaxs;
		entity.GetBounds(entMins, entMaxs);
		return entity.CoordToParent((entMaxs + entMins) * 0.5);
	}

	//------------------------------------------------------------------------------------------------
	//! Returns the radius of the entity based on the length of its bounding box
	static float GetEntityRadius(notnull IEntity entity)
	{
		return GetEntitySize(entity).Length() * 0.5;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns a list of all entities in the hierarchy
	static void GetHierarchyEntityList(notnull IEntity entity, notnull inout array<IEntity> output)
	{
		IEntity child = entity.GetChildren();
		while (child)
		{
			GetHierarchyEntityList(child, output);
			output.Insert(child);
			child = child.GetSibling();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	static void SnapToGround(notnull IEntity entity, array<IEntity> excludeArray = null, float maxLength = 10, vector startOffset = "0 0 0", bool onlyStatic = false)
	{
		vector origin = entity.GetOrigin();
		
		// Trace against terrain and entities to detect nearest ground
		TraceParam param = new TraceParam();
		param.Start = origin + startOffset;
		param.End = origin - vector.Up * maxLength;
		param.Flags = TraceFlags.WORLD | TraceFlags.ENTS;
		
		if (excludeArray)
		{
			excludeArray.Insert(entity);
			param.ExcludeArray = excludeArray;
		}
		else
			param.Exclude = entity;
		
		param.LayerMask = EPhysicsLayerPresets.Projectile;
		BaseWorld world = entity.GetWorld();
		float traceDistance;
		
		if (onlyStatic)
			traceDistance = world.TraceMove(param, OnlyStaticCallback);
		else
			traceDistance = world.TraceMove(param, null);
		
		if (float.AlmostEqual(traceDistance, 1.0))
			return;
		
		entity.SetOrigin(traceDistance * (param.End - param.Start) + param.Start);
	}
	
	//------------------------------------------------------------------------------------------------
	static void OrientUpToVector(vector newUp, inout vector mat[4])
	{
		vector origin = mat[3];
		vector perpend = newUp.Perpend();
		Math3D.DirectionAndUpMatrix(perpend, newUp, mat);
		
		vector basis[4];
		Math3D.AnglesToMatrix(Vector(-perpend.VectorToAngles()[0], 0, 0), basis);
		Math3D.MatrixMultiply3(mat, basis, mat);
		mat[3] = origin;
	}

	//------------------------------------------------------------------------------------------------
	static bool OnlyStaticCallback(notnull IEntity e)
	{
		Physics physics = e.GetPhysics();
		if (physics && physics.IsDynamic())
			return false;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the main parent of the input entity
	//! \param entity Entity to get the main parent from
	//! \param self Return itself if there is no parent, default = false
	static IEntity GetMainParent(IEntity entity, bool self = false)
	{
		if (!entity)
			return null;

		IEntity parent = entity.GetParent();
		if (!parent)
		{
			if (self)
				return entity;
			else
				return null;
		}

		while (parent.GetParent())
		{
			parent = parent.GetParent();
		}

		return parent;
	}

	//------------------------------------------------------------------------------------------------
	static IEntity GetPlayer()
	{
		return EntityUtils.GetPlayer();
	}

	//------------------------------------------------------------------------------------------------
	static bool IsPlayer(IEntity entity)
	{
		return entity && entity == EntityUtils.GetPlayer();
	}

	//------------------------------------------------------------------------------------------------
	static bool IsAPlayer(IEntity entity)
	{
		return entity && EntityUtils.IsPlayer(entity);
	}

	//------------------------------------------------------------------------------------------------
	//! Set transform for whole hierarchy
	static void SetHierarchyTransform(notnull IEntity entity, vector newTransform[4])
	{
		vector oldTransform[4];
		entity.GetTransform(oldTransform);
		entity.SetTransform(newTransform);

		IEntity child = entity.GetChildren();
		while (child)
		{
			SetHierarchyChildTransform(child, oldTransform, newTransform, true);
			child = child.GetSibling();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Called from SetHierarchyTransform
	protected static void SetHierarchyChildTransform(notnull IEntity entity, vector oldTransform[4], vector newTransform[4], bool recursive = true)
	{
		Physics entPhys = entity.GetPhysics();
		if (entPhys)
		{
			if (entPhys.IsDynamic())
			{
				vector mat[4];
				entity.GetTransform(mat);

				vector diffMat[4];
				Math3D.MatrixInvMultiply4(oldTransform, mat, diffMat);
				Math3D.MatrixMultiply4(newTransform, diffMat, mat);

				entity.SetTransform(mat);
			}
		}

		IEntity child = entity.GetChildren();
		while (child)
		{
			SetHierarchyChildTransform(child, oldTransform, newTransform, recursive);
			child = child.GetSibling();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get Faction of given entity
	static Faction GetEntityFaction(notnull IEntity ent)
	{
		FactionAffiliationComponent factionComp = FactionAffiliationComponent.Cast(ent.FindComponent(FactionAffiliationComponent));
		if (!factionComp)
			return null;

		Faction faction = factionComp.GetAffiliatedFaction();
		if (!faction)
			faction = factionComp.GetDefaultAffiliatedFaction();

		return faction;
	}
};

class SCR_EntityHelperT<Class T>
{
	//------------------------------------------------------------------------------------------------
	//! Search for an entity of given type in hierarchy of provided parent
	static T GetEntityInHierarchy(notnull IEntity parent)
	{
		IEntity child = parent.GetChildren();
		while (child)
		{
			if (T.Cast(child))
				return T.Cast(child);

			child = child.GetSibling();
		}

		return null;
	}
};
