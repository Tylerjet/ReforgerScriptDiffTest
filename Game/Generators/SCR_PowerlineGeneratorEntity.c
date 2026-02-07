//-----------------------------------------------------------------------
[EntityEditorProps(category: "GameLib/Scripted/Generator", description: "PowerlineGeneratorEntity", dynamicBox: true, visible: false)]
class SCR_PowerlineGeneratorEntityClass: SCR_GeneratorBaseEntityClass
{
};

//-----------------------------------------------------------------------
enum SCR_PowerlineGeneratorType
{
	SMALL,
	MIDDLE
};

//-----------------------------------------------------------------------
class SCR_Line : Managed
{
	vector m_vPoint1;
	vector m_vPoint2;
};

//-----------------------------------------------------------------------
[BaseContainerProps()]
class SCR_PowerlineGeneratorJunctionData
{
	[Attribute()]
	protected ResourceName m_JunctionPrefab;

	[Attribute()]
	protected bool m_bRotate180Degrees;

	bool GetRotate180Degrees()
	{
		return m_bRotate180Degrees;
	}

	ResourceName GetJunctionResourceName()
	{
		return m_JunctionPrefab;
	}
};

//-----------------------------------------------------------------------
class SCR_PowerlineGeneratorPointData : ShapePointDataScriptBase
{
	[Attribute()]
	float m_fPoleOffset;

	[Attribute("0", "Starts generating poles on points, until a point with this attribute unchecked is reached.")]
	bool m_bGeneratePerPoint;

	[Attribute()]
	ref SCR_PowerlineGeneratorJunctionData m_JunctionData;
};

//-----------------------------------------------------------------------
class SCR_PowerlineGeneratorEntity : SCR_GeneratorBaseEntity
{
	[Attribute(defvalue: "1", desc: "How far should the poles be from each other.", category: "Pole setup")]
	float m_fDistance;

	[Attribute(params: "et", category: "Pole setup")]
	ResourceName m_DefaultPole;

	[Attribute(params: "et", category: "Pole setup")]
	ResourceName m_StartPole;

	[Attribute(params: "et", category: "Pole setup")]
	ResourceName m_EndPole;

	[Attribute(params: "et", category: "Pole setup")]
	ResourceName m_DefaultJunctionPole;

	[Attribute(defvalue: "false", category: "Pole setup")]
	bool m_bRotate180DegreeYawStartPole;

	[Attribute(defvalue: "false", category: "Pole setup")]
	bool m_bRotate180DegreeYawEndPole;

	[Attribute(defvalue: "0", category: "Randomisation", desc: "Maximum random pitch angle (from Y-axis)", params: "0 180 1")]
	float m_fRandomPitchAngle;

	[Attribute(defvalue: "true", category: "Randomisation", desc: "Make the random pitch possible on both sides (-180..+180°) or, if disabled, one side only (0..180°)")]
	bool m_bRandomPitchOnBothSides;

	[Attribute(defvalue: "0", category: "Randomisation", desc: "Maximum random roll angle (from Y-axis)", params: "0 180 1")]
	float m_fRandomRollAngle;

	[Attribute(defvalue: "true", category: "Randomisation", desc: "Make the random roll possible on both sides (-180..+180°) or, if disabled, one side only (0..180°)")]
	bool m_bRandomRollOnBothSides;

	[Attribute(defvalue: "false", category: "Randomisation", desc: "Apply randomisation to default poles")]
	bool m_bApplyPitchAndRollDefault;

	[Attribute(defvalue: "false", category: "Randomisation", desc: "Apply randomisation to start pole")]
	bool m_bApplyPitchAndRollStart;

	[Attribute(defvalue: "false", category: "Randomisation", desc: "Apply randomisation to end pole")]
	bool m_bApplyPitchAndRollEnd;

	[Attribute(category: "Powerlines", params: "emat", uiwidget: UIWidgets.ResourceNamePicker)]
	ResourceName m_PowerlineMaterial;

	[Attribute(defvalue: "false", category: "Debug")]
	bool m_bDrawDebugShapes;

	// Using Clearance not m_fClearance for consistency with other generators (c++)
	// REQUIRED, do not remove even if apparently unused in code
	[Attribute(defvalue: "0", category: "Pole setup", desc: "Expected empty space around the poles line by other generators", params: "0 100 1")]
	float Clearance;

	static ref array<vector> s_aJunctionPoints = {};
	static ref map<int, IEntity> s_mJuctionPointsToEntitiesMap = new ref map<int, IEntity>();

	ref array<IEntity> m_aMyJunctionPoles = {};
	ref array<IEntity> m_aOtherJunctionPoles = {};

	ShapeEntity m_ParentShape;

