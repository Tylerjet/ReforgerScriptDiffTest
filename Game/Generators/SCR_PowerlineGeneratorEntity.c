[EntityEditorProps(category: "GameLib/Scripted/Generator", description: "Power Line Generator", dynamicBox: true, visible: false)]
class SCR_PowerlineGeneratorEntityClass : SCR_LineTerrainShaperGeneratorBaseEntityClass
{
}

//! NEW SLOT SYSTEM:
//!
//! Power Poles have slot groups of a certain type
//! (Low Voltage, High Voltage, Ultra High Voltage, Telephone, etc - see SCR_EPoleCableType)\n
//! - only distance to slot group is considered - same line factor is not\n
//! - only one connection per cable type is possible as of now (closest slots will connect)
//! e.g closest LV to closest LV, closest Telephone to closest Telephone

// TODO: move most of the properties to PrefabData (https://community.bistudio.com/wiki/Arma_Reforger:Prefab_Data)
class SCR_PowerlineGeneratorEntity : SCR_LineTerrainShaperGeneratorBaseEntity
{
	[Attribute(defvalue: "0", desc: "Use at the very end, as any other generator change will use the old system", category: "[NEW] New Placement")]
	protected bool m_bUseNewPlacementSystem;

	/*
		Pole Setup
	*/

	[Attribute(defvalue: "30", desc: "How far should the poles be from each other", params: "1 500" /* error when cable > 500 */, category: "Pole Setup")]
	protected float m_fDistance;

	[Attribute(params: "et", category: "Pole Setup")]
	protected ResourceName m_DefaultPole;

	[Attribute(params: "et", category: "Pole Setup")]
	protected ResourceName m_StartPole;

	[Attribute(params: "et", category: "Pole Setup")]
	protected ResourceName m_EndPole;

	[Attribute(params: "et", category: "Pole Setup")]
	protected ref array<ResourceName> m_aJunctionPoles;

	[Attribute(params: "et", category: "Pole Setup", desc: "[OBSOLETE (use Junction Poles above)]")]
	protected ResourceName m_DefaultJunctionPole;

	[Attribute(defvalue: "0", category: "Pole Setup")]
	protected bool m_bRotate180DegreeYawStartPole;

	[Attribute(defvalue: "0", category: "Pole Setup")]
	protected bool m_bRotate180DegreeYawEndPole;

	[Attribute(defvalue: "0", desc: "Expected empty space around the poles line by other generators", params: "0 100 1", category: "Pole Setup")]
	protected float Clearance; //!< Used by Obstacle Detector - TODO: rename to m_fClearance

	/*
		Randomisation
	*/

	[Attribute(defvalue: "0", desc: "Maximum random pitch angle", params: "0 180 1", category: "Randomisation")]
	protected float m_fRandomPitchAngle;

	[Attribute(defvalue: "1", desc: "Make the random pitch possible on both sides (-180..+180°) or, if disabled, one side only (0..180°)", category: "Randomisation")]
	protected bool m_bRandomPitchOnBothSides;

	[Attribute(defvalue: "0", desc: "Maximum random roll angle", params: "0 180 1", category: "Randomisation")]
	protected float m_fRandomRollAngle;

	[Attribute(defvalue: "1", desc: "Make the random roll possible on both sides (-180..+180°) or, if disabled, one side only (0..180°)", category: "Randomisation")]
	protected bool m_bRandomRollOnBothSides;

	[Attribute(defvalue: "0", desc: "Apply randomisation to default poles", category: "Randomisation")]
	protected bool m_bApplyPitchAndRollDefault;

	[Attribute(defvalue: "0", desc: "Apply randomisation to start pole", category: "Randomisation")]
	protected bool m_bApplyPitchAndRollStart;

	[Attribute(defvalue: "0", desc: "Apply randomisation to end pole", category: "Randomisation")]
	protected bool m_bApplyPitchAndRollEnd;

	/*
		Cables
	*/

	[Attribute(desc: "Cable types-emat correspondence list", category: "Cables")]
	protected ref array<ref SCR_PoleCable> m_aCables;

	[Attribute(defvalue: "{836210B285A81785}Assets/Structures/Infrastructure/Power/Powerlines/data/default_powerline_wire.emat", desc: "Default cable material if none above match", params: "emat", category: "Cables")]
	protected ResourceName m_PowerlineMaterial;

	/*
		Debug
	*/

	[Attribute(defvalue: "0", category: "Debug")]
	protected bool m_bDrawDebugShapes;

#ifdef WORKBENCH

	protected bool m_bLastJunctionWasSameLine = true;

	protected ref array<IEntity> m_aMyJunctionPoles = {};
	protected ref array<IEntity> m_aOtherJunctionPoles = {};

	protected ref SCR_DebugShapeManager m_DebugShapeManager;
	protected IEntitySource m_PreviousPowerPoleSource;

	// QUERY DATA
	protected static ref array<IEntitySource> s_aGenerators = {};
	protected static IEntitySource s_CurrentQueryGenerator;

	protected static bool s_bClearingNewPlacementSystem;

	protected static ref SCR_DebugShapeManager s_DebugShapeManager; // this one is static

	protected static const float TOLERANCE_SQUARED = 0.01; // 0.1 * 0.1
	protected static const float MIN_CABLE_LENGTH_SQ = 0.01; // 0.1 * 0.1
	protected static const float GENERATOR_BBOX_TOLERANCE = 5; // in metres
	protected static const string POWER_LINE_CLASS = "PowerlineEntity";

	protected static const SCR_EPoleCableType DEFAULT_CABLE_TYPE = SCR_EPoleCableType.POWER_LV;

	protected static const float POLE_DISTANCE_TOLERANCE = 1.0; // any pole closer than that will be dropped

	// DEBUG variables (color, pos, size)

	protected static const int DEBUG_POLE_COLOUR_POS = Color.BLUE & 0x00FFFFFF | 0x88000000;
	protected static const int DEBUG_POLE_COLOUR_ERROR = Color.RED & 0x00FFFFFF | 0x88000000;
	protected static const float DEBUG_POLE_SIZE = 1;
	protected static const vector DEBUG_POLE_POS = "0 5 0";

	protected static const int DEBUG_SLOT_COLOUR_POS = Color.DARK_GREEN & 0x00FFFFFF | 0x88000000;
	// protected static const int DEBUG_SLOT_COLOUR_ERROR = Color.RED & 0x00FFFFFF | 0x88000000;
	protected static const int DEBUG_SLOT_COLOUR_UNUSED = Color.ORANGE & 0x00FFFFFF | 0x88000000;
	protected static const float DEBUG_SLOT_SIZE = 0.1;

	protected static const int DEBUG_CABLE_LENGTH_COLOUR_ERROR_SPHERE = Color.RED & 0x00FFFFFF | 0xAA000000;
	protected static const int DEBUG_CABLE_LENGTH_COLOUR_ERROR_LINE = Color.RED;
	protected static const vector DEBUG_CABLE_LENGTH_POS = "0 3 0";
	protected static const float DEBUG_CABLE_LENGTH_POS_MAX_DIST = DEBUG_CABLE_LENGTH_POS.Length();
	protected static const float DEBUG_CABLE_LENGTH_SIZE = 0.5;

	protected static const int DEBUG_CABLE_COLOUR_POS = Color.YELLOW & 0x00FFFFFF | 0x88000000;	//!< virtual power cables

	protected static const int DEBUG_GENERATOR_COLOUR = Color.GREEN;								//!< generators's bounding box
	protected static const int DEBUG_GENERATOR_MIN_HEIGHT = 20; // plus bbox tolerance

