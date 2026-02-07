[EntityEditorProps(category: "GameLib/Scripted/Generator", description: "Power Line Generator", dynamicBox: true, visible: false)]
class SCR_PowerlineGeneratorEntityClass : SCR_GeneratorBaseEntityClass
{
}

class SCR_PowerlineGeneratorEntity : SCR_GeneratorBaseEntity
{
	[Attribute(defvalue: "1", desc: "How far should the poles be from each other.", category: "Pole Setup")]
	protected float m_fDistance;

	[Attribute(params: "et", category: "Pole Setup")]
	protected ResourceName m_DefaultPole;

	[Attribute(params: "et", category: "Pole Setup")]
	protected ResourceName m_StartPole;

	[Attribute(params: "et", category: "Pole Setup")]
	protected ResourceName m_EndPole;

	[Attribute(params: "et", category: "Pole Setup")]
	protected ResourceName m_DefaultJunctionPole;

	[Attribute(defvalue: "0", category: "Pole Setup")]
	protected bool m_bRotate180DegreeYawStartPole;

	[Attribute(defvalue: "0", category: "Pole Setup")]
	protected bool m_bRotate180DegreeYawEndPole;

	[Attribute(defvalue: "0", desc: "Expected empty space around the poles line by other generators", params: "0 100 1", category: "Pole Setup")]
	protected float Clearance; //!< Used by Obstacle Detector - TODO: rename to m_fClearance

	[Attribute(defvalue: "0", desc: "Maximum random pitch angle (from Y-axis)", params: "0 180 1", category: "Randomisation")]
	protected float m_fRandomPitchAngle;

	[Attribute(defvalue: "1", desc: "Make the random pitch possible on both sides (-180..+180°) or, if disabled, one side only (0..180°)", category: "Randomisation")]
	protected bool m_bRandomPitchOnBothSides;

	[Attribute(defvalue: "0", desc: "Maximum random roll angle (from Y-axis)", params: "0 180 1", category: "Randomisation")]
	protected float m_fRandomRollAngle;

	[Attribute(defvalue: "1", desc: "Make the random roll possible on both sides (-180..+180°) or, if disabled, one side only (0..180°)", category: "Randomisation")]
	protected bool m_bRandomRollOnBothSides;

	[Attribute(defvalue: "0", desc: "Apply randomisation to default poles", category: "Randomisation")]
	protected bool m_bApplyPitchAndRollDefault;

	[Attribute(defvalue: "0", desc: "Apply randomisation to start pole", category: "Randomisation")]
	protected bool m_bApplyPitchAndRollStart;

	[Attribute(defvalue: "0", desc: "Apply randomisation to end pole", category: "Randomisation")]
	protected bool m_bApplyPitchAndRollEnd;

	[Attribute(uiwidget: UIWidgets.ResourceNamePicker, params: "emat", category: "Power Lines")]
	protected ResourceName m_PowerlineMaterial;

	[Attribute(defvalue: "0", category: "Debug")]
	protected bool m_bDrawDebugShapes;

	protected ref array<IEntity> m_aMyJunctionPoles = {};
	protected ref array<IEntity> m_aOtherJunctionPoles = {};

	protected ShapeEntity m_ParentShape;

	protected ref array<ref Shape> m_aDebugShapes = {};
	protected IEntitySource m_PreviousPowerPoleSource;

	// QUERY DATA
	protected static ref array<IEntitySource> s_aGenerators = {};
	protected static IEntitySource s_CurrentQueryGenerator;

	protected bool m_bLastJunctionWasSameLine = true;

	protected IEntitySource m_StartPoleEntitySource;

#ifdef WORKBENCH
	protected static WorldEditorAPI s_WorldEditorAPI;

	// #ifdef debug?
	// protected static ref array<ref Shape> s_aDebugShapes = {};

	protected static const float TOLERANCE_SQUARED = 0.01; // 0.1 * 0.1
	protected static const string POINT_DATA_CLASS = "SCR_PowerlineGeneratorPointData"; // TODO: use .IsInherited(SCR_PowerlineGeneratorPointData)