	ref array<ref Shape> m_aDebugShapes = {};
	IEntity m_PreviousPowerPole;

	//QUERY DATA
	static ref array<IEntitySource> s_aGenerators = {};
	static IEntitySource s_CurrentQueryGenerator;

	protected bool m_bColorSwitch = 0;
	protected bool m_bLastJunctionWasSameLine = true;

	protected IEntity m_StartPoleEntity;

#ifdef WORKBENCH
	static WorldEditorAPI s_Api;
#endif

#ifdef WORKBENCH
	private IEntitySource m_ShapeSource;
	private IEntitySource m_PowerlineGeneratorSource;

	//-----------------------------------------------------------------------
	static void GenerateJunctions(notnull IEntitySource generator, bool newQuery)
	{
		if (!s_Api || s_Api.UndoOrRedoIsRestoring())
			return;

		if (!s_Api.IsDoingEditAction())
		{
			s_Api.BeginEntityAction();
			GenerateJunctions(generator, newQuery);
			s_Api.EndEntityAction();
			return;
		}

		if (newQuery)
		{
			if (s_aGenerators)
				s_aGenerators.Clear();
			else
				s_aGenerators = {};
		}

		s_CurrentQueryGenerator = generator;
		s_aGenerators.Insert(generator);
		QueryGenerators(generator);

		int generatorsCount = s_aGenerators.Count();
		for (int i = 0; i < generatorsCount; i++)
		{
			SCR_PowerlineGeneratorEntity queriedGenerator = SCR_PowerlineGeneratorEntity.Cast(s_Api.SourceToEntity(s_aGenerators[i]));
			if (!queriedGenerator)
				continue;

			queriedGenerator.DeletePoles();
			queriedGenerator.GenerateJunctions();
		}

		for (int i = 0; i < generatorsCount; i++)
		{
			Print(s_aGenerators[i]);
			SCR_PowerlineGeneratorEntity queriedGenerator = SCR_PowerlineGeneratorEntity.Cast(s_Api.SourceToEntity(s_aGenerators[i]));
			if (!queriedGenerator)
				continue;

			for (int j = generatorsCount - 1; j >= 0; j--)
			{
				if (i == j)
					continue;

				SCR_PowerlineGeneratorEntity otherQueriedGenerator = SCR_PowerlineGeneratorEntity.Cast(s_Api.SourceToEntity(s_aGenerators[j]));
				if (!otherQueriedGenerator)
					continue;

				queriedGenerator.FindCommonJunctionsPoints(otherQueriedGenerator.m_aMyJunctionPoles);
			}

			queriedGenerator.GeneratePoles(null);
		}
	}

	//-----------------------------------------------------------------------
	bool IsJunctionPoint(vector point)
	{
		const float tolerance = 0.1 * 0.1;

		for (int i = m_aMyJunctionPoles.Count() - 1; i >= 0; i--)
		{
			if (vector.DistanceSq(point, CoordToLocal(m_aMyJunctionPoles[i].GetOrigin())) < tolerance)
				return true;
		}

		return false;
	}

	//-----------------------------------------------------------------------
	bool HasCommonPoint(SCR_PowerlineGeneratorEntity other)
	{
		IEntitySource sourceOther;
		sourceOther = s_Api.EntityToSource(other);
		if (!sourceOther)
			return false;

		IEntitySource shapeSourceOther;
		shapeSourceOther = sourceOther.GetParent();
		if (!m_ShapeSource || ! shapeSourceOther)
			return false;

		array<vector> pointsThis, pointsOther;
		pointsThis = GetPoints(m_ShapeSource);
		pointsOther = GetPoints(shapeSourceOther);
		if (!pointsThis || !pointsOther)
			return false;

		const float tolerance = 0.1 * 0.1;

		for (int i = pointsThis.Count() - 1; i >= 0; i--)
		{
			for (int j = pointsOther.Count() - 1; j >= 0; j--)
			{
				vector coordThis = CoordToParent(pointsThis[i]);
				vector coordOther = other.CoordToParent(pointsOther[j]);
				if (vector.DistanceSq(coordThis, coordOther) < tolerance)
					return true;
			}
		}

		return false;
	}

	//-----------------------------------------------------------------------
	void FindCommonJunctionsPoints(array<IEntity> otherJunctionPoles)
	{
		array<vector> points = GetPoints(m_Source.GetParent());
		if (!points)
			return;

		const float tolerance = 0.1 * 0.1;

		for (int i = otherJunctionPoles.Count() - 1; i >= 0; i--)
		{
			for (int j = points.Count() - 1; j >= 0; j--)
			{
				if (vector.DistanceSq(CoordToParent(points[j]), otherJunctionPoles[i].GetOrigin()) < tolerance)
				{
					// Isn't in the array yet
					if (m_aOtherJunctionPoles.Find(otherJunctionPoles[i]) == -1)
						m_aOtherJunctionPoles.Insert(otherJunctionPoles[i]);

					// We skip the rest of the points
					break;
				}
			}
		}
	}

