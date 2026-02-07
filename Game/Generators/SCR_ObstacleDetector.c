#ifdef WORKBENCH
//! Obstacle detector class used for example by SCR_GeneratorBaseEntity and ObjectBrushTool
//! Refreshing Obstacles grabs all the possible obstacles
//! HasObstacle checks depending on Avoid* settings
//! Refreshing Obstacles -then- setting AvoidX is possible as first all obstacles are stored then used or not in the detection
class SCR_ObstacleDetector
{
	protected bool m_bAvoidObjects;
	protected float m_fAvoidObjectsDetectionRadius;
	protected float m_fAvoidObjectsDetectionHeight;
	protected bool m_bAvoidRoads;
	protected bool m_bAvoidRivers;
	protected bool m_bAvoidPowerLines;
	protected bool m_bAvoidForests;
	protected bool m_bAvoidLakes;
	protected bool m_bAvoidLand;
	protected bool m_bAvoidOcean;
	protected ref array<ref SCR_ObstacleDetectorSplineInfo> m_aAvoidInfoRoads;
	protected ref array<ref SCR_ObstacleDetectorSplineInfo> m_aAvoidInfoRivers;
	protected ref array<ref SCR_ObstacleDetectorSplineInfo> m_aAvoidInfoPowerLines;
	protected ref array<ref SCR_ObstacleDetectorSplineInfo> m_aAvoidInfoLakes;
	protected ref array<ref SCR_ObstacleDetectorSplineInfo> m_aAvoidInfoForests;
	protected float m_fOceanLevel;
	protected ref TraceSphere m_AvoidObjectTraceSphere;

	protected WorldEditorAPI m_WorldEditorAPI;

	//------------------------------------------------------------------------------------------------
	//! changing worlds or something else can lose ref to WorldEditorAPI
	//! \return true if World Editor API reference is valid, false otherwise
	bool IsValid()
	{
		return m_WorldEditorAPI != null;
	}

	//------------------------------------------------------------------------------------------------
	void SetAvoidObjects(bool avoidObjects)
	{
		m_bAvoidObjects = avoidObjects;
	}

	//------------------------------------------------------------------------------------------------
	//! \param radius accepts 0..inf radius - a negative value will be floored to zero
	void SetAvoidObjectsDetectionRadius(float radius)
	{
		if (radius < 0)
			radius = 0;

		m_fAvoidObjectsDetectionRadius = radius;
		if (m_AvoidObjectTraceSphere)
			m_AvoidObjectTraceSphere.Radius = m_fAvoidObjectsDetectionRadius;
	}

	//------------------------------------------------------------------------------------------------
	void SetAvoidObjectsDetectionHeight(float height)
	{
		m_fAvoidObjectsDetectionHeight = height;
		if (m_AvoidObjectTraceSphere)
			m_AvoidObjectTraceSphere.End[1] = m_AvoidObjectTraceSphere.Start[1] + m_fAvoidObjectsDetectionHeight;
	}

	//------------------------------------------------------------------------------------------------
	void SetAvoidRoads(bool avoidRoads)
	{
		m_bAvoidRoads = avoidRoads;
	}

	//------------------------------------------------------------------------------------------------
	void SetAvoidRivers(bool avoidRivers)
	{
		m_bAvoidRivers = avoidRivers;
	}

	//------------------------------------------------------------------------------------------------
	void SetAvoidPowerLines(bool avoidPowerLines)
	{
		m_bAvoidPowerLines = avoidPowerLines;
	}

	//------------------------------------------------------------------------------------------------
	void SetAvoidForests(bool avoidForests)
	{
		m_bAvoidForests = avoidForests;
	}

	//------------------------------------------------------------------------------------------------
	void SetAvoidLakes(bool avoidLakes)
	{
		m_bAvoidLakes = avoidLakes;
	}

	//------------------------------------------------------------------------------------------------
	void SetAvoidLand(bool avoidLand)
	{
		m_bAvoidLand = avoidLand;
	}

	//------------------------------------------------------------------------------------------------
	void SetAvoidOcean(bool avoidOcean)
	{
		m_bAvoidOcean = avoidOcean;
	}

