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
	protected float Clearance; //!< Used by Obstacle Detector

	/*
		Randomisation
	*/

	[Attribute(defvalue: "0", desc: "Maximum random yaw angle (×2, -value to +value) - only applied to default poles", params: "0 180 1", category: "Randomisation")]
	protected float m_fRandomYawAngle;

	[Attribute(defvalue: "0", desc: "Maximum random pitch angle (×2, -value to +value)", params: "0 180 1", category: "Randomisation")]
	protected float m_fRandomPitchAngle;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.None)] // obsolete since 2024-10-08
	protected bool m_bRandomPitchOnBothSides;

	[Attribute(defvalue: "0", desc: "Maximum random roll angle (×2, -value to +value)", params: "0 180 1", category: "Randomisation")]
	protected float m_fRandomRollAngle;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.None)] // obsolete since 2024-10-08
	protected bool m_bRandomRollOnBothSides;

	[Attribute(defvalue: "0", desc: "Apply pitch and roll randomisation to default poles", category: "Randomisation")]
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

	[Attribute(defvalue: "{613763D114E7CD1C}Prefabs/WEGenerators/Powerline.et", desc: "Cable Prefab", params: "et class=" + POWER_LINE_CLASS, category: "Cables")]
	protected ResourceName m_sPowerLinePrefab;

	[Attribute(defvalue: "{836210B285A81785}Assets/Structures/Infrastructure/Power/Powerlines/data/default_powerline_wire.emat", desc: "Default cable material if none above match", params: "emat", category: "Cables")]
	protected ResourceName m_PowerlineMaterial;

	/*
		Debug
	*/

	[Attribute(defvalue: "0", category: "Debug")]
	protected bool m_bDrawDebugShapes;

	protected static const string POWER_LINE_CLASS = "PowerlineEntity";