	//-----------------------------------------------------------------------
	void GenerateJunctions()
	{
		m_aMyJunctionPoles.Clear();

		if (!m_ShapeSource)
		{
			IEntitySource thisSource = s_Api.EntityToSource(this);
			m_ShapeSource = thisSource.GetParent();
		}

		IEntity shapeEntity = s_Api.SourceToEntity(m_ShapeSource);
		if (!shapeEntity)
			return;

		BaseContainerList points = m_ShapeSource.GetObjectArray("Points");

		vector parentPositionXZ = shapeEntity.GetOrigin();
		float parentY = parentPositionXZ[1];
		vector lastPointPosition;
		vector currentPointPosition;
		float yaw;

		for (int i = 0, count = points.Count(); i < count; i++)
		{
			BaseContainer point = points.Get(i);

			lastPointPosition = currentPointPosition;
			point.Get("Position", currentPointPosition);

			if (i > 0)
				yaw = (currentPointPosition - lastPointPosition).Normalized().ToYaw();
			else
			{
				if (points.Count() > 1)
				{
					BaseContainer nextPoint = points.Get(i + 1);
					vector nextPointPosition;
					nextPoint.Get("Position", nextPointPosition);
					yaw = (nextPointPosition - currentPointPosition).Normalized().ToYaw();
				}
			}

			GenerateJunctionOnPoint(point, parentPositionXZ, parentY, yaw);
		}
	}

	//-----------------------------------------------------------------------
	override bool _WB_OnKeyChanged(BaseContainer src, string key, BaseContainerList ownerContainers, IEntity parent)
	{
		super._WB_OnKeyChanged(src, key, ownerContainers, parent);

		if (key == "coords")
			return false;

		s_Api = _WB_GetEditorAPI();
		if (!s_Api)
			return false;

		IEntitySource thisSrc = s_Api.EntityToSource(this);
		BaseContainerTools.WriteToInstance(this, thisSrc);

		m_ParentShape = ShapeEntity.Cast(parent);
		if (m_ParentShape)
		{
			IEntitySource parentSrc = s_Api.EntityToSource(m_ParentShape);
			OnShapeInit(parentSrc, m_ParentShape);
		}

		return true;
	}

	//-----------------------------------------------------------------------
	protected void GenerateJunctionOnPoint(notnull BaseContainer point, vector parentPositionXZ, float parentY, float yaw)
	{
		SCR_PowerlineGeneratorJunctionData junctionData;

		BaseContainerList dataArr = point.GetObjectArray("Data");
		int dataCount = dataArr.Count();
		for (int j = 0; j < dataCount; ++j)
		{
			BaseContainer data = dataArr.Get(j);
			if (data.GetClassName() == "SCR_PowerlineGeneratorPointData")
			{
				data.Get("m_JunctionData", junctionData);
				break;
			}
		}

		if (!junctionData)
			return;

		ResourceName junctionResourceName = junctionData.GetJunctionResourceName();
		if (junctionResourceName.IsEmpty())
			junctionResourceName = m_DefaultJunctionPole;

		if (junctionResourceName.IsEmpty())
			return;

		vector pointPosition;
		point.Get("Position", pointPosition);

		IEntity pole = s_Api.CreateEntity(junctionResourceName, "", s_Api.GetCurrentEntityLayerId(), s_Api.EntityToSource(this), pointPosition, "0 0 0");
		if (!pole)
			return;

		if (junctionData.GetRotate180Degrees())
		{
			yaw += 180;
			if (yaw > 360)
				yaw -= 360;
		}

		s_Api.ModifyEntityKey(pole, "angleY", yaw.ToString());

		SCR_PowerPole powerPole = SCR_PowerPole.Cast(pole);
		if (!powerPole)
			return;

		m_aMyJunctionPoles.Insert(powerPole);
	}

	//-----------------------------------------------------------------------
	void DeletePoles()
	{
		if (s_Api.IsDoingEditAction())
		{
			// Delete old poles
			IEntitySource entSrc = s_Api.EntityToSource(this);
			int childCount = entSrc.GetNumChildren();
			for (int i = childCount - 1; i >= 0; --i)
			{
				IEntitySource childSrc = entSrc.GetChild(i);
				IEntity child = s_Api.SourceToEntity(childSrc);
				s_Api.DeleteEntity(child);
			}
		}
		else
		{
			s_Api.BeginEntityAction();
			DeletePoles();
			s_Api.EndEntityAction();
		}

		m_aMyJunctionPoles.Clear();
	}

