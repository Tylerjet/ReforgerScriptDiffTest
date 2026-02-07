// #define _ALLOW_FOREST_REGENERATION
//------------------------------------------------------------------------------------------------
class ForestGeneratorPointData : ShapePointDataScriptBase
{
	[Attribute("1", UIWidgets.CheckBox, "Generate the middle outline along this line?")]
	bool m_bMiddleOutline;

	[Attribute("1", UIWidgets.CheckBox, "Generate the small outline along this line?")]
	bool m_bSmallOutline;
};

//------------------------------------------------------------------------------------------------
class ForestGeneratorPoint
{
	vector m_vPos;
	float m_fMinAngle;
	float m_fMaxAngle;
	float m_fAngle;
	bool m_bSmallOutline = true;
	bool m_bMiddleOutline = true;
	ref ForestGeneratorLine m_Line1;
	ref ForestGeneratorLine m_Line2;
};

//------------------------------------------------------------------------------------------------
class ForestGeneratorLine
{
	ref ForestGeneratorPoint p1 = new ForestGeneratorPoint();
	ref ForestGeneratorPoint p2 = new ForestGeneratorPoint();
	float m_fLength;
};

//------------------------------------------------------------------------------------------------
enum TreeType
{
	TOP,
	BOTTOM,
	MOUTLINE,
	SOUTLINE,
	CLUSTER,
	COUNT,
};

//------------------------------------------------------------------------------------------------
enum ForestGeneratorClusterType
{
	CIRCLE = 0,
	STRIP = 1,
};

//------------------------------------------------------------------------------------------------
enum ForestGeneratorLevelType
{
	TOP = 0,
	BOTTOM = 1,
	OUTLINE = 2,
};

//------------------------------------------------------------------------------------------------
enum ForestGeneratorOutlineType
{
	SMALL = 0,
	MIDDLE = 1,
};

//------------------------------------------------------------------------------------------------
class ForestGeneratorRectangle
{
	ref ForestGeneratorLine m_Line1 = new ForestGeneratorLine();
	ref ForestGeneratorLine m_Line2 = new ForestGeneratorLine();
	ref ForestGeneratorLine m_Line3 = new ForestGeneratorLine();
	ref ForestGeneratorLine m_Line4 = new ForestGeneratorLine();
	ref array<ForestGeneratorLine> m_aLines = {}; // outline lines that intersect this rectangle
	ref array<vector> m_aPoints = {}; // points of this rectangle
	ref array<IEntitySource> m_aPresentRoadShapes = {};
	float m_fWidth;
	float m_fLength;
	float m_fArea;
	int m_iX;
	int m_iY;

	//------------------------------------------------------------------------------------------------
	void GetBounds(out vector mins, out vector maxs)
	{
		array<vector> points = {};
		points.Insert(m_Line1.p1.m_vPos);
		points.Insert(m_Line1.p2.m_vPos);
		points.Insert(m_Line2.p2.m_vPos);
		points.Insert(m_Line3.p2.m_vPos);
		float minX = float.MAX;
		float maxX = -float.MAX;
		float minZ = float.MAX;
		float maxZ = -float.MAX;

		foreach (vector point : points)
		{
			if (point[0] < minX)
				minX = point[0];

			if (point[0] > maxX)
				maxX = point[0];

			if (point[2] < minZ)
				minZ = point[2];

			if (point[2] > maxZ)
				maxZ = point[2];
		}

		mins[0] = minX;
		mins[2] = minZ;
		maxs[0] = maxX;
		maxs[2] = maxZ;
	}

