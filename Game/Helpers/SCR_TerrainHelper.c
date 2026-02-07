class SCR_TerrainHelper
{

	//------------------------------------------------------------------------------------------------
	/*!
	Get terrain height at given position.
	\param[out] pos World position
	\param world World to be checked (default world is used when undefined)
	\param noUnderwater When true, sea surface will be used instead of seabed
	\return Position above sea
	*/
	static float GetTerrainY(vector pos, BaseWorld world = null, bool noUnderwater = false)
	{
		if (!world)
			world = GetGame().GetWorld();
		if (!world)
			return 0;

		float surfaceY = world.GetSurfaceY(pos[0], pos[2]);
		if (noUnderwater)
			surfaceY = Math.Max(surfaceY, 0);
		return surfaceY;
	}
	/*!
	Get height above terrain of given position.
	\param[out] pos World position
	\param world World to be checked (default world is used when undefined)
	\param noUnderwater When true, sea surface will be used instead of seabed
	\return Position above terrain
	*/
	static float GetHeightAboveTerrain(vector pos, BaseWorld world = null, bool noUnderwater = false)
	{
		if (!world)
			world = GetGame().GetWorld();
		if (!world)
			return 0;

		float surfaceY = world.GetSurfaceY(pos[0], pos[2]);
		if (noUnderwater)
			surfaceY = Math.Max(surfaceY, 0);
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
	static vector GetTerrainNormal(out vector pos, BaseWorld world = null, bool noUnderwater = false)
	{
		//--- Get world
		if (!world)
			world = GetGame().GetWorld();
		if (!world)
			return vector.Zero;

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
	static bool GetTerrainBasis(vector pos, out vector result[4], BaseWorld world = null, bool noUnderwater = false)
	{
		vector normal = GetTerrainNormal(pos, world, noUnderwater);
		if (normal == vector.Zero)
			return false;

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
	static bool SnapToTerrain(out vector transform[4], BaseWorld world = null, bool noUnderwater = false)
	{
		//--- Get world
		if (!world)
			world = GetGame().GetWorld();

		if (!world)
			return false;

		//--- Get surface basis
		vector surfaceBasis[4];
		if (!SCR_TerrainHelper.GetTerrainBasis(transform[3], surfaceBasis, world, noUnderwater))
			return false;

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
	static bool OrientToTerrain(out vector transform[4], BaseWorld world = null, bool noUnderwater = false)
	{
		//--- Get world
		if (!world)
			world = GetGame().GetWorld();

		if (!world)
			return false;

		//--- Get surface basis
		vector surfaceBasis[4];
		if (!SCR_TerrainHelper.GetTerrainBasis(transform[3], surfaceBasis, world, noUnderwater))
			return false;

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
	static bool SnapAndOrientToTerrain(out vector transform[4], BaseWorld world = null, bool noUnderwater = false, float height = 0)
	{
		//--- Get world
		if (!world)
			world = GetGame().GetWorld();

		if (!world)
			return false;

		//--- Get surface basis
		vector surfaceBasis[4];
		if (!SCR_TerrainHelper.GetTerrainBasis(transform[3], surfaceBasis, world, noUnderwater))
			return false;

		//--- Set position to surface
		transform[3] = surfaceBasis[3];

		//--- Reset pitch and roll, but preserve yaw
		//vector angles = Math3D.MatrixToAngles(transform);
		//Math3D.AnglesToMatrix(Vector(angles[0], 0, 0), transform);

		//--- Combine surface and entity transformations
		Math3D.MatrixMultiply3(surfaceBasis, transform, transform);

		return true;
	}
}