	//-----------------------------------------------------------------------
	protected void GeneratePoles(BaseContainerList points)
	{
		if (!m_bDrawDebugShapes && m_aDebugShapes)
			m_aDebugShapes.Clear();

		s_Api = _WB_GetEditorAPI();
		if (!s_Api)
			return;

		if (!s_Api.IsDoingEditAction())
		{
			s_Api.BeginEntityAction();
			GeneratePoles(points);
			s_Api.EndEntityAction();
			return;
		}

		//Hack, enfusion doesn't collaborate for some reason TODO: FIND REPRO
		BaseContainerList points2;
		if (!points)
		{
			if (!m_ParentShape)
				return;

			IEntitySource shapeEntitySrc = s_Api.EntityToSource(m_ParentShape);
			if (!shapeEntitySrc)
				return;

			points2 = shapeEntitySrc.GetObjectArray("Points");
			points = points2;
		}
		//End of hack

		if (!m_ParentShape)
			return;

		float currentDistance = 0;
		vector currentPoint, lastPoint;
		bool spawnPerPoint = false, previousWasPerPoint = false;
		vector parentPositionXZ = m_ParentShape.GetOrigin();
		float parentY = parentPositionXZ[1];
		parentPositionXZ[1] = 0;
		for (int i = 0, count = points.Count(); i < count; i++)
		{
			float pointDistance;
			float poleOffset = 0;

			AttachJunctionOnPoint(currentPoint);

			BaseContainer point = points.Get(i);
			lastPoint = currentPoint;
			previousWasPerPoint = spawnPerPoint;
			point.Get("Position", currentPoint);

			//Get data of previous point
			if (i > 0)
			{
				BaseContainerList dataArr = points.Get(i - 1).GetObjectArray("Data");
				int dataCount = dataArr.Count();
				for (int j = 0; j < dataCount; ++j)
				{
					BaseContainer data = dataArr.Get(j);
					if (data.GetClassName() == "SCR_PowerlineGeneratorPointData")
					{
						data.Get("m_fPoleOffset", poleOffset);
						if (m_fDistance + poleOffset < 1)
						{
							poleOffset = 1 - m_fDistance;
							Print("Pole offset is too small!", LogLevel.WARNING);
						}
						data.Get("m_bGeneratePerPoint", spawnPerPoint);
						break;
					}
				}
			}

			vector direction = (currentPoint - lastPoint).Normalized();
			float yaw = direction.ToYaw();

			if (i == 1) //Generate start pole
				GenerateStartPole(lastPoint, parentY, parentPositionXZ, yaw);

			if (spawnPerPoint)
			{
				if (i == 1)
					continue;

				if (!IsJunctionPoint(lastPoint))
					GeneratePole(lastPoint, parentPositionXZ, parentY, yaw);

				if (i == count - 1) //Generate end pole
					GenerateEndPole(currentPoint, parentY, parentPositionXZ, yaw);
			}
			else
			{
				if (previousWasPerPoint)
				{
					GeneratePole(lastPoint, parentPositionXZ, parentY, yaw);
					currentDistance = 0;
				}

				if (lastPoint)
					pointDistance = vector.Distance(currentPoint, lastPoint);
				else continue;

				vector lastPolePosition = lastPoint;

				if (currentDistance + pointDistance > m_fDistance + poleOffset)
				{
					float nextPoleDistance = (m_fDistance + poleOffset - currentDistance);
					pointDistance -= nextPoleDistance;

					vector nextPolePosition = lastPoint + direction * nextPoleDistance;

					lastPolePosition = nextPolePosition;
					GeneratePole(lastPolePosition, parentPositionXZ, parentY, yaw);
					currentDistance = 0;
				}
				else
				{
					currentDistance += pointDistance - poleOffset;
					if (i == count - 1) //Generate end pole
						GenerateEndPole(currentPoint, parentY, parentPositionXZ, yaw);
					continue;
				}

				while (pointDistance > m_fDistance + poleOffset)
				{
					if (m_fDistance == 0 && poleOffset == 0)
					{
						Print("m_fDistance in Powerline Generator and pole offset is 0. Not generating any poles.", LogLevel.WARNING);
						break;
					}

					vector nextPolePosition = lastPolePosition + direction * (poleOffset + m_fDistance);

					lastPolePosition = nextPolePosition;
					GeneratePole(lastPolePosition, parentPositionXZ, parentY, yaw);
					pointDistance -= m_fDistance + poleOffset;
				}

				if (i == count - 1) //Generate end pole
				{
					GenerateEndPole(currentPoint, parentY, parentPositionXZ, yaw);
					return;
				}

				if (pointDistance > 0)
					currentDistance += pointDistance;
			}
		}
	}