	//------------------------------------------------------------------------------------------------
	protected static void GenerateGeneratorJunctions(notnull IEntitySource generator)
	{
		if (!s_WorldEditorAPI || s_WorldEditorAPI.UndoOrRedoIsRestoring())
			return;

		if (!s_WorldEditorAPI.IsDoingEditAction())
		{
			s_WorldEditorAPI.BeginEntityAction();
			GenerateGeneratorJunctions(generator);
			s_WorldEditorAPI.EndEntityAction();
			return;
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
		foreach (IEntitySource generatorSource : s_aGenerators)
		{
			queriedGenerator = SCR_PowerlineGeneratorEntity.Cast(s_WorldEditorAPI.SourceToEntity(generatorSource));
			if (!queriedGenerator)
				continue;

			queriedGenerator.DeletePoles();
			queriedGenerator.GenerateJunctions();
		}

		SCR_PowerlineGeneratorEntity otherQueriedGenerator;
		foreach (IEntitySource generatorSource : s_aGenerators)
		{
			queriedGenerator = SCR_PowerlineGeneratorEntity.Cast(s_WorldEditorAPI.SourceToEntity(generatorSource));
			if (!queriedGenerator)
				continue;

			foreach (IEntitySource otherGeneratorSource : s_aGenerators)
			{
				if (generatorSource == otherGeneratorSource)
					continue;

				otherQueriedGenerator = SCR_PowerlineGeneratorEntity.Cast(s_WorldEditorAPI.SourceToEntity(otherGeneratorSource));
				if (!otherQueriedGenerator)
					continue;

				queriedGenerator.FindCommonJunctionsPoints(otherQueriedGenerator.m_aMyJunctionPoles);
			}

			queriedGenerator.GeneratePoles();
		}
	}

	//------------------------------------------------------------------------------------------------
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
	protected bool HasCommonPoint(SCR_PowerlineGeneratorEntity other)
	{
		IEntitySource sourceOther;
		sourceOther = s_WorldEditorAPI.EntityToSource(other);
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
	// Generate junctions, happening before generating poles
	protected void GenerateJunctions()
	{
		m_aMyJunctionPoles.Clear();

		IEntity shapeEntity = s_WorldEditorAPI.SourceToEntity(m_ParentShapeSource);
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
		super._WB_OnKeyChanged(src, key, ownerContainers, parent);

		if (key == "coords")
			return false;

		s_WorldEditorAPI = _WB_GetEditorAPI();
		if (!s_WorldEditorAPI)
			return false;

		IEntitySource thisSrc = s_WorldEditorAPI.EntityToSource(this);
		BaseContainerTools.WriteToInstance(this, thisSrc);

		m_ParentShape = ShapeEntity.Cast(parent);
		if (m_ParentShape)
			OnShapeInit(m_ParentShapeSource, m_ParentShape);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void GenerateJunctionOnPoint(notnull BaseContainer point, float yaw)
	{
		SCR_PowerlineGeneratorJunctionData junctionData;

		BaseContainerList dataArr = point.GetObjectArray("Data");
		BaseContainer data;
		for (int j = 0, dataCount = dataArr.Count(); j < dataCount; ++j)
		{
			data = dataArr.Get(j);
			if (data.GetClassName() == POINT_DATA_CLASS)
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

		IEntitySource poleSrc = s_WorldEditorAPI.CreateEntity(junctionResourceName, "", 0, m_Source, pointPosition, vector.Zero);
		if (!poleSrc)
			return;

		yaw += junctionData.m_fYawOffset;
		if (yaw > 360)
			yaw -= 360;

		s_WorldEditorAPI.SetVariableValue(poleSrc, null, "angleY", yaw.ToString());

		SCR_PowerPole powerPole = GetPowerPoleFromEntitySource(poleSrc);
		if (!powerPole)
			return;

		if (junctionData.m_bPowerSource)
		{
			IEntitySource powerPoleSrc = s_WorldEditorAPI.EntityToSource(powerPole);
			if (powerPoleSrc)
			{
				s_WorldEditorAPI.SetVariableValue(powerPoleSrc, null, "PowerSource", junctionData.m_bPowerSource.ToString(true));

				//Refresh powerpole pointer
				powerPole = GetPowerPoleFromEntitySource(poleSrc);
			}
		}

		m_aMyJunctionPoles.Insert(powerPole);
	}

	//------------------------------------------------------------------------------------------------
	protected void DeletePoles()
	{
		if (!s_WorldEditorAPI.IsDoingEditAction())
		{
			Print("DeletePoles was called while not in edit action", LogLevel.ERROR);
			return;
		}

		DeleteAllChildren();
		m_aMyJunctionPoles.Clear();
	}

	//------------------------------------------------------------------------------------------------
	// happens after having generated junctions
	protected void GeneratePoles()
	{
		if (!m_ParentShape)
		{
			Print("Parent Shape is null, cannot generate power poles", LogLevel.ERROR);
			return;
		}

		if (!m_bDrawDebugShapes && m_aDebugShapes)
			m_aDebugShapes.Clear();

		s_WorldEditorAPI = _WB_GetEditorAPI();
		if (!s_WorldEditorAPI)
			return;

		if (!s_WorldEditorAPI.IsDoingEditAction())
		{
			Print("GeneratePoles was called while not in edit action", LogLevel.ERROR);
			return;
		}

		array<vector> anchorPoints = {};
		array<vector> shapePoints = {};
		m_ParentShape.GetPointsPositions(anchorPoints);
		m_ParentShape.GenerateTesselatedShape(shapePoints);
		int shapePointsCount = shapePoints.Count();
		if (shapePointsCount < 2)
		{
			Print("Power Line Generator shape only has one point - no power line will be generated", LogLevel.WARNING);
			return;
		}

		BaseContainerList pointsSource = m_ParentShapeSource.GetObjectArray("Points");

		float pointDistance, poleOffset, currentDistance, yaw, nextPoleDistance;
		vector currentPoint, prevPoint, prevAnchorPoint;
		bool isAnchorPoint, spawnPerPoint, previousWasPerPoint;
		BaseContainerList dataArr;
		BaseContainer data;
		vector direction, lastPolePosition, nextPolePosition;
		vector parentPositionXZ = m_ParentShape.GetOrigin();
		float parentY = parentPositionXZ[1];
		parentPositionXZ[1] = 0;
		vector currAnchorPoint = anchorPoints[0];
		int currentAnchorIndex;
		int anchorCount = anchorPoints.Count();

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
						if (data.GetClassName() != POINT_DATA_CLASS)
							continue;

						data.Get("m_fPoleOffset", poleOffset);
						if (m_fDistance + poleOffset < 1)
						{
							Print("Pole offset is too small!", LogLevel.WARNING);
							poleOffset = 1 - m_fDistance;
						}

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

					lastPolePosition = nextPolePosition;
					GeneratePole(lastPolePosition, parentPositionXZ, parentY, yaw);
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

						lastPolePosition = nextPolePosition;
						GeneratePole(lastPolePosition, parentPositionXZ, parentY, yaw);
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
	protected void AttachJunction(notnull IEntity junction, bool sameLine)
	{
		IEntitySource junctionSource = s_WorldEditorAPI.EntityToSource(junction);
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
	protected void GenerateEndPole(vector currentPoint, float parentY, vector parentPositionXZ, float yaw)
	{
		if (m_EndPole.IsEmpty())
			return;

		vector endPolePos = currentPoint;
		bool sameLine;
		IEntitySource poleSource = s_WorldEditorAPI.EntityToSource(FindJunctionOnPoint(currentPoint, sameLine));

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

			s_WorldEditorAPI.SetVariableValue(poleSource, null, "angleY", yaw.ToString());
			if (m_bApplyPitchAndRollEnd)
				ApplyRandomPitchAndRoll(poleSource);
		}

		SCR_PowerPole endPole = GetPowerPoleFromEntitySource(poleSource);
		if (endPole)
			CreatePowerLines(GetPowerPoleFromEntitySource(m_PreviousPowerPoleSource), endPole, sameLine, m_bRotate180DegreeYawEndPole);

		if (m_bDrawDebugShapes)
		{
			Shape shape = Shape.CreateSphere(ARGB(255, 255, 0, 0), ShapeFlags.NOZBUFFER, endPolePos + parentPositionXZ, 1);
			m_aDebugShapes.Insert(shape);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void GenerateStartPole(vector relativePos, float parentY, vector parentPositionXZ, float yaw)
	{
		if (!s_WorldEditorAPI)
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
			poleSrc = s_WorldEditorAPI.EntityToSource(pole);
		}
		else
		{
			poleSrc = GeneratePoleAt(startPolePos, parentPositionXZ, parentY, m_StartPole);
			pole = s_WorldEditorAPI.SourceToEntity(poleSrc);
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

			s_WorldEditorAPI.SetVariableValue(poleSrc, null, "angleY", yaw.ToString());
			if (m_bApplyPitchAndRollStart)
				ApplyRandomPitchAndRoll(poleSrc);
		}

		m_PreviousPowerPoleSource = poleSrc;
		m_StartPoleEntitySource = poleSrc;

		if (m_bDrawDebugShapes)
		{
			Shape shape = Shape.CreateSphere(ARGB(255, 255, 0, 0), ShapeFlags.NOZBUFFER, startPolePos + parentPositionXZ, 1);
			m_aDebugShapes.Insert(shape);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected IEntitySource GeneratePoleAt(vector localPos, vector parentPositionXZ, float parentY, ResourceName poleResourceName)
	{
		float y;
		if (s_WorldEditorAPI.TryGetTerrainSurfaceY(localPos[0] + parentPositionXZ[0], localPos[2] + parentPositionXZ[2], y))
			localPos[1] = y - parentY;

		return s_WorldEditorAPI.CreateEntity(poleResourceName, string.Empty, 0, m_Source, localPos, vector.Zero);
	}

	//------------------------------------------------------------------------------------------------
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

		s_WorldEditorAPI.SetVariableValue(powerPole, null, "angleX", pitch.ToString());
		s_WorldEditorAPI.SetVariableValue(powerPole, null, "angleZ", roll.ToString());
	}

	//------------------------------------------------------------------------------------------------
	protected IEntitySource GeneratePole(vector lastPolePosition, vector parentPositionXZ, float parentY, float yaw, ResourceName customPoleResourceName = string.Empty)
	{
		float y;
		IEntitySource poleSource;
		if (s_WorldEditorAPI.TryGetTerrainSurfaceY(lastPolePosition[0] + parentPositionXZ[0], lastPolePosition[2] + parentPositionXZ[2], y))
			lastPolePosition[1] = y - parentY;

		ResourceName poleResourceName;
		if (customPoleResourceName.GetPath().IsEmpty())
			poleResourceName = m_DefaultPole; // already checked for empty string
		else
			poleResourceName = customPoleResourceName;

		poleSource = s_WorldEditorAPI.CreateEntity(poleResourceName, string.Empty, 0, m_Source, lastPolePosition, vector.Zero);
		if (!poleSource)
			return null;

		s_WorldEditorAPI.SetVariableValue(poleSource, null, "angleY", yaw.ToString());
		if (m_bApplyPitchAndRollDefault)
			ApplyRandomPitchAndRoll(poleSource);

		if (m_PreviousPowerPoleSource && poleSource)
		{
			SCR_PowerPole thisPowerPole = GetPowerPoleFromEntitySource(poleSource);
			if (thisPowerPole)
			{
				bool inverse = m_StartPoleEntitySource == m_PreviousPowerPoleSource && m_bRotate180DegreeYawStartPole;
				CreatePowerLines(GetPowerPoleFromEntitySource(m_PreviousPowerPoleSource), thisPowerPole, inverse: inverse);
			}
		}

		m_PreviousPowerPoleSource = poleSource;

		if (m_bDrawDebugShapes)
		{
			Shape shape = Shape.CreateSphere(ARGB(255, 255, 0, 0), ShapeFlags.NOZBUFFER, lastPolePosition + parentPositionXZ, 1);
			m_aDebugShapes.Insert(shape);
		}

		return poleSource;
	}

	//------------------------------------------------------------------------------------------------
	override void OnShapeChangedInternal(IEntitySource shapeEntitySrc, ShapeEntity shapeEntity, array<vector> mins, array<vector> maxes)
	{
		super.OnShapeChangedInternal(shapeEntitySrc, shapeEntity, mins, maxes);

		// TODO handle this case better if needed, use the bbox arrays
		m_ParentShape = shapeEntity;
		OnShapeInit(shapeEntitySrc, shapeEntity);
	}

	//------------------------------------------------------------------------------------------------
	//! Poles order matter - otherwise previous pole's B slots are ignored
	//! \param[in] existingPole
	//! \param[in] addedPole
	//! \param[in] sameLine
	//! \param[in] inverse is the added pole turned 180°
	//! \return the created power line entity source, or null on error
	protected IEntitySource CreatePowerLines(SCR_PowerPole existingPole, SCR_PowerPole addedPole, bool sameLine = true, bool inverse = false)
	{
		if (!existingPole || !addedPole || existingPole == addedPole)
			return null;

		if (SCR_JunctionPowerPole.Cast(existingPole) || SCR_JunctionPowerPole.Cast(addedPole))
			sameLine = m_bLastJunctionWasSameLine;

		int numCables = Math.Min(addedPole.GetSlotsCount(sameLine), existingPole.GetSlotsCount(sameLine));
		if (numCables < 1)
		{
			Print("0 slots found on one of the power poles, please check the setup of your power poles.", LogLevel.WARNING);
			return null;
		}

		// Gather slot positions for both poles in world coordinates
		array<vector> startPoints = {};
		array<vector> endPoints = {};
		vector startSlotPos;
		SCR_JunctionPowerPole junctionPowerPole;
		for (int i = 0; i < numCables; ++i)
		{
			startSlotPos = addedPole.CoordToParent(addedPole.GetSlot(i, sameLine));
			startPoints.Insert(startSlotPos);
			endPoints.Insert(existingPole.TryGetSlot(i, startSlotPos, sameLine));
		}

		// Create the PowerlineEntity
		IEntitySource powerlineSrc = s_WorldEditorAPI.CreateEntity("PowerlineEntity", string.Empty, 0, m_Source, CoordToLocal(startPoints[0]), vector.Zero);

		s_WorldEditorAPI.SetVariableValue(powerlineSrc, null, "Material", m_PowerlineMaterial);

		array<ref ContainerIdPathEntry> containerPath;
		vector startPos, endPos;
		// Create cables for the entity
		foreach (int i, vector startPoint : startPoints)
		{
			s_WorldEditorAPI.CreateObjectArrayVariableMember(powerlineSrc, null, "Cables", "Cable", i);

			containerPath = { new ContainerIdPathEntry("Cables", i) };

			// Add cable between each slot, convert to local coordinate space
			startPos = s_WorldEditorAPI.SourceToEntity(powerlineSrc).CoordToLocal(startPoint);
			s_WorldEditorAPI.SetVariableValue(powerlineSrc, containerPath, "StartPoint", startPos.ToString(false));

			IEntity powerline = s_WorldEditorAPI.SourceToEntity(powerlineSrc);

			if (inverse)
				endPos = powerline.CoordToLocal(endPoints[numCables - i - 1]);
			else
				endPos = powerline.CoordToLocal(endPoints[i]);

			s_WorldEditorAPI.SetVariableValue(powerlineSrc, containerPath, "EndPoint", endPos.ToString(false));
		}

		return powerlineSrc;
	}

	//------------------------------------------------------------------------------------------------
	//! Get the first power pole, either the entity or one of its direct children
	//! \param[in] powerPoleEntitySource the power pole's entity source
	//! \return the casted power pole entity or null if not found
	protected static SCR_PowerPole GetPowerPoleFromEntitySource(notnull IEntitySource powerPoleEntitySource)
	{
		IEntity powerPoleEntity = s_WorldEditorAPI.SourceToEntity(powerPoleEntitySource);
		SCR_PowerPole result = SCR_PowerPole.Cast(powerPoleEntity);
		if (result)
			return result;

		for (int i = 0, childrenCount = powerPoleEntitySource.GetNumChildren(); i < childrenCount; i++)
		{
			result = SCR_PowerPole.Cast(s_WorldEditorAPI.SourceToEntity(powerPoleEntitySource.GetChild(i)));
			if (result)
				return result;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] generator the generator to check
	//! \param[out] checkedGenerators an array of already-checked generators (if not provided, will be created and filled)
	protected static void QueryGenerators(notnull IEntitySource generator, out array<IEntitySource> checkedGenerators = null)
	{
		if (!s_WorldEditorAPI || !s_aGenerators)
			return;

		IEntitySource shapeSource = generator.GetParent();
		if (!shapeSource)
			return;

		IEntity shapeEntity = s_WorldEditorAPI.SourceToEntity(shapeSource);
		if (!shapeEntity)
			return;

		if (!checkedGenerators)
			checkedGenerators = {};
		else if (checkedGenerators.Contains(generator))
			return;

		checkedGenerators.Insert(generator);

		BaseContainerList points = shapeSource.GetObjectArray("Points");

		// Get bbox
		array<vector> vectorPoints = GetPoints(shapeSource);
		SCR_AABB bbox = new SCR_AABB(vectorPoints);

		// Query entities in bbox
		vector mat[4];
		shapeEntity.GetTransform(mat);

		IEntity generatorEntity = s_WorldEditorAPI.SourceToEntity(generator);
		BaseWorld world = generatorEntity.GetWorld();

		bbox.m_vMin[1] = -50;
		bbox.m_vMax[1] = 50;

		// #ifdef debug?
		//s_aDebugShapes.Insert(Shape.Create(ShapeType.BBOX, ARGB(255, Math.RandomFloat(0, 255), Math.RandomFloat(0, 255), Math.RandomFloat(0, 255)), ShapeFlags.NOZWRITE | ShapeFlags.WIREFRAME, generatorEntity.CoordToParent(bbox.m_vMin), generatorEntity.CoordToParent(bbox.m_vMax)));

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

		IEntitySource shapeSource = s_WorldEditorAPI.EntityToSource(shape);
		if (!shapeSource)
			return true;

		IEntitySource otherGeneratorSource;
		SCR_PowerlineGeneratorEntity otherGeneratorEntity;
		SCR_PowerlineGeneratorEntity currentGeneratorEntity;
		for (int i = shapeSource.GetNumChildren() - 1; i >= 0; --i)
		{
			otherGeneratorSource = shapeSource.GetChild(i);
			otherGeneratorEntity = SCR_PowerlineGeneratorEntity.Cast(s_WorldEditorAPI.SourceToEntity(otherGeneratorSource));
			if (!otherGeneratorEntity)
				continue;

			//Has to be refreshed here
			currentGeneratorEntity = SCR_PowerlineGeneratorEntity.Cast(s_WorldEditorAPI.SourceToEntity(s_CurrentQueryGenerator));
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
	//! Create a Point Data on each shape's anchor point if none is present
	protected void AddPointData(BaseContainerList points)
	{
		if (!s_WorldEditorAPI || s_WorldEditorAPI.UndoOrRedoIsRestoring())
			return;

		BaseContainerList dataArr;
		for (int i = points.Count() - 1; i >= 0; i--)
		{
			dataArr = points.Get(i).GetObjectArray("Data");
			int dataCount = dataArr.Count();
			bool hasPointData = false;
			for (int j = 0; j < dataCount; ++j)
			{
				if (dataArr.Get(j).GetClassName() == POINT_DATA_CLASS)
				{
					hasPointData = true;
					break;
				}
			}

			if (!hasPointData)
				s_WorldEditorAPI.CreateObjectArrayVariableMember(points[i], null, "Data", POINT_DATA_CLASS, dataCount);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnShapeInitInternal(IEntitySource shapeEntitySrc, ShapeEntity shapeEntity)
	{
		super.OnShapeInitInternal(shapeEntitySrc, shapeEntity);

		s_WorldEditorAPI = _WB_GetEditorAPI();

		BaseContainerList points = shapeEntitySrc.GetObjectArray("Points");
		if (!points)
			return;

		AddPointData(points);

		GenerateGeneratorJunctions(m_Source);
	}
#endif

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] parent
	void SCR_PowerlineGeneratorEntity(IEntitySource src, IEntity parent)
	{
#ifdef WORKBENCH
		if (GetGame().InPlayMode())
			return;

		m_ParentShape = ShapeEntity.Cast(parent);

		SetEventMask(EntityEvent.INIT);
#endif
	}
}

[BaseContainerProps()]
class SCR_PowerlineGeneratorJunctionData
{
	[Attribute(desc: "Junction prefab to be used", uiwidget: UIWidgets.ResourcePickerThumbnail, params: "et")]
	ResourceName m_sJunctionPrefab;

	// not a slider for performance reason (type the value directly)
	[Attribute(defvalue: "0", desc: "Set the junction's yaw offset; can be used to setup the prefab properly", /* uiwidget: UIWidgets.Slider, */params: "0 360")]
	float m_fYawOffset;

	[Attribute(defvalue: "0", desc: "Define whether or not this junction is a power source")]
	bool m_bPowerSource;
}

class SCR_PowerlineGeneratorPointData : ShapePointDataScriptBase
{
	[Attribute(desc: "Offset of the pole, in metres")]
	float m_fPoleOffset;

	[Attribute(defvalue: "0", desc: "Generate poles on anchor points until a point with this attribute unchecked is reached.")]
	bool m_bGeneratePerPoint;

	[Attribute()]
	ref SCR_PowerlineGeneratorJunctionData m_JunctionData;
}