	//------------------------------------------------------------------------------------------------
	// used by SCR_GeneratorBaseEntity.RefreshObstacles() (and RefreshObstaclesByWorld() which is unused)
	void RefreshObstaclesByAABB(vector worldMin, vector worldMax)
	{
		if (!m_WorldEditorAPI.GetWorld())
			return;

		SetupObstacleArrays(true, true);
		m_fOceanLevel = m_WorldEditorAPI.GetWorld().GetOceanBaseHeight();
		m_WorldEditorAPI.GetWorld().QueryEntitiesByAABB(worldMin, worldMax, AllSplineQueryFilter);
	}

	//------------------------------------------------------------------------------------------------
	// unused
	void RefreshObstaclesByWorld()
	{
		if (!m_WorldEditorAPI.GetWorld())
			return;

		vector min, max;
		m_WorldEditorAPI.GetWorld().GetBoundBox(min, max);
		RefreshObstaclesByAABB(min, max);
	}

	//------------------------------------------------------------------------------------------------
	// used by ObjectBrushTool.CreateObjects()
	void RefreshRoadObstaclesBySphere(vector worldPos, float radius)
	{
		if (!m_WorldEditorAPI.GetWorld())
			return;

		SetupObstacleArrays(true, false);
		m_fOceanLevel = m_WorldEditorAPI.GetWorld().GetOceanBaseHeight();
		m_WorldEditorAPI.GetWorld().QueryEntitiesBySphere(worldPos, radius, RoadSplineQueryFilter);
	}

	//------------------------------------------------------------------------------------------------
	// used by ObjectBrushTool.OnMousePressEvent()
	void RefreshAreaObstaclesBySphere(vector worldPos, float radius)
	{
		if (!m_WorldEditorAPI.GetWorld())
			return;

		SetupObstacleArrays(false, true);
		m_fOceanLevel = m_WorldEditorAPI.GetWorld().GetOceanBaseHeight();
		m_WorldEditorAPI.GetWorld().QueryEntitiesBySphere(worldPos, radius, AreaSplineQueryFilter);
	}

	//------------------------------------------------------------------------------------------------
	// used by ObjectBrushTool.OnMousePressEvent()
	void RefreshAreaObstaclesByWorld()
	{
		if (!m_WorldEditorAPI.GetWorld())
			return;

		SetupObstacleArrays(false, true);
		m_fOceanLevel = m_WorldEditorAPI.GetWorld().GetOceanBaseHeight();
		vector min, max;
		m_WorldEditorAPI.GetWorld().GetBoundBox(min, max);
		m_WorldEditorAPI.GetWorld().QueryEntitiesByAABB(min, max, AreaSplineQueryFilter);
	}