	//-----------------------------------------------------------------------
	IEntity FindJunctionOnPoint(vector point, out bool sameLine)
	{
		float tolerance = 0.1 * 0.1;

		for (int i = m_aMyJunctionPoles.Count() - 1; i >= 0; i--)
		{
			if (m_aMyJunctionPoles[i] && vector.DistanceSq(point, CoordToLocal(m_aMyJunctionPoles[i].GetOrigin())) < tolerance)
			{
				sameLine = true;
				return m_aMyJunctionPoles[i];
			}
		}

		for (int i = m_aOtherJunctionPoles.Count() - 1; i >= 0; i--)
		{
			if (m_aOtherJunctionPoles[i] && vector.DistanceSq(point, CoordToLocal(m_aOtherJunctionPoles[i].GetOrigin())) < tolerance)
			{
				sameLine = false;
				return m_aOtherJunctionPoles[i];
			}
		}

		return null;
	}

	//-----------------------------------------------------------------------
	void AttachJunctionOnPoint(vector point)
	{
		float tolerance = 0.1 * 0.1;

		for (int i = m_aMyJunctionPoles.Count() - 1; i >= 0; i--)
		{
			if (m_aMyJunctionPoles[i] && vector.DistanceSq(point, CoordToLocal(m_aMyJunctionPoles[i].GetOrigin())) < tolerance)
			{
				AttachJunction(m_aMyJunctionPoles[i], true);
			}
		}

		for (int i = m_aOtherJunctionPoles.Count() - 1; i >= 0; i--)
		{
			if (m_aOtherJunctionPoles[i] && vector.DistanceSq(point, CoordToLocal(m_aOtherJunctionPoles[i].GetOrigin())) < tolerance)
			{
				AttachJunction(m_aOtherJunctionPoles[i], false);
			}
		}
	}

	//-----------------------------------------------------------------------
	void AttachJunction(IEntity junction, bool sameLine)
	{
		SCR_PowerPole junctionPole = SCR_PowerPole.Cast(junction);
		if (!junctionPole)
			return;

		m_bLastJunctionWasSameLine = sameLine;

		if (m_PreviousPowerPole)
		{
			junctionPole.AttachTo(m_PreviousPowerPole);
			CreatePowerLine(junctionPole, SCR_PowerPole.Cast(m_PreviousPowerPole), sameLine);
		}

		m_PreviousPowerPole = junctionPole;
	}

	//-----------------------------------------------------------------------
	protected vector FindIntersection(float p0_x, float p0_y, float p1_x, float p1_y,
    float p2_x, float p2_y, float p3_x, float p3_y)
	{
	    float s1_x, s1_y, s2_x, s2_y;
	    s1_x = p1_x - p0_x;
		s1_y = p1_y - p0_y;
	    s2_x = p3_x - p2_x;
		s2_y = p3_y - p2_y;

	    float s, t;
	    s = (-s1_y * (p0_x - p2_x) + s1_x * (p0_y - p2_y)) / (-s2_x * s1_y + s1_x * s2_y);
	    t = ( s2_x * (p0_y - p2_y) - s2_y * (p0_x - p2_x)) / (-s2_x * s1_y + s1_x * s2_y);

	    if (s >= 0 && s <= 1 && t >= 0 && t <= 1)
	    {
	        // Collision detected
			vector intersect = vector.Zero;
	        intersect[0] = p0_x + (t * s1_x);
	        intersect[2] = p0_y + (t * s1_y);
	        return intersect;
	    }

	    return vector.Zero; // No collision
	}

	//-----------------------------------------------------------------------
	protected void LoadLines(BaseContainerList points, array<ref SCR_Line> lines)
	{
		vector currentPoint, lastPoint;
		for (int i = 0, count = points.Count(); i < count; i++)
		{
			points.Get(i).Get("Position", currentPoint);

			if (lastPoint)
			{
				SCR_Line line = new SCR_Line();
				line.m_vPoint1 = lastPoint;
				line.m_vPoint2 = currentPoint;
				lines.Insert(line);
			}

			lastPoint = currentPoint;
		}
	}

