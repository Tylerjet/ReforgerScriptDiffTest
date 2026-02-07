//------------------------------------------------------------------------------------------------
//! SCR_Math2D Class
//!
//! Contains various scripted 2D math functions
//------------------------------------------------------------------------------------------------
class SCR_Math2D
{
	//------------------------------------------------------------------------------------------------
	//! Get 2D points array from vector array, format { x1,y1, x2,y2, x3,y3 }
	static void Get2DPolygon(notnull array<vector> points3D, out notnull array<float> points2D)
	{
		points2D.Clear();
		if (points3D.IsEmpty())
			return;

		foreach (vector point3D : points3D)
		{
			points2D.Insert(point3D[0]);
			points2D.Insert(point3D[2]);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Get an array of vectors from 2D array with 0 for Y value
	//! { x1, y1, x2, y2 } = { { x1, 0, y1 }, { x2, 0, y2 }
	static void Get3DPolygon(notnull array<float> points2D, out notnull array<vector> points3D)
	{
		points3D.Clear();
		if (points2D.IsEmpty())
			return;

		for (int i = 0, count = points2D.Count(); i < count; i += 2)
		{
			points3D.Insert(Vector(points2D[i], 0, points2D[i + 1]));
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	static bool GetMinMaxPolygon(notnull array<float> polygon, out float minX, out float maxX, out float minY, out float maxY)
	{
		if (!IsPolygonValid(polygon))
			return false;

		minX = polygon[0];
		maxX = polygon[0];
		minY = polygon[1];
		maxY = polygon[1];
		float x, y;

		for (int i = 2, count = polygon.Count(); i < count; i += 2)
		{
			x = polygon[i];
			y = polygon[i + 1];

			if (x < minX)
				minX = x;
			else if (x > maxX)
				maxX = x;

			if (y < minY)
				minY = y;
			else if (y > maxY)
				maxY = y;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Get provided polygon's area
	//! \return -1 on invalid polygon, otherwise the polygon's surface
	static float GetPolygonArea(notnull array<float> polygon)
	{
		if (!IsPolygonValid(polygon))
			return -1;

		float result;

		int j;
		for (int i = 0, count = polygon.Count(); i < count; i += 2) // step 2
		{
			j = (i + 2) % count;
			result += 0.5 * (polygon[i] * polygon[j + 1] - polygon[j] * polygon[i + 1]);
		}

		if (result < 0)
			result = -result;

		return result;
	}

	//------------------------------------------------------------------------------------------------
	// TODO: better
	/*!
		Get a random point within the provided 2D polygon
		\param polygon
		\param x
		\param x
		\return true on success, false otherwise
	*/
	static bool GetRandomPointInPolygon(notnull array<float> polygon, out float x, out float y)
	{
		float minX, minY, maxX, maxY;
		if (!GetMinMaxPolygon(polygon, minX, maxX, minY, maxY))
			return false;

		float tempX, tempY;
		GetRandomPointInRectangle(minX, maxX, minY, maxY, tempX, tempY);
		while (!Math2D.IsPointInPolygon(polygon, tempX, tempY)) // ugh
		{
			GetRandomPointInRectangle(minX, maxX, minY, maxY, tempX, tempY);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Get a random point in the provided rectangle
	protected static bool GetRandomPointInRectangle(float minX, float maxX, float minY, float maxY, out float x, out float y)
	{
		x = Math.RandomFloat(minX, maxX);
		y = Math.RandomFloat(minY, maxY);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get a random point in the provided circular sector - uniform distribution
	//! \param angleFrom angle in radians from which we calculate random angle
	//! \param angleTo angle in radians to which we calculate random angle
	static bool GetRandomPointInSector(float originX, float originY, float angleFrom, float angleTo, float radius, out float x, out float y)
	{
		float distance = radius * Math.Sqrt(Math.RandomFloat01()); // to have it uniformly distributed
		float angle = Math.RandomFloat(angleFrom,angleTo);
		x = originX + distance * Math.Cos(angle);
		y = originY + distance * Math.Sin(angle);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Checks the amount of points for a 2D polygon (needs 3 points to make a polygon and an even amount of values to have valid points)
	static bool IsPolygonValid(notnull array<float> polygon)
	{
		int count = polygon.Count();

		if (count < 6) // less than 3 points? not a polygon
			return false;

		if (count & 1) // odd number = one missing/extra point
			return false;

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Translates polar coordinates (angle and radius) into cartesian (x,y)
	static bool PolarToCartesian(float angle, float radius, out float x, out float y) 
	{
		x = Math.Cos(angle) * radius;
		y = Math.Sin(angle) * radius;
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Generates random point in given polygon
	\param polygon Consecutive floats give polygon in 2D (2 floats = Vector2)
	\param bbMin Bounding box minimum corner
	\param bbMax Bounding box maximum corner
	\return Vector3 point in polygon
	*/
	static vector GenerateRandomPoint(array<float> polygon,  vector bbMin, vector bbMax)
	{
		return SCR_Math.GetMathRandomGenerator().GenerateRandomPoint(polygon, bbMin, bbMax);
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Generates a random point around `center` in range min/max radius
	\param minRadius All generated points will be at least this far from center
	\param maxRadius All generated points will be at most this far from center
	\param center Position around which to generate. Vector2 XZ
	\param uniform If false, has a small bias towards the center which may be desirable in some situations
	\return Vector2 XZ set, Y = 0
	*/
	static vector GenerateRandomPointInRadius(float minRadius, float maxRadius, vector center, bool uniform = true)
	{
		return SCR_Math.GetMathRandomGenerator().GenerateRandomPointInRadius(minRadius, maxRadius, center, uniform);
	}
};