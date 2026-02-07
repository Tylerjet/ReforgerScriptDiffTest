//------------------------------------------------------------------------------------------------
//! SCR_Global Class
//!
//! Contains various useful global script functions
//------------------------------------------------------------------------------------------------
class SCR_Global
{
	//! For debug canvas drawing funcitons
	static Widget m_DebugLayoutCanvas;
	
	//! Pointer to entity that should be used for various trace filters (below)
	static IEntity g_TraceFilterEnt = null;
	
	//! List of entities for use in trace filters (below)
	static ref set<IEntity> g_TraceFilterList = new set<IEntity>();
	
	//! Physics update in Hz (TODO: Get via PhysicsWorld instead once implemented)
	static float g_fPhysicsHz = 60;
	
	//------------------------------------------------------------------------------------------------
	static bool IsScope2DEnabled()
	{
		BaseContainer gameplaySettings = GetGame().GetGameUserSettings().GetModule("SCR_GameplaySettings");
		if (gameplaySettings)
		{
			bool bval;
			if (gameplaySettings.Get("m_b2DScopes", bval))
				return bval;
		}
		
		// 2d scopes enabled by default = supresses PIP which is less performant
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	static string GetPlatformName(PlatformKind kind)
	{
		switch (kind)
		{
			case PlatformKind.NONE:
				return "platform-windows";

			case PlatformKind.PSN:
				return "platform-playstation";

			case PlatformKind.XBOX:
				return "platform-xbox";

			case PlatformKind.STEAM:
				return "platform-windows";
		}

		return string.Empty;
	}

	//------------------------------------------------------------------------------------------------
	static string GetProfileName()
	{
		string name;

		if( GetGame().GetBackendApi().IsLocalPlatformAssigned() )
		{
			Print("Profile - Using Local Platform Name!");
			name = GetGame().GetBackendApi().GetCredentialsItem(EBackendCredentials.EBCRED_PLATFORMUID);
			return name;
		}
	
		BaseContainer gameplaySettings = GetGame().GetGameUserSettings().GetModule("SCR_GameplaySettings");
		if (gameplaySettings)
		{
			gameplaySettings.Get("m_sProfileName", name);
		}
		
		if (name == string.Empty)
		{
			name = System.GetProfileName();
			if (name == string.Empty)
			{
				Print("Profile - Using MachineName!");
				name = System.GetMachineName();
			}
		}
		
		return name;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Returns the current player
	[Obsolete("Use SCR_PlayerController.GetLocalControlledEntity()")]
	static IEntity GetPlayer()
	{
		return SCR_PlayerController.GetLocalControlledEntity();
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Returns the main parent of the input entity
	//! \param ent Entity to get the main parent from
	//! \param self Return itself if there is no parent, default = false
	[Obsolete("Use SCR_EntityHelper.GetMainParent()")]
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
	
	//-----------------------------------------------------------------------------------------------------------
	//! Fixes the angles in the input vector to be be between -180 and 180
	// unused (and to fix!)
	static vector FixVector180(vector vec)
	{
		for (int a = 0; a < 3; a++)
		{
			float v = vec[a];
			while (v > 180)
				v -= 360;
			while (v < -180)
				v += 360;
			vec[a] = v;
		}
		
		return vec;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Clamps input vector to within world bounds
	//! \param pos Position to clamp to world bounds
	// unused
	static vector GetVectorClampedToWorldBounds(vector pos)
	{
		if (!GetGame().GetWorldEntity().GetWorld().IsOcean())
			return pos;
		
		if (!GetGame().GetWorldEntity())
			return pos;
		
		vector min, max;
		GetGame().GetWorldEntity().GetWorldBounds(min, max);
		
		for (int a = 0; a < 3; a++)
		{
			if (pos[a] < min[a])
				pos[a] = min[a];
			if (pos[a] > max[a])
				pos[a] = max[a];
		}
		
		return pos;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Returns the intersect point between 2 points and a height
	//! \param start Start position to check intersect from
	//! \param end End position to check intersect to
	//! \param result Returned intersect position
	//! \param height World height to do intersect plane at
	// unused
	static bool GetIntersectPointPlane(vector start, vector end, out vector result, float height)
	{
		vector dir = end - start;
		float dist = dir.NormalizeSize();
		
		vector mins = start;
		vector maxs = start;
		
		for (int a = 0; a < 3; a++)
		{
			if (a == 1)
				continue;
			
			float val1 = start[a];
			float val2 = end[a];
			if (val1 < mins[a])
				mins[a] = val1;
			if (val2 < mins[a])
				mins[a] = val2;
			if (val1 > maxs[a])
				maxs[a] = val1;
			if (val2 > maxs[a])
				maxs[a] = val2;
		}
		mins[1] = height;
		maxs[1] = height;
		float intersectPct = Math3D.IntersectRayBox(start, end, mins - "1 0 1", maxs + "1 0 1");
		if (intersectPct < 0)
			return false;
		
		result = dir * dist * intersectPct + start;
		
		return true;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Fills the 2 out strings with distance in the appropriate unit and unit of measurement
	//! \param dist Input distance
	//! \param imperial Whether to use imperial system of units (else uses metric system)
	//! \param tgtDist Output string for distance
	//! \param distFormat Output string for distance units (m, km, etc)
	// unused
	static void GetDistForHUD(float dist, bool imperial, out string tgtDist, out string distFormat)
	{
		if (imperial)
		{
			dist *= 0.9144; // To yards
			if (dist >= 1000)
			{
				dist /= 1760; // To miles
				dist = Math.Floor(dist * 10) * 0.1;
				tgtDist = dist.ToString();
				distFormat = "mi";
			}
			else if (dist >= 100)
			{
				dist = Math.Floor(dist * 10) * 0.1;
				tgtDist = dist.ToString();
				distFormat = "yd";
			}
			else if (dist >= 1)
			{
				dist = Math.Floor(dist * 100) * 0.01;
				tgtDist = dist.ToString();
				distFormat = "yd";
			}
			else
			{
				dist = Math.Floor(dist * 360) * 0.1;
				tgtDist = dist.ToString();
				distFormat = "in";
			}
		}
		else
		{
			if (dist >= 1000)
			{
				dist /= 1000; // To kilometers
				dist = Math.Floor(dist * 10) * 0.1;
				tgtDist = dist.ToString();
				distFormat = "km";
			}
			else if (dist >= 100)
			{
				dist = Math.Floor(dist * 10) * 0.1;
				tgtDist = dist.ToString();
				distFormat = "m";
			}
			else if (dist >= 1)
			{
				dist = Math.Floor(dist * 100) * 0.01;
				tgtDist = dist.ToString();
				distFormat = "m";
			}
			else
			{
				dist = Math.Floor(dist * 1000) * 0.1;
				tgtDist = dist.ToString();
				distFormat = "cm";
			}
		}
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Trace filter callback function - ignores all but for the objects in the g_TraceFilterList list
	static bool FilterCallback_IgnoreNotInList(notnull IEntity target)
	{
		if (g_TraceFilterList.Contains(target))
			return true;
		
		return false;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Trace filter callback function - ignores characters and their children
	// unused
	static bool FilterCallback_IgnoreCharactersWithChildren(notnull IEntity target)
	{
		while (target)
		{
			if (ChimeraCharacter.Cast(target))
				return false;
			
			target = target.GetParent();
		}
		
		return true;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Trace filter callback function - ignores characters
	// unused
	static bool FilterCallback_IgnoreCharacters(notnull IEntity target)
	{
		if (ChimeraCharacter.Cast(target))
			return false;
		
		return true;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Trace filter callback function - ignores g_TraceFilterEnt and its children
	// unused
	static bool FilterCallback_IgnoreEntityWithChildren(notnull IEntity target, vector rayorigin, vector raydirection)
	{
		if (g_TraceFilterEnt == null)
			return true;
		
		while (target)
		{
			if (target == g_TraceFilterEnt)
				return false;
			
			target = target.GetParent();
		}
		
		return true;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Trace filter callback function - ignores all BUT g_TraceFilterEnt and its children
	static bool FilterCallback_IgnoreAllButEntityWithChildren(notnull IEntity target, vector rayorigin, vector raydirection)
	{
		if (g_TraceFilterEnt == null)
			return false;
		
		while (target)
		{
			if (target == g_TraceFilterEnt)
				return true;
			
			target = target.GetParent();
		}
		
		return false;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Trace filter callback function - ignores all BUT g_TraceFilterEnt
	static bool FilterCallback_IgnoreAllButEntity(notnull IEntity target, vector rayorigin, vector raydirection)
	{
		if (g_TraceFilterEnt == null)
			return false;
		
		if (target == g_TraceFilterEnt)
			return true;
		
		return false;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Trace filter callback function - ignores all BUT Characters/Trees
	//! TODO:
	//!   - add small object destruction
	// unused
	static bool FilterCallback_IgnoreAllButMeleeAttackable(notnull IEntity target, vector rayorigin, vector raydirection)
	{
		typename type = target.Type();
		
		if (ChimeraCharacter == type)
			return true;
		
		if (Tree == type)
			return true;
		
		if (Building == type)
			return true;
		
		if (target.FindComponent(DamageManagerComponent))
			return true;
			
		return false;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Trace filter callback function - ignores all but building regions type
	// unused
	static bool FilterCallback_IgnoreAllButBuildingRegions(notnull IEntity target, vector rayorigin, vector raydirection)
	{
		typename type = target.Type();
		if (SCR_BuildingRegionEntity == type)
			return true;
		
		if (SCR_DestructibleBuildingEntity == type)
			return true;
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Scales damage to structural components by damage type and returns final value
	static float GetScaledStructuralDamage(float damage, EDamageType type)
	{
		float dmgScale = 1;
		
		switch (type)
		{
			case EDamageType.REGENERATION:
			case EDamageType.BLEEDING:
			{
				dmgScale = 0;
				break;
			}
		}
		
		return damage * dmgScale;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Returns number of children the input entity has
	[Obsolete("Use SCR_EntityHelper.CountChildren()")]
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
	
	//-----------------------------------------------------------------------------------------------------------
	//! Returns child index for the input child entity, returns -1 if not found
	// unused
	static int GetChildIndex(IEntity ent)
	{
		IEntity parent = ent.GetParent();
		if (!parent)
			return -1;
		
		int index = 0;
		IEntity child = parent.GetChildren();
		while (child)
		{
			if (child == ent)
				return index;
			
			index++;
			child = child.GetSibling();
		}
		
		return -1;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Deletes all children of the input parent entity
	[Obsolete("Use SCR_EntityHelper.DeleteChildren()")]
	static void DeleteChildren(IEntity parent)
	{
		if (!parent) return;
		
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
	[Obsolete("Use SCR_EntityHelper.DeleteEntityAndChildren()")]
	static void DeleteEntityAndChildren(IEntity ent)
	{
		if (ent)
			RplComponent.DeleteRplEntity(ent, false);
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Creates array of ParamEnums filled with pairs of bones names and their indexes in the EntityTagsComponent
	static array<ref ParamEnum> GetBonesAsParamEnums(IEntity entity)
	{
		array<ref ParamEnum> retEnums = new array<ref ParamEnum>;
		array<string> boneNames = new array<string>; 
		entity.GetBoneNames(boneNames);
		
		retEnums.Insert(new ParamEnum("NONE", "-1", "")); // Always have NONE as an option
		foreach (string s : boneNames)
		{
			int nodeid = entity.GetBoneIndex(s);
			retEnums.Insert(new ParamEnum(s, nodeid.ToString(), ""));		
		}
		
		return retEnums;
	}	
	
	//-----------------------------------------------------------------------------------------------------------
	//! Sets the input child entity as a child of the input parent entity, handling adjustments of transformations
	// unused
	static void SetEntityAsChildToParent(IEntity parent, IEntity child)
	{
		vector parentMat[4];
		parent.GetTransform(parentMat);
		
		vector childMat[4];
		child.GetTransform(childMat);
		
		vector childLocalMat[4];
		Math3D.MatrixInvMultiply4(parentMat, childMat, childLocalMat);
		child.SetTransform(childLocalMat);
		
		parent.AddChild(child, -1);
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Returns a scaled vector by scaling each axis separately
	// unused
	static vector ScaleVectorByVector(vector inputVec, vector scaleVec)
	{
		vector result = vector.Zero;
		
		result[0] = inputVec[0] * scaleVec[0];
		result[1] = inputVec[1] * scaleVec[1];
		result[2] = inputVec[2] * scaleVec[2];
		
		return result;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Returns global yaw orientation for input matrix
	// unused
	static float GetGlobalYawForMat(vector mat[4])
	{
		vector fw = mat[0] * vector.Up;
		fw.Normalize();
		vector angs = fw.VectorToAngles();
		
		return angs[0];
	}
	
	//-----------------------------------------------------------------------------------------------------------
	// Lerps between 2 matrices
	// unused
	static void LerpMatrix(vector mat1[4], vector mat2[4], out vector matOut[4], float pct)
	{
		if (pct == 0)
		{
			matOut[0] = mat1[0];
			matOut[1] = mat1[1];
			matOut[2] = mat1[2];
			matOut[3] = mat1[3];
			return;
		}
		else if (pct == 1)
		{
			matOut[0] = mat2[0];
			matOut[1] = mat2[1];
			matOut[2] = mat2[2];
			matOut[3] = mat2[3];
			return;
		}
		
		float q1[4], q2[4], qOut[4];
		Math3D.MatrixToQuat(mat1, q1);
		Math3D.MatrixToQuat(mat2, q2);
		Math3D.QuatLerp(qOut, q1, q2, pct);
		Math3D.QuatToMatrix(qOut, matOut);
		matOut[3] = (mat2[3] - mat1[3]) * pct + mat1[3];
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Returns local matrix of input matrices
	// unused
	static void GetLocalMatrix(vector parentMat[4], vector childMat[4])
	{
		vector childOrigMat[4];
		Math3D.MatrixCopy(childMat, childOrigMat);
		Math3D.MatrixInvMultiply4(parentMat, childOrigMat, childMat);
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Returns world bounds from input matrix and bounds
	// unused
	static void GetWorldBoundsForEntity(vector mat[4], out vector mins, out vector maxs)
	{
		vector pt1, pt2;
		pt1 = mins.Multiply4(mat);
		pt2 = maxs.Multiply4(mat);
		mins = pt1;
		maxs = pt2;
		for (int i = 0; i < 3; i++)
		{
			if (pt1[i] < mins[i])
				mins[i] = pt1[i];
			if (pt2[i] < mins[i])
				mins[i] = pt2[i];
			if (pt1[i] > maxs[i])
				maxs[i] = pt1[i];
			if (pt2[i] > maxs[i])
				maxs[i] = pt2[i];
		}
	}
	/*!
	Get world bounds of whole entity hierarchy, i.e., of itself and all its children.
	\param entity Evaluated entity
	\param[out] min Bounding box minimum
	\param[out] max Bounding box maximum
	*/
	static void GetWorldBoundsWithChildren(IEntity entity, out vector min, out vector max, bool isChild = false)
	{
		if (!entity)
			return;
		
		if (!isChild)
		{
			min = Vector(float.MAX, float.MAX, float.MAX);
			max = -Vector(float.MAX, float.MAX, float.MAX);
		}
		
		if (entity.GetVObject())
		{
			vector entityMin, entityMax;
			entity.GetWorldBounds(entityMin, entityMax);
			
			min[0] = Math.Min(min[0], entityMin[0]);
			min[1] = Math.Min(min[1], entityMin[1]);
			min[2] = Math.Min(min[2], entityMin[2]);
			
			max[0] = Math.Max(max[0], entityMax[0]);
			max[1] = Math.Max(max[1], entityMax[1]);
			max[2] = Math.Max(max[2], entityMax[2]);
		}
		
		entity = entity.GetChildren();
		while (entity)
		{
			GetWorldBoundsWithChildren(entity, min, max, true);
			entity = entity.GetSibling();
		}
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Returns whether object 1 is at the desired position in object 2
	// unused
	static bool GetObjectAtOffsetFromObject(IEntity parent, IEntity child, vector localPos, vector localAng, float tolerancePos, float toleranceAng)
	{
		vector parentMat[4];
		parent.GetTransform(parentMat);
		
		vector offsetMat[4];
		Math3D.AnglesToMatrix(localAng, offsetMat);
		offsetMat[3] = localPos;
		
		vector parentOffsetMat[4];
		Math3D.MatrixMultiply4(parentMat, offsetMat, parentOffsetMat);
		
		vector childMat[4];
		child.GetTransform(childMat);
		
		vector relativeMat[4];
		Math3D.MatrixInvMultiply4(parentOffsetMat, childMat, relativeMat);
		
		vector pos = relativeMat[3];
		if (pos.Length() > tolerancePos)
			return false;
		
		float cross;
		vector dir;
		
		dir = relativeMat[0];
		cross = dir * vector.Right;
		if (Math.AbsFloat(cross) > toleranceAng)
			return false;
		
		dir = relativeMat[1];
		cross = dir * vector.Up;
		if (Math.AbsFloat(cross) > toleranceAng)
			return false;
		
		dir = relativeMat[2];
		cross = dir * vector.Forward;
		if (Math.AbsFloat(cross) > toleranceAng)
			return false;
		
		return true;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Returns whether input point is within bounds
	static bool IntersectBoxPoint(vector pos, vector mins, vector maxs)
	{
		for (int i = 0; i < 3; i++)
		{
			if (pos[i] > maxs[i])
				return false;
			if (pos[i] < mins[i])
				return false;
		}
		
		return true;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Returns whether input sphere intersects with bounds with optional intersection distance output (negative values are within the box)
	// unused
	static bool IntersectBoxSphere(vector center, float radius, vector mins, vector maxs, out float intersectDist = 0)
	{
		float dist_sq = -Math.Pow(radius, 2);
		
		if (center[0] < mins[0])
			dist_sq += Math.Pow(center[0] - mins[0], 2);
		else if (center[0] > maxs[0])
			dist_sq += Math.Pow(center[0] - maxs[0], 2);
		if (center[1] < mins[1])
			dist_sq += Math.Pow(center[1] - mins[1], 2);
		else if (center[1] > maxs[1])
			dist_sq += Math.Pow(center[1] - maxs[1], 2);
		if (center[2] < mins[2])
			dist_sq += Math.Pow(center[2] - mins[2], 2);
		else if (center[2] > maxs[2])
			dist_sq += Math.Pow(center[2] - maxs[2], 2);
		
		if (dist_sq < 0)
			intersectDist = -Math.Sqrt(-dist_sq);
		else
			intersectDist = Math.Sqrt(dist_sq);
		
		if (dist_sq < 0)
			return true;
		else
			return false;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Limit float between -1 and 1
	static float FractionOf(float input, float fracOf)
	{
		float result = input / fracOf;
		
		while (result >= 1)
			result -= 1;
		while (result <= -1)
			result += 1;
		
		return result;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Clamp value to a grid size
	// local usage, can be protected
	static float ClampToGrid(float input, float grid)
	{
		float frac = FractionOf(input, grid);
		
		if (frac >= 0.5) input += (1 - frac) * grid;
		else if (frac > 0 && frac < 0.5) input -= frac * grid;
		else if (frac <= -0.5) input -= (1 + frac) * grid;
		else if (frac < 0 && frac > -0.5) input += -frac * grid;
		
		return input;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Returns relative yaw/pitch/roll to position from input view matrix
	// unused
	static vector GetDirectionAngles(vector viewMat[4], vector posTo)
	{
		vector dir = posTo - viewMat[3];
		dir.Normalize();
		vector dirAng = dir.InvMultiply3(viewMat);
		dirAng = dirAng.VectorToAngles();
		dirAng = dirAng.MapAngles();
		
		return dirAng;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Takes two floats and returns the remainder after division
	// move to SCR_Math?
	static float fmod(float dividend, float divisor)
	{
		if (divisor != 0)
			return dividend - Math.Floor(dividend/divisor) * divisor;
		return 0;
	
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Takes input matrix and keeps it within the input bounds
	// unused
	static void WorldClampMatrixWithinBounds(vector mat[4], vector mins, vector maxs)
	{
		vector pos = mat[3];
		for (int i = 0; i < 3; i++)
		{
			if (pos[i] > maxs[i])
				pos[i] = maxs[i];
			if (pos[i] < mins[i])
				pos[i] = mins[i];
		}
		mat[3] = pos;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Return full color using input vector for color and input alpha (0 - 255)
	// unused
	static int VectorToRGBA255(vector colorVec, float alpha)
	{
		float x,y,z;
		int r,g,b,a;
	
		x = colorVec[0];
		y = colorVec[1];
		z = colorVec[2];
	
		a = (int)alpha << 24;
		r = (int)x << 16;
		g = (int)y << 8;
		b = z;
	
		return r | g | b | a;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Takes input object and matrix and keeps it within the input bounds and optional gridsnap
	// unused
	static void WorldClampObjectAndMatrixWithinBounds(GenericEntity ent, vector mat[4], vector mins, vector maxs, vector gridSize)
	{
		bool snapToGrid = false;
		if (gridSize != vector.Zero)
			snapToGrid = true;
		
		vector entmins, entmaxs;
		ent.GetBounds(entmins, entmaxs);
		vector point1 = entmins.Multiply3(mat);
		vector point2 = entmaxs.Multiply3(mat);
		entmins = point1;
		entmaxs = point2;
		for (int i = 0; i < 3; i++)
		{
			if (point1[i] < entmins[i])
				entmins[i] = point1[i];
			if (point2[i] < entmins[i])
				entmins[i] = point2[i];
			if (point1[i] > entmaxs[i])
				entmaxs[i] = point1[i];
			if (point2[i] > entmaxs[i])
				entmaxs[i] = point2[i];
		}
		
		vector pos = mat[3];
		for (int i = 0; i < 3; i++)
		{
			float posAxis = pos[i];
			float min = entmins[i];
			float max = entmaxs[i];
			float grid = gridSize[i];
			
			if (snapToGrid)
			{
				min = ClampToGrid(min, grid * 0.5);
				max = ClampToGrid(max, grid * 0.5);
			}
			if (pos[i] > maxs[i] - max)
				posAxis = maxs[i] - max;
			if (pos[i] < mins[i] - min)
				posAxis = mins[i] - min;
			
			pos[i] = posAxis;
		}
		mat[3] = pos;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Takes input object and matrix and keeps it within the input bounds and optional gridsnap
	// unused
	static void LocalClampObjectAndMatrixWithinBounds(vector localMat[4], GenericEntity ent, vector origMat[4], vector localMins, vector localMaxs, vector gridSize)
	{
		vector mat[4];
		Math3D.MatrixInvMultiply4(localMat, origMat, mat);
		
		bool snapToGrid = false;
		if (gridSize != vector.Zero)
			snapToGrid = true;
		
		vector entmins, entmaxs;
		ent.GetBounds(entmins, entmaxs);
		vector point1 = entmins.Multiply3(mat);
		vector point2 = entmaxs.Multiply3(mat);
		entmins = point1;
		entmaxs = point2;
		for (int i = 0; i < 3; i++)
		{
			if (point1[i] < entmins[i])
				entmins[i] = point1[i];
			if (point2[i] < entmins[i])
				entmins[i] = point2[i];
			if (point1[i] > entmaxs[i])
				entmaxs[i] = point1[i];
			if (point2[i] > entmaxs[i])
				entmaxs[i] = point2[i];
		}
		
		vector pos = mat[3];
		for (int i = 0; i < 3; i++)
		{
			float posAxis = pos[i];
			float min = entmins[i];
			float max = entmaxs[i];
			float grid = gridSize[i];
			
			if (snapToGrid)
			{
				min = ClampToGrid(min, grid * 0.5);
				max = ClampToGrid(max, grid * 0.5);
			}
			if (pos[i] > localMaxs[i] - max)
				posAxis = localMaxs[i] - max;
			if (pos[i] < localMins[i] - min)
				posAxis = localMins[i] - min;
			
			pos[i] = posAxis;
		}
		mat[3] = pos;
		
		Math3D.MatrixMultiply4(localMat, mat, origMat);
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Take the input matrix and snaps the rotation to 90 degree, world-oriented angles. Also snaps the position to the input grid size
	// unused
	static void WorldSnapMatrix(vector mat[4], vector gridSize)
	{
		// Snap position
		if (gridSize != vector.Zero)
		{
			vector tempPos = mat[3];
			for (int i = 0; i < 3; i++)
			{
				float grid = gridSize[i];
				float pos = ClampToGrid(tempPos[i], grid);
				
				tempPos[i] = pos;
			}
			mat[3] = tempPos;
		}
		
		// Create world oriented normals
		vector norms[6];
		norms[0] = "1 0 0";
		norms[1] = "0 1 0";
		norms[2] = "0 0 1";
		norms[3] = "-1 0 0";
		norms[4] = "0 -1 0";
		norms[5] = "0 0 -1";
	
		// Snap orientation
		vector mat0 = mat[0];
		vector mat1 = mat[1];
		vector mat2 = mat[2];
		vector bestVecs[3];
		float bestVecsVal[3];
		bestVecsVal[0] = -1;
		bestVecsVal[1] = -1;
		bestVecsVal[2] = -1;
	
		for (int i = 0; i < 6; i++)
		{
			vector norm = norms[i];
			float calc = mat0 * norm;
			if (calc > bestVecsVal[0])
			{
				bestVecsVal[0] = calc;
				bestVecs[0] = norms[i];
			}
			
			calc = mat1 * norm;
			if (calc > bestVecsVal[1])
			{
				bestVecsVal[1] = calc;
				bestVecs[1] = norms[i];
			}
			
			calc = mat2 * norm;
			if (calc > bestVecsVal[2])
			{
				bestVecsVal[2] = calc;
				bestVecs[2] = norms[i];
			}
		}
		
		float bestAxisVal1 = -1;
		int bestAxisNum1;
		for (int i = 0; i < 3; i++)
		{
			if (bestVecsVal[i] > bestAxisVal1)
			{
				bestAxisVal1 = bestVecsVal[i];
				bestAxisNum1 = i;
			}
		}
		float bestAxisVal2 = -1;
		int bestAxisNum2;
		for (int i = 0; i < 3; i++)
		{
			if (i == bestAxisNum1)
				continue;
			
			if (bestVecsVal[i] > bestAxisVal2)
			{
				bestAxisVal2 = bestVecsVal[i];
				bestAxisNum2 = i;
			}
		}
		
		vector bestAxis1 = bestVecs[bestAxisNum1];
		vector bestAxis2 = bestVecs[bestAxisNum2];
		vector bestAxis3;
		if (bestAxisNum1 == 2 && bestAxisNum2 == 0)
			bestAxis3 = bestAxis1 * bestAxis2;
		else if (bestAxisNum1 == 0 && bestAxisNum2 == 2)
			bestAxis3 = bestAxis2 * bestAxis1;
		else if (bestAxisNum2 < bestAxisNum1)
			bestAxis3 = bestAxis2 * bestAxis1;
		else
			bestAxis3 = bestAxis1 * bestAxis2;
	
		for (int i = 0; i < 3; i++)
		{
			if (i == bestAxisNum1 || i == bestAxisNum2)
				continue;
			
			mat[i] = bestAxis3;
			break;
		}
		mat[bestAxisNum1] = bestAxis1;
		mat[bestAxisNum2] = bestAxis2;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Take the input matrix and snaps the rotation to 90 degree, local-oriented angles. Also snaps the position to the input grid size
	// unused
	static void LocalSnapMatrix(vector localMat[4], vector origMat[4], vector gridSize)
	{
		vector mat[4];
		Math3D.MatrixInvMultiply4(localMat, origMat, mat);
		
		// Snap position
		if (gridSize != vector.Zero)
		{
			vector tempPos = mat[3];
			for (int i = 0; i < 3; i++)
			{
				float grid = gridSize[i];
				float pos = ClampToGrid(tempPos[i], grid);
				
				tempPos[i] = pos;
			}
			mat[3] = tempPos;
		}
		
		// Create world oriented normals
		vector norms[6];
		norms[0] = "1 0 0";
		norms[1] = "0 1 0";
		norms[2] = "0 0 1";
		norms[3] = "-1 0 0";
		norms[4] = "0 -1 0";
		norms[5] = "0 0 -1";
	
		// Snap orientation
		vector mat0 = mat[0];
		vector mat1 = mat[1];
		vector mat2 = mat[2];
		vector bestVecs[3];
		float bestVecsVal[3];
		bestVecsVal[0] = -1;
		bestVecsVal[1] = -1;
		bestVecsVal[2] = -1;
	
		for (int i = 0; i < 6; i++)
		{
			vector norm = norms[i];
			float calc = mat0 * norm;
			if (calc > bestVecsVal[0])
			{
				bestVecsVal[0] = calc;
				bestVecs[0] = norms[i];
			}
			
			calc = mat1 * norm;
			if (calc > bestVecsVal[1])
			{
				bestVecsVal[1] = calc;
				bestVecs[1] = norms[i];
			}
			
			calc = mat2 * norm;
			if (calc > bestVecsVal[2])
			{
				bestVecsVal[2] = calc;
				bestVecs[2] = norms[i];
			}
		}
		
		float bestAxisVal1 = -1;
		int bestAxisNum1;
		for (int i = 0; i < 3; i++)
		{
			if (bestVecsVal[i] > bestAxisVal1)
			{
				bestAxisVal1 = bestVecsVal[i];
				bestAxisNum1 = i;
			}
		}
		float bestAxisVal2 = -1;
		int bestAxisNum2;
		for (int i = 0; i < 3; i++)
		{
			if (i == bestAxisNum1)
				continue;
			
			if (bestVecsVal[i] > bestAxisVal2)
			{
				bestAxisVal2 = bestVecsVal[i];
				bestAxisNum2 = i;
			}
		}
		
		vector bestAxis1 = bestVecs[bestAxisNum1];
		vector bestAxis2 = bestVecs[bestAxisNum2];
		vector bestAxis3;
		if (bestAxisNum1 == 2 && bestAxisNum2 == 0)
			bestAxis3 = bestAxis1 * bestAxis2;
		else if (bestAxisNum1 == 0 && bestAxisNum2 == 2)
			bestAxis3 = bestAxis2 * bestAxis1;
		else if (bestAxisNum2 < bestAxisNum1)
			bestAxis3 = bestAxis2 * bestAxis1;
		else
			bestAxis3 = bestAxis1 * bestAxis2;
	
		for (int i = 0; i < 3; i++)
		{
			if (i == bestAxisNum1 || i == bestAxisNum2)
				continue;
			
			mat[i] = bestAxis3;
			break;
		}
		mat[bestAxisNum1] = bestAxis1;
		mat[bestAxisNum2] = bestAxis2;
		
		Math3D.MatrixMultiply4(localMat, mat, origMat);
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Returns 2D surface area (in m^2) for the input bound box and matrix
	// can be protected
	static float GetMatAndBoundsSurfaceAreaInDir(IEntity ent, vector dir, vector mat[4], vector mins, vector maxs)
	{
		float min_x, min_y, min_z;
		float max_x, max_y, max_z;
		min_x = mins[0];
		min_y = mins[1];
		min_z = mins[2];
		max_x = maxs[0];
		max_y = maxs[1];
		max_z = maxs[2];
		
		float surfArea_top = Math.AbsFloat(max_z - min_z) * Math.AbsFloat(max_x - min_x);
		float surfArea_right = Math.AbsFloat(max_z - min_z) * Math.AbsFloat(max_y - min_y);
		float surfArea_front = Math.AbsFloat(max_x - min_x) * Math.AbsFloat(max_y - min_y);
		
		vector objRt = mat[0];
		vector objUp = mat[1];
		vector objFw = mat[2];
		
		float result = Math.AbsFloat(dir * objFw * surfArea_front);
		result += Math.AbsFloat(dir * objRt * surfArea_right);
		result += Math.AbsFloat(dir * objUp * surfArea_top);
		
		return result;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Returns 2D surface area (in m^2) of the object's bounding box exposed in the input direction
	static float GetSurfaceAreaInDir(IEntity ent, vector dir)
	{
		vector matObj[4];
		ent.GetTransform(matObj);
		
		vector mins, maxs;
		ent.GetBounds(mins, maxs); // Get the bounding box
		
		return GetMatAndBoundsSurfaceAreaInDir(ent, dir, matObj, mins, maxs);
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Returns the size of an entity from its bounding box
	[Obsolete("Use SCR_EntityHelper.GetEntitySize()")]
	static vector GetEntitySize(IEntity ent)
	{
		vector entMins, entMaxs;
		ent.GetBounds(entMins, entMaxs);
	
		return entMaxs - entMins;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Returns the center of the entity from its bounding box in local coordinates
	// unused
	static vector GetEntityCenterLocal(IEntity ent)
	{
		vector entMins, entMaxs;
		ent.GetBounds(entMins, entMaxs);
		
		return (entMaxs + entMins) * 0.5;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Returns the center of the entity from its bounding box in world coordinates
	[Obsolete("Use SCR_EntityHelper.GetEntityCenterWorld()")]
	static vector GetEntityCenterWorld(IEntity ent)
	{
		vector entMins, entMaxs;
		ent.GetBounds(entMins, entMaxs);
		vector result = (entMaxs + entMins) * 0.5;
		return ent.CoordToParent(result);
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Returns the radius of the entity based on the length of its bounding box
	[Obsolete("Use SCR_EntityHelper.GetEntityRadius()")]
	static float GetEntityRadius(IEntity ent)
	{
		float r = GetEntitySize(ent).Length() * 0.5;
		
		return r;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Returns whether the input string contains invalid characters (for file names)
	// unused
	static bool GetStringContainsInvalidCharacters(string s)
	{
		int ascii_FwSlash = 47;
		int ascii_BkSlash = 92;
		int ascii_Quote = 34;
		
		if (s == "") return true;
		else if (s.Contains("~")) return true;
		else if (s.Contains("#")) return true;
		else if (s.Contains("%")) return true;
		else if (s.Contains("\"")) return true;
		else if (s.Contains("&")) return true;
		else if (s.Contains("*")) return true;
		else if (s.Contains("{")) return true;
		else if (s.Contains("}")) return true;
		else if (s.Contains(ascii_BkSlash.AsciiToString())) return true;
		else if (s.Contains(":")) return true;
		else if (s.Contains("<")) return true;
		else if (s.Contains(">")) return true;
		else if (s.Contains("?")) return true;
		else if (s.Contains(ascii_FwSlash.AsciiToString())) return true;
		else if (s.Contains("|")) return true;
		else if (s.Contains(ascii_Quote.AsciiToString())) return true;
		
		return false;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Show the name of the entity above it using the input camera index for direction etc
	// unused
	static void DisplayEntityNameText(IEntity ent, int color, int camIndex)
	{
		vector pos = GetEntityCenterWorld(ent);
		
		vector textMat[4];
		ent.GetWorld().GetCamera(camIndex, textMat);
		
		float distScale = vector.Distance(textMat[3], pos) * 0.07;
		distScale = Math.Clamp(distScale, 0.5, 10);
		
		float textEndSize = (0.2 * distScale) / vector.Distance(textMat[3], ent.GetOrigin());
		if (textEndSize < 0.005)
			return;
		
		textMat[3] = pos + textMat[1] * 0.7;
		CreateSimpleText(ent.GetName(), textMat, 0.2 * distScale, color, ShapeFlags.NOZBUFFER | ShapeFlags.ONCE, null);
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Called from SetHierarchyTransform
	[Obsolete("Use SCR_EntityHelper.SetHierarchyChildTransform()")]
	private static void SetHierarchyChildTransform(notnull IEntity ent, vector oldTransform[4], vector newTransform[4], bool recursive = true)
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
	
	//-----------------------------------------------------------------------------------------------------------
	//! Set transform for whole hierarchy
	[Obsolete("Use SCR_EntityHelper.SetHierarchyTransform()")]
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
	
	//-----------------------------------------------------------------------------------------------------------
	//! Called from SetHierarchyVelocity
	protected static void SetHierarchyChildVelocity(notnull IEntity ent, vector newVelocity, bool recursive = true)
	{
		Physics entPhys = ent.GetPhysics();
		if (entPhys)
		{
			if (entPhys.IsDynamic())
				entPhys.SetVelocity(newVelocity);
		}
		
		IEntity child = ent.GetChildren();
		while (child)
		{
			SetHierarchyChildVelocity(child, newVelocity, recursive);
			child = child.GetSibling();
		}
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Set linear velocity for whole hierarchy
	// unused
	static void SetHierarchyVelocity(notnull IEntity ent, vector newVelocity)
	{
		Physics entPhys = ent.GetPhysics();
		if (entPhys)
		{
			if (entPhys.IsDynamic())
				entPhys.SetVelocity(newVelocity);
		}
		
		IEntity child = ent.GetChildren();
		while (child)
		{
			SetHierarchyChildVelocity(child, newVelocity, true);
			child = child.GetSibling();
		}
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Called from SetHierarchyAngularVelocity
	protected static void SetHierarchyChildAngularVelocity(notnull IEntity ent, vector newAngularVelocity, IEntity entFrom, bool recursive = true)
	{
		Physics entPhys = ent.GetPhysics();
		Physics entFromPhys = entFrom.GetPhysics();
		if (entPhys)
		{	
			if (entPhys.IsDynamic())
			{
				entPhys.SetAngularVelocity(newAngularVelocity);
				if (entFrom && entFromPhys && entFromPhys.IsDynamic())
				{
					vector velAt = entFromPhys.GetVelocityAt(ent.GetOrigin());
					entPhys.SetVelocity(velAt);
				}
			}
			
		}
		
		IEntity child = ent.GetChildren();
		while (child)
		{
			SetHierarchyChildAngularVelocity(child, newAngularVelocity, entFrom, recursive);
			child = child.GetSibling();
		}
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Set angular velocity for whole hierarchy
	// unused
	static void SetHierarchyAngularVelocity(notnull IEntity ent, vector newAngularVelocity)
	{
		Physics entPhys = ent.GetPhysics();
		if (entPhys && entPhys.IsDynamic())
			entPhys.SetAngularVelocity(newAngularVelocity);
		
		IEntity child = ent.GetChildren();
		while (child)
		{
			SetHierarchyChildAngularVelocity(child, newAngularVelocity, ent, true);
			child = child.GetSibling();
		}
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Called from SetHierarchyBodyActive
	protected static void SetHierarchyChildBodyActive(notnull IEntity ent, ActiveState activeState, bool recursive = true, bool resetVelocity = false)
	{
		Physics entPhys = ent.GetPhysics();
		if (entPhys)
		{	
			if (entPhys.IsDynamic())
			{
				entPhys.SetActive(activeState);
				if (resetVelocity)
				{
					entPhys.SetVelocity( vector.Zero );
					entPhys.SetAngularVelocity( vector.Zero );
				}
				
			}
		}
		
		IEntity child = ent.GetChildren();
		while (child)
		{
			SetHierarchyChildBodyActive(child, activeState, resetVelocity);
			child = child.GetSibling();
		}
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Sets physics active state for whole hierarchy
	// unused
	static void SetHierarchyBodyActive(notnull IEntity ent, ActiveState activeState, bool resetVelocity = false)
	{
		Physics entPhys = ent.GetPhysics();
		if (entPhys)
		{	
			if (entPhys.IsDynamic())
			{
	
				entPhys.SetActive(activeState);
				if (resetVelocity)
				{
					entPhys.SetVelocity( vector.Zero );
					entPhys.SetAngularVelocity( vector.Zero );
				}
				
			}
		}
		
		IEntity child = ent.GetChildren();
		while (child)
		{
			SetHierarchyChildBodyActive(child, activeState, true);
			child = child.GetSibling();
		}
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Returns a list of all entities in the hierarchy
	[Obsolete("Use SCR_EntityHelper.GetHierarchyEntityList()")]
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
	
	//! Returns true if the given entity is inherited from list of typesnames given as argument
	// unused
	static bool IsAnyInherited(notnull IEntity entity, array<typename> typenames)
	{
		bool ret;
		for( int i = 0; i < typenames.Count(); i++ )
		{
			ret = ret || entity.IsInherited( typenames.Get(i) );
		}
		
		return ret;
	}
	
	/**
	\brief Given array of weights, get a random index based on the weight values, ie in an array [80,15,5], 80% of the time, we get index 0, 15% index 1 and 5% index 2, weight sum does not have to add up to any particular value, ie [100,100,100,100] gives equal 25% distribution(provided random distribution) for all 4 indices
		\param weights \p array with weights
		\param value \p a value between [0..1], can be random, perlin, or any other
		\return \p int index selected based on weight
		@code
			array<float> weights = new array<float>;
			weights.Insert(80);
			weights.Insert(15);
			weights.Insert(5);
			
			float rand01;
			int index;
		
			rand01 = Math.RandomFloat01();
			index = SCR_Global.GetWeighedElement(weights,rand01);
			Print(index);
			rand01 = Math.RandomFloat01();
			index = SCR_Global.GetWeighedElement(weights,rand01);
			Print(index);
			rand01 = Math.RandomFloat01();
			index = SCR_Global.GetWeighedElement(weights,rand01);
			Print(index);
	
			>> 'index =   0'
			>> 'index =   1'
			>> 'index =   0'
		@endcode
	*/
	[Obsolete("Use SCR_ArrayHelper.GetWeighedElement()")]
	static int GetWeighedElement(array<float> weights, float value)// TODO: Move to general functions
	{
		float probabilitySum;
	
		for(int i = 0; i < weights.Count(); i++)
		{
			probabilitySum += weights[i];
		}
		
		float numberAdjusted = value * probabilitySum;
		
		float add = 0;

		for(int i = 0; i < weights.Count(); i++)
		{
			add += weights[i];
			
			if( add > numberAdjusted )
			{
				return i;
			}
		}
		return 0;
	}
	
	

	static bool ApplyDamage(IEntity entity, IEntity weapon, float dmg, EDamageType dmgType, string hitZone, SurfaceProperties surface, int nodeIdx)
	{
		vector hitPosDirNorm[3];

		/*
		//! destructible trees
		Tree tree = Tree.Cast(entity);
		if (tree)
		{
			tree.OnDamage(
				dmg,
			 	dmgType,
			  	entity,
			  	hitPosDirNorm,
			  	weapon, 
			  	null, 
			  	hitZone,
				0, 0);

			return true;
		}
		else
		{
			Building building = Building.Cast(entity);
			if (building)
			{
				building.OnDamage(
				dmg,
			 	dmgType,
			  	entity,
			  	hitPosDirNorm,
			  	weapon, 
			  	null, 
			  	hitZone, 
			  	0, 0);
				
				return true;
			}
		}
		*/
		//! Keep that as the last check		
		GenericEntity genEnt = GenericEntity.Cast(entity);
		if (!genEnt)
			return false;

		//! check if the entity has the damage manager component
		DamageManagerComponent dmc = DamageManagerComponent.Cast(genEnt.FindComponent(DamageManagerComponent));
		if (dmc)
		{
			//! no hitzone given, use the default one if exist
			bool useDefaultHZ = false;
			if (hitZone.Empty)
				useDefaultHZ = true;

			if (useDefaultHZ && dmc.GetDefaultHitZone())
				hitZone = dmc.GetDefaultHitZone().GetName();


			
			return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check whether entity has actions manager and return it if it exists.
	//! \param entity The entity to find the actions manager component on.
	//! \return True when actions manager is present, false otherwise.
	// unused
	static ActionsManagerComponent FindActionsManagerComponent( IEntity entity, bool activeOnly = true) 
	{
		auto genericEntity = GenericEntity.Cast(entity);
		if (!genericEntity)
			return null;
		
		auto actionsManager = ActionsManagerComponent.Cast(genericEntity.FindComponent(ActionsManagerComponent));
		if (actionsManager != null)
		{
			if (activeOnly && !actionsManager.IsActive())
				return null;
			
			return actionsManager;
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check whether entity has editor actions manager and return it if it exists.
	//! \param entity The entity to find the actions manager component on.
	//! \return True when actions manager is present, false otherwise.
	// unused
	static SCR_EditorActionsManagerComponent FindEditorActionsManagerComponent( IEntity entity ) 
	{
		auto genericEntity = GenericEntity.Cast(entity);
		if (!genericEntity)
			return null;
		
		auto actionsManager = SCR_EditorActionsManagerComponent.Cast(genericEntity.FindComponent(SCR_EditorActionsManagerComponent));
		if (actionsManager != null)
			return actionsManager;
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Read the input resource and returns whether a model was successfully found, filling the output model and remap paths
	static bool GetModelAndRemapFromResource(ResourceName resourcePath, out ResourceName modelPath, out string remap)
	{
		modelPath = ResourceName.Empty;
		remap = string.Empty;
		
		if (resourcePath == ResourceName.Empty)
			return false;
		
		Resource resource = Resource.Load(resourcePath);
		if (!resource)
			return false;
		
		BaseResourceObject prefabBase = resource.GetResource();
		if (!prefabBase)
			return false;
		
		// Is a model not a prefab
		if (prefabBase.ToVObject())
		{
			modelPath = resourcePath;
			
			return true;
		}
		
		// Prefab
		BaseContainer prefabSrc = prefabBase.ToBaseContainer();
		if (!prefabSrc)
			return false;
		
		BaseContainerList components = prefabSrc.GetObjectArray("components");
		if (!components)
			return false;
		
		BaseContainer meshComponent = null;
		for (int c = components.Count() - 1; c >= 0; c--)
		{
			meshComponent = components.Get(c);
			if (meshComponent.GetClassName() == "MeshObject")
				break;
			
			meshComponent = null;
		}
		
		if (!meshComponent)
			return false;
		
		meshComponent.Get("Object", modelPath);
		BaseContainerList materialsRemap = meshComponent.GetObjectArray("Materials");
		if (materialsRemap)
		{
			ResourceName matSrc = ResourceName.Empty;
			ResourceName matTgt = ResourceName.Empty;
			for (int m = materialsRemap.Count() - 1; m >= 0; m--)
			{
				BaseContainer material = materialsRemap.Get(m);
				
				material.Get("SourceMaterial", matSrc);
				material.Get("AssignedMaterial", matTgt);
				remap += "$remap '" + matSrc + "' '" + matTgt + "';";
			}
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Read the input resource and returns whether it contains a component on input name, filling whether the resource is a prefab
	static bool GetResourceContainsComponent(ResourceName resourcePath, string componentClassName, out bool isPrefab)
	{
		isPrefab = false;
		
		if (resourcePath == ResourceName.Empty)
			return false;
		
		Resource resource = Resource.Load(resourcePath);
		if (!resource)
			return false;
		
		BaseResourceObject prefabBase = resource.GetResource();
		if (!prefabBase)
			return false;
		
		// Is not a prefab
		if (!prefabBase.ToEntitySource())
			return false;
		
		// Prefab
		BaseContainer prefabSrc = prefabBase.ToBaseContainer();
		if (!prefabSrc)
			return false;
		
		isPrefab = true;
		
		BaseContainerList components = prefabSrc.GetObjectArray("components");
		if (!components)
			return false;
		
		BaseContainer component = null;
		for (int c = components.Count() - 1; c >= 0; c--)
		{
			component = components.Get(c);
			if (component.GetClassName() == componentClassName)
				return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check if the game is running in edit mode.
	//! \return True during edit mode in Workbench
	static bool IsEditMode()
	{
		return !GetGame().InPlayMode();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check if the entity is running in edit mode.
	//! \param entity Entity whose world will be checked
	//! \return True during edit mode in Workbench
	static bool IsEditMode(notnull IEntity entity)
	{
		BaseWorld world = entity.GetWorld();
		return world && world.IsEditMode();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Calculates the world direction of the mouse cursor projected into world space in the World Editor
	//! \param referenceEntity Entity to get the world, camera, and editor API from
	//! \return Direction in world space
	static vector ProjWorldEditorMouseScreenToWorld(GenericEntity referenceEntity)
	{
		vector camMat[4];
		referenceEntity.GetWorld().GetCurrentCamera(camMat);
		vector dir = camMat[2];
		
		#ifdef WORKBENCH
			WorldEditorAPI worldEdAPI = referenceEntity._WB_GetEditorAPI();
			WorkspaceWidget workspace = GetGame().GetWorkspace();	
			if (worldEdAPI && workspace)
			{
				int imouseX = worldEdAPI.GetMousePosX(true);
				int imouseY = worldEdAPI.GetMousePosY(true);
				int realScreenX = worldEdAPI.GetScreenWidth();
				int realScreenY = worldEdAPI.GetScreenHeight();
				if (imouseX > 0 && imouseX < realScreenX && imouseY > 0 && imouseY < realScreenY)
				{
					float xMult = realScreenX / realScreenY;
					float mouseXPct = (imouseX / realScreenX - 0.5) * xMult;
					float mouseYPct = imouseY / realScreenY;
					float width, height;
					workspace.GetScreenSize(width, height);
					float mouseX = mouseXPct * width + width * 0.5;
					float mouseY = mouseYPct * height;
					workspace.ProjScreenToWorldNative(mouseX, mouseY, dir, referenceEntity.GetWorld(), referenceEntity.GetWorld().GetCurrentCameraId());
				}
			}
		#endif
		
		return dir;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get terrain height at given position.
	\param[out] pos World position
	\param world World to be checked (default world is used when undefined)
	\param noUnderwater When true, sea surface will be used instead of seabed
	\return Position above sea
	*/
	[Obsolete("Use SCR_TerrainHelper.GetTerrainY()")]
	static float GetTerrainY(vector pos, BaseWorld world = null, bool noUnderwater = false)
	{
		if (!world) world = GetGame().GetWorld();
		if (!world) return 0;
		
		float surfaceY = world.GetSurfaceY(pos[0], pos[2]);
		if (noUnderwater) surfaceY = Math.Max(surfaceY, 0);
		return surfaceY;
	}
	/*!
	Get height above terrain of given position.
	\param[out] pos World position
	\param world World to be checked (default world is used when undefined)
	\param noUnderwater When true, sea surface will be used instead of seabed
	\return Position above terrain
	*/
	[Obsolete("Use SCR_TerrainHelper.GetHeightAboveTerrain()")]
	static float GetHeightAboveTerrain(vector pos, BaseWorld world = null, bool noUnderwater = false)
	{
		if (!world) world = GetGame().GetWorld();
		if (!world) return 0;
		
		float surfaceY = world.GetSurfaceY(pos[0], pos[2]);
		if (noUnderwater) surfaceY = Math.Max(surfaceY, 0);
		return pos[1] - surfaceY;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get terrain normal vector on given position.
	\param[out] pos World position, its vertical axis will be modified to be the surface height
	\param world World to be checked (default world is used when undefined)
	\param noUnderwater When true, sea surface will be used instead of seabed
	\return Normal vector
	*/
	[Obsolete("Use SCR_TerrainHelper.GetTerrainNormal()")]
	static vector GetTerrainNormal(out vector pos, BaseWorld world = null, bool noUnderwater = false)
	{
		//--- Get world
		if (!world) world = GetGame().GetWorld();
		if (!world) return vector.Zero;

		//--- Get surface position
		float surfaceY = world.GetSurfaceY(pos[0], pos[2]);
		if (noUnderwater && surfaceY < 0)
		{
			pos[1] = Math.Max(surfaceY, 0);
			return vector.Up;
		}
		
		//--- Get surface normal
		pos[1] = surfaceY;
		TraceParam trace = new TraceParam;
		trace.Start = pos + vector.Up;
		trace.End = pos - vector.Up;
		trace.Flags = TraceFlags.WORLD;
		world.TraceMove(trace, null);
		
		return trace.TraceNorm;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get terrain basis vectors on given position.
	\param pos World position
	\param[out] result Matrix to be filled with basis vectors
	\param world World to be checked (default world is used when undefined)
	\param noUnderwater When true, sea surface will be used instead of seabed
	\return True if the basis was defined successfully
	*/
	[Obsolete("Use SCR_TerrainHelper.GetTerrainBasis()")]
	static bool GetTerrainBasis(vector pos, out vector result[4], BaseWorld world = null, bool noUnderwater = false)
	{
		vector normal = GetTerrainNormal(pos, world, noUnderwater);
		if (normal == vector.Zero) return false;
		
		//--- Get basis matrix
		vector perpend = normal.Perpend();
		Math3D.DirectionAndUpMatrix(perpend, normal, result);
		
		//--- Rotate the matrix to always point North
		vector basis[4];
		Math3D.AnglesToMatrix(Vector(-perpend.VectorToAngles()[0], 0, 0), basis);
		Math3D.MatrixMultiply3(result, basis, result);
		
		//--- Set terrain position
		result[3] = pos;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Snap transformation to terrain position.
	\param[out] transform Matrix to be modified
	\param world World to be checked (default world is used when undefined)
	\param noUnderwater When true, sea surface will be used instead of seabed
	\return True if the operation was performed successfully
	*/
	[Obsolete("Use SCR_TerrainHelper.SnapToTerrain()")]
	static bool SnapToTerrain(out vector transform[4], BaseWorld world = null, bool noUnderwater = false)
	{
		//--- Get world
		if (!world)
			world = GetGame().GetWorld();
		
		if (!world) return false;
		
		//--- Get surface basis
		vector surfaceBasis[4];
		if (!SCR_Global.GetTerrainBasis(transform[3], surfaceBasis, world, noUnderwater)) return false;
		
		//--- Set position to surface
		transform[3] = surfaceBasis[3];
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Orient transformation to terrain normal.
	\param[out] transform Matrix to be modified
	\param world World to be checked (default world is used when undefined)
	\param noUnderwater When true, sea surface will be used instead of seabed
	\return True if the operation was performed successfully
	*/
	[Obsolete("Use SCR_TerrainHelper.OrientToTerrain()")]
	static bool OrientToTerrain(out vector transform[4], BaseWorld world = null, bool noUnderwater = false)
	{
		//--- Get world
		if (!world)
			world = GetGame().GetWorld();
		
		if (!world) return false;
		
		//--- Get surface basis
		vector surfaceBasis[4];
		if (!SCR_Global.GetTerrainBasis(transform[3], surfaceBasis, world, noUnderwater)) return false;

		//--- Reset pitch and roll, but preserve yaw
		//vector angles = Math3D.MatrixToAngles(transform);
		//Math3D.AnglesToMatrix(Vector(angles[0], 0, 0), transform);
		
		//--- Combine surface and entity transformations
		Math3D.MatrixMultiply3(surfaceBasis, transform, transform);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Snap transformation to terrain position and orient it to terrain normal.
	\param[out] transform Matrix to be modified
	\param world World to be checked (default world is used when undefined)
	\param noUnderwater When true, sea surface will be used instead of seabed
	\return True if the operation was performed successfully
	*/
	[Obsolete("Use SCR_TerrainHelper.SnapAndOrientToTerrain()")]
	static bool SnapAndOrientToTerrain(out vector transform[4], BaseWorld world = null, bool noUnderwater = false, float height = 0)
	{
		//--- Get world
		if (!world)
			world = GetGame().GetWorld();
		
		if (!world) return false;
		
		//--- Get surface basis
		vector surfaceBasis[4];
		if (!SCR_Global.GetTerrainBasis(transform[3], surfaceBasis, world, noUnderwater)) return false;
		
		//--- Set position to surface
		transform[3] = surfaceBasis[3];

		//--- Reset pitch and roll, but preserve yaw
		//vector angles = Math3D.MatrixToAngles(transform);
		//Math3D.AnglesToMatrix(Vector(angles[0], 0, 0), transform);
		
		//--- Combine surface and entity transformations
		Math3D.MatrixMultiply3(surfaceBasis, transform, transform);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Teleport local player to given position.
	\param worldPosition World position
	\return True if the operation was performed successfully
	*/
	static bool TeleportPlayer(vector worldPosition)
	{
		IEntity player = GetPlayer();
		if (!player)
			return false;
		
		//--- When in a vehcile, teleport the vehicle instead
		SCR_CompartmentAccessComponent compartmentAccess = SCR_CompartmentAccessComponent.Cast(player.FindComponent(SCR_CompartmentAccessComponent));
		if (compartmentAccess)
		{
			IEntity vehicle = compartmentAccess.GetVehicle();
			if (vehicle)
				player = vehicle;
		}
		
		//--- Apply transformation
		vector transform[4];
		player.GetWorldTransform(transform);
		transform[3] = worldPosition;
		OrientToTerrain(transform);
		
		BaseGameEntity baseGameEntity = BaseGameEntity.Cast(player);
		if (baseGameEntity)
			baseGameEntity.Teleport(transform);
		else
			player.SetWorldTransform(transform);
		
		Physics phys = player.GetPhysics();
		if (phys)
		{
			phys.SetVelocity(vector.Zero);
			phys.SetAngularVelocity(vector.Zero);
		}
		
		return true;
		
		/*
		ArmaReforgerScripted game = GetGame();
		if (!world && game)
		{
			world = game.GetWorld();
		}
		if (!world) return false;
		
		if (!playerEntity) return false;
		
		ChimeraCharacter character = ChimeraCharacter.Cast(playerEntity);
		if (character)
		{
			CompartmentAccessComponent compartmentAccess = character.GetCompartmentAccessComponent();
			if (compartmentAccess && compartmentAccess.IsInCompartment())
			{
				BaseCompartmentSlot compartmentSlot = compartmentAccess.GetCompartment();
				if (compartmentSlot && compartmentSlot.GetOwner())
				{
					playerEntity = compartmentSlot.GetOwner();
				}
			}
		}
		
		BaseGameEntity baseGameEntity = BaseGameEntity.Cast(playerEntity);
		if (baseGameEntity)
		{
			vector mat[4];
			baseGameEntity.GetWorldTransform(mat);
			mat[3] = worldPosition;
			baseGameEntity.Teleport(mat);
		}
		else
			playerEntity.SetOrigin(worldPosition);
		return true;
		*/
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Draw lines representing transformation matrix.
	\param matrix Transformation matrix in world space
	\param scale Length of lines in meters
	\param flags Shape flags
	\param colorX Color of longitudinal line
	\param colorY Color of vertical line
	\param colorZ Color of lateral line
	*/
	static void DrawMatrix(vector matrix[4], float scale = 1, ShapeFlags flags = ShapeFlags.ONCE | ShapeFlags.NOZBUFFER, int colorX = Color.RED, int colorY = Color.GREEN, int colorZ = Color.BLUE)
	{
		vector line[2] = {matrix[3], matrix[3] + matrix[0] * scale};
		Shape.CreateLines(colorX, flags, line, 2);
		
		line = {matrix[3], matrix[3] + matrix[1] * scale};
		Shape.CreateLines(colorY, flags, line, 2);
		
		line = {matrix[3], matrix[3] + matrix[2] * scale};
		Shape.CreateLines(colorZ, flags, line, 2);
	}
	
	/*!
	Splits given seconds to days, hours, minutes and remaining seconds
	\param totalSeconds total seconds to be converted
	\param[out] outDays Returns amount of Days
	\param[out] outHours Returns amount of Hours
	\param[out] outMinutes Returns amount of Minutes
	\param[out] outRemainingSeconds Returns remaining amount of Seconds
	*/
	[Obsolete("Use SCR_DateTimeHelper.GetDayHourMinuteSecondFromSeconds()")]
	static void ConvertSecondsToDaysHoursMinutesSeconds(int totalSeconds, out int outDays, out int outHours, out int outMinutes, out int outRemainingSeconds)
	{
		outDays = totalSeconds / 86400;
		outHours = (totalSeconds % 86400) / 3600;
		outMinutes = (totalSeconds % 3600) / 60;
		outRemainingSeconds = totalSeconds % 60;
	}

	/*!
	Depending on the flags given it will return a variation on dd:hh:mm:ss and d:h:m:s.
	Time varriables that are -1 will always be hidden (So without days it would look: hh:mm:ss)
	Useage: SCR_Global.GetTimeFormatting(0, 13, 37, 0, ETimeFormatParam.DAYS | ETimeFormatParam.HOURS, ETimeFormatParam.MINUTES). Hide Days and Hours if 0, Never show leading 0 for minutes
	\param days Int amount of days
	\param hours Int amount of hours
	\param minutes Int amount of minutes
	\param seconds Int amount of seconds
	\param hideEmpty flag time that needs to be hidden if zero (Example: ETimeFormatParam.DAYS hides days if 0 or less)
	\param hideLeadingZeroes flag that hides leading zero for Hours, Minutes and Seconds (Example: ETimeFormatParam.HOURS displays h:mm:ss instead of hh:mm:ss)
	\return string Time ordered in days, hours, minutes and seconds (dd:hh:mm:ss)
	*/
	static string GetTimeFormatting(int days, int hours, int minutes, int seconds, ETimeFormatParam hideEmpty = 0, ETimeFormatParam hideLeadingZeroes = 0)
	{
		//Todo: Display days properly rather then dd:hh:mm:ss (Though right now it is extreamly edge case if days should be shown)
		string returnString = "";
		
		//To do: Decide the formatting of days. Currently it is dd:hh:mm:ss and by default days are hidden if it is 0 as it would rarely be a day or more
		//Only show days
		//if (days > -1 && ((hideEmpty & ETimeFormatParam.DAYS) && days > 0) || !(hideEmpty & ETimeFormatParam.DAYS))
		if (days > 0 && ((hideEmpty & ETimeFormatParam.DAYS)) || !(hideEmpty & ETimeFormatParam.DAYS))
			returnString += days.ToString(); 
		else 
			days = -1;
		
		//hours (Check if should display hours and check if should display leading zero)
		if (hours > 0 && ((hideEmpty & ETimeFormatParam.HOURS)) || !(hideEmpty & ETimeFormatParam.HOURS))
			returnString += ReturnTimeTypeString(days, hours, ((hideLeadingZeroes & ETimeFormatParam.HOURS) == 0) +1);
		else 
			hours = -1;
		
		//Minutes (Check if should display Minutes and check if should display leading zero)
		if (minutes > 0 && ((hideEmpty & ETimeFormatParam.MINUTES)) || !(hideEmpty & ETimeFormatParam.MINUTES))
			returnString += ReturnTimeTypeString(hours, minutes, ((hideLeadingZeroes & ETimeFormatParam.MINUTES) == 0) +1);
		else 
			minutes = -1;
		
		//Seconds (Check if should display Seconds and check if should display leading zero)
		if (seconds > 0 && ((hideEmpty & ETimeFormatParam.SECONDS)) || !(hideEmpty & ETimeFormatParam.SECONDS)) 
			returnString += ReturnTimeTypeString(minutes, seconds, ((hideLeadingZeroes & ETimeFormatParam.SECONDS) == 0) +1);

		return returnString;
	}
	
	//Returns proper formatting of TimeType
	//numberLenght checks if should have leading zero			
	//Always have number lenght 1 then check if ETimeFormatParam (for example) SECONDS is given is true (aka 1) or not (aka 0) 
	protected static string ReturnTimeTypeString(int prevTimeTypeAmount, int currentTimeTypeAmount, int numberLenght)
	{
		if (prevTimeTypeAmount > -1)
			return ":" + currentTimeTypeAmount.ToString(numberLenght);
		else
			return currentTimeTypeAmount.ToString(numberLenght);
	}
	
	/*!
	Combines GetTimeFormatting() and ConvertSecondsToDaysHoursMinutesSeconds()
	It is possible to hide a time varriable like days using the hide bool when it is 0 or less
	Useage: SCR_Global.GetTimeFormatting(totalSeconds, ETimeFormatParam.DAYS | ETimeFormatParam.HOURS, ETimeFormatParam.MINUTES). Hide Days and Hours if 0, Never show leading 0 for minutes
	\param totalSeconds Int total amount of seconds to be converted to dd:hh:mm:ss
	\param hideEmpty flag time that needs to be hidden if zero (Example: ETimeFormatParam.DAYS hides days if 0 or less)
	\param hideLeadingZeroes flag that hides leading zero for Hours, Minutes and Seconds (Example: ETimeFormatParam.HOURS displays h:mm:ss instead of hh:mm:ss)
	\return string Time ordered in days, hours, minutes and seconds (dd:hh:mm:ss)
	*/
	static string GetTimeFormatting(int totalSeconds, ETimeFormatParam hideEmpty = 0, ETimeFormatParam hideLeadingZeroes = 0)
	{
		int days, hours, minutes, seconds;
		ConvertSecondsToDaysHoursMinutesSeconds(totalSeconds, days, hours, minutes, seconds);
		return GetTimeFormatting(days, hours, minutes, seconds, hideEmpty, hideLeadingZeroes);
	}
	
	/*!
	Combines GetTimeFormatting() and ConvertSecondsToDaysHoursMinutesSeconds() while hiding seconds
	It is possible to hide a time varriable like days using the hide bool when it is 0 or less
	Useage: SCR_Global.GetTimeFormatting(totalSeconds, ETimeFormatParam.DAYS | ETimeFormatParam.HOURS, ETimeFormatParam.MINUTES). Hide Days and Hours if 0, Never show leading 0 for minutes
	\param totalSeconds Int total amount of seconds to be converted to dd:hh:mm
	\param hideEmpty flag time that needs to be hidden if zero (Example: ETimeFormatParam.DAYS hides days if 0 or less)
	\param hideLeadingZeroes flag that hides leading zero for Hours, Minutes and Seconds (Example: ETimeFormatParam.HOURS displays h:mm instead of hh:mm)
	\return string Time ordered in days, hours and minutes (dd:hh:mm)
	*/
	static string GetTimeFormattingHideSeconds(int totalSeconds, ETimeFormatParam hideEmpty = 0, ETimeFormatParam hideLeadingZeroes = 0)
	{
		int days, hours, minutes, seconds;
		ConvertSecondsToDaysHoursMinutesSeconds(totalSeconds, days, hours, minutes, seconds);
		return GetTimeFormatting(days, hours, minutes, -1, ETimeFormatParam.SECONDS | hideEmpty, hideLeadingZeroes);
	}
	
	/*!
	Returns a variant of hh:mm and h:m
	\param hours Int amount of hours
	\param minutes Int amount of minutes
	\param hideEmpty flag time that needs to be hidden if zero (Example: ETimeFormatParam.HOURS hides hours if 0 or less)
	\param hideLeadingZeroes flag that hides leading zero (Example: ETimeFormatParam.HOURS displays h:mm instead of hh:mm)
	\return string Time ordered in hours and minutes (hh:mm)
	*/
	static string GetTimeFormattingHoursMinutes(int hours, int minutes, ETimeFormatParam hideEmpty = 0, ETimeFormatParam hideLeadingZeroes = 0)
	{
		return GetTimeFormatting(-1, hours, minutes, -1, (ETimeFormatParam.DAYS | ETimeFormatParam.SECONDS | hideEmpty), hideLeadingZeroes);
	}
	
	/*!
	Returns a variant of mm:ss and m:s
	\param minutes Int amount of minutes
	\param seconds Int amount of seconds
	\param hideEmpty flag time that needs to be hidden if zero (Example: ETimeFormatParam.MINUTES hides minutes if 0 or less)
	\param hideLeadingZeroes flag that hides leading zero (Example: ETimeFormatParam.MINUTES displays m:ss instead of mm:ss)
	\return string Time ordered in minutes and hours (mm:ss)
	*/
	static string GetTimeFormattingMinutesSeconds(int minutes, int seconds, ETimeFormatParam hideEmpty = 0, ETimeFormatParam hideLeadingZeroes = 0)
	{
		return GetTimeFormatting(-1, -1, minutes, seconds, (ETimeFormatParam.DAYS | ETimeFormatParam.HOURS | hideEmpty), hideLeadingZeroes);
	}
	
	/*!
	Returns a variant of hh:mm:ss and h:m:s
	\param hours Int amount of hours
	\param minutes Int amount of minutes
	\param seconds Int amount of seconds
	\param hideEmpty flag time that needs to be hidden if zero (Example: ETimeFormatParam.HOURS hides hours if 0 or less)
	\param hideLeadingZeroes flag that hides leading zero for Hours, Minutes and Seconds (Example: ETimeFormatParam.HOURS displays h:mm:ss instead of hh:mm:ss)
	\return string Time ordered in minutes and hours (hh:mm:ss)
	*/
	static string GetTimeFormattingHourMinutesSeconds(int hours, int minutes, int seconds, ETimeFormatParam hideEmpty = 0, ETimeFormatParam hideLeadingZeroes = 0)
	{
		return GetTimeFormatting(-1, hours, minutes, seconds, (ETimeFormatParam.DAYS | hideEmpty), hideLeadingZeroes);
	}
	
	/*!
	Returns month in translated striing formating. 
	\param int month, nr of month (Jan being 1, Dec being 12, etc.)
	\return month in translated string
	*/
	[Obsolete("Use SCR_DateTimeHelper.GetMonthString()")]
	static string GetMonthString(int month)
	{
		if (month < 1 || month > 12) return string.Empty;
		
		if (month == 1)
			return "#AR-Date_January";
		else if (month == 2)
			return "#AR-Date_February";
		else if (month == 3)
			return "#AR-Date_March";
		else if (month == 4)
			return "#AR-Date_April";
		else if (month == 5)
			return "#AR-Date_May";
		else if (month == 6)
			return "#AR-Date_June";
		else if (month == 7)
			return "#AR-Date_July";
		else if (month == 8)
			return "#AR-Date_August";
		else if (month == 9)
			return "#AR-Date_September";
		else if (month == 10)
			return "#AR-Date_October";
		else if (month == 11)
			return "#AR-Date_November";
		else if (month == 12)
			return "#AR-Date_December";
		
		return string.Empty;
	}
	
	
	//-----------------------------------------------------------------------------------
	//! Helps with time formatting - returns "0 minutes", "3 minutes", but "1 minute".
	[Obsolete("Use SCR_FormatHelper.GetTimeSinceEventString()")]
	protected static string GetTimeSinceEventString(int amount, string oneUnit, string manyUnits)
	{
		if (amount == 1)
			return WidgetManager.Translate(oneUnit, amount);
		else
			return WidgetManager.Translate(manyUnits, amount);
	}
	
	
	//-----------------------------------------------------------------------------------
	//! Returns text of how much time has passed since event: "23 days ago", or "23 hours ago", or "23 minutes ago".
	//! When timeDiffSeconds is below 60, it returns "0 minutes ago".
	[Obsolete("Use SCR_FormatHelper.GetTimeSinceEventImprecise()")]
	static string GetTimeSinceEventImprecise(int timeDiffSeconds)
	{
		if (timeDiffSeconds < 0)
			return string.Empty;
		
		int days, hours, minutes, remainingSeconds;
		SCR_Global.ConvertSecondsToDaysHoursMinutesSeconds(timeDiffSeconds, days, hours, minutes, remainingSeconds);
		
		if (days > 0)
			return GetTimeSinceEventString(days, "#AR-Date_DayAgo_LC", "#AR-Date_DaysAgo_LC");			// 23 days ago
		else if (hours > 0)
			return GetTimeSinceEventString(hours, "#AR-Time_HourAgo_LC", "#AR-Time_HoursAgo_LC");		// 23 hours ago
		else
			return GetTimeSinceEventString(minutes, "#AR-Time_MinuteAgo_LC", "#AR-Time_MinutesAgo_LC");	// 23 minutes ago
	}
	
	
	/*!
	Set material to an entity.
	\param entity Affected entity
	\param material Material to be applied
	\param recursively True to apply the material also to children
	*/
	static void SetMaterial(IEntity entity, ResourceName material, bool recursively = true)
	{
		//--- Remap textures of the current mesh
		VObject mesh = entity.GetVObject();
		if (mesh)
		{
			string remap;
			string materials[256];
			int numMats = mesh.GetMaterials(materials);
			for (int i = 0; i < numMats; i++)
			{
				remap += string.Format("$remap '%1' '%2';", materials[i], material);
			}
			entity.SetObject(mesh, remap);
		}
		
		//--- Iterate through children
		if (recursively)
		{
			IEntity child = entity.GetChildren();
			while (child)
			{
				SetMaterial(child, material);
				child = child.GetSibling();
			}
		}
	}
	/*!
	Check if the change triggered by GetGame().OnInputDeviceUserChangedInvoker() is only between mouse and keyboard.
	Used in scripts which need to reset some variables when switching from mouse+keyboard and gamepad, but not when switching just between mouse and keyboard.
	\param oldDevice Previously active device
	\param newDevice Currently active device
	\param True when the change is from mouse to keyboard or vice versa
	*/
	static bool IsChangedMouseAndKeyboard(EInputDeviceType oldDevice, EInputDeviceType newDevice)
	{
		return (oldDevice == EInputDeviceType.KEYBOARD && newDevice == EInputDeviceType.MOUSE) || (oldDevice == EInputDeviceType.MOUSE && newDevice == EInputDeviceType.KEYBOARD);
	}
	
	/*!
	Get path to the root world.
	If the world is a sub-scene, it will go through all its ancestors to find the root.
	\param worldPath Checked world. When empty, current world will be used.
	\return Resource name with GUID
	*/
	static ResourceName GetRootWorld(ResourceName worldPath = ResourceName.Empty)
	{
		if (!worldPath)
			worldPath = GetResourceName(GetGame().GetWorldFile());
		
		Resource worldResource = BaseContainerTools.LoadContainer(worldPath);
		if (!worldResource || !worldResource.IsValid())
			return ResourceName.Empty;
		
		BaseContainer container = worldResource.GetResource().ToBaseContainer();
		ResourceName parent;
		container.Get("Parent", parent);
		if (parent)
			return GetRootWorld(parent);
		else
			return worldPath;
	}
	/*!
	Get resource name of a file on given path.
	The file must exist and have meta file, otherwise empty string is returned!
	\param path File path
	\return Resource name with GUID
	*/
	static ResourceName GetResourceName(string path)
	{
		Resource meta = BaseContainerTools.LoadContainer(path + ".meta");
		if (!meta || !meta.IsValid())
			return ResourceName.Empty;
		
		ResourceName resourceName;
		meta.GetResource().ToBaseContainer().Get("Name", resourceName);
		return resourceName;
	}
	
	/*!
	Get all frequencies entity with given gadget manager listens to.
	\param gadgetManager Gadget manager
	\param[out] outFrequencies List to be filled with unique frequencies
	\return Number of frequencies
	*/
	static int GetFrequencies(SCR_GadgetManagerComponent gadgetManager, out notnull set<int> outFrequencies)
	{
		if (!gadgetManager)
			return 0;
		
		BaseRadioComponent radioComponent;
		array<SCR_GadgetComponent> gadgets = gadgetManager.GetGadgetsByType(EGadgetType.RADIO);
		gadgets.InsertAll(gadgetManager.GetGadgetsByType(EGadgetType.RADIO_BACKPACK));
		foreach (SCR_GadgetComponent gadget: gadgets)
		{
			radioComponent = BaseRadioComponent.Cast(gadget.GetOwner().FindComponent(BaseRadioComponent));
			if (radioComponent && radioComponent.IsPowered())
				outFrequencies.Insert(radioComponent.GetFrequency());
		}
		return outFrequencies.Count();
	}
	/*!
	Format the list of frequencies as single-line text.
	\param frequencies List of frequencies
	\return Text with frequencies
	*/
	[Obsolete("Use SCR_FormatHelper.FormatFrequencies()")]
	static string FormatFrequencies(notnull set<int> frequencies, set<int> highlightFrequencies = null)
	{
		if (frequencies.IsEmpty())
			return string.Empty;
		
		string text;
		foreach (int i, int frequency: frequencies)
		{
			if (i > 0)
				text += ", ";
			
			if (highlightFrequencies && highlightFrequencies.Contains(frequency))
				text += string.Format("<color rgba='226,168,79,255'>%1</color>", frequency / 1000); //--- ToDo: Don't hardcode color
			else
				text += (frequency / 1000).ToString();
		}
		return text + " " + "#AR-VON_FrequencyUnits_MHz";
	}
	/*!
	Check if given player role has admin rights.
	\param role Player role
	\return True when the role has admin rights
	*/
	static bool IsAdminRole(EPlayerRole role)
	{
		return SCR_Enum.HasPartialFlag(role, EPlayerRole.ADMINISTRATOR | EPlayerRole.SESSION_ADMINISTRATOR);
	}
	/*!
	Check if given player is an admin.
	\param playerID ID of queried player
	\return True when player with given ID is an admin.
	*/
	static bool IsAdmin(int playerID)
	{
		return GetGame().GetPlayerManager().HasPlayerRole(playerID, EPlayerRole.ADMINISTRATOR)
			|| GetGame().GetPlayerManager().HasPlayerRole(playerID, EPlayerRole.SESSION_ADMINISTRATOR);
	}
	/*!
	Check if local player is an admin.
	\return True when player with given ID is an admin.
	*/
	static bool IsAdmin()
	{
		int playerID = GetGame().GetPlayerController().GetPlayerId();
		return GetGame().GetPlayerManager().HasPlayerRole(playerID, EPlayerRole.ADMINISTRATOR)
			|| GetGame().GetPlayerManager().HasPlayerRole(playerID, EPlayerRole.SESSION_ADMINISTRATOR);
	}
	
	/*Get attribute resourceName from a specified component of the entity's prefab
	\param entity Entity to get resourceName for
	\param containerType Name of the container, for example "MeshObject"
	\param attributeName Name of the attribute, for example "Object"
	\return resourceName Path to resource
	*/
	static ResourceName GetPrefabAttributeResource(notnull IEntity entity, string containerType, string attributeName)
	{
		ResourceName resourceName;
		EntityPrefabData prefabData = entity.GetPrefabData();
		if (!prefabData)
			return resourceName;
		
		IEntitySource entitySource = IEntitySource.Cast(prefabData.GetPrefab());
		if (!entitySource)
			return resourceName;
		
		BaseContainer container = SCR_BaseContainerTools.FindComponentSource(entitySource, containerType);
		if (!container)
			return resourceName;
		
		container.Get(attributeName, resourceName);
		
		return resourceName;
	}
};