	//-----------------------------------------------------------------------
	protected void GenerateEndPole(vector currentPoint, float parentY, vector parentPositionXZ, float yaw)
	{
		if (m_EndPole.GetPath() != "")
		{
			vector endPolePos = currentPoint;
			bool sameLine;
			IEntity pole = FindJunctionOnPoint(currentPoint, sameLine);

			if (pole)
			{
				m_bLastJunctionWasSameLine = sameLine;
			}
			else
			{
				pole = GeneratePoleAt(endPolePos, parentPositionXZ, parentY, m_EndPole);

				if (SCR_JunctionPowerPole.Cast(pole))
					Print("End pole is of type SCR_JunctionPowerPole, make sure to do this using junction data on the last point instead.", LogLevel.WARNING);

				if (m_bRotate180DegreeYawEndPole)
				{
					yaw += 180;
					if (yaw > 360)
						yaw -= 360;
				}

				s_Api.ModifyEntityKey(pole, "angleY", yaw.ToString());
				if (m_bApplyPitchAndRollEnd)
					ApplyRandomPitchAndRoll(pole);
			}

			SCR_PowerPole endPole = SCR_PowerPole.Cast(pole);
			if (endPole)
			{
				endPole.AttachTo(m_PreviousPowerPole);
				CreatePowerLine(SCR_PowerPole.Cast(m_PreviousPowerPole), endPole, sameLine, m_bRotate180DegreeYawEndPole);
			}

			if (m_bDrawDebugShapes)
			{
				return;
				Shape shape = Shape.CreateSphere(ARGB(255, 255, 0, 0), ShapeFlags.NOZBUFFER, endPolePos + parentPositionXZ, 1);
				m_aDebugShapes.Insert(shape);
			}
		}
	}

	//-----------------------------------------------------------------------
	protected void GenerateStartPole(vector lastPoint, float parentY, vector parentPositionXZ, float yaw)
	{
		if (!s_Api)
		{
			Print("WorldEditorAPI not found. In SCR_PowerlineGeneratorEntity.", LogLevel.WARNING);
			return;
		}

		if (m_StartPole.GetPath() != "")
		{
			vector startPolePos = lastPoint;
			bool sameLine;
			IEntity pole = FindJunctionOnPoint(lastPoint, sameLine);

			if (pole)
			{
				m_bLastJunctionWasSameLine = sameLine;
			}
			else
			{
				pole = GeneratePoleAt(startPolePos, parentPositionXZ, parentY, m_StartPole);
				if (!pole)
				{
					Print("Pole entity not created. In SCR_PowerlineGeneratorEntity.", LogLevel.WARNING);
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

				s_Api.ModifyEntityKey(pole, "angleY", yaw.ToString());
				if (m_bApplyPitchAndRollStart)
					ApplyRandomPitchAndRoll(pole);
			}

			m_PreviousPowerPole = pole;
			m_StartPoleEntity = pole;

			if (m_bDrawDebugShapes)
			{
				return;
				Shape shape = Shape.CreateSphere(ARGB(255, 255, 0, 0), ShapeFlags.NOZBUFFER, startPolePos + parentPositionXZ, 1);
				m_aDebugShapes.Insert(shape);
			}
		}
	}

	//-----------------------------------------------------------------------
	protected IEntity GeneratePoleAt(vector localPos, vector parentPositionXZ, float parentY, ResourceName poleResourceName)
	{
		float y;
		if (s_Api.TryGetTerrainSurfaceY(localPos[0] + parentPositionXZ[0], localPos[2] + parentPositionXZ[2], y))
			localPos[1] = y - parentY;
		IEntity pole = s_Api.CreateEntity(poleResourceName, "", s_Api.GetCurrentEntityLayerId(), s_Api.EntityToSource(this), localPos, "0 0 0");

		return pole;
	}

	//-----------------------------------------------------------------------
	protected void ApplyRandomPitchAndRoll(IEntity powerPole)
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

		s_Api.ModifyEntityKey(powerPole, "angleX", pitch.ToString());
		s_Api.ModifyEntityKey(powerPole, "angleZ", roll.ToString());
	}

	//-----------------------------------------------------------------------
	protected IEntity GeneratePole(vector lastPolePosition, vector parentPositionXZ, float parentY, float yaw, ResourceName customPoleResourceName = string.Empty)
	{
		float y;
		IEntity pole;
		if (s_Api.TryGetTerrainSurfaceY(lastPolePosition[0] + parentPositionXZ[0], lastPolePosition[2] + parentPositionXZ[2], y))
			lastPolePosition[1] = y - parentY;

		ResourceName poleResourceName;
		if (customPoleResourceName.GetPath() != string.Empty)
			poleResourceName = customPoleResourceName;
		else
			poleResourceName = m_DefaultPole; //Already checked for empty string

		pole = s_Api.CreateEntity(poleResourceName, "", s_Api.GetCurrentEntityLayerId(), s_Api.EntityToSource(this), lastPolePosition, "0 0 0");
		if (!pole)
			return null;

		s_Api.ModifyEntityKey(pole, "angleY", yaw.ToString());
		if (m_bApplyPitchAndRollDefault)
			ApplyRandomPitchAndRoll(pole);

		if (m_PreviousPowerPole && pole)
		{
			auto thisPowerPole = SCR_PowerPole.Cast(pole);

			if (thisPowerPole)
			{
				thisPowerPole.AttachTo(m_PreviousPowerPole);
				bool inverse = m_StartPoleEntity == m_PreviousPowerPole && m_bRotate180DegreeYawStartPole;
				CreatePowerLine(thisPowerPole, SCR_PowerPole.Cast(m_PreviousPowerPole), inverse: inverse);
			}
		}
		m_PreviousPowerPole = pole;

		return pole;

		if (m_bDrawDebugShapes)
		{
			Shape shape = Shape.CreateSphere(ARGB(255, 255, 0, 0), ShapeFlags.NOZBUFFER, lastPolePosition + parentPositionXZ, 1);
			m_aDebugShapes.Insert(shape);
		}
	}