#ifdef WORKBENCH

	protected ref map<IEntitySource, IEntitySource> m_mJunctionEntitySources = new map<IEntitySource, IEntitySource>();			// the parent object - the power pole (can be the object)
	protected ref map<IEntitySource, IEntitySource> m_mOtherJunctionEntitySources = new map<IEntitySource, IEntitySource>();	// same, but for other shapes

	protected ref SCR_DebugShapeManager m_DebugShapeManager;
	protected IEntitySource m_PreviousPowerPoleSource;

	// QUERY DATA
	protected static ref array<IEntitySource> s_aGenerators = {};
	protected static IEntitySource s_CurrentQueryGenerator;

	protected static ref SCR_DebugShapeManager s_DebugShapeManager; // this one is static

	protected static const float TOLERANCE_SQUARED = 0.01; // 0.1 * 0.1
	protected static const float MIN_CABLE_LENGTH_SQ = 0.01; // 0.1 * 0.1
	protected static const float GENERATOR_BBOX_TOLERANCE = 5; // in metres

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

	protected static const int DEBUG_GENERATOR_COLOUR = Color.GREEN;							//!< generators's bounding box
	protected static const int DEBUG_GENERATOR_MIN_HEIGHT = 20; // plus bbox tolerance

	//------------------------------------------------------------------------------------------------
	//! Generates junctions for selected powerline generators, optionally drawing debug shapes, and generates poles for them.
	//! \param[in] generator Generates junctions for powerline generators in the world editor, also handles pole generation based on new placement system.
	//! \param[in] drawDebugShapes Determines whether debug shapes for generator junctions are drawn during execution.
	protected static void GenerateGeneratorJunctions(notnull IEntitySource generator, bool drawDebugShapes = false)
	{
		WorldEditorAPI worldEditorAPI = ((WorldEditor)Workbench.GetModule(WorldEditor)).GetApi(); // cannot use _WB_GetEditorAPI() in a static method
		if (!worldEditorAPI || worldEditorAPI.UndoOrRedoIsRestoring())
			return;

		if (!worldEditorAPI.IsDoingEditAction())
		{
			PrintFormat("[SCR_PowerlineGeneratorEntity.GenerateGeneratorJunctions] not in Edit action, leaving (" + __FILE__ + " L" + __LINE__ + ")", this, level: LogLevel.WARNING);
			return;
		}

		Debug.BeginTimeMeasure();

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

		SCR_PowerlineGeneratorEntity queriedGenerator;
		int generatorsCount = s_aGenerators.Count();
		Debug.BeginTimeMeasure();
		foreach (IEntitySource generatorSource : s_aGenerators)
		{
			queriedGenerator = SCR_PowerlineGeneratorEntity.Cast(worldEditorAPI.SourceToEntity(generatorSource));
			if (!queriedGenerator)
				continue;

			queriedGenerator.DeleteAllChildren();
			queriedGenerator.GenerateJunctions();
		}
		Debug.EndTimeMeasure("Previous poles deletion and Junctions generation");

		SCR_PowerlineGeneratorEntity otherQueriedGenerator;
		foreach (int i, IEntitySource generatorSource : s_aGenerators)
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

				queriedGenerator.FindCommonJunctionSourcesPointsXZ(otherQueriedGenerator.m_mJunctionEntitySources);
			}

			Debug.BeginTimeMeasure();
			queriedGenerator.GeneratePoles();
			Debug.EndTimeMeasure(string.Format("Pole Generation for generator %1/%2", i + 1, generatorsCount));
		}

		Debug.EndTimeMeasure("All Pole Regenerations");
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] other the other power line generator
	//! \return whether or not the two generators (current & provided) have a point in common (< 0.1 distance)
	protected bool HasCommonPointXZ(notnull SCR_PowerlineGeneratorEntity other)
	{
		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		IEntitySource sourceOther = worldEditorAPI.EntityToSource(other);
		if (!sourceOther)
			return false;

		IEntitySource shapeSourceOther;
		shapeSourceOther = sourceOther.GetParent();
		if (!m_ParentShapeSource || !shapeSourceOther)
			return false;

		array<vector> pointsThis = GetAnchorPoints(m_ParentShapeSource);
		array<vector> pointsOther = GetAnchorPoints(shapeSourceOther);

		foreach (vector pointThis : pointsThis)
		{
			foreach (vector pointOther : pointsOther)
			{
				if (vector.DistanceSqXZ(CoordToParent(pointThis), other.CoordToParent(pointOther)) < TOLERANCE_SQUARED)
					return true;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Fills m_mOtherJunctionEntitySources with the provided junction poles that are on the current shape's anchor points
	//! It prevents duplicates but does not filter out if the provided junction poles belong to the current generator
	//! \param[in] otherJunctionPoles
	protected void FindCommonJunctionSourcesPointsXZ(notnull map<IEntitySource, IEntitySource> otherJunctionSources)
	{
		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		array<vector> points = GetAnchorPoints(m_ParentShapeSource);
		foreach (IEntitySource objectSource, IEntitySource poleSource : otherJunctionSources)
		{
			foreach (vector point : points)
			{
				if (vector.DistanceSqXZ(CoordToParent(point), worldEditorAPI.SourceToEntity(objectSource).GetOrigin()) < TOLERANCE_SQUARED)
				{
					if (!m_mOtherJunctionEntitySources.Contains(objectSource))
						m_mOtherJunctionEntitySources.Insert(objectSource, poleSource);

					break;
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Generate junctions, happening before generating poles
	protected void GenerateJunctions()
	{
		m_mJunctionEntitySources.Clear();

		array<vector> anchorPoints = m_ShapeNextPointHelper.GetAnchorPoints();
		int anchorPointsCountMinus1 = anchorPoints.Count() - 1;

		if (anchorPointsCountMinus1 < 1)
			return;

		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();

		SCR_PowerlineGeneratorPointData pointData;
		SCR_PowerlineGeneratorJunctionData junctionData;
		IEntitySource objectSource;
		IEntitySource poleSource;
		map<int, ref ShapePointDataScriptBase> pointDataMap = GetFirstPointDataMap(SCR_PowerlineGeneratorPointData);
		foreach (int i, vector anchorPoint : anchorPoints)
		{
			pointData = SCR_PowerlineGeneratorPointData.Cast(pointDataMap.Get(i));
			if (!pointData)
				continue;

			junctionData = pointData.m_JunctionData;
			if (!junctionData)
				continue;

			ResourceName junctionResourceName = junctionData.m_sJunctionPrefab;
			if (junctionResourceName.IsEmpty())
			{
				if (m_DefaultJunctionPole.IsEmpty())
					continue;

				junctionResourceName = m_DefaultJunctionPole;
			}

			vector vectorDir;
			if (i == 0)
			{
				vectorDir = vector.Direction(anchorPoint, anchorPoints[1]);
				if (m_bRotate180DegreeYawStartPole)
					vectorDir = -vectorDir;
			}
			else if (i == anchorPointsCountMinus1)
			{
				vectorDir = vector.Direction(anchorPoints[i - 1], anchorPoint);
				if (m_bRotate180DegreeYawEndPole)
					vectorDir = -vectorDir;
			}
			else
			{
				vectorDir = vector.Direction(
					vector.Direction(anchorPoint, anchorPoints[i - 1]).Normalized(),
					vector.Direction(anchorPoint, anchorPoints[i + 1]).Normalized());
			}

			vectorDir = vectorDir.VectorToAngles(); // variable reuse
			vectorDir = { 0, Math.Repeat(vectorDir[0] + junctionData.m_fYawOffset, 360), 0 };

			if (junctionData.m_fYOffset)
				anchorPoint[1] = anchorPoint[1] + junctionData.m_fYOffset;

			objectSource = worldEditorAPI.CreateEntity(junctionResourceName, string.Empty, m_iSourceLayerID, m_Source, anchorPoint, vectorDir);
			if (!objectSource)
				continue;

			poleSource = GetPowerPoleSourceFromEntitySource(objectSource);
			if (!poleSource)
			{
				worldEditorAPI.DeleteEntity(objectSource);
				continue;
			}

			if (junctionData.m_bApplyPitchAndRollRandomisation)
			{
				if (m_fRandomPitchAngle != 0)	// pitch
				{
					float angle = m_RandomGenerator.RandFloatXY(-m_fRandomPitchAngle, m_fRandomPitchAngle);
					if (angle != 0)
						worldEditorAPI.SetVariableValue(objectSource, null, "angleX", angle.ToString());
				}

				if (m_fRandomRollAngle != 0)	// roll
				{
					float angle = m_RandomGenerator.RandFloatXY(-m_fRandomRollAngle, m_fRandomRollAngle);
					if (angle != 0)
						worldEditorAPI.SetVariableValue(objectSource, null, "angleZ", angle.ToString());
				}
			}

			if (junctionData.m_bPowerSource)
				worldEditorAPI.SetVariableValue(poleSource, null, "PowerSource", "1");

			m_mJunctionEntitySources.Insert(objectSource, poleSource);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected override bool _WB_OnKeyChanged(BaseContainer src, string key, BaseContainerList ownerContainers, IEntity parent)
	{
		super._WB_OnKeyChanged(src, key, ownerContainers, parent);

		if (key == "coords")
			return false;

		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		if (!worldEditorAPI || worldEditorAPI.UndoOrRedoIsRestoring())
			return false;

		BaseContainerTools.WriteToInstance(this, worldEditorAPI.EntityToSource(this));
		if (m_ParentShapeSource)
			OnShapeInit(m_ParentShapeSource, ShapeEntity.Cast(worldEditorAPI.SourceToEntity(m_ParentShapeSource)));
		// TODO instead of OnShapeInit
//		if (m_ParentShapeSource)
//			GenerateGeneratorJunctions(src, m_bDrawDebugShapes);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Generates power poles position first then creates poles and cables if valid
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

		ShapeEntity parentShapeEntity = ShapeEntity.Cast(worldEditorAPI.SourceToEntity(m_ParentShapeSource));
		if (!parentShapeEntity) // wat
		{
			Print("Parent Shape is null, cannot generate power poles", LogLevel.ERROR);
			return;
		}

		array<vector> anchorPointsRelPos = m_ShapeNextPointHelper.GetAnchorPoints();
		if (anchorPointsRelPos.Count() < 2)
		{
			Print("Not enough shape points", LogLevel.ERROR);
			return;
		}

		// ANCHORS DATA COLLECTION
		map<int, ref ShapePointDataScriptBase> pointsData = GetFirstPointDataMap(SCR_PowerlineGeneratorPointData);

		// POLES'S POSITION
		array<vector> polePointsRelPos = {};
		map<int, ref SCR_PowerlineGeneratorPointData> pointDataMap = new map<int, ref SCR_PowerlineGeneratorPointData>();
		GetPolesRelPosition(pointsData, polePointsRelPos, pointDataMap);
		if (!polePointsRelPos)
		{
			Print("Power Line Generator shape is invalid; no poles will be generated", LogLevel.ERROR);
			return;
		}

		if (s_DebugShapeManager)
		{
			array<vector> absolutePoints = {};
			absolutePoints.Reserve(polePointsRelPos.Count());
			foreach (vector relPoint : polePointsRelPos)
			{
				absolutePoints.Insert(CoordToParent(relPoint));
			}

			CreateStaticDebugShapes(absolutePoints);
		}

		// POLES (and cables) CREATION
		CreatePolesAndCables(polePointsRelPos, anchorPointsRelPos, pointDataMap);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] absolutePoints
	protected static void CreateStaticDebugShapes(notnull array<vector> absolutePoints)
	{
		if (!s_DebugShapeManager)
			return;

		vector prevPoint;
		int countMinusOne = absolutePoints.Count() - 1;
		foreach (int i, vector currPoint : absolutePoints)
		{
			// s_DebugShapeManager.AddSphere(currPoint, 1, Color.VIOLET, ShapeFlags.NOOUTLINE);

			if (i == 0)				// start point
				s_DebugShapeManager.AddSphere(currPoint + 3 * vector.Up, 2, Color.DARK_GREEN, ShapeFlags.NOOUTLINE);
			else
			if (i == countMinusOne)	// end point (not present if start == end, but there is a minimum of two points anyway)
				s_DebugShapeManager.AddSphere(currPoint + 3 * vector.Up, 2, Color.DARK_RED, ShapeFlags.NOOUTLINE);
			else					// normal point
				s_DebugShapeManager.AddLine(prevPoint + vector.Up, currPoint + vector.Up);

			prevPoint = currPoint;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Generates poles relative positions for powerline generator based on anchor points, tesselated points and powerline data
	//! \param[in] pointsData
	//! \param[out] polePointsRelPos pole positions
	//! \param[out] pointDataMap map of <pole point index, pointData> (unrelated to position, only index)
	//! \return true on success, false on failure (no shape helper, etc)
	protected bool GetPolesRelPosition(
		notnull map<int, ref ShapePointDataScriptBase> pointsData,
		notnull out array<vector> polePointsRelPos,
		notnull out map<int, ref SCR_PowerlineGeneratorPointData> pointDataMap)
	{
		if (!m_ShapeNextPointHelper || !m_ShapeNextPointHelper.IsValid())
		{
			Print("[SCR_PowerlineGeneratorEntity.GetPolesRelPosition] invalid shapeNextPointHelper (" + __FILE__ + " L" + __LINE__ + ")", LogLevel.ERROR);
			return false;
		}

		if (m_fDistance < POLE_DISTANCE_TOLERANCE)
			m_fDistance = POLE_DISTANCE_TOLERANCE;

		const float distanceSq = m_fDistance * m_fDistance;

		float poleOffset;
		bool generatePerPoint;
		vector lastPoleRelPos;
		IEntity junction;
		SCR_PowerlineGeneratorPointData pointData;
		array<vector> anchorPointsRelPos = m_ShapeNextPointHelper.GetAnchorPoints();
		int anchorPointsCountMinus1 = anchorPointsRelPos.Count() - 1;
		foreach (int i, vector anchorPointRelPos : anchorPointsRelPos)
		{
			bool isStart = i == 0;
			bool isEnd = i == anchorPointsCountMinus1;
			bool hasJunction;

			bool wasGeneratedPerPoint = generatePerPoint;
			float previousOffset = poleOffset;

			pointData = SCR_PowerlineGeneratorPointData.Cast(pointsData.Get(i));
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
				if (previousOffset != 0 && m_ShapeNextPointHelper.GetNextPoint(m_fDistance + previousOffset, lastPoleRelPos, i, xzMode: m_bNextPointMeasurementXZ))
					polePointsRelPos.Insert(lastPoleRelPos);

				// straight to the current anchor point!
				while (m_ShapeNextPointHelper.GetNextPoint(m_fDistance, lastPoleRelPos, i, xzMode: m_bNextPointMeasurementXZ))
				{
					polePointsRelPos.Insert(lastPoleRelPos);
				}
			}

			if (wasGeneratedPerPoint || generatePerPoint || hasJunction || isEnd || isStart || vector.DistanceSq(lastPoleRelPos, anchorPointRelPos) == 0)
			{
				int pointsCount = polePointsRelPos.Count();
				if (hasJunction)
				{
					if (pointData)
						pointDataMap.Insert(pointsCount, pointData);
				}
				else
				{
					if (pointsCount > 0 && vector.DistanceSq(lastPoleRelPos, anchorPointRelPos) < POLE_DISTANCE_TOLERANCE)
						polePointsRelPos.Remove(pointsCount - 1);
				}

				polePointsRelPos.Insert(anchorPointRelPos);

				m_ShapeNextPointHelper.SetOnAnchor(i);
				lastPoleRelPos = anchorPointRelPos;
			}
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Creates poles and cables between given points with optional random pitch and roll, snaps to terrain if no junction found
	//! \param[in] polePointsRelpos
	//! \param[in] anchorPointsRelPos
	//! \param[in] pointsData
	protected void CreatePolesAndCables(notnull array<vector> polePointsRelPos, notnull array<vector> anchorPointsRelPos, notnull map<int, ref SCR_PowerlineGeneratorPointData> pointsData)
	{
		bool snapToGround = m_bSnapOffsetShapeToTheGround && !m_bSculptTerrain;
		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		int poleWorldPointsCountMinus1 = polePointsRelPos.Count() - 1;
		vector prevPoleRelPoint;
		ResourceName prefab;
		vector vectorDir; // no need for it to ever be normalised as it is .ToYaw()'d

		IEntitySource prevPoleObject;
		IEntitySource currPoleObject;

		IEntitySource prevPoleSource;
		IEntitySource currPoleSource;

		SCR_PowerlineGeneratorPointData pointData;
		foreach (int i, vector currPoleRelPoint : polePointsRelPos)
		{
			bool applyRandomPitchAndRoll;
			bool isStart = i == 0;
			bool isEnd = i == poleWorldPointsCountMinus1;
			bool isSameLine = true;
			pointData = pointsData.Get(i);

			if (isStart || isEnd || pointData != null)	// anchor: start, end or potential junction
			{
				// one must look for junction on all points
				// as another generator could make a junction here
				currPoleObject = FindJunctionSourceOnPointXZ(currPoleRelPoint, isSameLine); // isSameLine is set here (out bool)
				if (currPoleObject) // found junction
				{
					if (isSameLine)
						currPoleSource = m_mJunctionEntitySources.Get(currPoleObject);
					else
						currPoleSource = m_mOtherJunctionEntitySources.Get(currPoleObject);

					if (m_DebugShapeManager)
						m_DebugShapeManager.AddSphere(CoordToParent(currPoleRelPoint), 2);

					applyRandomPitchAndRoll = isSameLine;

					// cannot set junction's yaw here
					// as other lines joining before this line is initialised would not match the new junction's direction
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
			}
			else																					// normal pole
			{
				vectorDir = vector.Direction(
					vector.Direction(currPoleRelPoint, prevPoleRelPoint).Normalized(),
					vector.Direction(currPoleRelPoint, polePointsRelPos[i + 1]).Normalized());

				prefab = m_DefaultPole;
				applyRandomPitchAndRoll = m_bApplyPitchAndRollDefault;
			}

			if (!currPoleObject)
			{
				if (snapToGround)
				{
					vector worldPos = CoordToParent(currPoleRelPoint);
					worldPos[1] = worldEditorAPI.GetTerrainSurfaceY(worldPos[0], worldPos[2]);
					currPoleRelPoint = CoordToLocal(worldPos);
				}

				currPoleObject = worldEditorAPI.CreateEntity(prefab, string.Empty, m_iSourceLayerID, m_Source, currPoleRelPoint, vector.Zero);
				currPoleSource = GetPowerPoleSourceFromEntitySource(currPoleObject);

				float yaw = vectorDir.ToYaw();

				if (m_fRandomYawAngle != 0)
					yaw += m_RandomGenerator.RandFloatXY(-m_fRandomYawAngle, m_fRandomYawAngle);

				if (applyRandomPitchAndRoll)
				{
					if (m_fRandomPitchAngle != 0)
					{
						float pitch = m_RandomGenerator.RandFloatXY(-m_fRandomPitchAngle, m_fRandomPitchAngle);
						if (pitch != 0)
							worldEditorAPI.SetVariableValue(currPoleObject, null, "angleX", pitch.ToString());
					}

					if (m_fRandomRollAngle != 0)
					{
						float roll = m_RandomGenerator.RandFloatXY(-m_fRandomRollAngle, m_fRandomRollAngle);
						if (roll != 0)
							worldEditorAPI.SetVariableValue(currPoleObject, null, "angleZ", roll.ToString());
					}
				}

				if (yaw != 0)
					worldEditorAPI.SetVariableValue(currPoleObject, null, "angleY", yaw.ToString());

				if (m_DebugShapeManager)
				{
					m_DebugShapeManager.AddSphere(CoordToParent(currPoleRelPoint), DEBUG_POLE_SIZE, DEBUG_POLE_COLOUR_POS, ShapeFlags.NOOUTLINE);
					m_DebugShapeManager.AddArrow(CoordToParent(currPoleRelPoint), CoordToParent(currPoleRelPoint + (3 * { Math.Sin(vectorDir.ToYaw() * Math.DEG2RAD), 0, Math.Cos(vectorDir.ToYaw() * Math.DEG2RAD) })));
				}
			}

			if (i > 0)
			{
				CreatePowerLines(
					SCR_PowerPole.Cast(worldEditorAPI.SourceToEntity(prevPoleSource)),
					SCR_PowerPole.Cast(worldEditorAPI.SourceToEntity(currPoleSource)),
					isSameLine);
			}

			prevPoleObject = currPoleObject;
			prevPoleSource = currPoleSource;
			prevPoleRelPoint = currPoleRelPoint;
			currPoleObject = null;
			currPoleSource = null;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Get the junction holder present on the provided point
	//! \param[in] point the relative position to check
	//! \param[out] sameLine whether the found junction is one of the current generator's junction points
	//! \return true on success, false on failure
	protected IEntitySource FindJunctionSourceOnPointXZ(vector point, out bool sameLine)
	{
		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		if (!worldEditorAPI)
			return null;

		IEntity entity;

		foreach (IEntitySource junctionObject, IEntitySource junctionPole : m_mJunctionEntitySources)
		{
			entity = worldEditorAPI.SourceToEntity(junctionObject);
			if (!entity)
			{
				Print("[SCR_PowerlineGeneratorEntity.FindJunctionSourceOnPointXZ] null entity (own junction) (" + __FILE__ + " L" + __LINE__ + ")", LogLevel.WARNING);
				continue;
			}

			if (vector.DistanceSqXZ(point, CoordToLocal(entity.GetOrigin())) < TOLERANCE_SQUARED)
			{
				sameLine = true;
				return junctionObject;
			}
		}

		foreach (IEntitySource junctionObject, IEntitySource junctionPole : m_mOtherJunctionEntitySources)
		{
			entity = worldEditorAPI.SourceToEntity(junctionObject);
			if (!entity)
			{
				Print("[SCR_PowerlineGeneratorEntity.FindJunctionSourceOnPointXZ] null entity (others' junction) (" + __FILE__ + " L" + __LINE__ + ")", LogLevel.WARNING);
				continue;
			}

			if (vector.DistanceSqXZ(point, CoordToLocal(entity.GetOrigin())) < TOLERANCE_SQUARED)
			{
				sameLine = false;
				return junctionObject;
			}
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Creates power lines between two power poles based on their cable types, if they have compatible slots
	//! \param[in] existingPole existing pole represents the current power pole in the scene
	//! \param[in] addedPole added pole represents the new pole being added to the power line
	//! \param[in] isSameLine true if poles are on the same line, false if a junction is connecting another line
	//! \return the created power line entity sources, or null array on error / no cables
	// TODO: fix cable bug on rotated shape
	protected array<IEntitySource> CreatePowerLines(SCR_PowerPole existingPole, SCR_PowerPole addedPole, bool isSameLine)
	{
		if (!existingPole || !addedPole || existingPole == addedPole)
			return null;

		vector existingPoleWorldPos = existingPole.GetOrigin();
		vector addedPoleWorldPos = addedPole.GetOrigin();

		map<SCR_EPoleCableType, ref SCR_PoleCableSlotGroup> existingPoleGroups = existingPole.GetClosestCableSlotGroupsForEachCableType(addedPoleWorldPos, isSameLine);
		if (existingPoleGroups.IsEmpty())
		{
			Print("No valid slots found on the existing power pole, please check the setup of your power poles.", LogLevel.WARNING);

			if (m_DebugShapeManager)
				m_DebugShapeManager.AddSphere(existingPoleWorldPos + DEBUG_POLE_POS, DEBUG_POLE_SIZE, DEBUG_POLE_COLOUR_ERROR, ShapeFlags.NOOUTLINE);

			return null;
		}

		map<SCR_EPoleCableType, ref SCR_PoleCableSlotGroup> addedPoleGroups = addedPole.GetClosestCableSlotGroupsForEachCableType(existingPoleWorldPos, isSameLine);
		if (addedPoleGroups.IsEmpty())
		{
			Print("No valid slots found on the new power pole, please check the setup of your power poles.", LogLevel.WARNING);

			if (m_DebugShapeManager)
				m_DebugShapeManager.AddSphere(addedPoleWorldPos + DEBUG_POLE_POS, DEBUG_POLE_SIZE, DEBUG_POLE_COLOUR_ERROR, ShapeFlags.NOOUTLINE);

			return null;
		}

		array<SCR_EPoleCableType> cableTypes = {};
		if (SCR_Enum.GetEnumValues(SCR_EPoleCableType, cableTypes) < 1) // like, wat
			return null;

		array<SCR_EPoleCableType> cableTypesInCommon = {};
		foreach (SCR_EPoleCableType cableType : cableTypes)
		{
			if (existingPoleGroups.Contains(cableType) && addedPoleGroups.Contains(cableType))
				cableTypesInCommon.Insert(cableType);

			// code below could be done here directly, but for clarity's sake, one foreach won't kill perfs
		}

		if (cableTypesInCommon.IsEmpty())
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
		IEntitySource powerLineEntitySource;
		IEntity referenceEntity;
		array<ref ContainerIdPathEntry> containerPath;
		foreach (SCR_EPoleCableType cableType : cableTypesInCommon)
		{
			existingGroup = existingPoleGroups.Get(cableType);
			int existingPoleCableCount = existingGroup.m_aSlots.Count();	// cannot be zero

			addedGroup = addedPoleGroups.Get(cableType);
			int addedPoleCableCount = addedGroup.m_aSlots.Count();			// cannot be zero

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

			string powerLineEntity;
			if (m_sPowerLinePrefab) // !.IsEmpty()
				powerLineEntity = m_sPowerLinePrefab;
			else
				powerLineEntity = POWER_LINE_CLASS;

			vector startPos, endPos;
			// Create cables for the entity

			int cableIndex;
			foreach (int i, vector startPoint : startPoints)
			{
				if (powerLineEntitySource)
				{
					referenceEntity = worldEditorAPI.SourceToEntity(powerLineEntitySource);
					startPos = referenceEntity.CoordToLocal(startPoint);
					endPos = referenceEntity.CoordToLocal(endPoints[i]);
				}
				else
				{
					referenceEntity = worldEditorAPI.SourceToEntity(m_Source);
					startPos = referenceEntity.CoordToLocal(startPoint);
					endPos = referenceEntity.CoordToLocal(endPoints[i]);
				}

				if (vector.DistanceSqXZ(startPos, endPos) < MIN_CABLE_LENGTH_SQ)
				{
					Print("Cable's 2D length is too small! Skipping (" + vector.DistanceXZ(startPos, endPos).ToString(-1, 2) + "m < " + Math.Sqrt(MIN_CABLE_LENGTH_SQ) + "m)", LogLevel.WARNING);

					if (m_DebugShapeManager)
					{
						float localX = existingPole.CoordToLocal(startPoint)[0];
						vector forward = vector.Direction(startPoint, endPoints[i]); // not normalised
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
					powerLineEntitySource = worldEditorAPI.CreateEntity(powerLineEntity, string.Empty, m_iSourceLayerID, m_Source, referenceEntity.CoordToLocal(existingPoleWorldPos), vector.Zero);
					if (!powerLineEntitySource)
					{
						Print("CreateEntity returned null trying to create " + powerLineEntity, LogLevel.ERROR);
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
						cableMaterial = m_PowerlineMaterial; // default value

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
	//! Get the first power pole, either the entity or one of its direct children
	//! \param[in] powerPoleEntitySource the power pole's entity source
	//! \return the source for the power pole itself or null if not found
	protected static IEntitySource GetPowerPoleSourceFromEntitySource(notnull IEntitySource powerPoleEntitySource)
	{
		WorldEditorAPI worldEditorAPI = ((WorldEditor)Workbench.GetModule(WorldEditor)).GetApi();
		if (powerPoleEntitySource.GetClassName().ToType().IsInherited(SCR_PowerPole))
			return powerPoleEntitySource;

		for (int i, childrenCount = powerPoleEntitySource.GetNumChildren(); i < childrenCount; i++)
		{
			if (powerPoleEntitySource.GetChild(i).GetClassName().ToType().IsInherited(SCR_PowerPole))
				return powerPoleEntitySource.GetChild(i);
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

		array<vector> vectorPoints = GetAnchorPoints(shapeSource);
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
	//! Filter used by QueryGenerators' QueryEntitiesByAABB call (QueryEntitiesCallback signature)
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
			if (!otherGeneratorSource)
				continue;

			otherGeneratorEntity = SCR_PowerlineGeneratorEntity.Cast(worldEditorAPI.SourceToEntity(otherGeneratorSource));
			if (!otherGeneratorEntity)
				continue;

			currentGeneratorEntity = SCR_PowerlineGeneratorEntity.Cast(worldEditorAPI.SourceToEntity(s_CurrentQueryGenerator));
			if (!currentGeneratorEntity)
				continue;

			if (otherGeneratorSource != s_CurrentQueryGenerator
				&& !s_aGenerators.Contains(otherGeneratorSource)
				&& currentGeneratorEntity.HasCommonPointXZ(otherGeneratorEntity))
			{
				s_aGenerators.Insert(otherGeneratorSource);
				break;
			}
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnShapeInitInternal(IEntitySource shapeEntitySrc, ShapeEntity shapeEntity)
	{
		super.OnShapeInitInternal(shapeEntitySrc, shapeEntity);

		if (!m_bEnableGeneration)
		{
			Print("Power line generation is disabled for this shape (beware of junction issues with other lines) - tick it back on before saving", LogLevel.NORMAL);
			return;
		}

		m_RandomGenerator.SetSeed(m_iSeed);

		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		if (!worldEditorAPI || worldEditorAPI.UndoOrRedoIsRestoring())
			return;

		bool manageEntityAction = !worldEditorAPI.IsDoingEditAction();
		if (manageEntityAction)
			worldEditorAPI.BeginEntityAction();

		GenerateGeneratorJunctions(m_Source, m_bDrawDebugShapes);

		if (manageEntityAction)
			worldEditorAPI.EndEntityAction();
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnShapeChangedInternal(IEntitySource shapeEntitySrc, ShapeEntity shapeEntity, array<vector> mins, array<vector> maxes)
	{
		super.OnShapeChangedInternal(shapeEntitySrc, shapeEntity, mins, maxes);

		if (!m_bEnableGeneration)
		{
			Print("Power line generation is disabled for this shape (beware of junction issues with other lines) - tick it back on before saving", LogLevel.NORMAL);
			return;
		}

		m_RandomGenerator.SetSeed(m_iSeed);

		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		if (!worldEditorAPI || worldEditorAPI.UndoOrRedoIsRestoring())
			return;

		bool manageEntityAction = !worldEditorAPI.IsDoingEditAction();
		if (manageEntityAction)
			worldEditorAPI.BeginEntityAction();

		GenerateGeneratorJunctions(m_Source, m_bDrawDebugShapes); // TODO handle this case better if needed, use the bbox arrays

		if (manageEntityAction)
			worldEditorAPI.EndEntityAction();
	}
#endif // WORKBENCH
}
