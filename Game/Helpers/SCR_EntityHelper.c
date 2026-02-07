class SCR_EntityHelper
{
	//-----------------------------------------------------------------------------------------------------------
	//! Returns number of children the input entity has
	static int CountChildren(IEntity parent)
	{
		if (!parent)
			return 0;

		int num = 0;
		IEntity child = parent.GetChildren();
		while (child)
		{
			num++;
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
	static void DeleteEntityAndChildren(IEntity ent)
	{
		if (ent)
			RplComponent.DeleteRplEntity(ent, false);
	}

	//------------------------------------------------------------------------------------------------
	//! Returns the size of an entity from its bounding box
	static vector GetEntitySize(IEntity ent)
	{
		vector entMins, entMaxs;
		ent.GetBounds(entMins, entMaxs);

		return entMaxs - entMins;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns the center of the entity from its bounding box in world coordinates
	static vector GetEntityCenterWorld(IEntity ent)
	{
		vector entMins, entMaxs;
		ent.GetBounds(entMins, entMaxs);
		vector result = (entMaxs + entMins) * 0.5;
		return ent.CoordToParent(result);
	}

	//------------------------------------------------------------------------------------------------
	//! Returns the radius of the entity based on the length of its bounding box
	static float GetEntityRadius(IEntity ent)
	{
		return GetEntitySize(ent).Length() * 0.5;
	}

	//-----------------------------------------------------------------------------------------------------------
	//! Returns a list of all entities in the hierarchy
	static void GetHierarchyEntityList(IEntity ent, inout array<IEntity> output)
	{
		IEntity child = ent.GetChildren();
		while (child)
		{
			GetHierarchyEntityList(child, output);
			output.Insert(child);
			child = child.GetSibling();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Returns the main parent of the input entity
	//! \param ent Entity to get the main parent from
	//! \param self Return itself if there is no parent, default = false
	static IEntity GetMainParent(IEntity ent, bool self = false)
	{
		IEntity parent = ent.GetParent();
		if (!parent)
		{
			if (self)
				return ent;
			else
				return null;
		}

		while (parent.GetParent())
			parent = parent.GetParent();

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
		return entity == EntityUtils.GetPlayer();
	}

	//------------------------------------------------------------------------------------------------
	static bool IsAPlayer(IEntity entity)
	{
		return EntityUtils.IsPlayer(entity);
	}

	//------------------------------------------------------------------------------------------------
	//! Set transform for whole hierarchy
	static void SetHierarchyTransform(notnull IEntity ent, vector newTransform[4])
	{
		vector oldTransform[4];
		ent.GetTransform(oldTransform);
		ent.SetTransform(newTransform);

		IEntity child = ent.GetChildren();
		while (child)
		{
			SetHierarchyChildTransform(child, oldTransform, newTransform, true);
			child = child.GetSibling();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Called from SetHierarchyTransform
	protected static void SetHierarchyChildTransform(notnull IEntity ent, vector oldTransform[4], vector newTransform[4], bool recursive = true)
	{
		Physics entPhys = ent.GetPhysics();
		if (entPhys)
		{
			if (entPhys.IsDynamic())
			{
				vector mat[4];
				ent.GetTransform(mat);

				vector diffMat[4];
				Math3D.MatrixInvMultiply4(oldTransform, mat, diffMat);
				Math3D.MatrixMultiply4(newTransform, diffMat, mat);

				ent.SetTransform(mat);
			}
		}

		IEntity child = ent.GetChildren();
		while (child)
		{
			SetHierarchyChildTransform(child, oldTransform, newTransform, recursive);
			child = child.GetSibling();
		}
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