	//-----------------------------------------------------------------------
	override void OnShapeChangedInternal(IEntitySource shapeEntitySrc, ShapeEntity shapeEntity, array<vector> mins, array<vector> maxes)
	{
		// TODO handle this case better if needed, use the bbox arrays
		m_ParentShape = shapeEntity;
		m_Source = _WB_GetEditorAPI().EntityToSource(this);
		OnShapeInit(shapeEntitySrc, shapeEntity);
	}

	//-----------------------------------------------------------------------
	protected IEntity CreatePowerLine(SCR_PowerPole startPole, SCR_PowerPole endPole, bool sameLine = true, bool inverse = false)
	{
		if (!endPole || startPole == endPole)
			return null;

		ref array<vector> startPoints = new array<vector>();
		ref array<vector> endPoints = new array<vector>();

		if (SCR_JunctionPowerPole.Cast(startPole) || SCR_JunctionPowerPole.Cast(endPole))
			sameLine = m_bLastJunctionWasSameLine;

		int numCables = Math.Min(startPole.GetSlotsCount(sameLine), endPole.GetSlotsCount(sameLine));
		if (numCables <= 0)
		{
			Print("0 slots found on one of the power poles, please check the setup of your power poles.", LogLevel.WARNING);
			return null;
		}

		// Gather slot positions for both poles in world coordinates
		for (int i = 0; i < numCables; ++i)
		{
			vector startSlotPos = startPole.CoordToParent(startPole.GetSlot(i, sameLine));
			startPoints.Insert(startSlotPos);
			endPoints.Insert(endPole.TryGetSlot(i, startSlotPos, sameLine));
		}

		// Create the PowerlineEntity
		IEntity powerline = s_Api.CreateEntity("PowerlineEntity", "", s_Api.GetCurrentEntityLayerId(), m_PowerlineGeneratorSource, CoordToLocal(startPoints[0]), "0 0 0");
		IEntitySource powerlineSrc = s_Api.EntityToSource(powerline);

		s_Api.SetVariableValue(powerlineSrc, null, "Material", m_PowerlineMaterial);

		// Create cables for the entity
		for (int i = 0; i < numCables; ++i)
		{
			s_Api.CreateObjectArrayVariableMember(powerlineSrc, null, "Cables", "Cable", i);

			//Refresh powerline pointer
			powerline = s_Api.SourceToEntity(powerlineSrc);

			auto containerPath = new array<ref ContainerIdPathEntry>();
			containerPath.Insert(new ContainerIdPathEntry("Cables", i));

			// Add cable between each slot, convert to local coordinate space
			vector startPos = powerline.CoordToLocal(startPoints[i]);
			s_Api.SetVariableValue(powerlineSrc, containerPath, "StartPoint", startPos.ToString(false));

			//Refresh powerline pointer
			powerline = s_Api.SourceToEntity(powerlineSrc);

			vector endPos = powerline.CoordToLocal(endPoints[i]);
			if (inverse)
				endPos = powerline.CoordToLocal(endPoints[numCables - i - 1]);
			s_Api.SetVariableValue(powerlineSrc, containerPath, "EndPoint", endPos.ToString(false));
		}

		return powerline;
	}