	//------------------------------------------------------------------------------------------------
	//! Detects obstacles based on current settings - a SCR_ObstacleDetector.RefreshObstaclesBy*() method must have been called earlier
	//! \param worldPos the location to check - only the 2D position will be checked
	//! \param exclusionList list of entities that should not be considered as obstacles
	//! \return true if an obstacle has been detected or on error, false otherwise
	bool HasObstacle(vector worldPos, array<IEntity> exclusionList = null)
	{
		if (!m_aAvoidInfoRoads) // a null array means obstacles were cleared or never initialised
		{
			Print("SCR_ObstacleDetector.HasObstacle() method requires obtaining obstacles info through SCR_ObstacleDetector.RefreshObstaclesBy*() method first", LogLevel.ERROR);
			return true; // prevent placement by default
		}

		BaseWorld world = m_WorldEditorAPI.GetWorld();
		if (!world)
			return true;

		if (world.IsOcean())
		{
			if (m_bAvoidLand && m_bAvoidOcean)
				return true; // let's save some time

			float terrainY = world.GetSurfaceY(worldPos[0], worldPos[2]);

			if (m_bAvoidOcean && terrainY < m_fOceanLevel)
				return true;

			if (m_bAvoidLand && terrainY > m_fOceanLevel)
				return true;
		}

		if (m_bAvoidForests)
		{
			foreach (SCR_ObstacleDetectorSplineInfo info : m_aAvoidInfoForests)
			{
				if (Math2D.IsPointInPolygon(info.m_a2DPoints, worldPos[0], worldPos[2]))
					return true;
			}
		}

		if (m_bAvoidLakes)
		{
			foreach (SCR_ObstacleDetectorSplineInfo info : m_aAvoidInfoLakes)
			{
				if (worldPos[1] < info.m_fLakeSurfaceY && Math2D.IsPointInPolygon(info.m_a2DPoints, worldPos[0], worldPos[2]))
					return true;
			}
		}

		if (m_bAvoidRoads)
		{
			foreach (SCR_ObstacleDetectorSplineInfo info : m_aAvoidInfoRoads)
			{
				if (SCR_Math3D.GetDistanceFromSpline(info.m_aTesselatedPoints, info.m_SplineEntity, worldPos) < info.m_fClearance)
					return true;
			}
		}

		if (m_bAvoidRivers)
		{
			foreach (SCR_ObstacleDetectorSplineInfo info : m_aAvoidInfoRivers)
			{
				if (SCR_Math3D.GetDistanceFromSpline(info.m_aTesselatedPoints, info.m_SplineEntity, worldPos) < info.m_fClearance)
					return true;
			}
		}

		if (m_bAvoidPowerLines)
		{
			foreach (SCR_ObstacleDetectorSplineInfo info : m_aAvoidInfoPowerLines)
			{
				if (SCR_Math3D.GetDistanceFromSpline(info.m_aTesselatedPoints, info.m_SplineEntity, worldPos) < info.m_fClearance)
					return true;
			}
		}

		if (m_bAvoidObjects && (m_fAvoidObjectsDetectionHeight != 0 || m_fAvoidObjectsDetectionHeight > 0))
		{
			if (!m_AvoidObjectTraceSphere)
			{
				m_AvoidObjectTraceSphere = new TraceSphere();
				m_AvoidObjectTraceSphere.Radius = m_fAvoidObjectsDetectionRadius;
				m_AvoidObjectTraceSphere.LayerMask = EPhysicsLayerPresets.Main | EPhysicsLayerDefs.Buoyancy | EPhysicsLayerDefs.Water; // are the last two required?
				m_AvoidObjectTraceSphere.Flags = TraceFlags.ENTS;
			}

			m_AvoidObjectTraceSphere.Start = worldPos;
			m_AvoidObjectTraceSphere.End = { worldPos[0], worldPos[1] + m_fAvoidObjectsDetectionHeight, worldPos[2] }; // move up

			if (exclusionList)
				m_AvoidObjectTraceSphere.ExcludeArray = exclusionList;
			else if (m_AvoidObjectTraceSphere.ExcludeArray)
				m_AvoidObjectTraceSphere.ExcludeArray.Clear();

			world.TraceMove(m_AvoidObjectTraceSphere, null);

			if (m_AvoidObjectTraceSphere.TraceEnt) // or TraceMove < 1
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Sets up needed arrays
	//! \param setupRoadSplines instanciates "road" spline arrays
	//! \param setupAreaSplines instanciates area spline arrays
	protected void SetupObstacleArrays(bool setupRoadSplines, bool setupAreaSplines)
	{
		// roads
		if (setupRoadSplines || !m_aAvoidInfoRoads)
			m_aAvoidInfoRoads = {};

		if (setupRoadSplines || !m_aAvoidInfoRivers)
			m_aAvoidInfoRivers = {};

		if (setupRoadSplines || !m_aAvoidInfoPowerLines)
			m_aAvoidInfoPowerLines = {};

		// areas
		if (setupAreaSplines || !m_aAvoidInfoForests)
			m_aAvoidInfoForests = {};

		if (setupAreaSplines || !m_aAvoidInfoLakes)
			m_aAvoidInfoLakes = {};
	}

	//------------------------------------------------------------------------------------------------
	//! Empties obstacles information - a SCR_ObstacleDetector.RefreshObstaclesBy*() method must be called to use HasObstacle again
	void ClearObstacles()
	{
		m_aAvoidInfoRoads = null;
		m_aAvoidInfoRivers = null;
		m_aAvoidInfoPowerLines = null;
		m_aAvoidInfoForests = null;
		m_aAvoidInfoLakes = null;
	}

	//------------------------------------------------------------------------------------------------
	//! Fills m_aAvoidInfo* arrays from RefreshObstaclesBy* request for HasObstacle usage
	//! AllSplineQueryFilter, RoadSplineQueryFilter, AreaSplineQueryFilter call it
	//! \param getRoadSplines get "road-like" splines - roads, rivers, powerlines
	//! \param getAreaSplines get area splines - lakes, forests
	//! \return true when done
	protected bool BaseSplineQueryFilter(IEntity entity, bool getRoadSplines, bool getAreaSplines)
	{
		if (!getRoadSplines && !getAreaSplines)
			return true;

		ShapeEntity shapeEntity = ShapeEntity.Cast(entity);
		if (!shapeEntity)
			return true;

		if (!m_WorldEditorAPI)
			return true;

		IEntitySource shapeSource = m_WorldEditorAPI.EntityToSource(shapeEntity);
		if (!shapeSource)
			return true;

		IEntitySource generatorSource;
		GeneratorBaseEntity generatorEntity;
		bool isLake;
		for (int i = 0, count = shapeSource.GetNumChildren(); i < count; i++)
		{
			generatorSource = shapeSource.GetChild(i);
			generatorEntity = GeneratorBaseEntity.Cast(m_WorldEditorAPI.SourceToEntity(generatorSource));
			if (!generatorEntity)
				continue;

			if (getRoadSplines)
			{
				if (generatorEntity.IsInherited(RoadGeneratorEntity))
				{
					m_aAvoidInfoRoads.Insert(new SCR_ObstacleDetectorSplineInfo(shapeSource, shapeEntity, generatorSource, generatorEntity));
					break;
				}

				if (generatorEntity.IsInherited(RiverEntity))
				{
					m_aAvoidInfoRivers.Insert(new SCR_ObstacleDetectorSplineInfo(shapeSource, shapeEntity, generatorSource, generatorEntity));
					break;
				}

				if (generatorEntity.IsInherited(SCR_PowerlineGeneratorEntity))
				{
					m_aAvoidInfoPowerLines.Insert(new SCR_ObstacleDetectorSplineInfo(shapeSource, shapeEntity, generatorSource, generatorEntity));
					break;
				}
			}

			if (getAreaSplines)
			{
				if (generatorEntity.IsInherited(ForestGeneratorEntity))
				{
					m_aAvoidInfoForests.Insert(new SCR_ObstacleDetectorSplineInfo(shapeSource, shapeEntity, generatorSource, generatorEntity));
					break;
				}

				if (generatorEntity.IsInherited(LakeGeneratorEntity) && generatorSource.Get("m_bIsLake", isLake) && isLake)
				{
					m_aAvoidInfoLakes.Insert(new SCR_ObstacleDetectorSplineInfo(shapeSource, shapeEntity, generatorSource, generatorEntity));
					break;
				}
			}
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Fills m_aAvoidInfo* arrays from RefreshObstaclesBy* request for HasObstacle usage
	protected bool AllSplineQueryFilter(IEntity entity)
	{
		return BaseSplineQueryFilter(entity, true, true);
	}

	//------------------------------------------------------------------------------------------------
	//! Fills m_aAvoidInfo* arrays from RefreshObstaclesBy* request for HasObstacle usage
	protected bool RoadSplineQueryFilter(IEntity entity)
	{
		return BaseSplineQueryFilter(entity, true, false);
	}

	//------------------------------------------------------------------------------------------------
	//! Fills m_aAvoidInfo* arrays from RefreshObstaclesBy* request for HasObstacle usage
	protected bool AreaSplineQueryFilter(IEntity entity)
	{
		return BaseSplineQueryFilter(entity, false, true);
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_ObstacleDetector(WorldEditorAPI worldEditorAPI) // not using notnull to prevent World Editor lockup
	{
		m_WorldEditorAPI = worldEditorAPI;
		if (!m_WorldEditorAPI)
			Print("Something went wrong with SCR_ObstacleDetector init: passed worldEditorAPI is null", LogLevel.ERROR);
	}
};

class SCR_ObstacleDetectorSplineInfo
{
	float m_fClearance;
	float m_fLakeSurfaceY;
	ShapeEntity m_SplineEntity;
	ref array<float> m_a2DPoints;			//!< World coordinates. Lakes only
	// ref array<vector> m_a3DPoints;			//!< World coordinates. Lakes only - not used anywhere for now
	ref array<vector> m_aTesselatedPoints;	//!< Relative coordinates. Roads/Rivers/Powerlines

	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_ObstacleDetectorSplineInfo(notnull IEntitySource splineSource, notnull ShapeEntity splineEntity, notnull IEntitySource generatorSource, notnull GeneratorBaseEntity generatorEntity)
	{
		m_SplineEntity = splineEntity;

		/*
			ROAD
		*/
		if (generatorEntity.IsInherited(RoadGeneratorEntity))
		{
			generatorSource.Get("RoadClearance", m_fClearance);
			m_aTesselatedPoints = {};
			splineEntity.GenerateTesselatedShape(m_aTesselatedPoints);
			return;
		}

		/*
			RIVER &
			POWERLINE
		*/
		if (generatorEntity.IsInherited(RiverEntity) ||
			generatorEntity.IsInherited(SCR_PowerlineGeneratorEntity))
		{
			generatorSource.Get("Clearance", m_fClearance);
			m_aTesselatedPoints = {};
			splineEntity.GenerateTesselatedShape(m_aTesselatedPoints);
			return;
		}

		/*
			FOREST
		*/
		if (generatorEntity.IsInherited(ForestGeneratorEntity))
		{
			m_a2DPoints = {};
			GetPoints2D3D(splineSource, m_a2DPoints, null, m_SplineEntity._WB_GetEditorAPI());
			return;
		}

		/*
			LAKE
		*/
		if (generatorEntity.IsInherited(LakeGeneratorEntity))
		{
			m_a2DPoints = {};
			array<vector> a3DPoints = {};
			GetPoints2D3D(splineSource, m_a2DPoints, a3DPoints, m_SplineEntity._WB_GetEditorAPI());

			bool flattenByBottomPlane;
			generatorSource.Get("m_bFlattenByBottomPlane", flattenByBottomPlane);

			if (a3DPoints.IsEmpty())
				return;

			// all this to be replaced with a later LakeGeneratorEntity method
			float minOrMaxY = a3DPoints[0][1];
			for (int i = a3DPoints.Count() - 1; i >= 0; i--)
			{
				if (flattenByBottomPlane)
				{
					if (a3DPoints[i][1] < minOrMaxY)
						minOrMaxY = a3DPoints[i][1];
				}
				else
				{
					if (a3DPoints[i][1] > minOrMaxY)
						minOrMaxY = a3DPoints[i][1];
				}
			}

			m_fLakeSurfaceY = minOrMaxY;
			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Get 2D and 3D points from shape, in relative or world coordinates.
	//! \param pos2D can be null and won't be filled
	//! \param pos3D can be null and won't be filled
	//! \param worldEditorAPI providing worldEditorAPI implies World Coords
	//! \return number of points, or -1 in case of error/no array to fill
	static int GetPoints2D3D(notnull IEntitySource shapeEntitySrc, out array<float> pos2D, out array<vector> pos3D, WorldEditorAPI worldEditorAPI = null)
	{
		if (!pos2D && !pos3D)
			return -1;

		BaseContainerList points = shapeEntitySrc.GetObjectArray("Points");
		if (!points)
			return -1;

		IEntity entity;
		if (worldEditorAPI)
			entity = worldEditorAPI.SourceToEntity(shapeEntitySrc);

		if (pos2D)
			pos2D.Clear();

		if (pos3D)
			pos3D.Clear();

		int count = points.Count();
		vector pos;
		for (int i; i < count; i++)
		{
			points.Get(i).Get("Position", pos);
			if (entity)
				pos = entity.CoordToParent(pos);

			if (pos2D)
			{
				pos2D.Insert(pos[0]);
				pos2D.Insert(pos[2]);
			}

			if (pos3D)
				pos3D.Insert(pos);
		}

		return count;
	}
};
#endif