	//------------------------------------------------------------------------------------------------
	bool IsPointIn(float x, float y)
	{
		float startX = m_Line1.p1.m_vPos[0];
		float startY = m_Line1.p1.m_vPos[2];

		return
			x > startX &&
			x < startX + m_fWidth &&
			y > startY &&
			y < startY + m_fLength;
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class ForestGeneratorLevel
{
	[Attribute(uiwidget: UIWidgets.Object, desc: "Tree groups to spawn in this forest generator level")]
	ref array<ref TreeGroupClass> m_aTreeGroups;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Generate this level?")]
	bool m_bGenerate;

	[Attribute(defvalue: "1", desc: "How many trees per hectare should be generated.", uiwidget: UIWidgets.SpinBox)]
	float m_fDensity;

	ForestGeneratorLevelType m_Type;
	ref array<float> m_aGroupProbas = {};
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class ForestGeneratorOutline : ForestGeneratorLevel
{
	[Attribute(defvalue: "1", uiwidget: UIWidgets.SpinBox, desc: "How separated are the tree groups?")]
	float m_fClusterStrength;

	[Attribute(defvalue: "15", uiwidget: UIWidgets.SpinBox, desc: "In what radius should trees around be taken into count for clusters?")]
	float m_fClusterRadius;

	[Attribute("0", UIWidgets.ComboBox,enums: { ParamEnum("Small", "0"), ParamEnum("Middle", "1") }, desc: "Which type of outline is this?")]
	ForestGeneratorOutlineType m_OutlineType;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.SpinBox, desc: "Minimal distance from spline.")]
	float m_fMinDistance;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.SpinBox, desc: "Maximal distance from spline.")]
	float m_fMaxDistance;

	//------------------------------------------------------------------------------------------------
	void ForestGeneratorOutline()
	{
		m_Type = ForestGeneratorLevelType.OUTLINE;
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class ForestGeneratorTopLevel : ForestGeneratorLevel
{
	[Attribute(defvalue: "1", uiwidget: UIWidgets.SpinBox, desc: "How separated are the tree groups?")]
	float m_fClusterStrength;

	[Attribute(defvalue: "15", uiwidget: UIWidgets.SpinBox, desc: "In what radius should trees around be taken into count for clusters?")]
	float m_fClusterRadius;

	//------------------------------------------------------------------------------------------------
	void ForestGeneratorTopLevel()
	{
		m_Type = ForestGeneratorLevelType.TOP;
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class ForestGeneratorBottomLevel : ForestGeneratorLevel
{
	void ForestGeneratorBottomLevel()
	{
		m_Type = ForestGeneratorLevelType.BOTTOM;
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class FallenTree : ForestGeneratorTree
{
	[Attribute(desc: "Capsule start, defines offset from the pivot of the object, serves for collision detection")]
	vector m_CapsuleStartInEditor;

	[ForestGeneratorCapsuleStartAttribute()]
	vector m_CapsuleStart;

	[Attribute(desc: "Capsule end, defines offset from the pivot of the object, serves for collision detection")]
	vector m_CapsuleEndInEditor;

	[ForestGeneratorCapsuleEndAttribute()]
	vector m_CapsuleEnd;

	[Attribute(defvalue: "1", desc: "This overrides the setting from template library!")]
	bool m_bAlignToNormal;

	float m_fYaw = 0;
	protected float m_fMinDistanceFromLine = -1;

	//------------------------------------------------------------------------------------------------
	void Rotate()
	{
		// vector yawDirection = vector.YawToVector(yaw);
		// yawDirection = Vector(-yawDirection[0], 0, yawDirection[1]); // convert from Enforce to Enfusion format
		m_CapsuleStart = Rotate2D(m_CapsuleStartInEditor * m_fScale, Math.DEG2RAD * m_fYaw);
		m_CapsuleEnd = Rotate2D(m_CapsuleEndInEditor * m_fScale, Math.DEG2RAD * m_fYaw);
	}

	//------------------------------------------------------------------------------------------------
	vector Rotate2D(vector vec, float rads)
	{
		float sin = Math.Sin(rads);
		float cos = Math.Cos(rads);

		return Vector(
			vec[0] * cos - vec[2] * sin,
			0,
			vec[0] * sin + vec[2] * cos);
	}

	//------------------------------------------------------------------------------------------------
	float GetMinDistanceFromLine()
	{
		if (m_fMinDistanceFromLine != -1)
			return m_fMinDistanceFromLine;

		float distance = m_CapsuleStart.Length();
		if (distance > m_fMinDistanceFromLine)
			m_fMinDistanceFromLine = distance;

		distance = m_CapsuleEnd.Length();
		if (distance > m_fMinDistanceFromLine)
			m_fMinDistanceFromLine = distance;

		return m_fMinDistanceFromLine;
	}

	//------------------------------------------------------------------------------------------------
	void CopyValues(notnull FallenTree newObject)
	{
		super.CopyValues(newObject);

		newObject.m_bAlignToNormal = m_bAlignToNormal;
		newObject.m_CapsuleStartInEditor = m_CapsuleStartInEditor;
		newObject.m_CapsuleEndInEditor = m_CapsuleEndInEditor;
	}

	//------------------------------------------------------------------------------------------------
	override ForestGeneratorTreeBase Copy()
	{
		FallenTree newObject = new FallenTree();
		CopyValues(newObject);
		return newObject;
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class ForestGeneratorTree : SCR_ForestGeneratorTreeBase
{
	[Attribute(defvalue: "1", uiwidget: UIWidgets.SpinBox, desc: "Weight of this object for clusters?")]
	float m_fWeight;

	int m_iDebugGroupIndex;

	//------------------------------------------------------------------------------------------------
	void CopyValues(notnull ForestGeneratorTree newObject)
	{
		super.CopyValues(newObject);

		newObject.m_fWeight = m_fWeight;
		newObject.m_iDebugGroupIndex = m_iDebugGroupIndex;
	}

	//------------------------------------------------------------------------------------------------
	override ForestGeneratorTreeBase Copy()
	{
		ForestGeneratorTree newObject = new ForestGeneratorTree();
		CopyValues(newObject);
		return newObject;
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class ForestGeneratorTreeShort : ForestGeneratorTree
{
	//------------------------------------------------------------------------------------------------
	override void AdjustScale()
	{
		m_fBotDistance *= m_fScale;
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class ForestGeneratorTreeMiddle : ForestGeneratorTree
{
	[Attribute(defvalue: "5", uiwidget: UIWidgets.SpinBox, "Minimum required radius in the middle layer for this object to spawn"), ForestGeneratorDistaceAttribute(DistanceType.MID)]
	float m_fMidDistance;

	//------------------------------------------------------------------------------------------------
	override void AdjustScale()
	{
		m_fBotDistance *= m_fScale;
		m_fMidDistance *= m_fScale;
	}

	//------------------------------------------------------------------------------------------------
	void CopyValues(notnull ForestGeneratorTreeMiddle newObject)
	{
		super.CopyValues(newObject);

		newObject.m_fMidDistance = m_fMidDistance;
	}

	//------------------------------------------------------------------------------------------------
	override ForestGeneratorTreeBase Copy()
	{
		ForestGeneratorTreeMiddle newObject = new ForestGeneratorTreeMiddle();
		CopyValues(newObject);
		return newObject;
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class ForestGeneratorTreeTall : ForestGeneratorTree
{
	[Attribute(defvalue: "5", uiwidget: UIWidgets.SpinBox, "Minimum required radius in the middle layer for this object to spawn"), ForestGeneratorDistaceAttribute(DistanceType.MID)]
	float m_fMidDistance;

	[Attribute(defvalue: "10", uiwidget: UIWidgets.SpinBox, "Minimum required radius in the top layer for this object to spawn"), ForestGeneratorDistaceAttribute(DistanceType.TOP)]
	float m_fTopDistance;

	//------------------------------------------------------------------------------------------------
	override void AdjustScale()
	{
		m_fBotDistance *= m_fScale;
		m_fMidDistance *= m_fScale;
		m_fTopDistance *= m_fScale;
	}

	//------------------------------------------------------------------------------------------------
	void CopyValues(notnull ForestGeneratorTreeTall newObject)
	{
		super.CopyValues(newObject);

		newObject.m_fMidDistance = m_fMidDistance;
		newObject.m_fTopDistance = m_fTopDistance;
	}

	//------------------------------------------------------------------------------------------------
	override ForestGeneratorTreeBase Copy()
	{
		ForestGeneratorTreeTall newObject = new ForestGeneratorTreeTall();
		CopyValues(newObject);
		return newObject;
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class ForestGeneratorCluster
{
	[Attribute(defvalue: "", uiwidget: UIWidgets.Object, desc: "Define which tree groups should spawn in this cluster.")]
	ref array<ref SmallForestGeneratorClusterObject> m_aObjects;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.SpinBox, desc: "The minimum density of the filling in number of clusters for hectare.")]
	float m_fMinCDENSHA;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.SpinBox, desc: "The maximum density of the filling in number of clusters for hectare.")]
	float m_fMaxCDENSHA;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Generate this cluster?")]
	bool m_bGenerate;

	protected ForestGeneratorClusterType m_eType;

	//------------------------------------------------------------------------------------------------
	ForestGeneratorClusterType GetClusterType()
	{
		return m_eType;
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class ForestGeneratorCircleCluster : ForestGeneratorCluster
{
	float m_fRadius = 0;

	//------------------------------------------------------------------------------------------------
	void ForestGeneratorCircleCluster()
	{
		foreach (SmallForestGeneratorClusterObject object : m_aObjects)
		{
			if (object.m_fMinRadius > m_fRadius)
				m_fRadius = object.m_fMinRadius;

			if (object.m_fMaxRadius > m_fRadius)
				m_fRadius = object.m_fMaxRadius;
		}

		m_eType = ForestGeneratorClusterType.CIRCLE;
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class ForestGeneratorStripCluster : ForestGeneratorCluster
{
	[Attribute(defvalue: "10", uiwidget: UIWidgets.SpinBox, desc: "Offset that is applied to generated objects to avoid seeing the exact shape of the curve. [m]")]
	float m_fMaxXOffset;

	[Attribute(defvalue: "10", uiwidget: UIWidgets.SpinBox, desc: "Offset that is applied to generated objects to avoid seeing the exact shape of the curve. [m]")]
	float m_fMaxYOffset;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.SpinBox, desc: "Defines how many times should the curve repeat for this strip.")]
	float m_fFrequency;

	[Attribute(defvalue: "10", uiwidget: UIWidgets.SpinBox, desc: "Defines the maximum extent of the curve. [m]")]
	float m_fAmplitude;

	float m_fRadius = 0;

	//------------------------------------------------------------------------------------------------
	void ForestGeneratorStripCluster()
	{
		foreach (SmallForestGeneratorClusterObject object : m_aObjects)
		{
			if (object.m_fMinRadius > m_fRadius)
				m_fRadius = object.m_fMinRadius;

			if (object.m_fMaxRadius > m_fRadius)
				m_fRadius = object.m_fMaxRadius;
		}

		m_eType = ForestGeneratorClusterType.STRIP;
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class WideForestGeneratorClusterObject : SmallForestGeneratorClusterObject
{
	[Attribute(desc: "Capsule start, defines offset from the pivot of the object, serves for collision detection")]
	vector m_CapsuleStartInEditor;

	[ForestGeneratorCapsuleStartAttribute()]
	vector m_CapsuleStart;

	[Attribute(desc: "Capsule end, defines offset from the pivot of the object, serves for collision detection")]
	vector m_CapsuleEndInEditor;

	[ForestGeneratorCapsuleEndAttribute()]
	vector m_CapsuleEnd;

	[Attribute(defvalue: "1", desc: "This overrides the setting from template library!")]
	bool m_bAlignToNormal;

	float m_fYaw = 0;
	protected float m_fMinDistanceFromLine = -1;

	//------------------------------------------------------------------------------------------------
	void Rotate()
	{
		m_CapsuleStart = Rotate2D(m_CapsuleStartInEditor * m_fScale, Math.DEG2RAD * m_fYaw);
		m_CapsuleEnd = Rotate2D(m_CapsuleEndInEditor * m_fScale, Math.DEG2RAD * m_fYaw);
	}

	//------------------------------------------------------------------------------------------------
	vector Rotate2D(vector vec, float rads)
	{
		float sin = Math.Sin(rads);
		float cos = Math.Cos(rads);

		return Vector(
			vec[0] * cos - vec[2] * sin,
			0,
			vec[0] * sin + vec[2] * cos);
	}

	//------------------------------------------------------------------------------------------------
	float GetMinDistanceFromLine()
	{
		if (m_fMinDistanceFromLine != -1)
			return m_fMinDistanceFromLine;

		float distance = m_CapsuleStart.Length();
		if (distance > m_fMinDistanceFromLine)
			m_fMinDistanceFromLine = distance;

		distance = m_CapsuleEnd.Length();
		if (distance > m_fMinDistanceFromLine)
			m_fMinDistanceFromLine = distance;

		return m_fMinDistanceFromLine;
	}

	//------------------------------------------------------------------------------------------------
	void CopyValues(notnull WideForestGeneratorClusterObject newObject)
	{
		super.CopyValues(newObject);

		newObject.m_bAlignToNormal = m_bAlignToNormal;
		newObject.m_CapsuleStartInEditor = m_CapsuleStartInEditor;
		newObject.m_CapsuleEndInEditor = m_CapsuleEndInEditor;
	}

	//------------------------------------------------------------------------------------------------
	override ForestGeneratorTreeBase Copy()
	{
		WideForestGeneratorClusterObject newObject = new WideForestGeneratorClusterObject();
		CopyValues(newObject);
		return newObject;
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class SmallForestGeneratorClusterObject : SCR_ForestGeneratorTreeBase
{
	[Attribute(defvalue: "1", uiwidget: UIWidgets.SpinBox, desc: "Minimum count of this object to spawn")];
	int m_iMinCount;

	[Attribute(defvalue: "2", uiwidget: UIWidgets.SpinBox, desc: "Maximum count of this object to spawn")];
	int m_iMaxCount;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.SpinBox, desc: "Minimum distance of this object from the center of this cluster")]
	float m_fMinRadius;

	[Attribute(defvalue: "2", uiwidget: UIWidgets.SpinBox, desc: "Maximum distance of this object from the center of this cluster")]
	float m_fMaxRadius;

	//------------------------------------------------------------------------------------------------
	override void AdjustScale()
	{
		m_fBotDistance *= m_fScale;
	}

	//------------------------------------------------------------------------------------------------
	void CopyValues(notnull SmallForestGeneratorClusterObject newObject)
	{
		super.CopyValues(newObject);

		newObject.m_iMinCount = m_iMinCount;
		newObject.m_iMaxCount = m_iMaxCount;
		newObject.m_fMinRadius = m_fMinRadius;
		newObject.m_fMaxRadius = m_fMaxRadius;
	}

	//------------------------------------------------------------------------------------------------
	override ForestGeneratorTreeBase Copy()
	{
		SmallForestGeneratorClusterObject newObject = new SmallForestGeneratorClusterObject();
		CopyValues(newObject);
		return newObject;
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class MiddleForestGeneratorClusterObject : SmallForestGeneratorClusterObject
{
	[Attribute(defvalue: "5", uiwidget: UIWidgets.SpinBox, "Minimum required radius in the middle layer for this object to spawn"), ForestGeneratorDistaceAttribute(DistanceType.MID)]
	float m_fMidDistance;

	//------------------------------------------------------------------------------------------------
	override void AdjustScale()
	{
		m_fBotDistance *= m_fScale;
		m_fMidDistance *= m_fScale;
	}

	//------------------------------------------------------------------------------------------------
	void CopyValues(notnull MiddleForestGeneratorClusterObject newObject)
	{
		super.CopyValues(newObject);

		newObject.m_fMidDistance = m_fMidDistance;
	}

	//------------------------------------------------------------------------------------------------
	override ForestGeneratorTreeBase Copy()
	{
		MiddleForestGeneratorClusterObject newObject = new MiddleForestGeneratorClusterObject();
		CopyValues(newObject);
		return newObject;
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class BigForestGeneratorClusterObject : MiddleForestGeneratorClusterObject
{
	[Attribute(defvalue: "10", uiwidget: UIWidgets.SpinBox, "Minimum required radius in the top layer for this object to spawn"), ForestGeneratorDistaceAttribute(DistanceType.TOP)]
	float m_fTopDistance;

	//------------------------------------------------------------------------------------------------
	override void AdjustScale()
	{
		m_fBotDistance *= m_fScale;
		m_fMidDistance *= m_fScale;
		m_fTopDistance *= m_fScale;
	}

	//------------------------------------------------------------------------------------------------
	void CopyValues(notnull BigForestGeneratorClusterObject newObject)
	{
		super.CopyValues(newObject);
		newObject.m_fTopDistance = m_fTopDistance;
	}

	//------------------------------------------------------------------------------------------------
	override ForestGeneratorTreeBase Copy()
	{
		BigForestGeneratorClusterObject newObject = new BigForestGeneratorClusterObject();
		CopyValues(newObject);
		return newObject;
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class TreeGroupClass
{
	[Attribute(defvalue: "1", uiwidget: UIWidgets.SpinBox, desc: "Weight of this group")]
	float m_fWeight;

	[Attribute(uiwidget: UIWidgets.Object, params: "noDetails")]
	ref array<ref ForestGeneratorTree> m_aTrees;
};

//------------------------------------------------------------------------------------------------
class AAB : Managed
{
	vector m_vMin;
	vector m_vMax;
	float m_fWidth; // X-axis
	float m_fDepth; // Z-axis

	//------------------------------------------------------------------------------------------------
	void AAB()
	{
		m_vMin = { float.MAX, float.MAX, float.MAX };
		m_vMax = { -float.MAX, -float.MAX, -float.MAX };
	}

	//------------------------------------------------------------------------------------------------
	void Add(vector value)
	{
		for (int i = 0; i < 3; ++i)
		{
			m_vMin[i] = Math.Min(m_vMin[i], value[i]);
			m_vMax[i] = Math.Max(m_vMax[i], value[i]);
		}
	}

	//------------------------------------------------------------------------------------------------
	vector GetSize()
	{
		return Vector(
			m_vMax[0] - m_vMin[0],
			m_vMax[1] - m_vMin[1],
			m_vMax[2] - m_vMin[2]);
	}

	//------------------------------------------------------------------------------------------------
	bool DetectCollision2D(AAB other)
	{
		return
			m_vMin[0] < other.m_vMin[0] + other.m_fWidth &&
			m_vMin[0] + m_fWidth > other.m_vMin[0] &&
			m_vMin[2] < other.m_vMin[2] + other.m_fDepth &&
			m_vMin[2] + m_fDepth > other.m_vMin[2];
	}

	//------------------------------------------------------------------------------------------------
	static AAB MakeFromPoints(array<vector> points)
	{
		AAB bb = new AAB();
		foreach (vector v : points)
		{
			bb.Add(v);
		}
		bb.m_fWidth = bb.m_vMax[0] - bb.m_vMin[0];
		bb.m_fDepth = bb.m_vMax[2] - bb.m_vMin[2];
		return bb;
	}
};

//------------------------------------------------------------------------------------------------
[EntityEditorProps(category: "GameLib/Scripted/Generator", description: "ForestGeneratorEntity", dynamicBox: true, visible: false)]
class ForestGeneratorEntityClass : SCR_GeneratorBaseEntityClass
{
};

//------------------------------------------------------------------------------------------------
class ForestGeneratorEntity : SCR_GeneratorBaseEntity
{
	// General
	[Attribute(defvalue: "42", uiwidget: UIWidgets.SpinBox, desc: "Seed used by the random generator of this forest generator", category: "General")]
	protected int m_iSeed;

	// Debug
	[Attribute(defvalue: "0", uiwidget: UIWidgets.CheckBox, category: "Debug", desc: "Print the area of this forest generator polygon?")]
	protected bool m_bPrintArea;

	[Attribute(defvalue: "0", uiwidget: UIWidgets.CheckBox, category: "Debug", desc: "Draw debug shapes of objects spawned by this forest generator?")]
	protected bool m_bDrawDebugShapes;

	[Attribute(defvalue: "0", uiwidget: UIWidgets.CheckBox, category: "Debug", desc: "Draw debug shapes of rectangulation for this forest generator?")]
	protected bool m_bDrawDebugShapesRectangulation;

	[Attribute(defvalue: "0", uiwidget: UIWidgets.CheckBox, category: "Debug", desc: "Print the count of entities spawned by this forest generator?")]
	protected bool m_bPrintEntitiesCount;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, category: "Debug", desc: "Generate forest when something changed?")]
	protected bool m_bGenerateForest;

	[Attribute(defvalue: "", uiwidget: UIWidgets.Object, desc: "Forest generator levels to spawn in this forest generator polygon")]
	protected ref array<ref ForestGeneratorLevel> m_aLevels;

	[Attribute(defvalue: "", uiwidget: UIWidgets.Object, params: "noDetails", desc: "Forest generator clusters to spawn in this forest generator polygon")]
	protected ref array<ref ForestGeneratorCluster> m_aClusters;

	// Obstacles
#ifdef _ALLOW_FOREST_REGENERATION
	[Attribute(defvalue: "0", desc: "Regenerates the forest if a road or power line spline is changed inside the forest", category: "Obstacles")]
	protected bool m_bAllowRegenerationByNearbyChanges;
#endif

	//------------------------------------------------------------------------------------------------
	protected ref ForestGeneratorGrid m_Grid;
	protected ref RandomGenerator m_RandomGenerator;
	protected ref array<ref Shape> m_aDebugShapes;
	protected ref array<ref Shape> m_aDebugShapesRectangulation;
	protected ref array<ref ForestGeneratorPoint> m_aPoints = {};
	protected ref array<ref ForestGeneratorLine> m_aLines = {};
	protected ref array<ref ForestGeneratorPoint> m_aMiddleOutlinePoints = {};
	protected ref array<ref ForestGeneratorLine> m_aSmallOutlineLines = {};
	protected ref array<ref ForestGeneratorPoint> m_aSmallOutlinePoints = {};
	protected ref array<ref ForestGeneratorLine> m_aMiddleOutlineLines = {};
	protected ref array<ref ForestGeneratorRectangle> m_aRectangles = {};
	protected ref array<ref ForestGeneratorRectangle> m_aOutlineRectangles = {};
	protected ref array<ref ForestGeneratorRectangle> m_aNonOutlineRectangles = {};
	protected ref array<ref ForestGeneratorTreeBase> m_aGridEntries = {};

	protected ref array<ref ForestGeneratorOutline> m_aOutlines = {};

	protected float m_fOutlinesWidth = 0; // max outline width
	protected bool m_bGeneratedForest = false;

	protected float m_fArea = 0;

	protected IEntity m_QueriedEntity;
	protected ref array<IEntity> m_aGeneratedEntities = {};

	protected static const float HECTARE_CONVERSION_FACTOR = 0.0001; // x/10000

	//------------------------------------------------------------------------------------------------
	bool OnLine(ForestGeneratorLine line, ForestGeneratorPoint point)
	{
		float a = Math.Max(line.p1.m_vPos[0], line.p2.m_vPos[0]);
		float b = Math.Min(line.p1.m_vPos[0], line.p2.m_vPos[0]);
		float c = Math.Max(line.p1.m_vPos[2], line.p2.m_vPos[2]);
		float d = Math.Min(line.p1.m_vPos[2], line.p2.m_vPos[2]);

		return
			point.m_vPos[0] <= a &&
			point.m_vPos[0] <= b &&
			point.m_vPos[2] <= c &&
			point.m_vPos[2] <= d;
	}

	//------------------------------------------------------------------------------------------------
	//! \return 0 for colinear, 2 for counter-clockwise, 1 for clockwise
	int Direction(ForestGeneratorPoint a, ForestGeneratorPoint b, ForestGeneratorPoint c)
	{
		int val = (b.m_vPos[2] - a.m_vPos[2]) * (c.m_vPos[0] - b.m_vPos[0]) - (b.m_vPos[0] - a.m_vPos[0]) * (c.m_vPos[2] - b.m_vPos[2]);
		if (val == 0)
			return 0; // colinear

		if (val < 0)
			return 2; // counter-clockwise direction

		return 1; // clockwise direction
	}

	//------------------------------------------------------------------------------------------------
	bool IsIntersect(ForestGeneratorLine line1, ForestGeneratorLine line2)
	{
		// four Direction for two lines and points of other line
		int dir1 = Direction(line1.p1, line1.p2, line2.p1);
		int dir2 = Direction(line1.p1, line1.p2, line2.p2);
		int dir3 = Direction(line2.p1, line2.p2, line1.p1);
		int dir4 = Direction(line2.p1, line2.p2, line1.p2);

		return
			(dir1 != dir2 && dir3 != dir4) ||			// they are intersecting
			(dir1 == 0 && OnLine(line1, line2.p1)) ||	// when p2 of line2 are on the line1
			(dir2 == 0 && OnLine(line1, line2.p2)) ||	// when p1 of line2 are on the line1
			(dir3 == 0 && OnLine(line2, line1.p1)) ||	// when p2 of line1 are on the line2
			(dir4 == 0 && OnLine(line2, line1.p2));		// when p1 of line1 are on the line2
	}

	//------------------------------------------------------------------------------------------------
	bool IsIntersect(ForestGeneratorLine line, ForestGeneratorRectangle rectangle)
	{
		return
			IsIntersect(line, rectangle.m_Line1) ||
			IsIntersect(line, rectangle.m_Line2) ||
			IsIntersect(line, rectangle.m_Line3) ||
			IsIntersect(line, rectangle.m_Line4);
	}

#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	bool PreprocessTreeArr(array<ref ForestGeneratorTree> trees, int groupIdx, TreeType type, int debugGroupIdx)
	{
		float probaSum = 0;
		bool isAnyTreeValid = false;
		for (int i = trees.Count() - 1; i >= 0; --i)
		{
			if (trees[i].m_Prefab.Length() == 0 || trees[i].m_fWeight <= 0)
			{
				trees.RemoveOrdered(i);
			}
			else
			{
				probaSum += trees[i].m_fWeight;
				trees[i].m_iGroupIndex = groupIdx;
				trees[i].m_Type = type;
				isAnyTreeValid = true;
			}
		}

		if (probaSum > 0)
		{
			for (int i = 0, count = trees.Count(); i < count; ++i)
			{
				trees[i].m_fWeight = trees[i].m_fWeight / probaSum;
			}
		}

		return isAnyTreeValid;
	}

	//------------------------------------------------------------------------------------------------
	void PreprocessAllTrees()
	{
		int groupIdx = 0;
		int groupProbaSum = 0;
		int debugGroupIdx = 0;
		int count = m_aLevels.Count();
		for (int i = 0; i < count; i++)
		{
			groupIdx = 0;
			groupProbaSum = 0;
			ForestGeneratorLevel level = m_aLevels[i];
			ForestGeneratorLevelType type = level.m_Type;
			level.m_aGroupProbas = {};

			if (type == ForestGeneratorLevelType.BOTTOM)
			{
					for (int y = 0, groupCount = level.m_aTreeGroups.Count(); y < groupCount; y++)
					{
						PreprocessTreeArr(level.m_aTreeGroups[y].m_aTrees, 0, TreeType.BOTTOM, debugGroupIdx);
					}
			}
			else
			{
				for (int x = 0; x < level.m_aTreeGroups.Count();)
				{
					if (level.m_aTreeGroups[x].m_fWeight > 0 &&
						level.m_aTreeGroups[x].m_aTrees &&
						!level.m_aTreeGroups[x].m_aTrees.IsEmpty() &&
						PreprocessTreeArr(level.m_aTreeGroups[x].m_aTrees, groupIdx, TreeType.TOP, debugGroupIdx))
					{
						groupProbaSum += level.m_aTreeGroups[x].m_fWeight;
						++groupIdx;
						++debugGroupIdx;
						++x;
					}
					else
					{
						level.m_aTreeGroups.RemoveOrdered(x);
					}
				}

				for (int y = 0, groupCount = level.m_aTreeGroups.Count(); y < groupCount; ++y)
				{
					if (groupProbaSum > 0)
						level.m_aTreeGroups[y].m_fWeight = level.m_aTreeGroups[y].m_fWeight / groupProbaSum;

					level.m_aGroupProbas.Insert(level.m_aTreeGroups[y].m_fWeight);
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override bool _WB_OnKeyChanged(BaseContainer src, string key, BaseContainerList ownerContainers, IEntity parent)
	{
		super._WB_OnKeyChanged(src, key, ownerContainers, parent);

		if (key == "coords")
			return false;

/*
		foreach (ForestGeneratorLevel level : m_aLevels)
		{
			// TODO LIMIT COUNTS
		}
*/

		WorldEditorAPI api = _WB_GetEditorAPI();
		if (!api)
			return false;

		IEntitySource thisSrc = api.EntityToSource(this);
		BaseContainerTools.WriteToInstance(this, thisSrc);

		PreprocessAllTrees();

		ShapeEntity parentShape = ShapeEntity.Cast(parent);
		if (parentShape)
		{
			IEntitySource parentSrc = api.EntityToSource(parentShape);
			OnShapeInit(parentSrc, parentShape);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		m_Grid = new ForestGeneratorGrid(10);
		PreprocessAllTrees();
	}
#endif

	//------------------------------------------------------------------------------------------------
	void ClearPoints()
	{
		m_aPoints.Clear();
		m_aSmallOutlinePoints.Clear();
		m_aMiddleOutlinePoints.Clear();
		m_aSmallOutlineLines.Clear();
		m_aMiddleOutlineLines.Clear();
	}

	//------------------------------------------------------------------------------------------------
	protected void LoadPoints(BaseContainerList points)
	{
		if (!points)
			return;

#ifdef WORKBENCH
		WorldEditorAPI api = _WB_GetEditorAPI();

		BaseContainer point;
		vector pos;
		bool smallOutline, middleOutline;
		BaseContainerList dataArr;
		bool hasPointData;
		BaseContainer data;
		bool skip;
		ForestGeneratorPoint genPoint;
		for (int i = 0, pointCount = points.Count(); i < pointCount; i++)
		{
			point = points.Get(i);
			point.Get("Position", pos);

			smallOutline = true;
			middleOutline = true;
			dataArr = point.GetObjectArray("Data");

			hasPointData = false;
			for (int j = 0, dataCount = dataArr.Count(); j < dataCount; ++j)
			{
				data = dataArr.Get(j);
				if (data.GetClassName() == "ForestGeneratorPointData")
				{
					data.Get("m_bSmallOutline", smallOutline);
					data.Get("m_bMiddleOutline", middleOutline);
					hasPointData = true;
					break;
				}
			}

			if (!hasPointData && api && !api.UndoOrRedoIsRestoring())
			{
				api.CreateObjectArrayVariableMember(point, null, "Data", "ForestGeneratorPointData", dataArr.Count());
			}

			skip = false;
			foreach (ForestGeneratorPoint curPoint : m_aPoints)
			{
				if (curPoint.m_vPos == pos)
				{
					Print("Found two points on the same position: " + pos + ", Skipping.", LogLevel.WARNING);
					skip = true;
					break;
				}
			}
			if (skip)
				continue;

			genPoint = new ForestGeneratorPoint();
			pos[1] = 0;
			genPoint.m_vPos = pos;
			genPoint.m_bSmallOutline = smallOutline;
			genPoint.m_bMiddleOutline = middleOutline;

			m_aPoints.Insert(genPoint);
		}
#endif
	}

	//------------------------------------------------------------------------------------------------
	//! TODO: change initialisation
	//! NO WORKBENCH API OPERATIONS SHOULD HAPPEN FROM HERE
	//! see GeneratorBaseEntity.OnShapeInitInternal's documentation
	override void OnShapeInitInternal(IEntitySource shapeEntitySrc, ShapeEntity shapeEntity)
	{
		super.OnShapeInitInternal(shapeEntitySrc, shapeEntity);

#ifdef WORKBENCH
		if (!m_bGenerateForest)
			return;

		ClearPoints();
		LoadOutlines();

		m_fOutlinesWidth = 0;
		foreach (ForestGeneratorOutline outline : m_aOutlines)
		{
			if (m_fOutlinesWidth < outline.m_fMinDistance)
				m_fOutlinesWidth = outline.m_fMinDistance;
			if (m_fOutlinesWidth < outline.m_fMaxDistance)
				m_fOutlinesWidth = outline.m_fMaxDistance;
		}

		WorldEditorAPI api = _WB_GetEditorAPI();
		if (!api || api.UndoOrRedoIsRestoring())
			return;

		m_RandomGenerator.SetSeed(m_iSeed);

		BaseContainerList points = shapeEntitySrc.GetObjectArray("Points");

		LoadPoints(points);
		ClockwiseCheck();

		ForestGeneratorPoint genPoint;
		ForestGeneratorLine line = new ForestGeneratorLine();
		for (int i = 0, count = m_aPoints.Count(); i < count; i++)
		{
			genPoint = m_aPoints[i];

			if (i > 0)
			{
				line.p2 = genPoint;
				line.m_fLength = (line.p2.m_vPos - line.p1.m_vPos).Length();
				genPoint.m_Line1 = line;
				m_aLines.Insert(line);

				if (genPoint.m_bSmallOutline)
					m_aSmallOutlinePoints.Insert(genPoint);

				if (genPoint.m_bMiddleOutline)
					m_aMiddleOutlinePoints.Insert(genPoint);

				if (line.p1.m_bSmallOutline)
					m_aSmallOutlineLines.Insert(line);

				if (line.p1.m_bMiddleOutline)
					m_aMiddleOutlineLines.Insert(line);
			}

			line = new ForestGeneratorLine();
			line.p1 = genPoint;
			genPoint.m_Line2 = line;
		}

		if (m_aPoints.IsEmpty())
			return;

		genPoint = m_aPoints[0];
		line.p2 = genPoint;
		genPoint.m_Line1 = line;
		line.m_fLength = (line.p2.m_vPos - line.p1.m_vPos).Length();
		m_aLines.Insert(line);

		if (genPoint.m_bSmallOutline)
			m_aSmallOutlinePoints.Insert(genPoint);

		if (genPoint.m_bMiddleOutline)
			m_aMiddleOutlinePoints.Insert(genPoint);

		if (line.p1.m_bSmallOutline)
			m_aSmallOutlineLines.Insert(line);

		if (line.p1.m_bMiddleOutline)
			m_aMiddleOutlineLines.Insert(line);

		CalculateOutlineAnglesForPoints();

		// generate the forest
		m_aDebugShapes = {};
		array<vector> polygon = GetPoints(shapeEntitySrc);

		Print("ForestGenerator - Populating grid", LogLevel.DEBUG);
		Debug.BeginTimeMeasure();
		PopulateGrid(polygon);
		Debug.EndTimeMeasure("ForestGenerator - Populating grid done");

		Print("ForestGenerator - Generating entities", LogLevel.DEBUG);
		Debug.BeginTimeMeasure();
		GenerateEntities();
		Debug.EndTimeMeasure("ForestGenerator - Generating entities done");
#endif
	}

	//------------------------------------------------------------------------------------------------
	private void CalculateOutlineAnglesForPoints()
	{
		int count = m_aPoints.Count();
		if (count < 3)
			return;

		ForestGeneratorPoint previousPoint;
		ForestGeneratorPoint currentPoint;
		ForestGeneratorPoint nextPoint;
		vector dir1;
		vector dir2;
		float yaw1;
		float yaw2;
		for (int i = 0; i < count; i++)
		{
			if (i > 0)
				previousPoint = m_aPoints[i-1];
			else
				previousPoint = m_aPoints[count-1];

			currentPoint = m_aPoints[i];
			if (i < count-1)
				nextPoint = m_aPoints[i+1];
			else
				nextPoint = m_aPoints[0];

			dir1 = previousPoint.m_vPos - currentPoint.m_vPos;
			dir2 = nextPoint.m_vPos - currentPoint.m_vPos;
			yaw1 = dir1.ToYaw();
			yaw2 = dir2.ToYaw();
			if (yaw1 > yaw2)
			{
				currentPoint.m_fMinAngle = yaw1 - 360;
				currentPoint.m_fMaxAngle = yaw2;
			}
			else
			{
				currentPoint.m_fMinAngle = yaw1;
				currentPoint.m_fMaxAngle = yaw2;
			}

			currentPoint.m_fAngle = Math.AbsFloat(currentPoint.m_fMaxAngle - currentPoint.m_fMinAngle);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnShapeChangedInternal(IEntitySource shapeEntitySrc, ShapeEntity shapeEntity, array<vector> mins, array<vector> maxes)
	{
		// TODO handle this case better if needed, use the bbox arrays
		OnShapeInitInternal(shapeEntitySrc, shapeEntity);
	}

	//------------------------------------------------------------------------------------------------
	int GetColorForTree(int index, TreeType type)
	{
		int colCount = 11;
		int colIdx = ((int)TreeType.COUNT * index + (int)type) % colCount;
		int color;
		switch (colIdx)
		{
			case 0:		color = 0xff56e3d7; break;
			case 1:		color = 0xff428af5; break;
			case 2:		color = 0xfff57542; break;
			case 3:		color = 0xff8ae356; break;
			case 4:		color = 0xff2f636b; break;
			case 5:		color = 0xff818491; break;
			case 6:		color = 0xffed9dbb; break;
			case 7:		color = 0xff0009ab; break;
			case 8:		color = 0xffab003c; break;
			case 9:		color = 0xffa8ab00; break;
			case 10:	color = 0xffffffff; break;
			default:	color = 0xffffffff; break;
		}
		return color;
	}

	//------------------------------------------------------------------------------------------------
	void GenerateEntities()
	{
#ifdef WORKBENCH
		WorldEditorAPI api = _WB_GetEditorAPI();

		// delete old trees
		m_aGeneratedEntities.Clear();

		api.BeginEditSequence(m_Source);

		for (int i = m_Source.GetNumChildren() - 1; i >= 0; --i)
		{
			api.DeleteEntity(api.SourceToEntity(m_Source.GetChild(i)));
		}

		m_aLines.Clear();
		m_aSmallOutlineLines.Clear();
		m_aMiddleOutlineLines.Clear();
		m_aRectangles.Clear();
		m_aOutlineRectangles.Clear();
		m_aSmallOutlinePoints.Clear();
		m_aPoints.Clear();
		m_aMiddleOutlinePoints.Clear();
		m_aNonOutlineRectangles.Clear();

		// create new trees
		int topLevelEntitiesCount = 0;
		int bottomLevelEntitiesCount = 0;
		int smallOutlineEntitiesCount = 0;
		int middleOutlineEntitiesCount = 0;
		int clusterEntitiesCount = 0;
		int generatedEntitiesCount = 0;

		m_aDebugShapes.Clear();
		BaseWorld world = api.GetWorld();
		if (!world)
		{
			api.EndEditSequence(m_Source);
			return;
		}

		vector worldPos, localPos;
		SCR_ForestGeneratorTreeBase baseEntry;
		ForestGeneratorTree entry;
		FallenTree fallenTree;
		WideForestGeneratorClusterObject wideObject;

		IEntitySource source;
		IEntity entity;
		ShapeEntity splineEntity;

		IEntity tree;
		IEntitySource treeSrc;

		BaseContainerList baseContainer;
		bool alignToNormal, randomYaw;
		float yaw, pitch, roll;
		vector randomVerticalOffset;

		vector mat[4];
		vector newUp, newRight, newForward, angles;

		TraceParam traceParam = new TraceParam();
		traceParam.Flags = TraceFlags.WORLD;

		RefreshObstacles();
		for (int i = 0, count = m_Grid.GetEntryCount(); i < count; ++i)
		{
			baseEntry = m_Grid.GetEntry(i, worldPos);
			entry = ForestGeneratorTree.Cast(baseEntry);
			fallenTree = FallenTree.Cast(baseEntry);
			wideObject = WideForestGeneratorClusterObject.Cast(baseEntry);

			// snap the tree to terrain
			worldPos[1] = world.GetSurfaceY(worldPos[0], worldPos[2]);

			switch (baseEntry.m_Type)
			{
				case TreeType.TOP:		topLevelEntitiesCount++; break;
				case TreeType.BOTTOM:	bottomLevelEntitiesCount++; break;
				case TreeType.MOUTLINE:	middleOutlineEntitiesCount++; break;
				case TreeType.SOUTLINE:	smallOutlineEntitiesCount++; break;
				case TreeType.CLUSTER:	clusterEntitiesCount++; break;
			}

			if (baseEntry.m_Prefab.IsEmpty())
				continue;

			if (HasObstacle(worldPos, m_aGeneratedEntities))
				continue;

			localPos = CoordToLocal(worldPos);
			localPos[1] = localPos[1] + baseEntry.m_fVerticalOffset;

			tree = api.CreateEntity(baseEntry.m_Prefab, "", api.GetCurrentEntityLayerId(), m_Source, localPos, vector.Zero);
			treeSrc = api.EntityToSource(tree);
			api.BeginEditSequence(treeSrc);
			m_aGeneratedEntities.Insert(tree);
			generatedEntitiesCount++;

			api.ModifyEntityKey(tree, "scale", baseEntry.m_fScale.ToString());

			baseContainer = treeSrc.GetObjectArray("editorData");

			randomVerticalOffset = vector.Zero;
			alignToNormal = false;
			randomYaw = false;
			if (baseContainer && baseContainer.Count() > 0)
			{
				baseContainer.Get(0).Get("randomVertOffset", randomVerticalOffset);
				baseContainer.Get(0).Get("alignToNormal", alignToNormal);
				baseContainer.Get(0).Get("randomYaw", randomYaw);
			}

			if (randomVerticalOffset != vector.Zero)
			{
				localPos[1] = localPos[1] + Math.RandomFloat(randomVerticalOffset[0], randomVerticalOffset[1]);
				api.ModifyEntityKey(tree, "coords", localPos.ToString(false));
			}

			if (fallenTree)
				alignToNormal = fallenTree.m_bAlignToNormal;

			if (wideObject)
				alignToNormal = wideObject.m_bAlignToNormal;

			if (randomYaw)
			{
				if (wideObject)
					yaw = -wideObject.m_fYaw;
				else if (fallenTree)
					yaw = -fallenTree.m_fYaw; // clockwise / counter-clockwise
				else
					yaw = m_RandomGenerator.RandFloatXY(0, 360);

				api.ModifyEntityKey(tree, "angleY", yaw.ToString());
			}

			if (baseEntry.m_fRandomPitchAngle > 0)
				pitch = m_RandomGenerator.RandFloatXY(-baseEntry.m_fRandomPitchAngle, baseEntry.m_fRandomPitchAngle);
			else
				pitch = 0;

			if (baseEntry.m_fRandomRollAngle > 0)
				roll = m_RandomGenerator.RandFloatXY(-baseEntry.m_fRandomRollAngle, baseEntry.m_fRandomRollAngle);
			else
				roll = 0;

			api.ModifyEntityKey(tree, "angleX", pitch.ToString());
			api.ModifyEntityKey(tree, "angleZ", roll.ToString());

			if (alignToNormal)
			{
				traceParam.Start = worldPos + vector.Up;
				traceParam.End = worldPos - vector.Up;
				world.TraceMove(traceParam, null);
				tree.GetTransform(mat);

				newUp = traceParam.TraceNorm;
				newUp.Normalize();

				// Shape shape = Shape.Create(ShapeType.LINE, ARGB(255, 0, 255, 0), ShapeFlags.NOZBUFFER, worldPos, worldPos + newUp);
				// m_aDebugShapes.Insert(shape);
				newRight = newUp * mat[2];
				newRight.Normalize();
				// shape = Shape.Create(ShapeType.LINE, ARGB(255, 255, 0, 0), ShapeFlags.NOZBUFFER, worldPos, worldPos + newRight);
				// m_aDebugShapes.Insert(shape);
				newForward = newRight * newUp;
				newForward.Normalize();
				// shape = Shape.Create(ShapeType.LINE, ARGB(255, 0, 0, 255), ShapeFlags.NOZBUFFER, worldPos, worldPos + newForward);
				// m_aDebugShapes.Insert(shape);

				mat[0] = newRight;
				mat[1] = newUp;
				mat[2] = newForward;

				angles = Math3D.MatrixToAngles(mat);

				api.ModifyEntityKey(tree, "angleX", angles[1].ToString());
				api.ModifyEntityKey(tree, "angleY", angles[0].ToString());
				api.ModifyEntityKey(tree, "angleZ", angles[2].ToString());
			}

			api.EndEditSequence(treeSrc);
			if (m_bDrawDebugShapes)
			{
				Shape s = Shape.CreateSphere(
					GetColorForTree(baseEntry.m_iGroupIndex, baseEntry.m_Type),
					ShapeFlags.VISIBLE | ShapeFlags.NOOUTLINE,
					worldPos,
					1 + (int)TreeType.COUNT - (int)baseEntry.m_Type);
				m_aDebugShapes.Insert(s);
			}
		}
		ClearObstacles(); // frees RAM

		api.EndEditSequence(m_Source);

		if (m_bPrintEntitiesCount)
		{
			Print("Forest generator generated: " + topLevelEntitiesCount + " entities in top level.", LogLevel.NORMAL);
			Print("Forest generator generated: " + bottomLevelEntitiesCount + " entities in bottom level.", LogLevel.NORMAL);
			Print("Forest generator generated: " + middleOutlineEntitiesCount + " entities in middle outline.", LogLevel.NORMAL);
			Print("Forest generator generated: " + smallOutlineEntitiesCount + " entities in small outline.", LogLevel.NORMAL);
			Print("Forest generator generated: " + clusterEntitiesCount + " entities in clusters.", LogLevel.NORMAL);
			Print("Forest generator generated: " + generatedEntitiesCount + " entities in total.", LogLevel.NORMAL);
		}
#endif
	}

	//------------------------------------------------------------------------------------------------
	void PopulateGrid(array<vector> polygon)
	{
		m_Grid.Clear();
		m_aGridEntries.Clear();

		// we need to convert the polygon to 2D to have efficient IsPointInPolygon queries
		array<float> polygon2D = {};
		foreach (vector polyPoint : polygon)
		{
			polygon2D.Insert(polyPoint[0]);
			polygon2D.Insert(polyPoint[2]);
		}

		array<float> a = {};
		array<float> b = {};
		for (int i = 0, count = polygon2D.Count(); i < count; i += 2) // step 2
		{
			a.Insert(polygon2D[i]);
			b.Insert(polygon2D[i + 1]);
		}

		m_fArea = Math.AbsFloat(PolygonArea(a, b));

		if (m_bPrintArea)
			Print("Area of the polygon is: " + m_fArea + " square meters.", LogLevel.NORMAL);

		AAB bbox = AAB.MakeFromPoints(polygon);
		vector bboxSize = bbox.GetSize();
		m_Grid.Resize(bboxSize[0], bboxSize[2]);

		Debug.BeginTimeMeasure();
		Rectangulate(bbox, polygon2D);
		Debug.EndTimeMeasure("ForestGenerator - Rectangulation done");

		vector worldMat[4];
		GetWorldTransform(worldMat);
		vector bboxMin = bbox.m_vMin;
		vector bboxMinWorld = bboxMin.Multiply4(worldMat);
		m_Grid.SetPointOffset(bboxMinWorld[0], bboxMinWorld[2]);

		GenerateTrees(polygon2D, bbox);
	}

	//------------------------------------------------------------------------------------------------
	void Rectangulate(AAB bbox, array<float> polygon2D)
	{
		vector direction = bbox.m_vMax - bbox.m_vMin;
		float targetRectangleWidth = 50;
		float targetRectangleLength = 50;
		int targetRectangleCountW = Math.Ceil(direction[0] / targetRectangleWidth);
		int targetRectangleCountL = Math.Ceil(direction[2] / targetRectangleLength);
		bool areLineArraysIdentical = AreIdentical(m_aSmallOutlineLines, m_aMiddleOutlineLines);

		vector ownerOrigin = GetOrigin();
		m_aDebugShapesRectangulation = {};

		bool isInPolygon;
		ForestGeneratorRectangle rectangle;
		vector p1;
		int red, green, blue;
		bool found;
		Shape shape;
		for (int x = 0; x < targetRectangleCountW; x++)
		{
			for (int y = 0; y < targetRectangleCountL; y++)
			{
				isInPolygon = false;
				rectangle = new ForestGeneratorRectangle();
				rectangle.m_iX = x;
				rectangle.m_iY = y;
				rectangle.m_fWidth = targetRectangleWidth;
				rectangle.m_fLength = targetRectangleLength;
				rectangle.m_fArea = rectangle.m_fLength * rectangle.m_fWidth;
				p1 = bbox.m_vMin;
				p1[0] = p1[0] + (x * targetRectangleWidth);
				p1[2] = p1[2] + (y * targetRectangleLength);
				rectangle.m_Line4.p2.m_vPos = p1;
				rectangle.m_Line1.p1.m_vPos = p1;
				if (!isInPolygon)
					isInPolygon = Math2D.IsPointInPolygon(polygon2D, p1[0], p1[2]);
				rectangle.m_aPoints.Insert(p1);

				p1[0] = p1[0] + targetRectangleWidth;
				rectangle.m_Line1.p2.m_vPos = p1;
				rectangle.m_Line2.p1.m_vPos = p1;
				if (!isInPolygon)
					isInPolygon = Math2D.IsPointInPolygon(polygon2D, p1[0], p1[2]);
				rectangle.m_aPoints.Insert(p1);

				p1[2] = p1[2] + targetRectangleLength;
				rectangle.m_Line2.p2.m_vPos = p1;
				rectangle.m_Line3.p1.m_vPos = p1;
				if (!isInPolygon)
					isInPolygon = Math2D.IsPointInPolygon(polygon2D, p1[0], p1[2]);
				rectangle.m_aPoints.Insert(p1);

				p1[0] = p1[0] - targetRectangleWidth;
				rectangle.m_Line3.p2.m_vPos = p1;
				rectangle.m_Line4.p1.m_vPos = p1;
				if (!isInPolygon)
					isInPolygon = Math2D.IsPointInPolygon(polygon2D, p1[0], p1[2]);
				rectangle.m_aPoints.Insert(p1);

				red = 0;
				green = Math.Floor(m_RandomGenerator.RandFloatXY(0, 255));
				blue = Math.Floor(m_RandomGenerator.RandFloatXY(0, 255));

				foreach (ForestGeneratorLine line : m_aLines)
				{
					if (!NeedsCheck(line, rectangle) || !IsIntersect(line, rectangle))
						continue;

					found = false;
					foreach (ForestGeneratorLine rectLine : rectangle.m_aLines)
					{
						if (rectLine == line)
						{
							found = true;
							break;
						}
					}
					if (!found)
						rectangle.m_aLines.Insert(line);
				}

				if (rectangle.m_aLines.IsEmpty() && !isInPolygon)
					continue;

				m_aRectangles.Insert(rectangle);

				if (!rectangle.m_aLines.IsEmpty())
				{
					red = 255;
					blue = 0;
					green = 0;

					m_aOutlineRectangles.Insert(rectangle);
				}
				else
				{
					m_aNonOutlineRectangles.Insert(rectangle);
				}

				if (m_bDrawDebugShapesRectangulation)
				{
					shape = Shape.Create(ShapeType.BBOX, ARGB(255, red, green, blue), ShapeFlags.NOZBUFFER, ownerOrigin + rectangle.m_Line1.p1.m_vPos, ownerOrigin + rectangle.m_Line3.p1.m_vPos);
					m_aDebugShapesRectangulation.Insert(shape);
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	bool NeedsCheck(ForestGeneratorLine line, ForestGeneratorRectangle rectangle)
	{
		vector linePoint1 = line.p1.m_vPos;
		vector linePoint2 = line.p2.m_vPos;
		vector mins;
		vector maxs;
		rectangle.GetBounds(mins, maxs);

		float linePoint1x = linePoint1[0];
		float linePoint1z = linePoint1[2];
		float linePoint2x = linePoint2[0];
		float linePoint2z = linePoint2[2];
		float minsx = mins[0];
		float minsz = mins[2];
		float maxsx = maxs[0];
		float maxsz = maxs[2];

		if (linePoint1x < minsx && linePoint2x < minsx)
			return false;

		if (linePoint1x > maxsx && linePoint2x > maxsx)
			return false;

		if (linePoint1z < minsz && linePoint2z < minsz)
			return false;

		if (linePoint1z > maxsz && linePoint2z > maxsz)
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool AreIdentical(notnull array<ref ForestGeneratorLine> array1, notnull array<ref ForestGeneratorLine> array2)
	{
		int count = array1.Count();
		if (count != array2.Count())
			return false;

		for (int i = 0; i < count; i++)
		{
			if (array1[i] != array2[i])
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected float PolygonArea(array<float> x, array<float> y)
	{
		float area;

		int j;
		for (int i = 0, count = x.Count(); i < count; ++i)
		{
			j = (i + 1) % count;
			area += 0.5 * (x[i] * y[j] - x[j] * y[i]);
		}

		return area;
	}

	//------------------------------------------------------------------------------------------------
	protected void GenerateTrees(array<float> polygon2D, AAB bbox)
	{
		vector bboxSize = bbox.GetSize();
		float area = bboxSize[0] * bboxSize[2];
		if (area <= 0.01)
			return;

		GenerateClusters(polygon2D, bbox);

		ForestGeneratorLevel level;
		ForestGeneratorLevelType type;
		int iterCount;
		for (int i = 0, count = m_aLevels.Count(); i < count; i++)
		{
			level = m_aLevels[i];
			type = level.m_Type;

			if (!level.m_bGenerate)
				continue;

			iterCount = level.m_fDensity * area * HECTARE_CONVERSION_FACTOR;

			switch (type)
			{
				case ForestGeneratorLevelType.TOP:
					GenerateTopTrees(polygon2D, bbox, ForestGeneratorTopLevel.Cast(level));
					break;

				case ForestGeneratorLevelType.OUTLINE:
					GenerateOutlineTrees(polygon2D, bbox, ForestGeneratorOutline.Cast(level));
					break;

				case ForestGeneratorLevelType.BOTTOM:
					GenerateBottomTrees(polygon2D, bbox, ForestGeneratorBottomLevel.Cast(level));
					area = 0;
					break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void GenerateClusters(array<float> polygon2D, AAB bbox)
	{
		ForestGeneratorCluster cluster;
		ForestGeneratorClusterType type;
		for (int i = 0, count = m_aClusters.Count(); i < count; i++)
		{
			cluster = m_aClusters[i];
			if (!cluster.m_bGenerate)
				continue;

			type = cluster.GetClusterType();
			switch (type)
			{
				case ForestGeneratorClusterType.CIRCLE:	GenerateCircleCluster(ForestGeneratorCircleCluster.Cast(cluster), polygon2D, bbox); break;
				case ForestGeneratorClusterType.STRIP:	GenerateStripCluster(ForestGeneratorStripCluster.Cast(cluster), polygon2D, bbox); break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected int FindRectanglesInCircle(vector center, float radius, out array<ForestGeneratorRectangle> rectangles)
	{
		int count = 0;
		float deltaX, deltaY;

		foreach (ForestGeneratorRectangle rectangle : m_aRectangles)
		{
			foreach (vector point : rectangle.m_aPoints)
			{
				deltaX = center[0] - Math.Max(rectangle.m_Line1.p1.m_vPos[0], Math.Min(center[0], rectangle.m_Line1.p1.m_vPos[0] + rectangle.m_fWidth));
				deltaY = center[2] - Math.Max(rectangle.m_Line1.p1.m_vPos[2], Math.Min(center[2], rectangle.m_Line1.p1.m_vPos[2] + rectangle.m_fLength));

				if (deltaX * deltaX + deltaY * deltaY < radius * radius)
				{
					rectangles.Insert(rectangle);
					count++;
				}
			}
		}

		return count;
	}

	//------------------------------------------------------------------------------------------------
	protected bool GetPointOutsideOutlines(notnull array<float> polygon2D, AAB bbox, out vector clusterCenter, int numberOfTries = 10, float aditionalDistance = 0)
	{
		bool isInOutline = true;
		int maxTriesToFindCenter = 10; // TODO replace with proper parameter.
		int currentTriesCount = 0;

		array<ForestGeneratorRectangle> rectangles;
		ForestGeneratorRectangle rectangle;
		int rectanglesCount;
		while (isInOutline)
		{
			clusterCenter = GeneratePointInPolygon(polygon2D, bbox);
			rectangles = {};

			rectanglesCount = FindRectanglesInCircle(clusterCenter, aditionalDistance + m_fOutlinesWidth, rectangles);
			isInOutline = false;

			for (int i = 0; i < rectanglesCount; i++)
			{
				rectangle = rectangles[i];
				if (IsInOutline(rectangle, clusterCenter, aditionalDistance))
				{
					isInOutline = true;
					break;
				}
			}

			currentTriesCount++;

			if (currentTriesCount >= maxTriesToFindCenter)
				break;
		}

		return currentTriesCount < maxTriesToFindCenter;
	}

	//------------------------------------------------------------------------------------------------
	private void GenerateCircleCluster(ForestGeneratorCircleCluster cluster, array<float> polygon2D, AAB bbox)
	{
		vector worldMat[4];
		GetWorldTransform(worldMat);

		float CDENSHA = 0;
		if (m_RandomGenerator)
			CDENSHA = SafeRandomFloat(cluster.m_fMinCDENSHA, cluster.m_fMaxCDENSHA);

		int clusterCount = Math.Ceil(m_fArea * HECTARE_CONVERSION_FACTOR * CDENSHA);
		vector clusterCenter;
		SmallForestGeneratorClusterObject clusterObject;
		int objectCount;
		vector pointLocal;
		SmallForestGeneratorClusterObject newClusterObject;
		vector point;
		WideForestGeneratorClusterObject wideObject;
		for (int c = 0; c < clusterCount; c++)
		{
			if (!GetPointOutsideOutlines(polygon2D, bbox, clusterCenter, aditionalDistance: cluster.m_fRadius))
				continue;

			for (int x = 0; x < cluster.m_aObjects.Count(); x++)
			{
				clusterObject = cluster.m_aObjects[x];
				objectCount = SafeRandomInt(clusterObject.m_iMinCount, clusterObject.m_iMaxCount);

				for (int o = 0; o < objectCount; o++)
				{
					pointLocal = GeneratePointInCircle(clusterObject.m_fMinRadius, clusterObject.m_fMaxRadius, clusterCenter);
					if (Math2D.IsPointInPolygon(polygon2D, pointLocal[0], pointLocal[2]))
					{
						newClusterObject = SmallForestGeneratorClusterObject.Cast(cluster.m_aObjects[x].Copy());
						if (!newClusterObject)
							continue;

						m_aGridEntries.Insert(newClusterObject);
						point = pointLocal.Multiply4(worldMat);

						SetObjectScale(newClusterObject);

						wideObject = WideForestGeneratorClusterObject.Cast(newClusterObject);
						if (wideObject)
						{
							wideObject.m_fYaw = m_RandomGenerator.RandFloat01() * 360;
							wideObject.Rotate();
						}

						if (m_Grid.IsColliding(point, newClusterObject))
							continue;

						newClusterObject.m_Type = TreeType.CLUSTER;

						m_Grid.AddEntry(newClusterObject, point);
					}
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	private void GenerateStripCluster(ForestGeneratorStripCluster cluster, array<float> polygon2D, AAB bbox)
	{
		vector worldMat[4];
		GetWorldTransform(worldMat);

		float radius = 0;
		foreach (SmallForestGeneratorClusterObject clusterObject : cluster.m_aObjects)
		{
			if (clusterObject.m_fMinRadius > radius)
				radius = clusterObject.m_fMinRadius;

			if (clusterObject.m_fMaxRadius > radius)
				radius = clusterObject.m_fMaxRadius;
		}

		if (radius == 0)
			return;

		float yaw = Math.RandomFloat(0, 360);
		vector direction = vector.FromYaw(yaw);
		vector perpendicular;
		perpendicular[0] = direction[2];
		perpendicular[2] = -direction[0];

		float CDENSHA = 0;
		if (m_RandomGenerator)
			CDENSHA = SafeRandomFloat(cluster.m_fMinCDENSHA, cluster.m_fMaxCDENSHA);

		int clusterCount = Math.Ceil(m_fArea * HECTARE_CONVERSION_FACTOR * CDENSHA);
		vector clusterCenter;
		SmallForestGeneratorClusterObject clusterObject;
		int objectCount;
		vector pointLocal;
		float distance;
		int rnd;
		float y01, y360, ySin, y;
		vector offset;
		vector point;
		SmallForestGeneratorClusterObject newClusterObject;
		WideForestGeneratorClusterObject wideObject;
		for (int c = 0; c < clusterCount; c++)
		{
			if (!GetPointOutsideOutlines(polygon2D, bbox, clusterCenter, aditionalDistance: cluster.m_fRadius))
				continue;

			for (int x = 0; x < cluster.m_aObjects.Count(); x++)
			{
				clusterObject = cluster.m_aObjects[x];
				objectCount = SafeRandomInt(clusterObject.m_iMinCount, clusterObject.m_iMaxCount);

				for (int o = 0; o < objectCount; o++)
				{
					distance = SafeRandomFloat(clusterObject.m_fMinRadius, clusterObject.m_fMaxRadius);
					rnd = Math.RandomIntInclusive(0, 1);

					if (rnd == 0)
						distance *= -1;

					y01 = distance / radius * cluster.m_fFrequency;
					y360 = y01 * 360;
					ySin = Math.Sin(y360 * Math.DEG2RAD);
					y = ySin * cluster.m_fAmplitude;

					offset = vector.Zero;
					offset[0] = Math.RandomFloat(0, cluster.m_fMaxXOffset);
					offset[2] = Math.RandomFloat(0, cluster.m_fMaxYOffset);
					pointLocal = (direction * distance) + (y * perpendicular) + clusterCenter + offset;

					if (Math2D.IsPointInPolygon(polygon2D, pointLocal[0], pointLocal[2]))
					{
						newClusterObject = SmallForestGeneratorClusterObject.Cast(cluster.m_aObjects[x].Copy());
						if (!newClusterObject)
							continue;

						m_aGridEntries.Insert(newClusterObject);
						point = pointLocal.Multiply4(worldMat);

						SetObjectScale(newClusterObject);

						wideObject = WideForestGeneratorClusterObject.Cast(newClusterObject);
						if (wideObject)
						{
							wideObject.m_fYaw = m_RandomGenerator.RandFloatXY(0, 360);
							wideObject.Rotate();
						}

						if (m_Grid.IsColliding(point, newClusterObject))
							continue;

						newClusterObject.m_Type = TreeType.CLUSTER;
						m_Grid.AddEntry(newClusterObject, point);
					}
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	private vector GeneratePointInPolygon(notnull array<float> polygon2D, notnull AAB bbox)
	{
		vector point = GeneratePointInBbox(bbox);
		while (!Math2D.IsPointInPolygon(polygon2D, point[0], point[2]))
		{
			point = GeneratePointInBbox(bbox);
		}
		return point;
	}

	//------------------------------------------------------------------------------------------------
	private vector GeneratePointInBbox(notnull AAB bbox)
	{
		return Vector(
			Math.RandomFloat(bbox.m_vMin[0], bbox.m_vMax[0]),
			Math.RandomFloat(bbox.m_vMin[1], bbox.m_vMax[1]),
			Math.RandomFloat(bbox.m_vMin[2], bbox.m_vMax[2]));
	}

	//------------------------------------------------------------------------------------------------
	private vector GeneratePointAlongLine(vector start, vector direction, vector perpendicular, float minDist, float maxDist)
	{
		float distanceOnLine = m_RandomGenerator.RandFloat01();
		float distanceFromLine = SafeRandomFloat(minDist, maxDist);
		return start + (direction * distanceOnLine) + (perpendicular * distanceFromLine);
	}

	//------------------------------------------------------------------------------------------------
	private float SafeRandomFloat(float min, float max)
	{
		if (min < max)
			return m_RandomGenerator.RandFloatXY(min, max);

		if (min == max)
			return max;

		Print("Some of your forest generator min value > max value.", LogLevel.WARNING);
		return m_RandomGenerator.RandFloatXY(max, min);
	}

	//------------------------------------------------------------------------------------------------
	private int SafeRandomInt(int min, int max)
	{
		if (min < max)
			return Math.RandomIntInclusive(min, max);

		if (min == max)
			return max;

		Print("Your forest generator object has some min value > max value.", LogLevel.WARNING);
		return Math.RandomIntInclusive(max, min);
	}

	//------------------------------------------------------------------------------------------------
	private vector GeneratePointInCircle(float innerRadius, float outerRadius, vector circleCenter)
	{
		vector direction = GenerateRandomVectorBetweenAngles(0, 360);
		float rand = SafeRandomFloat(innerRadius, outerRadius);
		return circleCenter + rand * direction;
	}

	//------------------------------------------------------------------------------------------------
	private vector GeneratePointInCircle(float innerRadius, float outerRadius, ForestGeneratorPoint point)
	{
		vector direction = GenerateRandomVectorBetweenAngles(point.m_fMinAngle, point.m_fMaxAngle);
		float rand = SafeRandomFloat(innerRadius, outerRadius);
		return point.m_vPos + rand * direction;
	}

	//------------------------------------------------------------------------------------------------
	private vector GenerateRandomVectorBetweenAngles(float angle1, float angle2)
	{
		return vector.FromYaw(m_RandomGenerator.RandFloatXY(angle1, angle2));
	}

	//------------------------------------------------------------------------------------------------
	private vector GenerateRandomPointInRectangle(notnull ForestGeneratorRectangle rectangle)
	{
		return Vector(
			m_RandomGenerator.RandFloat01() * rectangle.m_fWidth + rectangle.m_Line1.p1.m_vPos[0],
			0,
			m_RandomGenerator.RandFloat01() * rectangle.m_fLength + rectangle.m_Line1.p1.m_vPos[2]);
	}

	//------------------------------------------------------------------------------------------------
	bool GetIsAnyTreeValid(notnull array<ref TreeGroupClass> treeGroups)
	{
		ForestGeneratorTree tree;
		for (int i = 0, iCnt = treeGroups.Count(); i < iCnt; ++i)
		{
			if (treeGroups[i].m_fWeight <= 0)
				continue;

			for (int j = 0, jCnt = treeGroups[i].m_aTrees.Count(); j < jCnt; ++j)
			{
				tree = treeGroups[i].m_aTrees[j];
				if (tree.m_Prefab.Length() > 0 && tree.m_fWeight > 0)
					return true;
			}
		}
		return false;
	}

	//------------------------------------------------------------------------------------------------
	void GenerateOutlineTrees(array<float> polygon, AAB bbox, ForestGeneratorOutline outline)
	{
		if (!outline || !outline.m_aTreeGroups || outline.m_aTreeGroups.Count() < 1)
			return;

		if (!GetIsAnyTreeValid(outline.m_aTreeGroups))
			return;

		array<ref ForestGeneratorPoint> currentOutlinePoints;
		array<ref ForestGeneratorLine> currentOutlineLines;

		switch (outline.m_OutlineType)
		{
			case ForestGeneratorOutlineType.SMALL:
			{
				currentOutlinePoints = m_aSmallOutlinePoints;
				currentOutlineLines = m_aSmallOutlineLines;
				break;
			}
			case ForestGeneratorOutlineType.MIDDLE:
			{
				currentOutlinePoints = m_aMiddleOutlinePoints;
				currentOutlineLines = m_aMiddleOutlineLines;
				break;
			}
		}

		if (!currentOutlinePoints || !currentOutlineLines)
			return;

		array<float> groupProbas = {};
		array<float> groupCounts = {};
		groupCounts.Resize(outline.m_aTreeGroups.Count());

		vector worldMat[4];
		GetWorldTransform(worldMat);

		int throwAwayCount = 0;
		int linesCount = currentOutlineLines.Count();
		int iterCount = 0;

		vector direction, perpendicular, pointLocal, point;
		float probaSumToNormalize, groupProba, probaSum;
		int groupIdx;
		ForestGeneratorTree tree;
		for (int lineIter = 0; lineIter < linesCount; lineIter++)
		{
			direction = currentOutlineLines[lineIter].p2.m_vPos - currentOutlineLines[lineIter].p1.m_vPos;
			perpendicular = Vector(direction[2], 0, -direction[0]);
			perpendicular.Normalize();
			iterCount = outline.m_fDensity * (CalculateAreaForOutline(currentOutlineLines[lineIter], outline) * HECTARE_CONVERSION_FACTOR);
			for (int treeIdx = 0; treeIdx < iterCount; ++treeIdx)
			{
				pointLocal = GeneratePointAlongLine(currentOutlineLines[lineIter].p1.m_vPos, direction, perpendicular, outline.m_fMinDistance, outline.m_fMaxDistance);
				if (pointLocal == vector.Zero || !Math2D.IsPointInPolygon(polygon, pointLocal[0], pointLocal[2]))
				{
					throwAwayCount++;
					continue;
				}

				point = pointLocal.Multiply4(worldMat);

				// see which trees are around - count the types
				groupProbas.Copy(outline.m_aGroupProbas);
				for (int i = 0; i < groupCounts.Count(); ++i)
				{
					groupCounts[i] = 1;
				}

				m_Grid.CountEntriesAround(point, outline.m_fClusterRadius, groupCounts);

				// skew the probability of given groups based on counts
				probaSumToNormalize = 0;
				for (int i = 0; i < groupProbas.Count(); ++i)
				{
					groupProbas[i] = groupProbas[i] * Math.Pow(groupCounts[i], outline.m_fClusterStrength);
					probaSumToNormalize += groupProbas[i];
				}

				if (probaSumToNormalize > 0)
				{
					for (int i = 0; i < groupProbas.Count(); ++i)
					{
						groupProbas[i] = groupProbas[i] / probaSumToNormalize;
					}
				}

				groupProba = m_RandomGenerator.RandFloat01();
				groupIdx = groupProbas.Count() - 1; // last because there is less than in the loop
				probaSum = 0;
				for (int i = 0, count = groupProbas.Count(); i < count; ++i)
				{
					probaSum += groupProbas[i];
					if (groupProba < probaSum) // less than to avoid accepting 0 probability tree
					{
						groupIdx = i;
						break;
					}
				}

				tree = SelectTreeToSpawn(point, outline.m_aTreeGroups[groupIdx].m_aTrees);

				if (!IsEntryValid(tree, pointLocal))
					continue;

				switch (outline.m_OutlineType)
				{
					case ForestGeneratorOutlineType.SMALL:	tree.m_Type = TreeType.SOUTLINE; break;
					case ForestGeneratorOutlineType.MIDDLE:	tree.m_Type = TreeType.MOUTLINE; break;
				}
				m_Grid.AddEntry(tree, point);
			}
		}

		ForestGeneratorPoint currentPoint;
		int pointsCount = currentOutlinePoints.Count();
		for (int pointIter = 0; pointIter < pointsCount; pointIter++)
		{
			currentPoint = currentOutlinePoints[pointIter];
			iterCount = outline.m_fDensity * CalculateAreaForOutline(currentPoint, outline) * HECTARE_CONVERSION_FACTOR;
			for (int treeIdx = 0; treeIdx < iterCount; treeIdx++)
			{
				pointLocal = GeneratePointInCircle(outline.m_fMinDistance, outline.m_fMaxDistance, currentPoint);

				bool lineDistance1 = IsPointInProperDistanceFromLine(pointLocal, currentPoint.m_Line1, outline.m_fMinDistance, outline.m_fMaxDistance);
				if (!lineDistance1)
					continue;

				bool lineDistance2 = IsPointInProperDistanceFromLine(pointLocal, currentPoint.m_Line2, outline.m_fMinDistance, outline.m_fMaxDistance);
				if (!lineDistance2)
					continue;

				if (!Math2D.IsPointInPolygon(polygon, pointLocal[0], pointLocal[2]))
				{
					throwAwayCount++;
					continue;
				}

				point = pointLocal.Multiply4(worldMat);

				// see which trees are around - count the types
				groupProbas.Copy(outline.m_aGroupProbas);
				for (int i = 0; i < groupCounts.Count(); ++i)
				{
					groupCounts[i] = 1;
				}

				m_Grid.CountEntriesAround(point, outline.m_fClusterRadius, groupCounts);

				// skew the probability of given groups based on counts
				probaSumToNormalize = 0;
				for (int i = 0; i < groupProbas.Count(); ++i)
				{
					groupProbas[i] = groupProbas[i] * Math.Pow(groupCounts[i], outline.m_fClusterStrength);
					probaSumToNormalize += groupProbas[i];
				}

				for (int i = 0; i < groupProbas.Count(); ++i)
				{
					if (probaSumToNormalize != 0)
						groupProbas[i] = groupProbas[i] / probaSumToNormalize;
				}

				groupProba = m_RandomGenerator.RandFloat01();
				groupIdx = groupProbas.Count() - 1; // last because there is less than in the loop
				probaSum = 0;
				for (int i = 0, count = groupProbas.Count(); i < count; ++i)
				{
					probaSum += groupProbas[i];
					if (groupProba < probaSum) // less than to avoid accepting 0 probability tree
					{
						groupIdx = i;
						break;
					}
				}

				tree = SelectTreeToSpawn(point, outline.m_aTreeGroups[groupIdx].m_aTrees);

				if (!IsEntryValid(tree, pointLocal))
					continue;

				switch (outline.m_OutlineType)
				{
					case ForestGeneratorOutlineType.SMALL:	tree.m_Type = TreeType.SOUTLINE; break;
					case ForestGeneratorOutlineType.MIDDLE:	tree.m_Type = TreeType.MOUTLINE; break;
				}

				m_Grid.AddEntry(tree, point);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	bool IsPointInProperDistanceFromLine(vector point, ForestGeneratorLine line, float minDistance, float maxDistance)
	{
		float distance = Math3D.PointLineSegmentDistance(Vector(point[0], 0, point[2]), Vector(line.p1.m_vPos[0], 0, line.p1.m_vPos[2]), Vector(line.p2.m_vPos[0], 0, line.p2.m_vPos[2]));
		return distance > minDistance && distance < maxDistance;
	}

	//------------------------------------------------------------------------------------------------
	bool IsEntryValid(ForestGeneratorTree tree, vector pointLocal)
	{
		if (!tree)
			return false;

		FallenTree fallenTree = FallenTree.Cast(tree);
		if (fallenTree)
		{
			float distance;
			float minDistance;
			foreach (ForestGeneratorLine line : m_aLines)
			{
				distance = Math3D.PointLineSegmentDistance(pointLocal, line.p1.m_vPos, line.p2.m_vPos);
				minDistance = fallenTree.GetMinDistanceFromLine();
				if (distance < minDistance)
					return false;
			}
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	private void GetPolygonPoints(array<float> polygon, out array<vector> points)
	{
		vector firstPoint;
		vector nPoint;
		for (int i = 0, count = polygon.Count(); i + 1 < count; i++)
		{
			nPoint = Vector(polygon[i], 0, polygon[i + 1]);
			if (i == 0)
				firstPoint = nPoint;

			points.Insert(nPoint);
			i++;
		}
		points.Insert(firstPoint);
	}

	//------------------------------------------------------------------------------------------------
	void GenerateBottomTrees(array<float> polygon, AAB bbox, ForestGeneratorBottomLevel bottomLevel)
	{
		if (!bottomLevel || bottomLevel.m_aTreeGroups.IsEmpty())
			return;

		if (!GetIsAnyTreeValid(bottomLevel.m_aTreeGroups))
			return;

		array<float> groupProbas = {};
		float totalWeight = 0;
		int groupCount = bottomLevel.m_aTreeGroups.Count();
		groupProbas.Resize(groupCount);
		for (int i = 0; i < groupCount; i++)
		{
			totalWeight += bottomLevel.m_aTreeGroups[i].m_fWeight;
		}

		if (totalWeight != 0)
		{
			for (int i = 0; i < groupCount; i++)
			{
				groupProbas[i] = (bottomLevel.m_aTreeGroups[i].m_fWeight / totalWeight);
			}
		}

		vector worldMat[4];
		GetWorldTransform(worldMat);

		array<vector> points = {};
		GetPolygonPoints(polygon, points);

		bool checkOutline;
		ForestGeneratorRectangle rectangle;

		int expectedIterCount = m_fArea * HECTARE_CONVERSION_FACTOR * bottomLevel.m_fDensity;
		int iterCount;
		float perlinValue;
		vector pointLocal;
		vector point;
		float rangeBeginning;
		int groupIdx;
		ForestGeneratorTree tree;
		for (int rectIdx = 0, rectCount = m_aRectangles.Count(); rectIdx < rectCount; rectIdx++)
		{
			checkOutline = false;
			rectangle = m_aRectangles[rectIdx];
			if (!rectangle.m_aLines.IsEmpty())
				checkOutline = true;

			iterCount = bottomLevel.m_fDensity * m_aRectangles[rectIdx].m_fArea * HECTARE_CONVERSION_FACTOR;
			for (int treeIdx = 0; treeIdx < iterCount; ++treeIdx)
			{
				expectedIterCount--;
				// generate random point inside the shape (polygon at first)
				pointLocal = GenerateRandomPointInRectangle(rectangle);

				if (checkOutline)
				{
					if (!Math2D.IsPointInPolygon(polygon, pointLocal[0], pointLocal[2]))
						continue;

					if (IsInOutline(rectangle, pointLocal))
						continue;
				}

				perlinValue = Math.PerlinNoise01(pointLocal[0], 0, pointLocal[2]); // TODO Can we change the size of perlin noise?

				if (perlinValue > 1 || perlinValue < 0)
				{
					Print("Perlin value is out of range <0,1>, something went wrong!", LogLevel.ERROR);
					continue;
				}

				rangeBeginning = 0;
				groupIdx = 0;
				for (int i = 0; i < groupCount; i++)
				{
					if (perlinValue > rangeBeginning && perlinValue < (groupProbas[i] + rangeBeginning))
					{
						groupIdx = i;
						break;
					}
					rangeBeginning += groupProbas[i];
				}

				point = pointLocal.Multiply4(worldMat);
				tree = SelectTreeToSpawn(point, bottomLevel.m_aTreeGroups[groupIdx].m_aTrees);

				if (!IsEntryValid(tree, pointLocal))
					continue;

				tree.m_Type = TreeType.BOTTOM;
				tree.m_iDebugGroupIndex = groupIdx;
				m_Grid.AddEntry(tree, point);
			}
		}

		int index;
		while (expectedIterCount > 0)
		{
			index = m_RandomGenerator.RandFloatXY(0, m_aRectangles.Count() - 1);
			GenerateTreeInsideRectangle(m_aRectangles[index], bottomLevel, polygon, worldMat);
			expectedIterCount--;
		}
	}

	//------------------------------------------------------------------------------------------------
	void GenerateTopTrees(array<float> polygon, AAB bbox, ForestGeneratorTopLevel topLevel)
	{
		if (!topLevel || topLevel.m_aTreeGroups.IsEmpty())
			return;

		if (!GetIsAnyTreeValid(topLevel.m_aTreeGroups))
			return;

		array<float> groupProbas = {};
		array<float> groupCounts = {};
		groupCounts.Resize(topLevel.m_aTreeGroups.Count());

		vector worldMat[4];
		GetWorldTransform(worldMat);

		array<vector> points = {};
		GetPolygonPoints(polygon, points);

		bool checkOutline;
		ForestGeneratorRectangle rectangle;
		float area;

		vector pointLocal;
		int expectedIterCount = m_fArea * HECTARE_CONVERSION_FACTOR * topLevel.m_fDensity;
		int iterCount = 0;
		vector point;
		float probaSumToNormalize, groupProba, probaSum;
		int groupIdx;
		ForestGeneratorTree tree;
		for (int rectIdx = 0, rectCount = m_aRectangles.Count(); rectIdx < rectCount; rectIdx++)
		{
			checkOutline = false;
			rectangle = m_aRectangles[rectIdx];
			if (rectangle.m_aLines.Count() > 0)
				checkOutline = true;

			area = m_aRectangles[rectIdx].m_fArea * HECTARE_CONVERSION_FACTOR;
			iterCount = topLevel.m_fDensity * area;

			for (int treeIdx = 0; treeIdx < iterCount; ++treeIdx)
			{
				// generate random point inside the shape (polygon at first)
				pointLocal = GenerateRandomPointInRectangle(rectangle);
				expectedIterCount--;

				if (checkOutline)
				{
					if (!Math2D.IsPointInPolygon(polygon, pointLocal[0], pointLocal[2]))
						continue;

					if (IsInOutline(rectangle, pointLocal))
						continue;
				}

				point = pointLocal.Multiply4(worldMat);

				// see which trees are around - count the types
				groupProbas.Copy(topLevel.m_aGroupProbas);
				for (int i = 0; i < groupCounts.Count(); ++i)
				{
					groupCounts[i] = 1;
				}

				m_Grid.CountEntriesAround(point, topLevel.m_fClusterRadius, groupCounts);

				// skew the probability of given groups based on counts
				probaSumToNormalize = 0;
				for (int i = 0; i < groupProbas.Count(); ++i)
				{
					groupProbas[i] = groupProbas[i] * Math.Pow(groupCounts[i], topLevel.m_fClusterStrength);
					probaSumToNormalize += groupProbas[i];
				}

				if (probaSumToNormalize != 0)
				{
					for (int i = 0; i < groupProbas.Count(); ++i)
					{
						groupProbas[i] = groupProbas[i] / probaSumToNormalize;
					}
				}

				groupProba = m_RandomGenerator.RandFloat01();
				groupIdx = groupProbas.Count() - 1; // last because there is less than in the loop
				probaSum = 0;
				for (int i = 0, count = groupProbas.Count(); i < count; ++i)
				{
					probaSum += groupProbas[i];
					if (groupProba < probaSum) // less than to avoid accepting 0 probability tree
					{
						groupIdx = i;
						break;
					}
				}

				tree = SelectTreeToSpawn(point, topLevel.m_aTreeGroups[groupIdx].m_aTrees);
				if (!IsEntryValid(tree, pointLocal))
					continue;

				tree.m_Type = TreeType.TOP;
				m_Grid.AddEntry(tree, point);
			}
		}

		int index;
		while (expectedIterCount > 0)
		{
			index = m_RandomGenerator.RandInt(0, m_aRectangles.Count());
			GenerateTreeInsideRectangle(m_aRectangles[index], topLevel, polygon, worldMat);
			expectedIterCount--;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void GenerateTreeInsideRectangle(ForestGeneratorRectangle rectangle, ForestGeneratorLevel level, array<float> polygon, vector worldMat[4])
	{
		array<float> groupProbas = {};
		array<float> groupCounts = {};
		groupCounts.Resize(level.m_aTreeGroups.Count());

		// generate random point inside the shape (polygon at first)
		vector pointLocal = GenerateRandomPointInRectangle(rectangle);

		if (rectangle.m_aLines.Count() > 0)
		{
			if (!Math2D.IsPointInPolygon(polygon, pointLocal[0], pointLocal[2]))
				return;

			if (IsInOutline(rectangle, pointLocal))
				return;
		}

		vector point = pointLocal.Multiply4(worldMat);

		// see which trees are around - count the types
		groupProbas.Copy(level.m_aGroupProbas);
		for (int i = 0; i < groupCounts.Count(); ++i)
		{
			groupCounts[i] = 1;
		}

		if (level.m_Type == ForestGeneratorLevelType.TOP)
		{
			ForestGeneratorTopLevel topLevel = ForestGeneratorTopLevel.Cast(level);
			if (topLevel)
			{
				m_Grid.CountEntriesAround(point, topLevel.m_fClusterRadius, groupCounts);

				// skew the probability of given groups based on counts
				float probaSumToNormalize = 0;
				for (int i = 0; i < groupProbas.Count(); ++i)
				{
					groupProbas[i] = groupProbas[i] * Math.Pow(groupCounts[i], topLevel.m_fClusterStrength);
					probaSumToNormalize += groupProbas[i];
				}

				if (probaSumToNormalize != 0)
				{
					for (int i = 0; i < groupProbas.Count(); ++i)
					{
						groupProbas[i] = groupProbas[i] / probaSumToNormalize;
					}
				}
			}
		}

		float groupProba = m_RandomGenerator.RandFloat01();
		int groupIdx = groupProbas.Count() - 1; // last because there is less than in the loop
		float probaSum = 0;
		for (int i = 0, count = groupProbas.Count(); i < count; ++i)
		{
			probaSum += groupProbas[i];
			if (groupProba < probaSum) // less than to avoid accepting 0 probability tree
			{
				groupIdx = i;
				break;
			}
		}

		if (groupIdx >= level.m_aTreeGroups.Count() || groupIdx < 0)
			groupIdx = SafeRandomInt(0, level.m_aTreeGroups.Count() - 1);

		ForestGeneratorTree tree = SelectTreeToSpawn(point, level.m_aTreeGroups[groupIdx].m_aTrees);
		if (!IsEntryValid(tree, pointLocal))
			return;

		tree.m_Type = TreeType.TOP;
		m_Grid.AddEntry(tree, point);
	}

	//------------------------------------------------------------------------------------------------
	private void LoadOutlines()
	{
		m_aOutlines.Clear();
		ForestGeneratorOutline outline;
		foreach (ForestGeneratorLevel level : m_aLevels)
		{
			outline = ForestGeneratorOutline.Cast(level);
			if (outline)
				m_aOutlines.Insert(outline);
		}
	}

	//------------------------------------------------------------------------------------------------
	ForestGeneratorTree SelectTreeToSpawn(vector point, array<ref ForestGeneratorTree> trees)
	{
		// take a random tree type
		float treeProba = m_RandomGenerator.RandFloat01();
		int treeTypeIdx = trees.Count() - 1; // last because there is less than in the loop
		float probaSum = 0;
		for (int i = 0, count = trees.Count(); i < count; ++i)
		{
			probaSum += trees[i].m_fWeight;
			if (treeProba < probaSum) // less than to avoid accepting 0 probability tree
			{
				treeTypeIdx = i;
				break;
			}
		}

		if (treeTypeIdx < 0)
			return null;

		ForestGeneratorTree tree = ForestGeneratorTree.Cast(trees[treeTypeIdx].Copy());
		if (!tree)
			return null;

		SetObjectScale(tree);
		FallenTree fallenTree = FallenTree.Cast(tree);
		if (fallenTree)
		{
			fallenTree.m_fYaw = m_RandomGenerator.RandFloat01() * 360;
			fallenTree.Rotate();
		}

		// see if it fits in given place, if not this type is not valid here
		if (m_Grid.IsColliding(point, tree))
			return null;

		/*
		if (fallenTree)
		{
			vector p1 = fallenTree.m_CapsuleStart + point;
			vector p2 = fallenTree.m_CapsuleEnd + point;
			Shape shape = Shape.Create(ShapeType.LINE, ARGB(255, 0, 0, 255), ShapeFlags.NOZBUFFER, p1, p2);
			m_aDebugShapes.Insert(shape);
			shape = Shape.CreateSphere(ARGB(255, 0, 255, 0), ShapeFlags.NOOUTLINE | ShapeFlags.NOZBUFFER, p1, 0.5);
			m_aDebugShapes.Insert(shape);
			shape = Shape.CreateSphere(ARGB(255, 255, 0, 0), ShapeFlags.NOOUTLINE | ShapeFlags.NOZBUFFER, p2, 0.5);
			m_aDebugShapes.Insert(shape);
		}
		*/

		m_aGridEntries.Insert(tree);

		return tree;
	}

	//------------------------------------------------------------------------------------------------
	void SetObjectScale(SCR_ForestGeneratorTreeBase object)
	{
		object.m_fScale = SafeRandomFloat(object.m_fMinScale, object.m_fMaxScale);
		object.AdjustScale();
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsInOutline(ForestGeneratorRectangle rectangle, vector pointLocal, float aditionalDistance = 0)
	{
		float distance;
		foreach (ForestGeneratorLine line : rectangle.m_aLines)
		{
			distance = Math3D.PointLineSegmentDistance(pointLocal, line.p1.m_vPos, line.p2.m_vPos);

			foreach (ForestGeneratorOutline outline : m_aOutlines)
			{
				if (!outline.m_bGenerate)
					continue;

				switch (outline.m_OutlineType)
				{
					case ForestGeneratorOutlineType.SMALL:
					{
						if (line.p1.m_bSmallOutline && distance > outline.m_fMinDistance - aditionalDistance && distance < outline.m_fMaxDistance + aditionalDistance)
							return true;
						break;
					}
					case ForestGeneratorOutlineType.MIDDLE:
					{
						if (line.p1.m_bMiddleOutline && distance > outline.m_fMinDistance - aditionalDistance && distance < outline.m_fMaxDistance + aditionalDistance)
							return true;
						break;
					}
				}
			}
		}
		return false;
	}

	//------------------------------------------------------------------------------------------------
	private float CalculateAreaForOutline(ForestGeneratorLine line, ForestGeneratorOutline outline)
	{
		if (!line || !outline)
			return 0;

		return line.m_fLength * (outline.m_fMaxDistance - outline.m_fMinDistance);
	}

	//------------------------------------------------------------------------------------------------
	private float CalculateAreaForOutline(ForestGeneratorPoint point, ForestGeneratorOutline outline)
	{
		if (!point || !outline)
			return 0;

		float areaBigger = Math.PI * Math.Pow(outline.m_fMaxDistance, 2);
		float areaSmaller = Math.PI * Math.Pow(outline.m_fMinDistance, 2);

		return (point.m_fAngle / 360) * (areaBigger - areaSmaller);
	}

	//------------------------------------------------------------------------------------------------
	private void ClockwiseCheck()
	{
		int count = m_aPoints.Count();
		if (count < 3)
			return;

		int sum = 0;
		vector currentPoint;
		vector nextPoint;
		for (int i = 0; i < count; i++)
		{
			currentPoint = m_aPoints[i].m_vPos;
			if (i == count -1)
				nextPoint = m_aPoints[0].m_vPos;
			else
				nextPoint = m_aPoints[i+1].m_vPos;

			sum += (nextPoint[0] - currentPoint[0]) * (nextPoint[2] + currentPoint[2]);
		}

		if (sum < 0)
		{
			// COUNTER-CLOCKWISE
			int pointsCount = m_aPoints.Count();
			int iterNum = pointsCount / 2;

			ForestGeneratorPoint genPoint;
			for (int i = 0; i < iterNum; i++)
			{
				genPoint = m_aPoints[i];
				m_aPoints[i] = m_aPoints[pointsCount - 1 - i];
				m_aPoints[pointsCount - 1 - i] = genPoint;
			}
		}
/*
		else
		{
			// CLOCKWISE
		}
*/
	}

#ifdef _ALLOW_FOREST_REGENERATION
	//------------------------------------------------------------------------------------------------
	protected override void OnIntersectingShapeChangedXZInternal(IEntitySource shapeEntitySrc, IEntitySource other, array<vector> mins, array<vector> maxes)
	{
		if (!m_bAllowRegenerationByNearbyChanges || !shapeEntitySrc || !other)
			return;

		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		if (!worldEditorAPI)
			return;

		IEntitySource childEntitySource;
		GeneratorBaseEntity generator;
		for (int i = 0, childrenCount = other.GetNumChildren(); i < childrenCount; i++)
		{
			childEntitySource = other.GetChild(i);
			generator = GeneratorBaseEntity.Cast(worldEditorAPI.SourceToEntity(childEntitySource));
			if (!generator)
				continue;

			if (generator.IsInherited(SCR_PowerlineGeneratorEntity) ||
				generator.IsInherited(RoadGeneratorEntity) ||
				generator.IsInherited(RiverEntity))
			{
				OnShapeInitInternal(shapeEntitySrc, ShapeEntity.Cast(worldEditorAPI.SourceToEntity(shapeEntitySrc)));
				return;
			}
		}
	}
#endif

	//------------------------------------------------------------------------------------------------
	void ForestGeneratorEntity(IEntitySource src, IEntity parent)
	{
#ifdef WORKBENCH
		SetEventMask(EntityEvent.INIT);
		m_Grid = new ForestGeneratorGrid(10);
		m_RandomGenerator = new RandomGenerator();
#endif
	}
};