	//------------------------------------------------------------------------------------------------
	//! Generates junctions for selected powerline generators, optionally drawing debug shapes, and generates poles for them.
	//! \param[in] generator Generates junctions for powerline generators in the world editor, also handles pole generation based on new placement system.
	//! \param[in] drawDebugShapes Determines whether debug shapes for generator junctions are drawn during execution.
	protected static void GenerateGeneratorJunctions(notnull IEntitySource generator, bool drawDebugShapes = false)
	{
		WorldEditorAPI worldEditorAPI = ((WorldEditor)Workbench.GetModule(WorldEditor)).GetApi();
		if (!worldEditorAPI || worldEditorAPI.UndoOrRedoIsRestoring())
			return;

		if (!worldEditorAPI.IsDoingEditAction())
		{
			worldEditorAPI.BeginEntityAction();
			GenerateGeneratorJunctions(generator, drawDebugShapes);
			worldEditorAPI.EndEntityAction();
			return;
		}

		if (drawDebugShapes)
		{
			if (s_DebugShapeManager)
				s_DebugShapeManager.Clear();
			else
				s_DebugShapeManager = new SCR_DebugShapeManager();
		}
		else
		{
			s_DebugShapeManager = null;
		}

		if (s_aGenerators)
			s_aGenerators.Clear();
		else
			s_aGenerators = {};

		s_CurrentQueryGenerator = generator;
		s_aGenerators.Insert(generator);
		QueryGenerators(generator);

		bool useNewPlacementSystem;
		if (generator.Get("m_bUseNewPlacementSystem", useNewPlacementSystem) && useNewPlacementSystem)
		{
			s_bClearingNewPlacementSystem = true;
			if (!worldEditorAPI.ClearVariableValue(generator, null, "m_bUseNewPlacementSystem"))
				Print("Cannot clear m_bUseNewPlacementSystem", LogLevel.WARNING);
		}
		else
		{
			useNewPlacementSystem = false; // in case there is no value
		}

		SCR_PowerlineGeneratorEntity queriedGenerator;
		int generatorsCount = s_aGenerators.Count();
		foreach (IEntitySource generatorSource : s_aGenerators)
		{
			queriedGenerator = SCR_PowerlineGeneratorEntity.Cast(worldEditorAPI.SourceToEntity(generatorSource));
			if (!queriedGenerator)
				continue;

			queriedGenerator.DeleteAllChildren();
			queriedGenerator.GenerateJunctions();
		}

		SCR_PowerlineGeneratorEntity otherQueriedGenerator;
		foreach (IEntitySource generatorSource : s_aGenerators)
		{
			queriedGenerator = SCR_PowerlineGeneratorEntity.Cast(worldEditorAPI.SourceToEntity(generatorSource));
			if (!queriedGenerator)
				continue;

			foreach (IEntitySource otherGeneratorSource : s_aGenerators)
			{
				if (generatorSource == otherGeneratorSource)
					continue;

				otherQueriedGenerator = SCR_PowerlineGeneratorEntity.Cast(worldEditorAPI.SourceToEntity(otherGeneratorSource));
				if (!otherQueriedGenerator)
					continue;

				queriedGenerator.FindCommonJunctionsPoints(otherQueriedGenerator.m_aMyJunctionPoles);
			}

			if (useNewPlacementSystem) // this makes "new placement" managed by the generator 
			{
				Debug.BeginTimeMeasure();
				queriedGenerator.GeneratePolesNew();
				Debug.EndTimeMeasure("Pole Generation (NEW)");
			}
			else
			{
				Debug.BeginTimeMeasure();
				queriedGenerator.GeneratePoles();
				Debug.EndTimeMeasure("Pole Generation (OLD)");
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] point
	//! \return whether or not the provided point is near a junction pole
	protected bool IsJunctionPoint(vector point)
	{
		foreach (IEntity junctionPole : m_aMyJunctionPoles)
		{
			if (vector.DistanceSq(point, CoordToLocal(junctionPole.GetOrigin())) < TOLERANCE_SQUARED)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] other the other power line generator
	//! \return whether or not the two generators (current & provided) have a point in common (< 0.1 distance)
	protected bool HasCommonPoint(notnull SCR_PowerlineGeneratorEntity other)
	{
		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		IEntitySource sourceOther = worldEditorAPI.EntityToSource(other);
		if (!sourceOther)
			return false;

		IEntitySource shapeSourceOther;
		shapeSourceOther = sourceOther.GetParent();
		if (!m_ParentShapeSource || !shapeSourceOther)
			return false;

		array<vector> pointsThis = GetPoints(m_ParentShapeSource);
		array<vector> pointsOther = GetPoints(shapeSourceOther);

		foreach (vector pointThis : pointsThis)
		{
			foreach (vector pointOther : pointsOther)
			{
				if (vector.DistanceSq(CoordToParent(pointThis), other.CoordToParent(pointOther)) < TOLERANCE_SQUARED)
					return true;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Fills m_aOtherJunctionPoles with the provided junction poles that are on the current shape's anchor points
	//! It prevents duplicates but does not filter out if the provided junction poles belong to the current generator
	//! \param[in] otherJunctionPoles
	protected void FindCommonJunctionsPoints(notnull array<IEntity> otherJunctionPoles)
	{
		array<vector> points = GetPoints(m_Source.GetParent());
		foreach (IEntity junctionPole : otherJunctionPoles)
		{
			foreach (vector point : points)
			{
				if (vector.DistanceSq(CoordToParent(point), junctionPole.GetOrigin()) < TOLERANCE_SQUARED)
				{
					// Isn't in the array yet
					if (!m_aOtherJunctionPoles.Contains(junctionPole))
						m_aOtherJunctionPoles.Insert(junctionPole);

					// We skip the rest of the points
					break;
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Generate junctions, happening before generating poles
	protected void GenerateJunctions()
	{
		m_aMyJunctionPoles.Clear();

		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		IEntity shapeEntity = worldEditorAPI.SourceToEntity(m_ParentShapeSource);
		if (!shapeEntity)
			return;

		BaseContainerList points = m_ParentShapeSource.GetObjectArray("Points");
		int count = points.Count();
		if (count < 2)
			return;

		vector parentPositionXZ = shapeEntity.GetOrigin();
		float parentY = parentPositionXZ[1];
		BaseContainer point;
		vector lastPointPosition;
		vector currentPointPosition;
		float yaw;

		for (int i; i < count; i++)
		{
			point = points.Get(i);

			lastPointPosition = currentPointPosition;
			point.Get("Position", currentPointPosition);

			if (i == 0)
			{
				points.Get(1).Get("Position", lastPointPosition); // variable reuse
				yaw = (lastPointPosition - currentPointPosition).Normalized().ToYaw();
			}
			else
			{
				yaw = (currentPointPosition - lastPointPosition).Normalized().ToYaw();
			}

			GenerateJunctionOnPoint(point, yaw);
		}
	}

	//------------------------------------------------------------------------------------------------
	override bool _WB_OnKeyChanged(BaseContainer src, string key, BaseContainerList ownerContainers, IEntity parent)
	{
		if (s_bClearingNewPlacementSystem && key == "m_bUseNewPlacementSystem")
		{
			s_bClearingNewPlacementSystem = false;
			return false;
		}

		super._WB_OnKeyChanged(src, key, ownerContainers, parent);

		if (key == "coords")
			return false;

		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		if (!worldEditorAPI)
			return false;

		IEntitySource thisSrc = worldEditorAPI.EntityToSource(this);
		BaseContainerTools.WriteToInstance(this, thisSrc);

		ShapeEntity shapeEntity = ShapeEntity.Cast(parent);
		if (shapeEntity)
			OnShapeInit(m_ParentShapeSource, shapeEntity);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] point
	//! \param[in] yaw
	protected void GenerateJunctionOnPoint(notnull BaseContainer point, float yaw)
	{
		SCR_PowerlineGeneratorJunctionData junctionData;

		BaseContainerList dataArr = point.GetObjectArray("Data");
		BaseContainer data;
		typename classNameType;
		for (int j = 0, dataCount = dataArr.Count(); j < dataCount; ++j)
		{
			data = dataArr.Get(j);
			classNameType = data.GetClassName().ToType();
			if (!classNameType)
				continue;

			if (classNameType.IsInherited(SCR_PowerlineGeneratorPointData))
			{
				data.Get("m_JunctionData", junctionData);
				break;
			}
		}

		if (!junctionData)
			return;

		ResourceName junctionResourceName = junctionData.m_sJunctionPrefab;
		if (junctionResourceName.IsEmpty())
			junctionResourceName = m_DefaultJunctionPole;

		if (junctionResourceName.IsEmpty())
			return;

		vector pointPosition;
		point.Get("Position", pointPosition);

		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		IEntitySource poleSrc = worldEditorAPI.CreateEntity(junctionResourceName, string.Empty, 0, m_Source, pointPosition, vector.Zero);
		if (!poleSrc)
			return;

		yaw = Math.Repeat(yaw + junctionData.m_fYawOffset, 360);

		worldEditorAPI.SetVariableValue(poleSrc, null, "angleY", yaw.ToString());

		SCR_PowerPole powerPole = GetPowerPoleFromEntitySource(poleSrc);
		if (!powerPole)
			return;

		if (junctionData.m_bPowerSource)
		{
			IEntitySource powerPoleSrc = worldEditorAPI.EntityToSource(powerPole);
			if (powerPoleSrc)
			{
				worldEditorAPI.SetVariableValue(powerPoleSrc, null, "PowerSource", junctionData.m_bPowerSource.ToString(true));

				//Refresh powerpole pointer
				powerPole = GetPowerPoleFromEntitySource(poleSrc);
			}
		}

		m_aMyJunctionPoles.Insert(powerPole);
	}

	//------------------------------------------------------------------------------------------------
	//! Delete all shape's children and clear the junction poles list
	protected override void DeleteAllChildren()
	{
		super.DeleteAllChildren();
		m_aMyJunctionPoles.Clear();
	}

	//------------------------------------------------------------------------------------------------
	//! Create all the poles - happens after having generated junctions
	// TODO: generate pole positions first -then- generate pole entities -then- generate cables
	// this would allow to:
	// - space poles evenly (with connections and ending poles) - "wished" spacing (e.g not exact)
	// - ease pole yaw angle in sharp turns
	protected void GeneratePoles()
	{
		if (!m_ParentShapeSource)
		{
			Print("Parent Shape Source is null, cannot generate power poles", LogLevel.ERROR);
			return;
		}

		if (m_bDrawDebugShapes)
			m_DebugShapeManager = new SCR_DebugShapeManager();
		else
			m_DebugShapeManager = null;

		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		if (!worldEditorAPI)
			return;

		if (!worldEditorAPI.IsDoingEditAction())
		{
			Print("GeneratePoles was called while not in edit action", LogLevel.ERROR);
			return;
		}

		array<vector> anchorPoints = {};
		array<vector> shapePoints = {};

		ShapeEntity parentShapeEntity = ShapeEntity.Cast(worldEditorAPI.SourceToEntity(m_ParentShapeSource));
		if (!parentShapeEntity) // wat
		{
			Print("Parent Shape is null, cannot generate power poles", LogLevel.ERROR);
			return;
		}

		parentShapeEntity.GenerateTesselatedShape(shapePoints);
		int shapePointsCount = shapePoints.Count();
		if (shapePointsCount < 2)
		{
			Print("Power Line Generator shape only has one point - no power line will be generated", LogLevel.WARNING);
			return;
		}

		parentShapeEntity.GetPointsPositions(anchorPoints);
		BaseContainerList pointsSource = m_ParentShapeSource.GetObjectArray("Points");

		float pointDistance, poleOffset, currentDistance, yaw, nextPoleDistance;
		vector currentPoint, prevPoint, prevAnchorPoint;
		bool isAnchorPoint, spawnPerPoint, previousWasPerPoint;
		BaseContainerList dataArr;
		BaseContainer data;
		vector direction, lastPolePosition, nextPolePosition;
		vector parentPositionXZ = parentShapeEntity.GetOrigin();
		float parentY = parentPositionXZ[1];
		parentPositionXZ[1] = 0;
		vector currAnchorPoint = anchorPoints[0];
		int currentAnchorIndex;
		int anchorCount = anchorPoints.Count();
		typename classNameType;

		// foreach = change GenerateEndPole at the end + prevPoint management
		for (int i; i < shapePointsCount; i++)
		{
			prevPoint = currentPoint;
			previousWasPerPoint = spawnPerPoint;
			currentPoint = shapePoints[i];

			if (isAnchorPoint) // if the -previous- point was an anchor, increment
			{
				prevAnchorPoint = currAnchorPoint;
				currentAnchorIndex++;
				if (currentAnchorIndex < anchorCount)
					currAnchorPoint = anchorPoints[currentAnchorIndex];
			}

			isAnchorPoint = currentPoint == currAnchorPoint;

			poleOffset = 0;

			if (i > 0)
			{
				if (spawnPerPoint)
					AttachJunctionOnPoint(prevAnchorPoint);
				else
					AttachJunctionOnPoint(prevPoint);

				// Get data of previous point
				if (currentAnchorIndex > 0) // currentAnchorIndex check not really required
				{
					dataArr = pointsSource.Get(currentAnchorIndex - 1).GetObjectArray("Data");
					for (int j = 0, dataCount = dataArr.Count(); j < dataCount; ++j)
					{
						data = dataArr.Get(j);
						classNameType = data.GetClassName().ToType();
						if (!classNameType)
							continue;

						if (!classNameType.IsInherited(SCR_PowerlineGeneratorPointData))
							continue;

						data.Get("m_fPoleOffset", poleOffset);
						if (m_fDistance + poleOffset < 1) // generatePole too close to the end
							poleOffset = 1 - m_fDistance;

						data.Get("m_bGeneratePerPoint", spawnPerPoint);
						break;
					}
				}
			}

			direction = (currentPoint - prevPoint).Normalized();
			yaw = direction.ToYaw();

			if (i == 1) //Generate start pole
				GenerateStartPole(prevPoint, parentY, parentPositionXZ, yaw);

			if (spawnPerPoint)
			{
				if (!isAnchorPoint)
					continue;

				if (i < 2) // neither 0 (no previous point) nor 1 (Start Pole created)
					continue;

				if (!IsJunctionPoint(prevAnchorPoint))
					GeneratePole(prevAnchorPoint, parentPositionXZ, parentY, yaw);
			}
			else // not spawnPerPoint
			{
				if (i == 0)
					continue;

				if (previousWasPerPoint)
				{
					GeneratePole(prevAnchorPoint, parentPositionXZ, parentY, yaw);
					currentDistance = 0;
				}

				pointDistance = vector.Distance(currentPoint, prevPoint);
				lastPolePosition = prevPoint;

				if (currentDistance + pointDistance > m_fDistance + poleOffset)
				{
					nextPoleDistance = (m_fDistance + poleOffset - currentDistance);
					pointDistance -= nextPoleDistance;

					nextPolePosition = prevPoint + direction * nextPoleDistance;

					GeneratePole(nextPolePosition, parentPositionXZ, parentY, yaw);

					lastPolePosition = nextPolePosition;
					currentDistance = 0;
				}
				else // the point is too close, we add the distance and continue to the next point
				{
					currentDistance += pointDistance - poleOffset;
					continue;
				}

				if (m_fDistance == 0 && poleOffset == 0)
				{
					Print("m_fDistance in Powerline Generator and pole offset is 0. Not generating any poles.", LogLevel.WARNING);
				}
				else
				{
					while (pointDistance > m_fDistance + poleOffset)
					{
						nextPolePosition = lastPolePosition + direction * (poleOffset + m_fDistance);

						GeneratePole(nextPolePosition, parentPositionXZ, parentY, yaw);

						lastPolePosition = nextPolePosition;
						pointDistance -= m_fDistance + poleOffset;
					}
				}

				if (pointDistance > 0)
					currentDistance += pointDistance;
			}
		}

		GenerateEndPole(currentPoint, parentY, parentPositionXZ, yaw);
	}

	//------------------------------------------------------------------------------------------------
	//! Generates power poles position first then creates poles and cables if valid
	protected void GeneratePolesNew()
	{
		if (!m_ParentShapeSource)
		{
			Print("Parent Shape Source is null, cannot generate power poles", LogLevel.ERROR);
			return;
		}

		if (m_bDrawDebugShapes)
			m_DebugShapeManager = new SCR_DebugShapeManager();
		else
			m_DebugShapeManager = null;

		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		if (!worldEditorAPI)
			return;

		if (!worldEditorAPI.IsDoingEditAction())
		{
			Print("GeneratePoles was called while not in edit action", LogLevel.ERROR);
			return;
		}

		ShapeEntity parentShapeEntity = ShapeEntity.Cast(worldEditorAPI.SourceToEntity(m_ParentShapeSource));
		if (!parentShapeEntity) // wat
		{
			Print("Parent Shape is null, cannot generate power poles", LogLevel.ERROR);
			return;
		}

		array<vector> anchorPointsRelPos = {};
		parentShapeEntity.GetPointsPositions(anchorPointsRelPos);

		if (anchorPointsRelPos.Count() < 2)
		{
			Print("Not enough shape points", LogLevel.ERROR);
			return;
		}

		// ANCHORS DATA COLLECTION
		map<int, ref SCR_PowerlineGeneratorPointData> pointsData = GetParentShapeAnchorsData();

		// GET POLES'S POSITION
		array<vector> tesselatedPointsRelPos = {};
		parentShapeEntity.GenerateTesselatedShape(tesselatedPointsRelPos);

		array<vector> polePointsRelPos = GetPolesRelPosition(anchorPointsRelPos, tesselatedPointsRelPos, pointsData);
		if (!polePointsRelPos)
		{
			Print("Power Line Generator shape is invalid; no poles will be generated", LogLevel.ERROR);
			return;
		}

		bool debugShapes = false;
		if (debugShapes && !s_DebugShapeManager)
		{
			s_DebugShapeManager = new SCR_DebugShapeManager();

			vector prevPoleWorldPos;
			foreach (int i, vector polePointRelPos : polePointsRelPos)
			{
				polePointRelPos = CoordToParent(polePointRelPos);
				s_DebugShapeManager.AddSphere(polePointRelPos, 1);

				if (prevPoleWorldPos)
					s_DebugShapeManager.AddLine(prevPoleWorldPos + vector.Up, polePointRelPos + vector.Up);
				else
					s_DebugShapeManager.AddSphere(polePointRelPos + 3 * vector.Up, 2, Color.DARK_GREEN, ShapeFlags.NOOUTLINE);

				if (i == polePointsRelPos.Count() - 1)
					s_DebugShapeManager.AddSphere(polePointRelPos + 3 * vector.Up, 2, Color.DARK_RED, ShapeFlags.NOOUTLINE);

				prevPoleWorldPos = polePointRelPos;
			}
		}

		// POLES (and cables) CREATION
		if (!debugShapes)
			CreatePolesAndCables(polePointsRelPos, anchorPointsRelPos, pointsData);
	}

	//------------------------------------------------------------------------------------------------
	//! Get point data from parent shape source points.
	//! \return map of anchor index / point data
	protected map<int, ref SCR_PowerlineGeneratorPointData> GetParentShapeAnchorsData()
	{
		map<int, ref SCR_PowerlineGeneratorPointData> result = new map<int, ref SCR_PowerlineGeneratorPointData>();
		BaseContainerList pointsSource = m_ParentShapeSource.GetObjectArray("Points");
		BaseContainerList dataArray;
		BaseContainer data;
		typename classNameType;
		SCR_PowerlineGeneratorPointData pointData;
		SCR_PowerlineGeneratorJunctionData junctionData;
		for (int i, count = pointsSource.Count(); i < count; i++)
		{
			dataArray = pointsSource.Get(i).GetObjectArray("Data");
			if (!dataArray)
				continue;

			for (int j = 0, dataCount = dataArray.Count(); j < dataCount; j++)
			{
				data = dataArray.Get(j);
				classNameType = data.GetClassName().ToType();
				if (!classNameType || !classNameType.IsInherited(SCR_PowerlineGeneratorPointData))
					continue;

				pointData = new SCR_PowerlineGeneratorPointData();
				data.Get("m_fPoleOffset", pointData.m_fPoleOffset);
				data.Get("m_bGeneratePerPoint", pointData.m_bGeneratePerPoint);
				data.Get("m_JunctionData", junctionData);
				if (junctionData)
					pointData.m_JunctionData = junctionData;

				result.Insert(i, pointData);
				break; // only take the first one
			}
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Generates poles relative positions for powerline generator based on anchor points, tesselated points, and powerline data
	//! \param[in] anchorPointsRelPos
	//! \param[in] tesselatedPointsRelPos
	//! \param[in] pointsData
	//! \return null if the shape is invalid
	protected array<vector> GetPolesRelPosition(
		notnull array<vector> anchorPointsRelPos,
		notnull array<vector> tesselatedPointsRelPos,
		notnull map<int, ref SCR_PowerlineGeneratorPointData> pointsData)
	{
		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		ShapeEntity shapeEntity = ShapeEntity.Cast(worldEditorAPI.SourceToEntity(m_ParentShapeSource));
		if (!shapeEntity)
			return null;

		SCR_ShapeNextPointHelper shapeNextPointHelper = new SCR_ShapeNextPointHelper(shapeEntity);
		if (!shapeNextPointHelper.IsValid())
			return null;

		array<vector> result = {};

		float poleOffset;

		if (m_fDistance < POLE_DISTANCE_TOLERANCE)
			m_fDistance = POLE_DISTANCE_TOLERANCE;

		const float distanceSq = m_fDistance * m_fDistance;

		bool generatePerPoint;
		vector lastPoleRelPos;
		IEntity junction;
		SCR_PowerlineGeneratorPointData pointData;
		int anchorPointsCountMinus1 = anchorPointsRelPos.Count() - 1;
		foreach (int i, vector anchorPointRelPos : anchorPointsRelPos)
		{
			bool isStart = i == 0;
			bool isEnd = i == anchorPointsCountMinus1;
			bool hasJunction;

			bool wasGeneratedPerPoint = generatePerPoint;
			float oldPoleOffset = poleOffset;

			pointData = pointsData.Get(i);
			if (pointData)
			{
				poleOffset = pointData.m_fPoleOffset;
				generatePerPoint = pointData.m_bGeneratePerPoint;

				// hardcoded safety
				if (m_fDistance + poleOffset < POLE_DISTANCE_TOLERANCE)
					poleOffset = POLE_DISTANCE_TOLERANCE - m_fDistance;

				hasJunction = pointData.m_JunctionData != null && (m_DefaultJunctionPole || pointData.m_JunctionData.m_sJunctionPrefab);
			}
			else
			{
				poleOffset = 0;
			}

			if (!isStart && !wasGeneratedPerPoint)
			{
				// add offset if it exists
				if (poleOffset > 0 && shapeNextPointHelper.GetNextPoint(m_fDistance + poleOffset, lastPoleRelPos, i))
					result.Insert(lastPoleRelPos);

				// straight to the current anchor point!
				while (shapeNextPointHelper.GetNextPoint(m_fDistance, lastPoleRelPos, i))
				{
					result.Insert(lastPoleRelPos);
				}
			}

			if (wasGeneratedPerPoint || generatePerPoint || hasJunction || isEnd || isStart || vector.Distance(lastPoleRelPos, anchorPointRelPos) == 0)
			{
				int resultCount = result.Count();
				if (resultCount > 0 && vector.Distance(lastPoleRelPos, anchorPointRelPos) < POLE_DISTANCE_TOLERANCE)
					result.Remove(resultCount - 1);

				result.Insert(anchorPointRelPos);
				shapeNextPointHelper.SetOnAnchor(i);
				// shapeNextPointHelper.SetPos(anchorPointRelPos);
				lastPoleRelPos = anchorPointRelPos;
			}
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Creates poles and cables between given points with optional random pitch and roll, snaps to terrain if no junction found
	//! \param[in] polePointsRelpos
	//! \param[in] anchorPointsRelPos
	//! \param[in] pointsData
	void CreatePolesAndCables(notnull array<vector> polePointsRelPos, notnull array<vector> anchorPointsRelPos, notnull map<int, ref SCR_PowerlineGeneratorPointData> pointsData)
	{
		bool snapToGround = !m_bSculptTerrain;
		float yOffset = m_ShapeEntity.GetOrigin()[1];

		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		int poleWorldPointsCountMinus1 = polePointsRelPos.Count() - 1;
		vector prevPoleRelPoint;
		ResourceName prefab;
		vector vectorDir;
		IEntitySource prevPole;
		IEntitySource currPole;
		IEntity junction;
		int anchorPointIndex;
		SCR_PowerlineGeneratorPointData pointData;
		foreach (int i, vector currPoleRelPoint : polePointsRelPos)
		{
			bool applyRandomPitchAndRoll;
			bool isStart = i == 0;
			bool isEnd = i == poleWorldPointsCountMinus1;
			if (isStart || isEnd || anchorPointsRelPos[anchorPointIndex] == currPoleRelPoint)	// anchor: start, end or junction
			{
				bool isSameLine;
				junction = FindJunctionOnPoint(currPoleRelPoint, isSameLine);	// one must look for junction on all points
																				// as another generator could make a junction here
				if (junction)
				{
					currPole = worldEditorAPI.EntityToSource(junction);
					if (m_DebugShapeManager)
						m_DebugShapeManager.AddSphere(CoordToParent(currPoleRelPoint), 2);
				}

				if (currPole)																		// found junction
				{
					pointData = pointsData.Get(anchorPointIndex);
					applyRandomPitchAndRoll = isSameLine;
					if (isSameLine)
					{
						vectorDir = vector.Direction(
							vector.Direction(currPoleRelPoint, prevPoleRelPoint),
							vector.Direction(currPoleRelPoint, polePointsRelPos[i + 1]));

						float yaw = vectorDir.ToYaw();
						if (pointData.m_JunctionData)
							yaw += pointData.m_JunctionData.m_fYawOffset;
	
						worldEditorAPI.SetVariableValue(currPole, null, "angleY", yaw.ToString());
					}
				}
				else
				{
					if (isStart)																	// start pole
					{
						vectorDir = vector.Direction(currPoleRelPoint, polePointsRelPos[1]);
						prefab = m_StartPole;
						applyRandomPitchAndRoll = m_bApplyPitchAndRollStart;
						if (m_bRotate180DegreeYawStartPole)
							vectorDir = -vectorDir;
					}
					else if (isEnd)																	// end pole
					{
						vectorDir = vector.Direction(prevPoleRelPoint, currPoleRelPoint);
						prefab = m_EndPole;
						applyRandomPitchAndRoll = m_bApplyPitchAndRollEnd;
						if (m_bRotate180DegreeYawEndPole)
							vectorDir = -vectorDir;
					}
					else																			// normal pole that happens to be EXACTLY on an anchor pos
					{
						vectorDir = vector.Direction(
							vector.Direction(currPoleRelPoint, prevPoleRelPoint).Normalized(),
							vector.Direction(currPoleRelPoint, polePointsRelPos[i + 1]).Normalized());

						prefab = m_DefaultPole;
						applyRandomPitchAndRoll = m_bApplyPitchAndRollDefault;
					}
				}

				anchorPointIndex++;
			}
			else																					// normal pole
			{
				vectorDir = vector.Direction(
					vector.Direction(currPoleRelPoint, prevPoleRelPoint).Normalized(),
					vector.Direction(currPoleRelPoint, polePointsRelPos[i + 1]).Normalized());

				prefab = m_DefaultPole;
				applyRandomPitchAndRoll = m_bApplyPitchAndRollDefault;
			}

			if (!currPole)
			{
				if (snapToGround)
				{
					vector worldPos = CoordToParent(currPoleRelPoint);
					worldPos[1] = worldEditorAPI.GetTerrainSurfaceY(worldPos[0], worldPos[2]);
					currPoleRelPoint = CoordToLocal(worldPos);
				}

				currPole = worldEditorAPI.CreateEntity(prefab, string.Empty, 0, m_Source, currPoleRelPoint, vector.Zero);

				worldEditorAPI.SetVariableValue(currPole, null, "angleY", vectorDir.ToYaw().ToString());
				if (applyRandomPitchAndRoll)
					ApplyRandomPitchAndRoll(currPole);

				if (m_DebugShapeManager)
				{
					m_DebugShapeManager.AddSphere(CoordToParent(currPoleRelPoint), DEBUG_POLE_SIZE, DEBUG_POLE_COLOUR_POS, ShapeFlags.NOOUTLINE);
					m_DebugShapeManager.AddArrow(CoordToParent(currPoleRelPoint), CoordToParent(currPoleRelPoint + (3 * { Math.Sin(vectorDir.ToYaw() * Math.DEG2RAD), 0, Math.Cos(vectorDir.ToYaw() * Math.DEG2RAD) })));
				}
			}

			if (i > 0)
				CreatePowerLines(GetPowerPoleFromEntitySource(prevPole), GetPowerPoleFromEntitySource(currPole));

			prevPole = currPole;
			prevPoleRelPoint = currPoleRelPoint;
			currPole = null;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Finds nearest junction pole from given point
	//! \param[in] point Finds nearest junction pole on given point, checks if it is on same line as the current generator
	//! \param[out] sameLine Represents whether the found junction is on the same line as the given point
	//! \return the junction pole closest to the given point, or null if no junction pole is found
	protected IEntity FindJunctionOnPoint(vector point, out bool sameLine)
	{
		foreach (IEntity junctionPole : m_aMyJunctionPoles)
		{
			if (junctionPole && vector.DistanceSq(point, CoordToLocal(junctionPole.GetOrigin())) < TOLERANCE_SQUARED)
			{
				sameLine = true;
				return junctionPole;
			}
		}

		foreach (IEntity junctionPole : m_aOtherJunctionPoles)
		{
			if (junctionPole && vector.DistanceSq(point, CoordToLocal(junctionPole.GetOrigin())) < TOLERANCE_SQUARED)
			{
				sameLine = false;
				return junctionPole;
			}
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Attaches junctions to point based on distance tolerance, separates junctions into attached and unattached groups.
	//! \param[in] point a world-space coordinate where the junction can be attached
	protected void AttachJunctionOnPoint(vector point)
	{
		foreach (IEntity junctionPole : m_aMyJunctionPoles)
		{
			if (junctionPole && vector.DistanceSq(point, CoordToLocal(junctionPole.GetOrigin())) < TOLERANCE_SQUARED)
				AttachJunction(junctionPole, true);
		}

		foreach (IEntity junctionPole : m_aOtherJunctionPoles)
		{
			if (junctionPole && vector.DistanceSq(point, CoordToLocal(junctionPole.GetOrigin())) < TOLERANCE_SQUARED)
				AttachJunction(junctionPole, false);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Attaches junction to power pole, creates power lines if necessary, updates last junction state
	//! \param[in] junction Connects power poles in a line, updates previous power pole source for next junction attachment
	//! \param[in] sameLine Whether the new junction is on the same line as the previous one
	protected void AttachJunction(notnull IEntity junction, bool sameLine)
	{
		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		IEntitySource junctionSource = worldEditorAPI.EntityToSource(junction);
		if (!junctionSource)
			return;

		SCR_PowerPole junctionPole = GetPowerPoleFromEntitySource(junctionSource);
		if (!junctionPole)
			return;

		m_bLastJunctionWasSameLine = sameLine;

		if (m_PreviousPowerPoleSource)
			CreatePowerLines(GetPowerPoleFromEntitySource(m_PreviousPowerPoleSource), junctionPole, sameLine);

		m_PreviousPowerPoleSource = junctionSource;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] currentPoint
	//! \param[in] parentY
	//! \param[in] parentPositionXZ
	//! \param[in] yaw
	protected void GenerateEndPole(vector currentPoint, float parentY, vector parentPositionXZ, float yaw)
	{
		if (m_EndPole.IsEmpty())
			return;

		vector endPolePos = currentPoint;
		bool sameLine;
		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		IEntitySource poleSource = worldEditorAPI.EntityToSource(FindJunctionOnPoint(currentPoint, sameLine));

		if (poleSource)
		{
			m_bLastJunctionWasSameLine = sameLine;
		}
		else
		{
			poleSource = GeneratePoleAt(endPolePos, parentPositionXZ, parentY, m_EndPole);
			sameLine = true;

			if (SCR_JunctionPowerPole.Cast(GetPowerPoleFromEntitySource(poleSource)))
				Print("End pole is of type SCR_JunctionPowerPole, make sure to do this using junction data on the last point instead.", LogLevel.WARNING);

			if (m_bRotate180DegreeYawEndPole)
			{
				yaw += 180;
				if (yaw > 360)
					yaw -= 360;
			}

			worldEditorAPI.SetVariableValue(poleSource, null, "angleY", yaw.ToString());
			if (m_bApplyPitchAndRollEnd)
				ApplyRandomPitchAndRoll(poleSource);
		}

		SCR_PowerPole endPole = GetPowerPoleFromEntitySource(poleSource);
		if (endPole)
			CreatePowerLines(GetPowerPoleFromEntitySource(m_PreviousPowerPoleSource), endPole, sameLine);

		if (m_DebugShapeManager)
			m_DebugShapeManager.AddSphere(worldEditorAPI.SourceToEntity(poleSource).GetOrigin(), DEBUG_POLE_SIZE, DEBUG_POLE_COLOUR_POS, ShapeFlags.NOOUTLINE);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] relativePos
	//! \param[in] parentY
	//! \param[in] parentPositionXZ
	//! \param[in] yaw
	protected void GenerateStartPole(vector relativePos, float parentY, vector parentPositionXZ, float yaw)
	{
		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		if (!worldEditorAPI)
		{
			Print("WorldEditorAPI not found in SCR_PowerlineGeneratorEntity.", LogLevel.WARNING);
			return;
		}

		if (m_StartPole.IsEmpty())
			return;

		vector startPolePos = relativePos;
		bool sameLine;
		IEntity pole = FindJunctionOnPoint(relativePos, sameLine);
		IEntitySource poleSrc;

		if (pole)
		{
			m_bLastJunctionWasSameLine = sameLine;
			poleSrc = worldEditorAPI.EntityToSource(pole);
		}
		else
		{
			poleSrc = GeneratePoleAt(startPolePos, parentPositionXZ, parentY, m_StartPole);
			pole = worldEditorAPI.SourceToEntity(poleSrc);
			if (!pole)
			{
				Print("Pole entity not created in SCR_PowerlineGeneratorEntity.", LogLevel.WARNING);
				return;
			}

			if (SCR_JunctionPowerPole.Cast(pole))
			{
				Print("Start pole is of type SCR_JunctionPowerPole, make sure to do this using junction data on the first point instead.", LogLevel.WARNING);
				m_bLastJunctionWasSameLine = true;
			}

			if (m_bRotate180DegreeYawStartPole)
			{
				yaw += 180;
				if (yaw > 360)
					yaw -= 360;
			}

			worldEditorAPI.SetVariableValue(poleSrc, null, "angleY", yaw.ToString());
			if (m_bApplyPitchAndRollStart)
				ApplyRandomPitchAndRoll(poleSrc);
		}

		m_PreviousPowerPoleSource = poleSrc;

		if (m_DebugShapeManager)
			m_DebugShapeManager.AddSphere(worldEditorAPI.SourceToEntity(poleSrc).GetOrigin(), DEBUG_POLE_SIZE, DEBUG_POLE_COLOUR_POS, ShapeFlags.NOOUTLINE);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] localPos
	//! \param[in] parentPositionXZ
	//! \param[in] parentY
	//! \param[in] poleResourceName
	//! \return
	protected IEntitySource GeneratePoleAt(vector localPos, vector parentPositionXZ, float parentY, ResourceName poleResourceName)
	{
		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		float y;
		if (worldEditorAPI.TryGetTerrainSurfaceY(localPos[0] + parentPositionXZ[0], localPos[2] + parentPositionXZ[2], y))
			localPos[1] = y - parentY;

		return worldEditorAPI.CreateEntity(poleResourceName, string.Empty, 0, m_Source, localPos, vector.Zero);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] powerPole
	protected void ApplyRandomPitchAndRoll(IEntitySource powerPole)
	{
		float pitch = 0, roll = 0;
		if (m_bRandomPitchOnBothSides)
			pitch = -m_fRandomPitchAngle + Math.RandomFloat(0, 2 * m_fRandomPitchAngle);
		else
			pitch = Math.RandomFloat(0, m_fRandomPitchAngle);

		if (m_bRandomRollOnBothSides)
			roll = -m_fRandomRollAngle + Math.RandomFloat(0, 2 * m_fRandomRollAngle);
		else
			roll = Math.RandomFloat(0, m_fRandomRollAngle);

		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		worldEditorAPI.SetVariableValue(powerPole, null, "angleX", pitch.ToString());
		worldEditorAPI.SetVariableValue(powerPole, null, "angleZ", roll.ToString());
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] lastPolePosition
	//! \param[in] parentPositionXZ
	//! \param[in] parentY
	//! \param[in] yaw
	//! \param[in] customPoleResourceName
	//! \return
	protected IEntitySource GeneratePole(vector lastPolePosition, vector parentPositionXZ, float parentY, float yaw, ResourceName customPoleResourceName = string.Empty)
	{
		float y;
		IEntitySource poleSource;
		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		if (worldEditorAPI.TryGetTerrainSurfaceY(lastPolePosition[0] + parentPositionXZ[0], lastPolePosition[2] + parentPositionXZ[2], y))
			lastPolePosition[1] = y - parentY;

		ResourceName poleResourceName;
		if (customPoleResourceName.IsEmpty())
			poleResourceName = m_DefaultPole;
		else
			poleResourceName = customPoleResourceName;

		if (poleResourceName.IsEmpty())
		{
			Print("Default pole is not defined", LogLevel.WARNING);
			return null;
		}

		poleSource = worldEditorAPI.CreateEntity(poleResourceName, string.Empty, 0, m_Source, lastPolePosition, vector.Zero);
		if (!poleSource)
			return null;

		worldEditorAPI.SetVariableValue(poleSource, null, "angleY", yaw.ToString());
		if (m_bApplyPitchAndRollDefault)
			ApplyRandomPitchAndRoll(poleSource);

		if (m_PreviousPowerPoleSource && poleSource)
		{
			SCR_PowerPole thisPowerPole = GetPowerPoleFromEntitySource(poleSource);
			if (thisPowerPole)
				CreatePowerLines(GetPowerPoleFromEntitySource(m_PreviousPowerPoleSource), thisPowerPole);
		}

		m_PreviousPowerPoleSource = poleSource;

		if (m_DebugShapeManager)
			m_DebugShapeManager.AddSphere(worldEditorAPI.SourceToEntity(poleSource).GetOrigin(), DEBUG_POLE_SIZE, DEBUG_POLE_COLOUR_POS, ShapeFlags.NOOUTLINE);

		return poleSource;
	}

	//------------------------------------------------------------------------------------------------
	//! Poles order matter - otherwise previous pole's B slots are ignored
	//! \param[in] existingPole
	//! \param[in] addedPole
	//! \param[in] sameLine
	//! \return the created power line entity sources, or null array on error / no cables
	// TODO: fix cable bug on rotated shape

	//------------------------------------------------------------------------------------------------
	//! Creates power lines between two power poles based on their cable types, if they have compatible slots
	//! \param[in] existingPole Existing pole represents the current power pole in the scene
	//! \param[in] addedPole Added pole represents the new pole being added to the power line
	//! \param[in] sameLine(Optional, default true) Creates power lines between existing pole and added pole if they belong to same line
	//! \return created power lines entity sources, null on error
	protected array<IEntitySource> CreatePowerLines(SCR_PowerPole existingPole, SCR_PowerPole addedPole, bool sameLine = true)
	{
		if (!existingPole || !addedPole || existingPole == addedPole)
			return null;

		if (SCR_JunctionPowerPole.Cast(existingPole) || SCR_JunctionPowerPole.Cast(addedPole))
			sameLine = m_bLastJunctionWasSameLine;

		vector existingPoleWorldPos = existingPole.GetOrigin();
		vector addedPoleWorldPos = addedPole.GetOrigin();

		map<SCR_EPoleCableType, ref SCR_PoleCableSlotGroup> existingPoleGroups = GetClosestSlotGroups(existingPole, addedPoleWorldPos, sameLine);
		if (!existingPoleGroups || existingPoleGroups.IsEmpty())
		{
			Print("No valid slots found on the existing power pole, please check the setup of your power poles.", LogLevel.WARNING);

			if (m_DebugShapeManager)
				m_DebugShapeManager.AddSphere(existingPoleWorldPos + DEBUG_POLE_POS, DEBUG_POLE_SIZE, DEBUG_POLE_COLOUR_ERROR, ShapeFlags.NOOUTLINE);

			return null;
		}

		map<SCR_EPoleCableType, ref SCR_PoleCableSlotGroup> addedPoleGroups = GetClosestSlotGroups(addedPole, existingPoleWorldPos, sameLine);
		if (!addedPoleGroups || addedPoleGroups.IsEmpty())
		{
			Print("No valid slots found on the new power pole, please check the setup of your power poles.", LogLevel.WARNING);

			if (m_DebugShapeManager)
				m_DebugShapeManager.AddSphere(addedPoleWorldPos + DEBUG_POLE_POS, DEBUG_POLE_SIZE, DEBUG_POLE_COLOUR_ERROR, ShapeFlags.NOOUTLINE);

			return null;
		}

		array<SCR_EPoleCableType> cableTypes = {};
		if (SCR_Enum.GetEnumValues(SCR_EPoleCableType, cableTypes) < 1) // like, wat
			return null;

		array<SCR_EPoleCableType> commonCableTypes = {};
		foreach (SCR_EPoleCableType cableType : cableTypes)
		{
			if (existingPoleGroups.Contains(cableType) && addedPoleGroups.Contains(cableType))
				commonCableTypes.Insert(cableType);

			// code below could be done here directly, but for clarity's sake, one foreach won't kill perfs
		}

		if (commonCableTypes.IsEmpty())
		{
			Print("No cable types in common between two poles - be sure to select compatible poles in Generator Prefab (" + existingPoleWorldPos.ToString(false) + ")", LogLevel.WARNING);

			if (m_DebugShapeManager)
			{
				m_DebugShapeManager.AddSphere(existingPoleWorldPos + DEBUG_POLE_POS, DEBUG_POLE_SIZE, DEBUG_POLE_COLOUR_ERROR, ShapeFlags.NOOUTLINE);
				m_DebugShapeManager.AddSphere(addedPoleWorldPos + DEBUG_POLE_POS, DEBUG_POLE_SIZE, DEBUG_POLE_COLOUR_ERROR, ShapeFlags.NOOUTLINE);
				m_DebugShapeManager.AddLine(existingPoleWorldPos + DEBUG_POLE_POS, addedPoleWorldPos + DEBUG_POLE_POS, DEBUG_POLE_COLOUR_ERROR);
			}

			return null;
		}

		array<IEntitySource> result = {};

		array<vector> startPoints = {};		// world position
		array<vector> endPoints = {};		// world position
		SCR_PoleCableSlotGroup existingGroup;
		SCR_PoleCableSlotGroup addedGroup;
		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		IEntitySource addedPoleEntitySource = worldEditorAPI.EntityToSource(addedPole);
		IEntitySource powerLineEntitySource;
		IEntity referenceEntity;
		array<ref ContainerIdPathEntry> containerPath;
		foreach (SCR_EPoleCableType cableType : commonCableTypes)
		{
			existingGroup = existingPoleGroups.Get(cableType);
			int existingPoleCableCount = existingGroup.m_aSlots.Count();	// cannot be zero as per GetClosestSlotGroups

			addedGroup = addedPoleGroups.Get(cableType);
			int addedPoleCableCount = addedGroup.m_aSlots.Count();			// cannot be zero as per GetClosestSlotGroups

			int cableCount;
			if (existingPoleCableCount < addedPoleCableCount)
				cableCount = existingPoleCableCount;
			else
				cableCount = addedPoleCableCount;

			startPoints.Clear();
			endPoints.Clear();
			for (int i; i < cableCount; ++i)
			{
				startPoints.Insert(existingPole.CoordToParent(existingGroup.m_aSlots[i].m_vPosition));
				endPoints.Insert(addedPole.CoordToParent(addedGroup.m_aSlots[i].m_vPosition));
			}

			if (m_DebugShapeManager) // warn visually about unused slots
			{
				for (int i = cableCount; i < existingPoleCableCount; ++i)
				{
					m_DebugShapeManager.AddSphere(existingPole.CoordToParent(existingGroup.m_aSlots[i].m_vPosition), DEBUG_SLOT_SIZE, DEBUG_SLOT_COLOUR_UNUSED, ShapeFlags.NOOUTLINE);
				}

				for (int i = cableCount; i < addedPoleCableCount; ++i)
				{
					m_DebugShapeManager.AddSphere(addedPole.CoordToParent(addedGroup.m_aSlots[i].m_vPosition), DEBUG_SLOT_SIZE, DEBUG_SLOT_COLOUR_UNUSED, ShapeFlags.NOOUTLINE);
				}
			}

			float normalLengthSq;
			float reversedLengthSq;
			foreach (int i, vector startPoint : startPoints)
			{
				normalLengthSq += vector.DistanceSq(startPoint, endPoints[i]);
				reversedLengthSq += vector.DistanceSq(startPoint, endPoints[cableCount - i - 1]);
			}

			// don't cross the streams!
			if (reversedLengthSq < normalLengthSq)
				SCR_ArrayHelperT<vector>.Reverse(endPoints);

			if (m_DebugShapeManager)
			{
				foreach (int i, vector startPoint : startPoints)
				{
					m_DebugShapeManager.AddSphere(startPoint, DEBUG_SLOT_SIZE, DEBUG_SLOT_COLOUR_POS, ShapeFlags.NOOUTLINE);
					m_DebugShapeManager.AddSphere(endPoints[i], DEBUG_SLOT_SIZE, DEBUG_SLOT_COLOUR_POS, ShapeFlags.NOOUTLINE);
					m_DebugShapeManager.AddLine(startPoint, endPoints[i], DEBUG_CABLE_COLOUR_POS);
				}
			}

			// power line preparation
			powerLineEntitySource = null;

			vector startPos, endPos;
			// Create cables for the entity

			int cableIndex;
			foreach (int i, vector startPoint : startPoints)
			{
				if (!powerLineEntitySource)
				{
					referenceEntity = worldEditorAPI.SourceToEntity(m_Source); // variable reuse
					startPos = referenceEntity.CoordToLocal(startPoint);
					endPos = referenceEntity.CoordToLocal(endPoints[i]);
				}
				else
				{
					referenceEntity = worldEditorAPI.SourceToEntity(powerLineEntitySource);
					startPos = referenceEntity.CoordToLocal(startPoint);
					endPos = referenceEntity.CoordToLocal(endPoints[i]);
				}

				if (vector.DistanceSqXZ(startPos, endPos) < MIN_CABLE_LENGTH_SQ)
				{
					Print("Cable's 2D length is too small! Skipping (" + vector.DistanceXZ(startPos, endPos).ToString(-1, 2) + "m < " + Math.Sqrt(MIN_CABLE_LENGTH_SQ) + "m)", LogLevel.WARNING);

					if (m_DebugShapeManager)
					{
						float localX = existingPole.CoordToLocal(startPoint)[0];
						vector forward = vector.Direction(startPoint, endPoints[i]);
						vector right = -{
							forward[0] * Math.Cos(Math.PI_HALF) - forward[2] * Math.Sin(Math.PI_HALF),
							0,													// Y rotation - 2D only
							forward[0] * Math.Sin(Math.PI_HALF) + forward[2] * Math.Cos(Math.PI_HALF),
						};

						if (localX > DEBUG_CABLE_LENGTH_POS_MAX_DIST)
							right *= DEBUG_CABLE_LENGTH_POS_MAX_DIST;
						else
							right *= localX;

						vector spherePos = (startPoint + endPoints[i]) * 0.5 + DEBUG_CABLE_LENGTH_POS + right;

						m_DebugShapeManager.AddSphere(spherePos, DEBUG_CABLE_LENGTH_SIZE, DEBUG_CABLE_LENGTH_COLOUR_ERROR_SPHERE, ShapeFlags.NOOUTLINE);
						m_DebugShapeManager.AddLine(spherePos, startPoint, DEBUG_CABLE_LENGTH_COLOUR_ERROR_LINE);
						m_DebugShapeManager.AddLine(spherePos, endPoints[i], DEBUG_CABLE_LENGTH_COLOUR_ERROR_LINE);
					}

					continue;
				}

				if (!powerLineEntitySource)
				{
					powerLineEntitySource = worldEditorAPI.CreateEntity(POWER_LINE_CLASS, string.Empty, 0, m_Source, referenceEntity.CoordToLocal(existingPoleWorldPos), vector.Zero);
					if (!powerLineEntitySource)
					{
						Print("Cannot create " + POWER_LINE_CLASS + " - CreateEntity returned null", LogLevel.ERROR);
						return null; // it would fail for the other ones too
					}

					// get the most adapted material
					ResourceName cableMaterial;
					foreach (SCR_PoleCable cable : m_aCables)
					{
						if (cable.m_eType == cableType && cable.m_sMaterial) // !.IsEmpty()
						{
							cableMaterial = cable.m_sMaterial;
							break;
						}
					}

					if (!cableMaterial)
						cableMaterial = m_PowerlineMaterial;

					if (cableMaterial)
						worldEditorAPI.SetVariableValue(powerLineEntitySource, null, "Material", cableMaterial);

					referenceEntity = worldEditorAPI.SourceToEntity(powerLineEntitySource);
					startPos = referenceEntity.CoordToLocal(startPoint);
					endPos = referenceEntity.CoordToLocal(endPoints[i]);
				}

				worldEditorAPI.CreateObjectArrayVariableMember(powerLineEntitySource, null, "Cables", "Cable", cableIndex);
				containerPath = { new ContainerIdPathEntry("Cables", cableIndex) };
				worldEditorAPI.SetVariableValue(powerLineEntitySource, containerPath, "StartPoint", startPos.ToString(false));
				worldEditorAPI.SetVariableValue(powerLineEntitySource, containerPath, "EndPoint", endPos.ToString(false));
				cableIndex++;
			}

			if (powerLineEntitySource)
				result.Insert(powerLineEntitySource);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] pole the pole from which to get cable slot groups
	//! \param[in] worldPos the world position from which get the closest groups
	//! \param[in] sameLine if poles are on the same lines or if it is a connection\n
	//! (used for backward compatibility with the slot system)
	//! \return cableType-slotGroup map of non-empty slot groups or null on all empty groups or error
	protected map<SCR_EPoleCableType, ref SCR_PoleCableSlotGroup> GetClosestSlotGroups(notnull SCR_PowerPole pole, vector worldPos, bool sameLine)
	{
		map<SCR_EPoleCableType, ref SCR_PoleCableSlotGroup> result = pole.GetClosestCableSlotGroupsPerCableType(worldPos);
		if (!result.IsEmpty())
			return result;

		Print("Pole uses the cable slot system at " + pole.GetOrigin(), LogLevel.WARNING);

		int slotCount = pole.GetSlotsCount(sameLine);
		if (pole.GetSlotsCount(sameLine) < 1)
			return null;

		SCR_PoleCableSlotGroup oldSlotsGroup = new SCR_PoleCableSlotGroup();
		oldSlotsGroup.m_eCableType = DEFAULT_CABLE_TYPE;
		oldSlotsGroup.m_aSlots = {};
		SCR_PoleCableSlot newSlot;
		vector slotPos;
		for (int i; i < slotCount; ++i)
		{
			slotPos = pole.TryGetSlot(i, worldPos, sameLine);
			if (slotPos == vector.Zero)
				continue;

			newSlot = new SCR_PoleCableSlot();
			newSlot.m_vPosition = pole.CoordToLocal(slotPos);
			oldSlotsGroup.m_aSlots.Insert(newSlot);
		}

		if (oldSlotsGroup.m_aSlots.IsEmpty())
			return null;

		result.Insert(oldSlotsGroup.m_eCableType, oldSlotsGroup);

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Get the first power pole, either the entity or one of its direct children
	//! \param[in] powerPoleEntitySource the power pole's entity source
	//! \return the casted power pole entity or null if not found
	protected static SCR_PowerPole GetPowerPoleFromEntitySource(notnull IEntitySource powerPoleEntitySource)
	{
		WorldEditorAPI worldEditorAPI = ((WorldEditor)Workbench.GetModule(WorldEditor)).GetApi();
		IEntity powerPoleEntity = worldEditorAPI.SourceToEntity(powerPoleEntitySource);
		SCR_PowerPole result = SCR_PowerPole.Cast(powerPoleEntity);
		if (result)
			return result;

		for (int i, childrenCount = powerPoleEntitySource.GetNumChildren(); i < childrenCount; i++)
		{
			result = SCR_PowerPole.Cast(worldEditorAPI.SourceToEntity(powerPoleEntitySource.GetChild(i)));
			if (result)
				return result;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Get all connected generators and store them in s_CurrentQueryGenerator
	//! \param[in] generator the generator to check
	//! \param[out] checkedGenerators an array of already-checked generators (if not provided, will be created and filled)
	protected static void QueryGenerators(notnull IEntitySource generator, out array<IEntitySource> checkedGenerators = null)
	{
		if (!s_aGenerators)
			return;

		WorldEditorAPI worldEditorAPI = ((WorldEditor)Workbench.GetModule(WorldEditor)).GetApi();
		if (!worldEditorAPI)
			return;

		IEntitySource shapeSource = generator.GetParent();
		if (!shapeSource)
			return;

		ShapeEntity shapeEntity = ShapeEntity.Cast(worldEditorAPI.SourceToEntity(shapeSource));
		if (!shapeEntity)
			return;

		if (!checkedGenerators)
			checkedGenerators = {};
		else if (checkedGenerators.Contains(generator))
			return;

		checkedGenerators.Insert(generator);

		BaseContainerList points = shapeSource.GetObjectArray("Points");

		array<vector> vectorPoints = GetPoints(shapeSource);
		SCR_AABB bbox = new SCR_AABB(vectorPoints);

		IEntity generatorEntity = worldEditorAPI.SourceToEntity(generator);
		if (!generatorEntity) // disabled, or anything else
		{
			Print("A Generator may have the Disabled flag - please check", LogLevel.WARNING);
			return;
		}

		BaseWorld world = generatorEntity.GetWorld();

		if (bbox.m_vMax[1] - bbox.m_vMin[1] < DEBUG_GENERATOR_MIN_HEIGHT)
		{
			float middle = (bbox.m_vMax[1] - bbox.m_vMin[1]) * 0.5;
			
			bbox.m_vMax[1] = middle + DEBUG_GENERATOR_MIN_HEIGHT * 0.5;
			bbox.m_vMin[1] = middle - DEBUG_GENERATOR_MIN_HEIGHT * 0.5;
		}

		bbox.m_vMin -= { GENERATOR_BBOX_TOLERANCE, GENERATOR_BBOX_TOLERANCE, GENERATOR_BBOX_TOLERANCE };
		bbox.m_vMax += { GENERATOR_BBOX_TOLERANCE, GENERATOR_BBOX_TOLERANCE, GENERATOR_BBOX_TOLERANCE };

		if (s_DebugShapeManager) // all triggered generators draw a box
			s_DebugShapeManager.AddBBox(generatorEntity.CoordToParent(bbox.m_vMin), generatorEntity.CoordToParent(bbox.m_vMax), DEBUG_GENERATOR_COLOUR, ShapeFlags.NOZWRITE | ShapeFlags.WIREFRAME);

		world.QueryEntitiesByAABB(generatorEntity.CoordToParent(bbox.m_vMin), generatorEntity.CoordToParent(bbox.m_vMax), QueryFilter);

		foreach (IEntitySource generator2 : s_aGenerators)
		{
			if (checkedGenerators.Contains(generator2))
				continue;

			s_CurrentQueryGenerator = generator2;
			QueryGenerators(generator2, checkedGenerators);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Filter used by QueryGenerators' QueryEntitiesByAABB call
	protected static bool QueryFilter(IEntity entity)
	{
		ShapeEntity shape = ShapeEntity.Cast(entity);
		if (!shape)
			return true;

		WorldEditorAPI worldEditorAPI = ((WorldEditor)Workbench.GetModule(WorldEditor)).GetApi();
		IEntitySource shapeSource = worldEditorAPI.EntityToSource(shape);
		if (!shapeSource)
			return true;

		IEntitySource otherGeneratorSource;
		SCR_PowerlineGeneratorEntity otherGeneratorEntity;
		SCR_PowerlineGeneratorEntity currentGeneratorEntity;
		for (int i = shapeSource.GetNumChildren() - 1; i >= 0; --i)
		{
			otherGeneratorSource = shapeSource.GetChild(i);
			otherGeneratorEntity = SCR_PowerlineGeneratorEntity.Cast(worldEditorAPI.SourceToEntity(otherGeneratorSource));
			if (!otherGeneratorEntity)
				continue;

			//Has to be refreshed here
			currentGeneratorEntity = SCR_PowerlineGeneratorEntity.Cast(worldEditorAPI.SourceToEntity(s_CurrentQueryGenerator));
			if (!currentGeneratorEntity)
				continue;

			if (otherGeneratorSource && otherGeneratorSource != s_CurrentQueryGenerator && !s_aGenerators.Contains(otherGeneratorSource) && currentGeneratorEntity.HasCommonPoint(otherGeneratorEntity))
			{
				s_aGenerators.Insert(otherGeneratorSource);
				break;
			}
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnShapeInitInternal(IEntitySource shapeEntitySrc, ShapeEntity shapeEntity)
	{
		super.OnShapeInitInternal(shapeEntitySrc, shapeEntity);

		GenerateGeneratorJunctions(m_Source, m_bDrawDebugShapes);
	}

	//------------------------------------------------------------------------------------------------
	override void OnShapeChangedInternal(IEntitySource shapeEntitySrc, ShapeEntity shapeEntity, array<vector> mins, array<vector> maxes)
	{
		super.OnShapeChangedInternal(shapeEntitySrc, shapeEntity, mins, maxes);

		// TODO handle this case better if needed, use the bbox arrays
		GenerateGeneratorJunctions(m_Source, m_bDrawDebugShapes);
	}
#endif // WORKBENCH

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] parent
	void SCR_PowerlineGeneratorEntity(IEntitySource src, IEntity parent)
	{
#ifdef WORKBENCH
		if (GetGame().InPlayMode())
			return;

		SetEventMask(EntityEvent.INIT);
#endif // WORKBENCH
	}
}
