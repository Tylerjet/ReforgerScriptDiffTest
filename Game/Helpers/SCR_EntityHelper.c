class SCR_EntityHelper
{
	/*!
	Function to find specific component on given entity or entity parent/slottedEntities/siblings/rootparents etc
	\param entity Entity to use to find the component on
	\param componentType Component Class to find
	\param queryFlags Where to find the component on. It can have multiple flags and is checked in order of lowest to highest flag value
	\return Component is any is found
	*/
	static Managed FindComponent(notnull IEntity entity, typename componentType, SCR_EComponentFinderQueryFlags queryFlags = SCR_EComponentFinderQueryFlags.ENTITY | SCR_EComponentFinderQueryFlags.SLOTS)
	{
		Managed foundComponent;
		
		//~ Find on entity itself
		if (SCR_Enum.HasFlag(queryFlags, SCR_EComponentFinderQueryFlags.ENTITY))
		{
			foundComponent = entity.FindComponent(componentType);
			if (foundComponent)
				return foundComponent;
		}
		
		//~ Find on slotted entities
		if (SCR_Enum.HasFlag(queryFlags, SCR_EComponentFinderQueryFlags.SLOTS))
		{
			SlotManagerComponent slotManager = SlotManagerComponent.Cast(entity.FindComponent(SlotManagerComponent));
			if (slotManager)
			{
				array<EntitySlotInfo> slotInfos = {};
				slotManager.GetSlotInfos(slotInfos);
				IEntity slotEntity;
				
				foreach (EntitySlotInfo slotInfo : slotInfos)
				{
					slotEntity = slotInfo.GetAttachedEntity();
					if (!slotEntity)
						continue;
					
					foundComponent = slotEntity.FindComponent(componentType);
					if (foundComponent)
						return foundComponent;
				}
			}
		}
		
		//~ Find on children
		if (SCR_Enum.HasFlag(queryFlags, SCR_EComponentFinderQueryFlags.CHILDREN))
		{
			IEntity child = entity.GetChildren();
		
			while (child)
			{
				foundComponent = child.FindComponent(componentType);
				if (foundComponent)
					return foundComponent;
				
				child = child.GetSibling();
			}
		}
		
		IEntity parent;
		
		//~ Find in parent
		if (SCR_Enum.HasFlag(queryFlags, SCR_EComponentFinderQueryFlags.PARENT))
		{
			parent = entity.GetParent();
			
			if (parent)
			{
				foundComponent = parent.FindComponent(componentType);
				if (foundComponent)
					return foundComponent;
			}
		}
		
		//~ Find on slotted entities of parent
		if (SCR_Enum.HasFlag(queryFlags, SCR_EComponentFinderQueryFlags.PARENT_SLOTS))
		{			
			if (!parent)
				parent = entity.GetParent();
			
			if (parent)
			{
				foundComponent = SCR_EntityHelper.FindComponent(parent, componentType, SCR_EComponentFinderQueryFlags.SLOTS);
				if (foundComponent)
					return foundComponent;
			}
		}
		
		IEntity rootParent;
		
		//~ Find in root parent
		if (SCR_Enum.HasFlag(queryFlags, SCR_EComponentFinderQueryFlags.PARENT))
		{
			rootParent = entity.GetRootParent();
			
			if (rootParent)
			{
				foundComponent = rootParent.FindComponent(componentType);
				if (foundComponent)
					return foundComponent;
			}
		}
		
		//~ Find on slotted entities of root parent
		if (SCR_Enum.HasFlag(queryFlags, SCR_EComponentFinderQueryFlags.PARENT_SLOTS))
		{			
			if (!rootParent)
				rootParent = entity.GetRootParent();
			
			if (rootParent)
			{
				foundComponent = SCR_EntityHelper.FindComponent(rootParent, componentType, SCR_EComponentFinderQueryFlags.SLOTS);
				if (foundComponent)
					return foundComponent;
			}
		}
		
		//~ Find in siblings
		if (SCR_Enum.HasFlag(queryFlags, SCR_EComponentFinderQueryFlags.SIBLINGS))
		{
			if (!parent)
				parent = entity.GetParent();
			
			if (parent)
			{
				//~ Get siblings from parent
				IEntity child = parent.GetChildren();
		
				while (child)
				{
					//~ Ignore self
					if (child == entity)
					{
						child = child.GetSibling();
						continue;
					}
					
					foundComponent = child.FindComponent(componentType);
					if (foundComponent)
						return foundComponent;
					
					child = child.GetSibling();
				}
			}
		}
		
		//~ Not found
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns number of children the input entity has
	//! \param parent
	//! \param recursive checks children's children if set to true, the number of direct children otherwise
	//! \return 0 if the provided entity is null
	// unused
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
	//! Deletes input parent entity and all children
	//! Just a wrapper for RplComponent.DeleteRplEntity(entity, false);
	//! \param entity
	static void DeleteEntityAndChildren(IEntity entity)
	{
		if (entity)
			RplComponent.DeleteRplEntity(entity, false);
	}

	//------------------------------------------------------------------------------------------------
	//! Returns the size of an entity from its bounding box
	//! \param entity
	//! \return vector of width, height, length
	static vector GetEntitySize(notnull IEntity entity)
	{
		vector entMins, entMaxs;
		entity.GetBounds(entMins, entMaxs);

		return entMaxs - entMins;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns the center of the entity from its bounding box in world coordinates
	//! \param entity
	//! \return bounding box' centre in world coordinates
	static vector GetEntityCenterWorld(notnull IEntity entity)
	{
		vector entMins, entMaxs;
		entity.GetBounds(entMins, entMaxs);
		return entity.CoordToParent((entMaxs + entMins) * 0.5);
	}

	//------------------------------------------------------------------------------------------------
	//! Returns the radius of the entity based on the size of its bounding box
	//! \param entity the entity to measure
	static float GetEntityRadius(notnull IEntity entity)
	{
		return GetEntitySize(entity).Length() * 0.5;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns a list of all entities in the hierarchy
	//! \param entity
	//! \param[in,out] output
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
	//!
	//! \param entity the entity to snap to the ground
	//! \param excludeArray
	//! \param maxLength
	//! \param startOffset
	//! \param onlyStatic only check static physics
	// unused
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
		{
			param.Exclude = entity;
		}

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
	//! \param newUp
	//! \param[in,out] mat
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
	//! \param entity
	// used by SnapToGround() which is unused
	protected static bool OnlyStaticCallback(notnull IEntity entity)
	{
		Physics physics = entity.GetPhysics();
		if (physics && physics.IsDynamic())
			return false;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the main parent of the input entity
	//! \param entity Entity to get the main parent from
	//! \param self return entity if there is no parent, false otherwise - default = false
	static IEntity GetMainParent(IEntity entity, bool self = false)
	{
		if (!entity)
			return null;

		IEntity parent = entity.GetRootParent();
		if (parent != entity)
			return parent;

		// root element
		if (self)
			return entity;
		else
			return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Return the player-controlled entity
	//! See EntityUtils.GetPlayer()
	static IEntity GetPlayer()
	{
		return EntityUtils.GetPlayer();
	}

	//------------------------------------------------------------------------------------------------
	//! Check if the provided entity is the local player
	//! See EntityUtils.GetPlayer()
	//! \param entity
	// unused
	static bool IsPlayer(IEntity entity)
	{
		return entity && entity == EntityUtils.GetPlayer();
	}

	//------------------------------------------------------------------------------------------------
	//! Check if the provided entity is -a- player - a human-controlled entity
	//! See EntityUtils.IsPlayer()
	//! \param entity
	// unused
	static bool IsAPlayer(IEntity entity)
	{
		return entity && EntityUtils.IsPlayer(entity);
	}

	//------------------------------------------------------------------------------------------------
	//! Set transform for the whole hierarchy
	//! \param entity
	//! \param newTransform
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
	//! Set child transformation
	//! \param entity
	//! \param oldTransform
	//! \param newTransform
	//! \param recursive
	// used by SetHierarchyTransform
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
}

class SCR_EntityHelperT<Class T>
{
	//------------------------------------------------------------------------------------------------
	//! Search for an entity of given type in hierarchy of provided parent
	//! \param parent
	//! \return the found entity or null if not found
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
}

//------------------------------------------------------------------------------------------------
//! Used for component finding to know where it can search to get the given component
enum SCR_EComponentFinderQueryFlags
{
	ENTITY =        		1 << 0, ///< Find on entity itself
	SLOTS = 				1 << 1, ///< Find in SlotManagerComponent
	CHILDREN =      		1 << 2, ///< Find on children of entity
	PARENT = 				1 << 3, ///< Find on parent of entity
	PARENT_SLOTS =			1 << 4, ///< Find in SlotManagerComponent of parent
	ROOT_PARENT = 			1 << 5, ///< Find on Root parent of entity
	ROOT_PARENT_SLOTS =		1 << 6, ///< Find in SlotManagerComponent of root parent
	SIBLINGS = 				1 << 7, ///< Find on Siblings in hierarchy
};