	//-----------------------------------------------------------------------
	static void QueryGenerators(notnull IEntitySource generator, array<IEntitySource> checkedGenerators = null)
	{
		if (!s_Api || !s_aGenerators)
			return;

		IEntitySource shapeSource = generator.GetParent();
		if (!shapeSource)
			return;

		IEntity shapeEntity = s_Api.SourceToEntity(shapeSource);
		if (!shapeEntity)
			return;

		array<IEntitySource> checkedGeneratorsArray = checkedGenerators;
		if (!checkedGeneratorsArray)
			checkedGeneratorsArray = {};

		checkedGenerators = checkedGeneratorsArray;

		if (checkedGenerators.Find(generator) != -1)
			return;

		checkedGenerators.Insert(generator);

		BaseContainerList points = shapeSource.GetObjectArray("Points");

		// Get bbox
		array<vector> vectorPoints = GetPoints(shapeSource);
		AAB bbox = AAB.MakeFromPoints(vectorPoints);

		// Query entities in bbox
		vector mat[4];
		shapeEntity.GetTransform(mat);

		IEntity generatorEntity = s_Api.SourceToEntity(generator);
		BaseWorld world = generatorEntity.GetWorld();

		bbox.m_vMin[1] = -50;
		bbox.m_vMax[1] = 50;

		world.QueryEntitiesByAABB(generatorEntity.CoordToParent(bbox.m_vMin), generatorEntity.CoordToParent(bbox.m_vMax), QueryFilter);

		for (int i = s_aGenerators.Count() - 1; i >= 0; i--)
		{
			if (checkedGenerators.Find(s_aGenerators[i]) != 1)
				continue;

			s_CurrentQueryGenerator = s_aGenerators[i];
			QueryGenerators(s_aGenerators[i], checkedGeneratorsArray);
		}
	}

	//-----------------------------------------------------------------------
	static bool QueryFilter(IEntity entity)
	{
		ShapeEntity shape = ShapeEntity.Cast(entity);
		if (!shape)
			return true;

		IEntitySource shapeSource = s_Api.EntityToSource(shape);
		if (!shapeSource)
			return true;

		int childCount = shapeSource.GetNumChildren();
		for (int i = childCount - 1; i >= 0; --i)
		{
			IEntitySource otherGeneratorSource = shapeSource.GetChild(i);
			SCR_PowerlineGeneratorEntity otherGeneratorEntity = SCR_PowerlineGeneratorEntity.Cast(s_Api.SourceToEntity(s_CurrentQueryGenerator));
			if (!otherGeneratorEntity)
				continue;

			//Has to be refreshed here
			SCR_PowerlineGeneratorEntity currentGeneratorEntity = SCR_PowerlineGeneratorEntity.Cast(s_Api.SourceToEntity(s_CurrentQueryGenerator));
			if (!currentGeneratorEntity)
				continue;

			if (otherGeneratorSource && otherGeneratorSource != s_CurrentQueryGenerator && s_aGenerators.Find(otherGeneratorSource) == -1 && currentGeneratorEntity.HasCommonPoint(otherGeneratorEntity))
			{
				s_aGenerators.Insert(otherGeneratorSource);
				return true;
			}
		}

		return true;
	}

	//-----------------------------------------------------------------------
	void AddPointData(BaseContainerList points)
	{
		for (int i = points.Count() - 1; i >= 0; i--)
		{
			BaseContainerList dataArr = points[i].GetObjectArray("Data");
			int dataCount = dataArr.Count();
			bool hasPointData = false;
			for (int j = 0; j < dataCount; ++j)
			{
				BaseContainer data = dataArr.Get(j);
				if (data.GetClassName() == "SCR_PowerlineGeneratorPointData")
				{
					hasPointData = true;
					break;
				}
			}

			if (!hasPointData && s_Api && !s_Api.UndoOrRedoIsRestoring())
				s_Api.CreateObjectArrayVariableMember(points[i], null, "Data", "SCR_PowerlineGeneratorPointData", dataArr.Count());
		}
	}

	//-----------------------------------------------------------------------
	override void OnShapeInitInternal(IEntitySource shapeEntitySrc, ShapeEntity shapeEntity)
	{
		super.OnShapeInitInternal(shapeEntitySrc, shapeEntity);

		m_ShapeSource = shapeEntitySrc;

		s_Api = _WB_GetEditorAPI();

		BaseContainerList points = shapeEntitySrc.GetObjectArray("Points");
		if (!points)
			return;
		
		AddPointData(points);

		GenerateJunctions(m_PowerlineGeneratorSource, true);
	}
#endif

	//-----------------------------------------------------------------------
	void SCR_PowerlineGeneratorEntity(IEntitySource src, IEntity parent)
	{
#ifdef WORKBENCH
		if (GetGame().InPlayMode())
			return;

		m_PowerlineGeneratorSource = src;

		ShapeEntity shape = ShapeEntity.Cast(parent);
		if (shape)
		{
			m_ParentShape = shape;
			m_ShapeSource = _WB_GetEditorAPI().EntityToSource(shape);
		}

		SetEventMask(EntityEvent.INIT);
#endif
	}

	//-----------------------------------------------------------------------
	void ~SCR_PowerlineGeneratorEntity()
	{
		if (m_aDebugShapes)
			m_aDebugShapes.Clear();
		m_aDebugShapes = null;
	}
};