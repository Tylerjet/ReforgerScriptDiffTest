//-----------------------------------------------------------------------
class ForestGeneratorPointData : ShapePointDataScriptBase
{
	[Attribute("1", UIWidgets.CheckBox, "Generate the middle outline along this line?")]
	bool m_bMiddleOutline;
	[Attribute("1", UIWidgets.CheckBox, "Generate the small outline along this line?")]
	bool m_bSmallOutline;
};

//-----------------------------------------------------------------------
class ForestGeneratorPoint
{
	vector m_vPos = "0 0 0";
	float m_fMinAngle = 0;
	float m_fMaxAngle = 0;
	float m_fAngle = 0;
	bool m_bSmallOutline = true;
	bool m_bMiddleOutline = true;
	ForestGeneratorLine m_Line1 = null;
	ForestGeneratorLine m_Line2 = null;
};

//-----------------------------------------------------------------------
class ForestGeneratorLine
{
	ref ForestGeneratorPoint p1 = new ref ForestGeneratorPoint();
	ref ForestGeneratorPoint p2 = new ref ForestGeneratorPoint();
	float m_fLength = 0;
};

//-----------------------------------------------------------------------
enum TreeType
{
	TOP,
	BOTTOM,
	MOUTLINE,
	SOUTLINE,
	CLUSTER,
	COUNT
};

//-----------------------------------------------------------------------
enum ForestGeneratorClusterType
{
	CIRCLE = 0,
	STRIP = 1
};

//-----------------------------------------------------------------------
enum ForestGeneratorLevelType
{
	TOP = 0,
	BOTTOM = 1,
	OUTLINE = 2
};

//-----------------------------------------------------------------------
enum ForestGeneratorOutlineType
{
	SMALL = 0,
	MIDDLE = 1
};

//-----------------------------------------------------------------------
class ForestGeneratorRectangle
{
	ref ForestGeneratorLine m_Line1 = new ForestGeneratorLine();
	ref ForestGeneratorLine m_Line2 = new ForestGeneratorLine();
	ref ForestGeneratorLine m_Line3 = new ForestGeneratorLine();
	ref ForestGeneratorLine m_Line4 = new ForestGeneratorLine();
	ref array<ForestGeneratorLine> m_aLines = new ref array<ForestGeneratorLine>(); //Outline lines that intersect this rectangle
	ref array<vector> m_aPoints = new ref array<vector>(); //Points of this rectangle
	ref array<IEntitySource> m_aPresentRoadShapes = {};
	float m_fWidth = 0;
	float m_fLength = 0;
	float m_fArea = 0;
	int m_iX = 0;
	int m_iY = 0;
	
	//-----------------------------------------------------------------------
	void GetBounds(out vector mins, out vector maxs)
	{
		array<vector> points = new array<vector>();
		points.Insert(m_Line1.p1.m_vPos);
		points.Insert(m_Line1.p2.m_vPos);
		points.Insert(m_Line2.p2.m_vPos);
		points.Insert(m_Line3.p2.m_vPos);
		float minX = 999999;
		float maxX = -999999;
		float minZ = 999999;
		float maxZ = -999999;
		
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
	
	//-----------------------------------------------------------------------
	bool IsPointIn(float x, float y)
	{
		float startX = m_Line1.p1.m_vPos[0];
		float startY = m_Line1.p1.m_vPos[2];
		
		if ((x > startX) && (x < startX + m_fWidth) && (y > startY) && (y < startY + m_fLength))
			return true;
		return false;
	}
};

//-----------------------------------------------------------------------
[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class ForestGeneratorLevel
{
	[Attribute(uiwidget: UIWidgets.Object, desc: "Tree groups to spawn in this forest generator level")]
	ref array<ref TreeGroupClass> m_aTreeGroups;
	
	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Generate this level?")]
	bool m_bGenerate;
	
	[Attribute(defvalue: "1", desc: "How many trees per hectare should be generated.", uiwidget: UIWidgets.SpinBox)]
	float m_fDensity;
	
	// For debug
	int m_iEntitiesCount = 0;
	
	ForestGeneratorLevelType m_Type;
	ref array<float> m_aGroupProbas = new ref array<float>();
};

//-----------------------------------------------------------------------
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
	
	//-----------------------------------------------------------------------
	void ForestGeneratorOutline()
	{
		m_Type = ForestGeneratorLevelType.OUTLINE;
	}
};

//-----------------------------------------------------------------------
[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class ForestGeneratorTopLevel : ForestGeneratorLevel
{
	[Attribute(defvalue: "1", uiwidget: UIWidgets.SpinBox, desc: "How separated are the tree groups?")]
	float m_fClusterStrength;
	
	[Attribute(defvalue: "15", uiwidget: UIWidgets.SpinBox, desc: "In what radius should trees around be taken into count for clusters?")]
	float m_fClusterRadius;
	
	//-----------------------------------------------------------------------
	void ForestGeneratorTopLevel()
	{
		m_Type = ForestGeneratorLevelType.TOP;
	}
};

//-----------------------------------------------------------------------
[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class ForestGeneratorBottomLevel : ForestGeneratorLevel
{
	void ForestGeneratorBottomLevel()
	{
		m_Type = ForestGeneratorLevelType.BOTTOM;
	}
};

//-----------------------------------------------------------------------
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
	float m_fMinDistanceFromLine = -1;
	
	//-----------------------------------------------------------------------
	void Rotate()
	{
		//vector yawDirection = vector.YawToVector(yaw);
		//yawDirection = Vector(-yawDirection[0], 0, yawDirection[1]); // Convert from Enforce to Enfusion format
		m_CapsuleStart = Rotate2D(m_CapsuleStartInEditor * m_fScale, Math.DEG2RAD * m_fYaw);
		m_CapsuleEnd = Rotate2D(m_CapsuleEndInEditor * m_fScale, Math.DEG2RAD * m_fYaw);
	}
	
	//-----------------------------------------------------------------------
	vector Rotate2D(vector vec, float rads)
	{
		vector result;
		float sin, cos;
		sin = Math.Sin(rads);
		cos = Math.Cos(rads);
		result[0] = vec[0] * cos - vec[2] * sin;
		result[1] = 0;
		result[2] = vec[0] * sin + vec[2] * cos;
		return result;
	}
	
	//-----------------------------------------------------------------------
	float GetMinDistanceFromLine()
	{
		if (m_fMinDistanceFromLine == -1)
		{
			float distance = 0;
			distance = m_CapsuleStart.Length();
			if (distance > m_fMinDistanceFromLine)
				m_fMinDistanceFromLine = distance;
			distance = m_CapsuleEnd.Length();
			if (distance > m_fMinDistanceFromLine)
				m_fMinDistanceFromLine = distance
		}
		
		return m_fMinDistanceFromLine;
	}
	
	//-----------------------------------------------------------------------
	void CopyValues(notnull FallenTree newObject)
	{
		super.CopyValues(newObject);
		
		newObject.m_bAlignToNormal = m_bAlignToNormal;
		
		newObject.m_CapsuleStartInEditor = m_CapsuleStartInEditor;
		newObject.m_CapsuleEndInEditor = m_CapsuleEndInEditor;
	}
	
	//-----------------------------------------------------------------------
	override ForestGeneratorTreeBase Copy()
	{
		FallenTree newObject = new FallenTree();
		CopyValues(newObject);
		return newObject;
	}
};

//-----------------------------------------------------------------------
[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class ForestGeneratorTree : SCR_ForestGeneratorTreeBase
{
	[Attribute(defvalue: "1", uiwidget: UIWidgets.SpinBox, desc: "Weight of this object for clusters?")]
	float m_fWeight;
	
	int m_iDebugGroupIndex;
	
	//-----------------------------------------------------------------------
	void CopyValues(notnull ForestGeneratorTree newObject)
	{
		super.CopyValues(newObject);
		
		newObject.m_fWeight = m_fWeight;
		newObject.m_iDebugGroupIndex = m_iDebugGroupIndex;
	}
	
	//-----------------------------------------------------------------------
	override ForestGeneratorTreeBase Copy()
	{
		ForestGeneratorTree newObject = new ForestGeneratorTree();
		CopyValues(newObject);
		return newObject;
	}
};

//-----------------------------------------------------------------------
[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class ForestGeneratorTreeShort : ForestGeneratorTree
{
	//-----------------------------------------------------------------------
	override void AdjustScale()
	{
		m_fBotDistance *= m_fScale;
	}
};

//-----------------------------------------------------------------------
[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class ForestGeneratorTreeMiddle : ForestGeneratorTree
{
	[Attribute(defvalue: "5", uiwidget: UIWidgets.SpinBox, "Minimum required radius in the middle layer for this object to spawn"), ForestGeneratorDistaceAttribute(DistanceType.MID)]
	float m_fMidDistance;
	
	//-----------------------------------------------------------------------
	override void AdjustScale()
	{
		m_fBotDistance *= m_fScale;
		m_fMidDistance *= m_fScale;
	}
	
	//-----------------------------------------------------------------------
	void CopyValues(notnull ForestGeneratorTreeMiddle newObject)
	{
		super.CopyValues(newObject);
		
		newObject.m_fMidDistance = m_fMidDistance;
	}
	
	//-----------------------------------------------------------------------
	override ForestGeneratorTreeBase Copy()
	{
		ForestGeneratorTreeMiddle newObject = new ForestGeneratorTreeMiddle();
		CopyValues(newObject);
		return newObject;
	}
};

//-----------------------------------------------------------------------
[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class ForestGeneratorTreeTall : ForestGeneratorTree
{
	[Attribute(defvalue: "5", uiwidget: UIWidgets.SpinBox, "Minimum required radius in the middle layer for this object to spawn"), ForestGeneratorDistaceAttribute(DistanceType.MID)]
	float m_fMidDistance;
	
	[Attribute(defvalue: "10", uiwidget: UIWidgets.SpinBox, "Minimum required radius in the top layer for this object to spawn"), ForestGeneratorDistaceAttribute(DistanceType.TOP)]
	float m_fTopDistance;
	
	//-----------------------------------------------------------------------
	override void AdjustScale()
	{
		m_fBotDistance *= m_fScale;
		m_fMidDistance *= m_fScale;
		m_fTopDistance *= m_fScale;
	}
	
	//-----------------------------------------------------------------------
	void CopyValues(notnull ForestGeneratorTreeTall newObject)
	{
		super.CopyValues(newObject);
		
		newObject.m_fMidDistance = m_fMidDistance;
		newObject.m_fTopDistance = m_fTopDistance;
	}
	
	//-----------------------------------------------------------------------
	override ForestGeneratorTreeBase Copy()
	{
		ForestGeneratorTreeTall newObject = new ForestGeneratorTreeTall();
		CopyValues(newObject);
		return newObject;
	}
};

//-----------------------------------------------------------------------
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
	
	//-----------------------------------------------------------------------
	ForestGeneratorClusterType GetClusterType()
	{
		return m_eType;
	}
};

//-----------------------------------------------------------------------
[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class ForestGeneratorCircleCluster : ForestGeneratorCluster
{
	float m_fRadius = 0;
	
	//-----------------------------------------------------------------------
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

//-----------------------------------------------------------------------
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
	
	//-----------------------------------------------------------------------
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

//-----------------------------------------------------------------------
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
	float m_fMinDistanceFromLine = -1;
	
	//-----------------------------------------------------------------------
	void Rotate()
	{
		m_CapsuleStart = Rotate2D(m_CapsuleStartInEditor * m_fScale, Math.DEG2RAD * m_fYaw);
		m_CapsuleEnd = Rotate2D(m_CapsuleEndInEditor * m_fScale, Math.DEG2RAD * m_fYaw);
	}
	
	//-----------------------------------------------------------------------
	vector Rotate2D(vector vec, float rads)
	{
		vector result;
		float sin, cos;
		sin = Math.Sin(rads);
		cos = Math.Cos(rads);
		result[0] = vec[0] * cos - vec[2] * sin;
		result[1] = 0;
		result[2] = vec[0] * sin + vec[2] * cos;
		return result;
	}
	
	//-----------------------------------------------------------------------
	float GetMinDistanceFromLine()
	{
		if (m_fMinDistanceFromLine == -1)
		{
			float distance = 0;
			distance = m_CapsuleStart.Length();
			if (distance > m_fMinDistanceFromLine)
				m_fMinDistanceFromLine = distance;
			distance = m_CapsuleEnd.Length();
			if (distance > m_fMinDistanceFromLine)
				m_fMinDistanceFromLine = distance
		}
		
		return m_fMinDistanceFromLine;
	}
	
	//-----------------------------------------------------------------------
	void CopyValues(notnull WideForestGeneratorClusterObject newObject)
	{
		super.CopyValues(newObject);
		
		newObject.m_bAlignToNormal = m_bAlignToNormal;
		newObject.m_CapsuleStartInEditor = m_CapsuleStartInEditor;
		newObject.m_CapsuleEndInEditor = m_CapsuleEndInEditor;
	}
	
	//-----------------------------------------------------------------------
	override ForestGeneratorTreeBase Copy()
	{
		WideForestGeneratorClusterObject newObject = new WideForestGeneratorClusterObject();
		CopyValues(newObject);
		return newObject;
	}
};

//-----------------------------------------------------------------------
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
	
	//-----------------------------------------------------------------------
	override void AdjustScale()
	{
		m_fBotDistance *= m_fScale;
	}
	
	//-----------------------------------------------------------------------
	void CopyValues(notnull SmallForestGeneratorClusterObject newObject)
	{
		super.CopyValues(newObject);
		
		newObject.m_iMinCount = m_iMinCount;
		newObject.m_iMaxCount = m_iMaxCount;
		newObject.m_fMinRadius = m_fMinRadius;
		newObject.m_fMaxRadius = m_fMaxRadius;
	}
	
	//-----------------------------------------------------------------------
	override ForestGeneratorTreeBase Copy()
	{
		SmallForestGeneratorClusterObject newObject = new SmallForestGeneratorClusterObject();
		CopyValues(newObject);
		return newObject;
	}
};

//-----------------------------------------------------------------------
[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class MiddleForestGeneratorClusterObject : SmallForestGeneratorClusterObject
{
	[Attribute(defvalue: "5", uiwidget: UIWidgets.SpinBox, "Minimum required radius in the middle layer for this object to spawn"), ForestGeneratorDistaceAttribute(DistanceType.MID)]
	float m_fMidDistance;
	
	//-----------------------------------------------------------------------
	override void AdjustScale()
	{
		m_fBotDistance *= m_fScale;
		m_fMidDistance *= m_fScale;
	}
	
	//-----------------------------------------------------------------------
	void CopyValues(notnull MiddleForestGeneratorClusterObject newObject)
	{
		super.CopyValues(newObject);
		
		newObject.m_fMidDistance = m_fMidDistance;
	}
	
	//-----------------------------------------------------------------------
	override ForestGeneratorTreeBase Copy()
	{
		MiddleForestGeneratorClusterObject newObject = new MiddleForestGeneratorClusterObject();
		CopyValues(newObject);
		return newObject;
	}
};

//-----------------------------------------------------------------------
[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class BigForestGeneratorClusterObject : MiddleForestGeneratorClusterObject
{
	[Attribute(defvalue: "10", uiwidget: UIWidgets.SpinBox, "Minimum required radius in the top layer for this object to spawn"), ForestGeneratorDistaceAttribute(DistanceType.TOP)]
	float m_fTopDistance;
	
	//-----------------------------------------------------------------------
	override void AdjustScale()
	{
		m_fBotDistance *= m_fScale;
		m_fMidDistance *= m_fScale;
		m_fTopDistance *= m_fScale;
	}
	
	//-----------------------------------------------------------------------
	void CopyValues(notnull BigForestGeneratorClusterObject newObject)
	{
		super.CopyValues(newObject);
		newObject.m_fTopDistance = m_fTopDistance;
	}
	
	//-----------------------------------------------------------------------
	override ForestGeneratorTreeBase Copy()
	{
		BigForestGeneratorClusterObject newObject = new BigForestGeneratorClusterObject();
		CopyValues(newObject);
		return newObject;
	}
};

//-----------------------------------------------------------------------
[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class TreeGroupClass
{
	[Attribute(defvalue: "1", uiwidget: UIWidgets.SpinBox, desc: "Weight of this group")]
	float m_fWeight;

	[Attribute(uiwidget: UIWidgets.Object, params: "noDetails")]
	ref array<ref ForestGeneratorTree> m_aTrees;
	
	//-----------------------------------------------------------------------
	void ~TreeGroupClass()
	{
		if (m_aTrees)
			m_aTrees.Clear();
		m_aTrees = null;
	}
};

//-----------------------------------------------------------------------
class AAB : Managed
{
	vector m_vMin;
	vector m_vMax;
	float m_fWidth; //X-axis
	float m_fDepth; //Z-axis
	
	//-----------------------------------------------------------------------
	void AAB()
	{
		// TODO init better
		m_vMin = {99999999, 99999999, 99999999};
		m_vMax = {-99999999, -99999999, -99999999};
	}
	
	//-----------------------------------------------------------------------
	void Add(vector value)
	{
		for (int i = 0; i < 3; ++i)
		{
			m_vMin[i] = Math.Min(m_vMin[i], value[i]);
			m_vMax[i] = Math.Max(m_vMax[i], value[i]);
		}
	}
	
	//-----------------------------------------------------------------------
	vector GetSize()
	{
		vector size;
		size[0] = m_vMax[0] - m_vMin[0];
		size[1] = m_vMax[1] - m_vMin[1];
		size[2] = m_vMax[2] - m_vMin[2];
		return size;
	}
	
	//-----------------------------------------------------------------------
	bool DetectCollision2D(AAB other)
	{
		if (m_vMin[0] < other.m_vMin[0] + other.m_fWidth &&
		   m_vMin[0] + m_fWidth > other.m_vMin[0] &&
		   m_vMin[2] < other.m_vMin[2] + other.m_fDepth &&
		   m_vMin[2] + m_fDepth > other.m_vMin[2])
		{
		   return true;
		}
		
		return false;
	}
	
	//-----------------------------------------------------------------------
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

//-----------------------------------------------------------------------
[EntityEditorProps(category: "GameLib/Scripted/Generator", description: "ForestGeneratorEntity", dynamicBox: true, visible: false)]
class ForestGeneratorEntityClass: SCR_GeneratorBaseEntityClass
{
};

//-----------------------------------------------------------------------
class ForestGeneratorEntity : SCR_GeneratorBaseEntity
{
	static ref array<ref Shape> s_aShapes = {};
	static ref array<IEntitySource> s_aQuerriedShapeSources = {};
	static float m_fTotalTimeRectangulation = 0;
	static float m_fTotalTimePopulatingGrid = 0;
	static float m_fTotalTimeGeneratingEntities = 0;
	
	//-----------------------------------------------------------------------
	// General
	[Attribute(defvalue: "42", uiwidget: UIWidgets.SpinBox, category: "General", desc: "Seed used by the random generator of this forest generator")]
	int m_iSeed;

	// Debug
	[Attribute(defvalue: "0", uiwidget: UIWidgets.CheckBox, category: "Debug", desc: "Print the area of this forest generator polygon?")]
	bool m_bPrintArea;
	
	[Attribute(defvalue: "0", uiwidget: UIWidgets.CheckBox, category: "Debug", desc: "Draw debug shapes of objects spawned by this forest generator?")]
	bool m_bDrawDebugShapes;
	
	[Attribute(defvalue: "0", uiwidget: UIWidgets.CheckBox, category: "Debug", desc: "Draw debug shapes of rectangulation for this forest generator?")]
	bool m_bDrawDebugShapesRectangulation;

	[Attribute(defvalue: "0", uiwidget: UIWidgets.CheckBox, category: "Debug", desc: "Print the count of entities spawned by this forest generator?")]
	bool m_bPrintEntitiesCount;
	
	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, category: "Debug", desc: "Generate forest when something changed?")]
	bool m_bGenerateForest;
	
	[Attribute(defvalue: "", uiwidget: UIWidgets.Object, desc: "Forest generator levels to spawn in this forest generator polygon")]
	ref array<ref ForestGeneratorLevel> m_aLevels;
	
	[Attribute(defvalue: "", uiwidget: UIWidgets.Object, params: "noDetails", desc: "Forest generator clusters to spawn in this forest generator polygon")]
	ref array<ref ForestGeneratorCluster> m_aClusters;
	
	[Attribute("0")]
	protected bool m_bAvoidObjects;
	
	[Attribute("0")]
	protected bool m_bAvoidRoads;
	
	[Attribute("0")]
	protected bool m_bAvoidRivers;
	
	[Attribute("0")]
	protected bool m_bAvoidPowerLines;
	
	//-----------------------------------------------------------------------
	ref ForestGeneratorGrid m_Grid;
	ref RandomGenerator m_RandomGenerator;
	ref array<ref Shape> m_aDebugShapes;
	ref array<ref Shape> m_aDebugShapesRectangulation;
	ref array<ref ForestGeneratorPoint> m_aPoints = new ref array<ref ForestGeneratorPoint>();
	ref array<ref ForestGeneratorLine> m_aLines = new ref array<ref ForestGeneratorLine>();
	ref array<ref ForestGeneratorPoint> m_aMiddleOutlinePoints = new ref array<ref ForestGeneratorPoint>();
	ref array<ref ForestGeneratorLine> m_aSmallOutlineLines = new ref array<ref ForestGeneratorLine>();
	ref array<ref ForestGeneratorPoint> m_aSmallOutlinePoints = new ref array<ref ForestGeneratorPoint>();
	ref array<ref ForestGeneratorLine> m_aMiddleOutlineLines = new ref array<ref ForestGeneratorLine>();
	ref array<ref ForestGeneratorRectangle> m_aRectangles = new ref array<ref ForestGeneratorRectangle>();
	ref array<ref ForestGeneratorRectangle> m_aOutlineRectangles = new ref array<ref ForestGeneratorRectangle>();
	ref array<ref ForestGeneratorRectangle> m_aNonOutlineRectangles = new ref array<ref ForestGeneratorRectangle>();
	ref array<ref ForestGeneratorTreeBase> m_aGridEntries = new ref array<ref ForestGeneratorTreeBase>();
	
	ref array<ref ForestGeneratorOutline> m_aOutlines = new ref array<ref ForestGeneratorOutline>();
	
	static ref map<IEntitySource, ref array<vector>> s_mRoadPoints = new map<IEntitySource, ref array<vector>>();
	
	private float m_fOutlinesWidth = 0; //Max outline width
	private bool m_bGeneratedForest = false;
	
	private float m_fArea = 0;
	
	protected IEntity m_QueriedEntity;
	protected ref array<IEntity> m_aGeneratedEntities = {};

	//-----------------------------------------------------------------------
	void ForestGeneratorEntity(IEntitySource src, IEntity parent)
	{
#ifdef WORKBENCH
		SetEventMask(EntityEvent.INIT);
		m_Grid = new ForestGeneratorGrid(10);
		m_RandomGenerator = new RandomGenerator();
#endif
	}
	
	//-----------------------------------------------------------------------
	void ~ForestGeneratorEntity()
	{
		if (m_aLevels)
			m_aLevels.Clear();
		m_aLevels = null;
		
		if (m_aClusters)
			m_aClusters.Clear();
		m_aClusters = null;
		
		if (m_aDebugShapes)
			m_aDebugShapes.Clear();
		m_aDebugShapes = null;
		
		if (m_aDebugShapesRectangulation)
			m_aDebugShapesRectangulation.Clear();
		m_aDebugShapesRectangulation = null;
		
		if (m_aPoints)
			m_aPoints.Clear();
		m_aPoints = null;
		
		if (m_aLines)
			m_aLines.Clear();
		m_aLines = null;
		
		if (m_aMiddleOutlinePoints)
			m_aMiddleOutlinePoints.Clear();
		m_aMiddleOutlinePoints = null;
		
		if (m_aSmallOutlineLines)
			m_aSmallOutlineLines.Clear();
		m_aSmallOutlineLines = null;
		
		if (m_aSmallOutlinePoints)
			m_aSmallOutlinePoints.Clear();
		m_aSmallOutlinePoints = null;
		
		if (m_aMiddleOutlineLines)
			m_aMiddleOutlineLines.Clear();
		m_aMiddleOutlineLines = null;
		
		if (m_aRectangles)
			m_aRectangles.Clear();
		m_aRectangles = null;
		
		if (m_aOutlineRectangles)
			m_aOutlineRectangles.Clear();
		m_aOutlineRectangles = null;
		
		if (m_aNonOutlineRectangles)
			m_aNonOutlineRectangles.Clear();
		m_aNonOutlineRectangles = null;
		
		if (m_aGridEntries)
			m_aGridEntries.Clear();
		m_aGridEntries = null;
	}
	
	//-----------------------------------------------------------------------
	bool OnLine(ForestGeneratorLine line, ForestGeneratorPoint point)
	{
		float a = Math.Max(line.p1.m_vPos[0], line.p2.m_vPos[0]);
		float b = Math.Min(line.p1.m_vPos[0], line.p2.m_vPos[0]);
		float c = Math.Max(line.p1.m_vPos[2], line.p2.m_vPos[2]);
		float d = Math.Min(line.p1.m_vPos[2], line.p2.m_vPos[2]);
		
		if (point.m_vPos[0] <= a && point.m_vPos[0] <= b && (point.m_vPos[2] <= c && point.m_vPos[2] <= d))
			return true;
		
		return false;
	}
	
	//-----------------------------------------------------------------------
	int Direction(ForestGeneratorPoint a, ForestGeneratorPoint b, ForestGeneratorPoint c) 
	{
	   int val = (b.m_vPos[2]-a.m_vPos[2])*(c.m_vPos[0]-b.m_vPos[0])-(b.m_vPos[0]-a.m_vPos[0])*(c.m_vPos[2]-b.m_vPos[2]);
	   if (val == 0)
	      return 0;     //colinear
	   else if(val < 0)
	      return 2;    //anti-clockwise direction
	   return 1;    //clockwise direction
	}
	
	//-----------------------------------------------------------------------
	bool IsIntersect(ForestGeneratorLine line1, ForestGeneratorLine line2) 
	{
		//four Direction for two lines and points of other line
		int dir1 = Direction(line1.p1, line1.p2, line2.p1);
		int dir2 = Direction(line1.p1, line1.p2, line2.p2);
		int dir3 = Direction(line2.p1, line2.p2, line1.p1);
		int dir4 = Direction(line2.p1, line2.p2, line1.p2);
		
		if (dir1 != dir2 && dir3 != dir4)
		  return true; //they are intersecting
		
		if (dir1==0 && OnLine(line1, line2.p1)) //when p2 of line2 are on the line1
		  return true;
		
		if (dir2==0 && OnLine(line1, line2.p2)) //when p1 of line2 are on the line1
		  return true;
		
		if (dir3==0 && OnLine(line2, line1.p1)) //when p2 of line1 are on the line2
		  return true;
		
		if (dir4==0 && OnLine(line2, line1.p2)) //when p1 of line1 are on the line2
		  return true;
			 
		return false;
	}
	
	//-----------------------------------------------------------------------
	bool IsIntersect(ForestGeneratorLine line, ForestGeneratorRectangle rectangle)
	{
		if (IsIntersect(line, rectangle.m_Line1) || IsIntersect(line, rectangle.m_Line2) || IsIntersect(line, rectangle.m_Line3) || IsIntersect(line, rectangle.m_Line4))
			return true;
		return false;
	}

	#ifdef WORKBENCH
	//-----------------------------------------------------------------------
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

	//-----------------------------------------------------------------------
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
			level.m_aGroupProbas = new ref array<float>();
			
			if (type != ForestGeneratorLevelType.BOTTOM)
			{
				for (int x = 0; x < level.m_aTreeGroups.Count(); )
				{
					if (level.m_aTreeGroups[x].m_fWeight > 0 && level.m_aTreeGroups[x].m_aTrees != null && level.m_aTreeGroups[x].m_aTrees.Count() > 0)
					{
						if (PreprocessTreeArr(level.m_aTreeGroups[x].m_aTrees, groupIdx, TreeType.TOP, debugGroupIdx))
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
					else
					{
						level.m_aTreeGroups.RemoveOrdered(x);
					}
				}
				
				int groupCount = level.m_aTreeGroups.Count();
				for (int y = 0; y < groupCount; ++y)
				{
					if (groupProbaSum > 0)
						level.m_aTreeGroups[y].m_fWeight = level.m_aTreeGroups[y].m_fWeight / groupProbaSum;
					level.m_aGroupProbas.Insert(level.m_aTreeGroups[y].m_fWeight);
				}
			}
			
			switch (type)
			{
				case ForestGeneratorLevelType.BOTTOM:
				{
					int groupCount = level.m_aTreeGroups.Count();
					for (int y = 0; y < groupCount; y++)
					{
						PreprocessTreeArr(level.m_aTreeGroups[y].m_aTrees, 0, TreeType.BOTTOM, debugGroupIdx);
					}
					break;
				}
			}
		}
	}

	//-----------------------------------------------------------------------
	override bool _WB_OnKeyChanged(BaseContainer src, string key, BaseContainerList ownerContainers, IEntity parent)
	{
		super._WB_OnKeyChanged(src, key, ownerContainers, parent);
		
		if (key == "coords")
			return false;
		
		foreach (ForestGeneratorLevel level : m_aLevels)
		{
			//TODO LIMIT COUNTS
		}
		
		super._WB_OnKeyChanged(src, key, ownerContainers, parent);

		WorldEditorAPI api = _WB_GetEditorAPI();
		if (!api)
			return false;

		IEntitySource thisSrc = api.EntityToSource(this);
		BaseContainerTools.WriteToInstance(this, thisSrc);

		PreprocessAllTrees();

		auto parentShape = ShapeEntity.Cast(parent);
		if (parentShape)
		{
			auto parentSrc = api.EntityToSource(parentShape);
			OnShapeInit(parentSrc, parentShape);
		}

		return true;
	}

	//-----------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		m_Grid = new ForestGeneratorGrid(10);
		PreprocessAllTrees();
	}
	#endif
	
	//-----------------------------------------------------------------------
	void ClearPoints()
	{
		if (m_aPoints)
			m_aPoints.Clear();
		else m_aPoints = new ref array<ref ForestGeneratorPoint>();
		
		if (m_aMiddleOutlinePoints)
			m_aMiddleOutlinePoints.Clear();
		else m_aMiddleOutlinePoints = new ref array<ref ForestGeneratorPoint>();
		
		if (m_aSmallOutlinePoints)
			m_aSmallOutlinePoints.Clear();
		else m_aSmallOutlinePoints = new ref array<ref ForestGeneratorPoint>();
		
		if (m_aSmallOutlineLines)
			m_aSmallOutlineLines.Clear();
		else m_aSmallOutlineLines = new ref array<ref ForestGeneratorLine>();
		
		if (m_aMiddleOutlineLines)
			m_aMiddleOutlineLines.Clear();
		else m_aMiddleOutlineLines = new ref array<ref ForestGeneratorLine>();
	}

	//-----------------------------------------------------------------------
	protected void LoadPoints(BaseContainerList points)
	{
		if (points != null)
		{
			#ifdef WORKBENCH
			WorldEditorAPI api = _WB_GetEditorAPI();
			
			
			int pointCount = points.Count();
			
			for (int i = 0; i < pointCount; i++)
			{
				BaseContainer point = points.Get(i);
				vector pos;
				point.Get("Position", pos);
				
				bool smallOutline = true;
				bool middleOutline = true;
				BaseContainerList dataArr = point.GetObjectArray("Data");
				int dataCount = dataArr.Count();
				bool hasPointData = false;
				for (int j = 0; j < dataCount; ++j)
				{
					BaseContainer data = dataArr.Get(j);
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
				
				
				bool skip = false;
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
				
				auto genPoint = new ForestGeneratorPoint();
				pos[1] = 0;
				genPoint.m_vPos = pos;
				genPoint.m_bSmallOutline = smallOutline;
				genPoint.m_bMiddleOutline = middleOutline;
				
				m_aPoints.Insert(genPoint);
			}
			#endif
		}
	}
	
	//-----------------------------------------------------------------------
	override void OnShapeInitInternal(IEntitySource shapeEntitySrc, ShapeEntity shapeEntity)
	{
	#ifdef WORKBENCH
		super.OnShapeInitInternal(shapeEntitySrc, shapeEntity);
		
		if (!m_bGenerateForest)
			return;
		
		ClearPoints();
		LoadOutlines();
		
		m_fOutlinesWidth = 0;
		foreach (ForestGeneratorOutline outline: m_aOutlines)
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
		
		ForestGeneratorLine line = new ForestGeneratorLine();
		for (int i = 0, count = m_aPoints.Count(); i < count; i++)
		{
			auto genPoint = m_aPoints[i];
			
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
		
		if (m_aPoints.Count() <= 0)
			return;
		auto genPoint = m_aPoints[0];
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

		// Generate the forest
		m_aDebugShapes = new array<ref Shape>();
		array<vector> polygon = GetPoints(shapeEntitySrc);
		
		QueryShapeEntities(shapeEntitySrc, api, m_Source);

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
	
	//-----------------------------------------------------------------------
	private void CalculateOutlineAnglesForPoints()
	{
		//CALCULATE ANGLE
		int count = m_aPoints.Count();
		if (count > 2)
		{
			for (int i = 0; i < count; i++)
			{
				ForestGeneratorPoint previousPoint;
				if (i > 0)
					previousPoint = m_aPoints[i-1];
				else previousPoint = m_aPoints[count-1];
				ForestGeneratorPoint currentPoint = m_aPoints[i];
				ForestGeneratorPoint nextPoint;
				if (i < count-1)
					nextPoint = m_aPoints[i+1];
				else nextPoint = m_aPoints[0];
				
				vector dir1 = previousPoint.m_vPos - currentPoint.m_vPos;
				vector dir2 = nextPoint.m_vPos - currentPoint.m_vPos;
				float yaw1 = dir1.ToYaw();
				float yaw2 = dir2.ToYaw();
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
	}

	//-----------------------------------------------------------------------
	override void OnShapeChangedInternal(IEntitySource shapeEntitySrc, ShapeEntity shapeEntity, array<vector> mins, array<vector> maxes)
	{
		// TODO handle this case better if needed, use the bbox arrays
		OnShapeInitInternal(shapeEntitySrc, shapeEntity);
	}
	
	//-----------------------------------------------------------------------
	void QueryShapeEntities(notnull IEntitySource shapeSource, notnull WorldEditorAPI api, IEntitySource generatorSource)
	{
#ifdef WORKBENCH
		ForestGeneratorEntity forestGenerator = ForestGeneratorEntity.Cast(api.SourceToEntity(generatorSource));
		if (!forestGenerator)
			return;
		
		IEntity shapeEntity = api.SourceToEntity(shapeSource);
		if (!shapeEntity)
			return;
		
		BaseContainerList points = shapeSource.GetObjectArray("Points");
		
		// Get bbox
		array<vector> vectorPoints = GetPoints(shapeSource);
		AAB bbox = AAB.MakeFromPoints(vectorPoints);
		
		// Query entities in bbox
		vector mat[4];
		shapeEntity.GetTransform(mat);
		
		BaseWorld world = shapeEntity.GetWorld();
		
		bbox.m_vMin[1] = -50;
		bbox.m_vMax[1] = 50;
		
		s_aQuerriedShapeSources.Clear();
		
		world.QueryEntitiesByAABB(shapeEntity.CoordToParent(bbox.m_vMin), shapeEntity.CoordToParent(bbox.m_vMax), forestGenerator.QueryFilter);
		s_mRoadPoints.Clear();
		
		ShapeEntity otherShapeEntity;
		for (int i = s_aQuerriedShapeSources.Count() - 1; i >= 0; i--)
		{
			if (shapeSource == s_aQuerriedShapeSources[i])
				continue;
			
			otherShapeEntity = ShapeEntity.Cast(api.SourceToEntity(s_aQuerriedShapeSources[i]));
			
			array<vector> pts = {};
			otherShapeEntity.GenerateTesselatedShape(pts);
			
			s_mRoadPoints.Insert(s_aQuerriedShapeSources[i], pts);
		}
#endif
	}
	
	//-----------------------------------------------------------------------
	bool QueryFilter(IEntity entity)
	{
		ShapeEntity shape = ShapeEntity.Cast(entity);
		if (!shape)
			return true;
		
		IEntitySource shapeSource = shape._WB_GetEditorAPI().EntityToSource(shape);
		if (!shapeSource)
			return true;
		
		RoadGeneratorEntity roadGenerator;
		SCR_PowerlineGeneratorEntity powerlineGenerator;
		RiverEntity riverEntity;
		int childrenCount = shapeSource.GetNumChildren();
		if (childrenCount <= 0)
			return true;
		
		for (int i = childrenCount - 1; i >= 0; i--)
		{
			if (m_bAvoidRoads)
			{
				roadGenerator = RoadGeneratorEntity.Cast(shape._WB_GetEditorAPI().SourceToEntity(shapeSource.GetChild(i)));
				if (roadGenerator)
					break;
			}
			if (m_bAvoidRivers)
			{
				riverEntity = RiverEntity.Cast(shape._WB_GetEditorAPI().SourceToEntity(shapeSource.GetChild(i)));
				if (riverEntity)
					break;
			}
			if (m_bAvoidPowerLines)
			{
				powerlineGenerator = SCR_PowerlineGeneratorEntity.Cast(shape._WB_GetEditorAPI().SourceToEntity(shapeSource.GetChild(i)));
				if (powerlineGenerator)
					break;
			}
		}
		
		if (roadGenerator || riverEntity || powerlineGenerator)
			s_aQuerriedShapeSources.Insert(shapeSource);
		
		return true;
	}
	
	//-----------------------------------------------------------------------
	bool QueryEntityOnPosition(vector position, notnull BaseWorld world)
	{
		m_QueriedEntity = null;
		
		vector start = position + "0 100 0";
		vector end = position;
		
		TraceSphere sphere = new TraceSphere;
		sphere.Radius = 0.1;
		sphere.Start = start;
		sphere.End = end;
		sphere.ExcludeArray = m_aGeneratedEntities;
		sphere.LayerMask = EPhysicsLayerPresets.Main | EPhysicsLayerDefs.Buoyancy | EPhysicsLayerDefs.Water;
		sphere.Flags = TraceFlags.ENTS;
		
		float done = world.TraceMove(sphere, null);
		m_QueriedEntity = sphere.TraceEnt;
		
		if (m_QueriedEntity && !GenericTerrainEntity.Cast(m_QueriedEntity))
			return true;
		
		return false;
	}
	
	//-----------------------------------------------------------------------
	bool QueryRoadOnPosition(vector position, notnull BaseWorld world)
	{
		m_QueriedEntity = null;
		
		vector start = position + "0 1 0";
		vector end = position + "0 -1 0";
		
		TraceParam trace = new TraceParam;
		trace.Start = start;
		trace.End = end;
		trace.LayerMask = EPhysicsLayerPresets.Terrain | EPhysicsLayerDefs.Water;
		trace.Flags = TraceFlags.WORLD;
		
		float done = world.TraceMove(trace, null);
		if (RoadEntity.Cast(trace.TraceEnt) || RiverEntity.Cast(trace.TraceEnt))
			return true;
		
		return false;
	}
	
	//-----------------------------------------------------------------------
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

	//-----------------------------------------------------------------------
	void GenerateEntities()
	{
	#ifdef WORKBENCH
		WorldEditorAPI api = _WB_GetEditorAPI();
		
		// Delete old trees
		m_aGeneratedEntities.Clear();
		IEntitySource entSrc = api.EntityToSource(this);
		
		api.BeginEditSequence(entSrc);
		
		int childCount = entSrc.GetNumChildren();
		for (int i = childCount - 1; i >= 0; --i)
		{
			IEntitySource childSrc = entSrc.GetChild(i);
			IEntity child = api.SourceToEntity(childSrc);
			api.DeleteEntity(child);
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

		// Create new trees
		int topLevelEntitiesCount = 0;
		int bottomLevelEntitiesCount = 0;
		int smallOutlineEntitiesCount = 0;
		int middleOutlineEntitiesCount = 0;
		int clusterEntitiesCount = 0;
		int generatedEntitiesCount = 0;
		
		// Road & river avoidance
		array<vector> splineOrigins = {};
		array<IEntitySource> splineSources = {};
		array<float> splineClearances = {};
		vector splineOrigin;
		IEntitySource splineSource;
		float clearance;
		if (m_bAvoidRoads || m_bAvoidRivers || m_bAvoidPowerLines)
		{
			for (int i = 0, count = s_mRoadPoints.Count(); i < count; i++)
			{
				splineSource = s_mRoadPoints.GetKey(i);
				splineSource.Get("pos", splineOrigin);
				
				int childNum = splineSource.GetNumChildren();
				for (int z = 0; z < childNum; z++)
				{
					if (m_bAvoidRoads)
					{
						if (splineSource.GetChild(z).Get("RoadClearance", clearance))
							break;
					}
					if (m_bAvoidRivers || m_bAvoidPowerLines)
					{
						if (splineSource.GetChild(z).Get("Clearance", clearance))
							break;
					}
				}
				
				splineSources.Insert(splineSource);
				splineOrigins.Insert(splineOrigin);
				splineClearances.Insert(clearance);
			}
		}
		
		m_aDebugShapes.Clear();
		BaseWorld world = api.GetWorld();
		if (!world)
		{
			api.EndEditSequence(entSrc);
			return;
		}
		
		
		
		for (int i = 0, count = m_Grid.GetEntryCount(); i < count; ++i)
		{
			vector entryPos;
			auto baseEntry = m_Grid.GetEntry(i, entryPos);
			auto entry = ForestGeneratorTree.Cast(baseEntry);
			auto fallenTree = FallenTree.Cast(baseEntry);
			auto wideObject = WideForestGeneratorClusterObject.Cast(baseEntry);
			
			string scaleStr = baseEntry.m_fScale.ToString();
			
			// Snap the tree to terrain
			vector worldPos = entryPos;
			float y;
			if (!api.TryGetTerrainSurfaceY(worldPos[0], worldPos[2], y))
				worldPos[1] = GetOrigin()[1];
			else
				worldPos[1] = y;
			
			vector pos = CoordToLocal(worldPos);
			pos[1] = pos[1] + baseEntry.m_fVerticalOffset;
			
			switch (baseEntry.m_Type)
			{
				case TreeType.TOP:
				topLevelEntitiesCount++;
				break;
				case TreeType.BOTTOM:
				bottomLevelEntitiesCount++;
				break;
				case TreeType.MOUTLINE:
				middleOutlineEntitiesCount++;
				break;
				case TreeType.SOUTLINE:
				smallOutlineEntitiesCount++;
				break;
				case TreeType.CLUSTER:
				clusterEntitiesCount++;
				break;
			}
			
			if (baseEntry.m_Prefab == "")
				continue;
			
			if (m_bAvoidObjects && QueryEntityOnPosition(worldPos, world))
				continue;
			
			if (m_bAvoidRoads || m_bAvoidRivers || m_bAvoidPowerLines)
			{
				bool skipTree = false;
				for (int x = 0, splinesCount = s_mRoadPoints.Count(); x < splinesCount; x++)
				{
					ShapeEntity splineEntity = ShapeEntity.Cast(api.SourceToEntity(s_mRoadPoints.GetKey(x)));
					
					if (SCR_Math3D.GetDistanceFromSpline(s_mRoadPoints.GetElement(x), splineOrigins[x], splineEntity, worldPos) < splineClearances[x])
					{
						skipTree = true;
						break;
					}
				}
				if (skipTree)
					continue;
			}
			
			IEntity tree = api.CreateEntity(baseEntry.m_Prefab, "", api.GetCurrentEntityLayerId(), m_Source, pos, "0 0 0");
			IEntitySource treeSrc = api.EntityToSource(tree);
			api.BeginEditSequence(treeSrc);
			m_aGeneratedEntities.Insert(tree);
			generatedEntitiesCount++;
			api.ModifyEntityKey(tree, "scale", scaleStr);			
			
			BaseContainerList bc = treeSrc.GetObjectArray("editorData");
			
			bool alignToNormal = false;
			bool randomYaw = false;
			if (bc && bc.Count() > 0)
			{
				bc.Get(0).Get("alignToNormal", alignToNormal);
				bc.Get(0).Get("randomYaw", randomYaw);
			}
			
			if (fallenTree)
				alignToNormal = fallenTree.m_bAlignToNormal;
			if (wideObject)
				alignToNormal = wideObject.m_bAlignToNormal;
			
			if (randomYaw)
			{
				float yaw = m_RandomGenerator.RandFloat01() * 360;
				if (fallenTree)
					yaw = -fallenTree.m_fYaw; //Clockwise / Counter-clockwise
				if (wideObject)
					yaw = -wideObject.m_fYaw;
				api.ModifyEntityKey(tree, "angleY", yaw.ToString());
			}
			
			float pitch = 0, roll = 0;
			if (baseEntry.m_fRandomPitchAngle > 0)
				pitch = m_RandomGenerator.RandFloatXY(-baseEntry.m_fRandomPitchAngle, baseEntry.m_fRandomPitchAngle);
			if (baseEntry.m_fRandomRollAngle > 0)
				roll = m_RandomGenerator.RandFloatXY(-baseEntry.m_fRandomRollAngle, baseEntry.m_fRandomRollAngle);
			api.ModifyEntityKey(tree, "angleX", pitch.ToString());
			api.ModifyEntityKey(tree, "angleZ", roll.ToString());
			
			if (alignToNormal)
			{
				vector mat[4];
				TraceParam param = new TraceParam();
				param.Start = worldPos + vector.Up;
				param.End = worldPos - vector.Up;
				param.Flags = TraceFlags.WORLD;
				tree.GetWorld().TraceMove(param, null);
				tree.GetTransform(mat);
				
				vector newUp = param.TraceNorm;
				newUp.Normalize();
				
				//Shape shape;
				//shape = Shape.Create(ShapeType.LINE, ARGB(255, 0, 255, 0), ShapeFlags.NOZBUFFER, worldPos, worldPos + newUp);
				//m_aDebugShapes.Insert(shape);
				vector newRight = newUp * mat[2];
				newRight.Normalize();
				//shape = Shape.Create(ShapeType.LINE, ARGB(255, 255, 0, 0), ShapeFlags.NOZBUFFER, worldPos, worldPos + newRight);
				//m_aDebugShapes.Insert(shape);
				vector newForward = newRight * newUp;
				newForward.Normalize();
				//shape = Shape.Create(ShapeType.LINE, ARGB(255, 0, 0, 255), ShapeFlags.NOZBUFFER, worldPos, worldPos + newForward);
				//m_aDebugShapes.Insert(shape);
				
				mat[0] = newRight;
				mat[1] = newUp;
				mat[2] = newForward;
				
				vector angles = Math3D.MatrixToAngles(mat);
				
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
		
		api.EndEditSequence(entSrc);
		if (m_bPrintEntitiesCount)
		{
			Print("Forest generator generated: " + topLevelEntitiesCount + " entities in top level.");
			Print("Forest generator generated: " + bottomLevelEntitiesCount + " entities in bottom level.");
			Print("Forest generator generated: " + middleOutlineEntitiesCount + " entities in middle outline.");
			Print("Forest generator generated: " + smallOutlineEntitiesCount + " entities in small outline.");
			Print("Forest generator generated: " + clusterEntitiesCount + " entities in clusters.");
			Print("Forest generator generated: " + generatedEntitiesCount + " entities in total.");
		}
	#endif
	}

	//-----------------------------------------------------------------------
	void PopulateGrid(array<vector> polygon)
	{
		m_Grid.Clear();
		m_aGridEntries.Clear();
		
		// We need to convert the polygon to 2D to have efficient IsPointInPolygon queries
		array<float> polygon2D = new array<float>();
		foreach (vector polyPoint : polygon)
		{
			polygon2D.Insert(polyPoint[0]);
			polygon2D.Insert(polyPoint[2]);
		}
		
		array<float> a = new array<float>();
		array<float> b = new array<float>();
		for (int i = 0, count = polygon2D.Count(); i < count; )
		{
			a.Insert(polygon2D[i]);
			b.Insert(polygon2D[i+1]);
			i+=2;
		}
		
		m_fArea = Math.AbsFloat(PolygonArea(a, b));
		
		if (m_bPrintArea)
			Print("Area of the polygon is: " + m_fArea + " square meters.");

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
	
	//-----------------------------------------------------------------------
	void Rectangulate(AAB bbox, array<float> polygon2D)
	{
		
		vector direction = bbox.m_vMax - bbox.m_vMin;
		float targetRectangleWidth = 50;
		float targetRectangleLength = 50;
		int targetRectangleCountW = Math.Ceil(direction[0] / targetRectangleWidth);
		int targetRectangleCountL = Math.Ceil(direction[2] / targetRectangleLength);
		bool areLineArraysIdentical = AreIdentical(m_aSmallOutlineLines, m_aMiddleOutlineLines);
		
		vector ownerOrigin = GetOrigin();
		m_aDebugShapesRectangulation = new ref array<ref Shape>();
		
		for (int x = 0; x < targetRectangleCountW; x++)
		{
			for (int y = 0; y < targetRectangleCountL; y++)
			{
				bool isInPolygon = false;
				ref ForestGeneratorRectangle rectangle = new ref ForestGeneratorRectangle();
				rectangle.m_iX = x;
				rectangle.m_iY = y;
				rectangle.m_fWidth = targetRectangleWidth;
				rectangle.m_fLength = targetRectangleLength;
				rectangle.m_fArea = rectangle.m_fLength * rectangle.m_fWidth;
				vector p1 = bbox.m_vMin;
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
				
				int blue = (int)Math.Floor(m_RandomGenerator.RandFloatXY(0, 255));
				int red = 0;
				int green = (int)Math.Floor(m_RandomGenerator.RandFloatXY(0, 255));
				
				foreach (ForestGeneratorLine line : m_aLines)
				{
					if (!NeedsCheck(line, rectangle))
					{
						continue;
					}
					if (IsIntersect(line, rectangle))
					{
						bool found = false;
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
				}
				
				if (rectangle.m_aLines.Count() == 0 && !isInPolygon)
					continue;
				
				m_aRectangles.Insert(rectangle);
				
				if (rectangle.m_aLines.Count() > 0)
				{
					red = 255;
					blue = 0;
					green = 0;
					
					m_aOutlineRectangles.Insert(rectangle);
				}
				else m_aNonOutlineRectangles.Insert(rectangle);
				
				if (m_bDrawDebugShapesRectangulation)
				{
					Shape s = Shape.Create(ShapeType.BBOX, ARGB(255, red, green, blue), ShapeFlags.NOZBUFFER, ownerOrigin + rectangle.m_Line1.p1.m_vPos, ownerOrigin + rectangle.m_Line3.p1.m_vPos);
					m_aDebugShapesRectangulation.Insert(s);
				}
			}
		}
	}
	
	//-----------------------------------------------------------------------
	void CheckRectangleSpline(ForestGeneratorRectangle rectangle, IEntitySource splineSource)
	{
		
	}
	
	//-----------------------------------------------------------------------
	bool NeedsCheck(ForestGeneratorLine line, ForestGeneratorRectangle rectangle)
	{
		vector linePoint1 = line.p1.m_vPos;
		vector linePoint2 = line.p2.m_vPos;
		vector mins = "0 0 0";
		vector maxs = "0 0 0";
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
	
	//-----------------------------------------------------------------------
	bool AreIdentical(array<ref ForestGeneratorLine> array1, array<ref ForestGeneratorLine> array2)
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
	
	//-----------------------------------------------------------------------
	float PolygonArea(array<float> x, array<float> y)
	{
	    float area = 0.0;
		
	    for (int i = 0, count = x.Count(); i < count; ++i)
	    {
	       int j = (i + 1)%count;
	       area += 0.5 * (x[i]*y[j] -  x[j]*y[i]);
	    }
		
	    return (area);
	}
	
	//-----------------------------------------------------------------------
	void GenerateTrees(array<float> polygon2D, AAB bbox)
	{
		vector bboxSize = bbox.GetSize();
		float area = bboxSize[0] * bboxSize[2];
		if (area <= 0.01)
			return;
		
		GenerateClusters(polygon2D, bbox);
		
		for (int i = 0, count = m_aLevels.Count(); i < count; i++)
		{
			ForestGeneratorLevel level = m_aLevels[i];
			ForestGeneratorLevelType type = level.m_Type;
			
			if (!level.m_bGenerate)
				continue;
			
			int iterCount = level.m_fDensity * (area / 10000);
			
			switch (type)
			{
				case ForestGeneratorLevelType.TOP:
				{
					ForestGeneratorTopLevel topLevel = ForestGeneratorTopLevel.Cast(level);
					GenerateTopTrees(polygon2D, bbox, topLevel);
					break;
				}
				case ForestGeneratorLevelType.OUTLINE:
				{
					ForestGeneratorOutline outline = ForestGeneratorOutline.Cast(level);
					GenerateOutlineTrees(polygon2D, bbox, outline);
					break;
				}
				case ForestGeneratorLevelType.BOTTOM:
				{
					area = 0;
					ForestGeneratorBottomLevel bottomLevel = ForestGeneratorBottomLevel.Cast(level);
					GenerateBottomTrees(polygon2D, bbox, bottomLevel);
					break;
				}
			}
		}
	}
	
	//-----------------------------------------------------------------------
	private void GenerateClusters(array<float> polygon2D, AAB bbox)
	{
		for (int i = 0, count = m_aClusters.Count(); i < count; i++)
		{
			ForestGeneratorCluster cluster = m_aClusters[i];
			if (!cluster.m_bGenerate)
				continue;
			ForestGeneratorClusterType type = cluster.GetClusterType();
			switch (type)
			{
				case ForestGeneratorClusterType.CIRCLE:
				{
					ForestGeneratorCircleCluster circleCluster = ForestGeneratorCircleCluster.Cast(cluster);
					GenerateCircleCluster(circleCluster, polygon2D, bbox);
					break;
				}
				case ForestGeneratorClusterType.STRIP:
				{
					ForestGeneratorStripCluster stripCluster = ForestGeneratorStripCluster.Cast(cluster);
					GenerateStripCluster(stripCluster, polygon2D, bbox);
					break;
				}
			}
		}
	}
	
	//-----------------------------------------------------------------------
	protected int FindRectanglesInCircle(vector center, float radius, out array<ForestGeneratorRectangle> rectangles)
	{
		int count = 0;
		
		foreach (ForestGeneratorRectangle rectangle : m_aRectangles)
		{
			foreach (vector point : rectangle.m_aPoints)
			{
				float deltaX = center[0] - Math.Max(rectangle.m_Line1.p1.m_vPos[0], Math.Min(center[0], rectangle.m_Line1.p1.m_vPos[0] + rectangle.m_fWidth));
				float deltaY = center[2] - Math.Max(rectangle.m_Line1.p1.m_vPos[2], Math.Min(center[2], rectangle.m_Line1.p1.m_vPos[2] + rectangle.m_fLength));
				
				if ((deltaX * deltaX + deltaY * deltaY) < (radius * radius))
				{
					rectangles.Insert(rectangle);
					count++;
					continue;
				}
			}
		}
		
		return count;
	}
	
	//-----------------------------------------------------------------------
	protected bool GetPointOutsideOutlines(array<float> polygon2D, AAB bbox, out vector clusterCenter, int numberOfTries = 10, float aditionalDistance = 0)
	{
		bool isInOutline = true;
		int maxTriesToFindCenter = 10; // TODO replace with proper parameter.
		int currentTriesCount = 0;
		
		while (isInOutline)
		{
			clusterCenter = GeneratePointInPolygon(polygon2D, bbox);
			array<ForestGeneratorRectangle> rectangles = new array<ForestGeneratorRectangle>();
			
			int rectanglesCount = FindRectanglesInCircle(clusterCenter, aditionalDistance + m_fOutlinesWidth, rectangles);
			isInOutline = false;
			
			for (int i = 0; i < rectanglesCount; i++)
			{
				ForestGeneratorRectangle rectangle = rectangles[i];
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
		
		if (currentTriesCount >= maxTriesToFindCenter)
			return false;
		return true;
	}
	
	//-----------------------------------------------------------------------
	private void GenerateCircleCluster(ForestGeneratorCircleCluster cluster, array<float> polygon2D, AAB bbox)
	{
		vector worldMat[4];
		GetWorldTransform(worldMat);
		
		float CDENSHA = 0;
		if (m_RandomGenerator)
			CDENSHA = SafeRandomFloat(cluster.m_fMinCDENSHA, cluster.m_fMaxCDENSHA);
		int clusterCount = Math.Ceil((m_fArea / 10000) * CDENSHA);
		for (int c = 0; c < clusterCount; c++)
		{
			vector clusterCenter;
			if (!GetPointOutsideOutlines(polygon2D, bbox, clusterCenter, aditionalDistance: cluster.m_fRadius))
				continue;
			
			for (int x = 0; x < cluster.m_aObjects.Count(); x++)
			{
				auto clusterObject = cluster.m_aObjects[x];
				int objectCount = SafeRandomInt(clusterObject.m_iMinCount, clusterObject.m_iMaxCount);
				
				for (int o = 0; o < objectCount; o++)
				{
					vector pointLocal = GeneratePointInCircle(clusterObject.m_fMinRadius, clusterObject.m_fMaxRadius, clusterCenter);
					if (Math2D.IsPointInPolygon(polygon2D, pointLocal[0], pointLocal[2]))
					{
						SmallForestGeneratorClusterObject newClusterObject = SmallForestGeneratorClusterObject.Cast(cluster.m_aObjects[x].Copy());
						if (newClusterObject == null)
							continue;
						
						m_aGridEntries.Insert(newClusterObject);
						vector point = pointLocal.Multiply4(worldMat);
						
						SetObjectScale(newClusterObject);
						
						WideForestGeneratorClusterObject wideObject = WideForestGeneratorClusterObject.Cast(newClusterObject);
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
	
	//-----------------------------------------------------------------------
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
		
		float yaw = Math.RandomFloat01() * 360;
		vector direction = vector.FromYaw(yaw);
		vector perpendicular;
		perpendicular[0] = direction[2];
		perpendicular[2] = -direction[0];
		
		float CDENSHA = 0;
		if (m_RandomGenerator)
			CDENSHA = SafeRandomFloat(cluster.m_fMinCDENSHA, cluster.m_fMaxCDENSHA);
		int clusterCount = Math.Ceil((m_fArea / 10000) * CDENSHA);
		for (int c = 0; c < clusterCount; c++)
		{
			vector clusterCenter;
			if (!GetPointOutsideOutlines(polygon2D, bbox, clusterCenter, aditionalDistance: cluster.m_fRadius))
				continue;
			
			for (int x = 0; x < cluster.m_aObjects.Count(); x++)
			{
				auto clusterObject = cluster.m_aObjects[x];
				int objectCount = SafeRandomInt(clusterObject.m_iMinCount, clusterObject.m_iMaxCount);
				
				for (int o = 0; o < objectCount; o++)
				{
					vector pointLocal;
					
					float distance = SafeRandomFloat(clusterObject.m_fMinRadius, clusterObject.m_fMaxRadius);
					int rnd = Math.RandomIntInclusive(0, 1);
					
					if (rnd == 0)
						distance *= -1;
					
					float y01 = distance / radius * cluster.m_fFrequency;
					float y360 = y01 * 360;
					float ySin = Math.Sin(y360 * Math.DEG2RAD);
					float y = ySin * cluster.m_fAmplitude;
					
					vector offset;
					offset[0] = Math.RandomFloat(0, cluster.m_fMaxXOffset);
					offset[2] = Math.RandomFloat(0, cluster.m_fMaxYOffset);
					pointLocal = (direction * distance) + (y * perpendicular) + clusterCenter + offset;
					
					if (Math2D.IsPointInPolygon(polygon2D, pointLocal[0], pointLocal[2]))
					{
						SmallForestGeneratorClusterObject newClusterObject = SmallForestGeneratorClusterObject.Cast(cluster.m_aObjects[x].Copy());
						if (newClusterObject == null)
							continue;
						
						m_aGridEntries.Insert(newClusterObject);
						vector point = pointLocal.Multiply4(worldMat);
						
						SetObjectScale(newClusterObject);
						
						WideForestGeneratorClusterObject wideObject = WideForestGeneratorClusterObject.Cast(newClusterObject);
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
	
	//-----------------------------------------------------------------------
	private vector GeneratePointInPolygon(array<float> polygon2D, AAB bbox)
	{
		vector point = GeneratePointInBBOX(bbox);
		while (!Math2D.IsPointInPolygon(polygon2D, point[0], point[2]))
		{
			point = GeneratePointInBBOX(bbox);
		}
		return point;
	}
	
	//-----------------------------------------------------------------------
	private vector GeneratePointInBBOX(AAB bbox)
	{
		vector point;
		point[0] = Math.RandomFloat(bbox.m_vMin[0], bbox.m_vMax[0]);
		point[1] = Math.RandomFloat(bbox.m_vMin[1], bbox.m_vMax[1]);
		point[2] = Math.RandomFloat(bbox.m_vMin[2], bbox.m_vMax[2]);
		return point;
	}
	
	//-----------------------------------------------------------------------
	private vector GeneratePointAlongLine(vector start, vector direction, vector perpendicular, float minDist, float maxDist)
	{
		float distanceOnLine = m_RandomGenerator.RandFloat01();
		
		float distanceFromLine = SafeRandomFloat(minDist, maxDist);
		vector position = start + (direction * distanceOnLine) + (perpendicular * distanceFromLine);
		return position;
	}
	
	//-----------------------------------------------------------------------
	private float SafeRandomFloat(float min, float max)
	{
		float random;
		if (min >= max)
		{
			if (min == max)
			{
				random = min;
			}
			else
			{
				random = m_RandomGenerator.RandFloatXY(max, min);
				Print("Some of your forest generator min value > max value.", LogLevel.WARNING);
			}
		}
		else
			random = m_RandomGenerator.RandFloatXY(min, max);
		
		return random;
	}
	
	//-----------------------------------------------------------------------
	private int SafeRandomInt(int min, int max)
	{
		int random;
		if (min >= max)
		{
			if (min == max)
			{
				random = max;
			}
			else
			{
				random = Math.RandomIntInclusive(max, min);
				Print("Your forest generator object has some min value > max value.", LogLevel.WARNING);
			}
		}
		else
			random = Math.RandomIntInclusive(min, max);
		
		return random;
	}
	
	//-----------------------------------------------------------------------
	private vector GeneratePointInCircle(float innerRadius, float outerRadius, vector circleCenter)
	{
		vector point;
		vector direction = GenerateRandomVectorBetweenAngles(0, 360);
		
		float rand = SafeRandomFloat(innerRadius, outerRadius);
		
		point = circleCenter + (rand * direction);
		return point;
	}
	
	//-----------------------------------------------------------------------
	private vector GeneratePointInCircle(float innerRadius, float outerRadius, ForestGeneratorPoint point)
	{
		vector pointVec;
		vector direction = GenerateRandomVectorBetweenAngles(point.m_fMinAngle, point.m_fMaxAngle);
		float rand = SafeRandomFloat(innerRadius, outerRadius);
		
		pointVec = point.m_vPos + (rand * direction);
		return pointVec;
	}
	
	//-----------------------------------------------------------------------
	private vector GenerateRandomVectorBetweenAngles(float angle1, float angle2)
	{
		float yaw = m_RandomGenerator.RandFloatXY(angle1, angle2);
		return vector.FromYaw(yaw);
	}
	
	//-----------------------------------------------------------------------
	private vector GenerateRandomPointInRectangle(ForestGeneratorRectangle rectangle)
	{
		float startX = rectangle.m_Line1.p1.m_vPos[0];
		float startZ = rectangle.m_Line1.p1.m_vPos[2];
		float width = rectangle.m_fWidth;
		float length = rectangle.m_fLength;
		float randX = m_RandomGenerator.RandFloat01() * width + startX;
		float randZ = m_RandomGenerator.RandFloat01() * length + startZ;
		vector point = Vector(randX, 0, randZ);
		return point;
	}
	
	//-----------------------------------------------------------------------
	bool GetIsAnyTreeValid(array<ref TreeGroupClass> treeGroups)
	{
		for (int i = 0; i < treeGroups.Count(); ++i)
		{
			if (treeGroups[i].m_fWeight <= 0)
				continue;

			for (int j = 0; j < treeGroups[i].m_aTrees.Count(); ++j)
			{
				auto tree = treeGroups[i].m_aTrees[j];
				if (tree.m_Prefab.Length() > 0 && tree.m_fWeight > 0)
				{
					return true;
				}
			}
		}
		return false;
	}
	
	//-----------------------------------------------------------------------
	void GenerateOutlineTrees(array<float> polygon, AAB bbox, ForestGeneratorOutline outline)
	{
		if (outline == null || outline.m_aTreeGroups == null || outline.m_aTreeGroups.Count() == 0)
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
		
		if (!currentOutlinePoints)
			return;
		if (!currentOutlineLines)
			return;
		
		array<float> groupProbas = new array<float>();
		array<float> groupCounts = new array<float>();
		groupCounts.Resize(outline.m_aTreeGroups.Count());

		vector worldMat[4];
		GetWorldTransform(worldMat);
		
		int throwAwayCount = 0;
		int linesCount = currentOutlineLines.Count();
		int iterCount = 0;
		for (int lineIter = 0; lineIter < linesCount; lineIter++)
		{
			vector direction = currentOutlineLines[lineIter].p2.m_vPos - currentOutlineLines[lineIter].p1.m_vPos;
			vector perpendicular = Vector(direction[2], 0, -direction[0]);
			perpendicular.Normalize();
			iterCount = outline.m_fDensity * (CalculateAreaForOutline(currentOutlineLines[lineIter], outline) / 10000);
			for (int treeIdx = 0; treeIdx < iterCount; ++treeIdx)
			{
				vector pointLocal = GeneratePointAlongLine(currentOutlineLines[lineIter].p1.m_vPos, direction, perpendicular, outline.m_fMinDistance, outline.m_fMaxDistance);
				if (pointLocal == vector.Zero || !Math2D.IsPointInPolygon(polygon, pointLocal[0], pointLocal[2]))
				{
					throwAwayCount++;
					continue;
				}
				
				vector point = pointLocal.Multiply4(worldMat);
				
				// See which trees are around - count the types
				groupProbas.Copy(outline.m_aGroupProbas);
				for (int i = 0; i < groupCounts.Count(); ++i)
					groupCounts[i] = 1;
				
				m_Grid.CountEntriesAround(point, outline.m_fClusterRadius, groupCounts);
	
				// Skew the probability of given groups based on counts
				float probaSumToNormalize = 0;
				for (int i = 0; i < groupProbas.Count(); ++i)
				{
					groupProbas[i] = groupProbas[i] * Math.Pow(groupCounts[i], outline.m_fClusterStrength);
					probaSumToNormalize += groupProbas[i];
				}
	
				for (int i = 0; i < groupProbas.Count(); ++i)
					groupProbas[i] = groupProbas[i] / probaSumToNormalize;
	
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
				
				ForestGeneratorTree tree = SelectTreeToSpawn(point, outline.m_aTreeGroups[groupIdx].m_aTrees);
				
				if (!IsEntryValid(tree, pointLocal))
					continue;
				
				switch (outline.m_OutlineType)
				{
					case ForestGeneratorOutlineType.SMALL:
					{
						tree.m_Type = TreeType.SOUTLINE;
						break;
					}
					case ForestGeneratorOutlineType.MIDDLE:
					{
						tree.m_Type = TreeType.MOUTLINE;
						break;
					}
				}
				m_Grid.AddEntry(tree, point);
			}
		}
		
		int pointsCount = currentOutlinePoints.Count();
		for (int pointIter = 0; pointIter < pointsCount; pointIter++)
		{
			ForestGeneratorPoint currentPoint = currentOutlinePoints[pointIter];
			iterCount = outline.m_fDensity * (CalculateAreaForOutline(currentPoint, outline) / 10000);
			for (int treeIdx = 0; treeIdx < iterCount; treeIdx++)
			{
				vector randomPointInCircle = GeneratePointInCircle(outline.m_fMinDistance, outline.m_fMaxDistance, currentPoint);
				vector pointLocal = randomPointInCircle;
				
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
				
				vector point = pointLocal.Multiply4(worldMat);
					
				// See which trees are around - count the types
				groupProbas.Copy(outline.m_aGroupProbas);
				for (int i = 0; i < groupCounts.Count(); ++i)
					groupCounts[i] = 1;
	
				m_Grid.CountEntriesAround(point, outline.m_fClusterRadius, groupCounts);
	
				// Skew the probability of given groups based on counts
				float probaSumToNormalize = 0;
				for (int i = 0; i < groupProbas.Count(); ++i)
				{
					groupProbas[i] = groupProbas[i] * Math.Pow(groupCounts[i], outline.m_fClusterStrength);
					probaSumToNormalize += groupProbas[i];
				}
	
				for (int i = 0; i < groupProbas.Count(); ++i)
					groupProbas[i] = groupProbas[i] / probaSumToNormalize;
	
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
				
				ForestGeneratorTree tree = SelectTreeToSpawn(point, outline.m_aTreeGroups[groupIdx].m_aTrees);
				
				if (!IsEntryValid(tree, pointLocal))
					continue;
				
				switch (outline.m_OutlineType)
				{
					case ForestGeneratorOutlineType.SMALL:
					{
						tree.m_Type = TreeType.SOUTLINE;
						break;
					}
					case ForestGeneratorOutlineType.MIDDLE:
					{
						tree.m_Type = TreeType.MOUTLINE;
						break;
					}
				}
				m_Grid.AddEntry(tree, point);
			}
		}
	}
	
	//-----------------------------------------------------------------------
	bool IsPointInProperDistanceFromLine(vector point, ForestGeneratorLine line, float minDistance, float maxDistance)
	{
		float distance = Math3D.PointLineSegmentDistance(Vector(point[0], 0, point[2]), Vector(line.p1.m_vPos[0], 0, line.p1.m_vPos[2]), Vector(line.p2.m_vPos[0], 0, line.p2.m_vPos[2]));
		if (distance > minDistance && distance < maxDistance)
			return true;
		return false;
	}
	
	//-----------------------------------------------------------------------
	bool IsEntryValid(ForestGeneratorTree tree, vector pointLocal)
	{
		if (tree == null)
			return false;
		
		FallenTree fallenTree = FallenTree.Cast(tree);
		if (fallenTree)
		{
			foreach (ForestGeneratorLine line: m_aLines)
			{
				float distance = Math3D.PointLineSegmentDistance(pointLocal, line.p1.m_vPos, line.p2.m_vPos);
				float minDistance = fallenTree.GetMinDistanceFromLine();
				if (distance < minDistance)
					return false;
			}
		}
		
		return true;
	}
	
	//-----------------------------------------------------------------------
	private void GetPolygonPoints(array<float> polygon , out array<vector> points)
	{
		int count = polygon.Count();
		vector firstPoint = "0 0 0";
		for (int i = 0; i + 1 < count; i++)
		{
			vector nPoint = Vector(polygon[i], 0, polygon[i + 1]);
			if (i == 0)
				firstPoint = nPoint;
			points.Insert(nPoint);
			i++;
		}
		points.Insert(firstPoint);
	}
	
	//-----------------------------------------------------------------------
	void GenerateBottomTrees(array<float> polygon, AAB bbox, ForestGeneratorBottomLevel bottomLevel)
	{
		if (bottomLevel == null || bottomLevel.m_aTreeGroups.Count() == 0)
			return;
		
		if (!GetIsAnyTreeValid(bottomLevel.m_aTreeGroups))
			return;
		
		array<float> groupProbas = new array<float>();
		float totalWeight = 0;
		int groupCount = bottomLevel.m_aTreeGroups.Count();
		groupProbas.Resize(groupCount);
		for (int i = 0; i < groupCount; i++)
		{
			totalWeight += bottomLevel.m_aTreeGroups[i].m_fWeight;
		}
		float weightSoFar = 0;
		for (int i = 0; i < groupCount; i++)
		{
			groupProbas[i] = (bottomLevel.m_aTreeGroups[i].m_fWeight / totalWeight);
		}
		
		vector worldMat[4];
		GetWorldTransform(worldMat);
		array<vector> points = new array<vector>();
		GetPolygonPoints(polygon, points);
		
		int expectedIterCount = (m_fArea / 10000) * bottomLevel.m_fDensity;
		int iterCount = 0;
		int rectCount = m_aRectangles.Count();
		for (int rectIdx = 0; rectIdx < rectCount; rectIdx++)
		{
			bool checkOutline = false;
			ForestGeneratorRectangle rectangle = m_aRectangles[rectIdx];
			if (rectangle.m_aLines.Count() > 0)
				checkOutline = true;
			iterCount = bottomLevel.m_fDensity * (m_aRectangles[rectIdx].m_fArea / 10000);
			for (int treeIdx = 0; treeIdx < iterCount; ++treeIdx)
			{
				expectedIterCount--;
				// Generate random point inside the shape (polygon at first)
				vector pointLocal = GenerateRandomPointInRectangle(rectangle);
				
				if (checkOutline)
				{
					if (!Math2D.IsPointInPolygon(polygon, pointLocal[0], pointLocal[2]))
						continue;
					if (IsInOutline(rectangle, pointLocal))
						continue;
				}
				
				float perlinValue = Math.PerlinNoise01(pointLocal[0], 0, pointLocal[2]); //TODO Can we change the size of perlin noise?
				
				if (perlinValue > 1 || perlinValue < 0)
				{
					Print("Perlin value is out of range <0,1>, something went wrong!", LogLevel.ERROR);
					continue;
				}
				
				float rangeBeginning = 0;
				int groupIdx = 0;
				for (int i = 0; i < groupCount; i++)
				{
					if (perlinValue > rangeBeginning && perlinValue < (groupProbas[i] + rangeBeginning))
					{
						groupIdx = i;
						break;
					}
					rangeBeginning += groupProbas[i];
				}
				
				vector point = pointLocal.Multiply4(worldMat);
				
				ForestGeneratorTree tree = SelectTreeToSpawn(point, bottomLevel.m_aTreeGroups[groupIdx].m_aTrees);
				
				if (!IsEntryValid(tree, pointLocal))
					continue;
				
				tree.m_Type = TreeType.BOTTOM;
				tree.m_iDebugGroupIndex = groupIdx;
				m_Grid.AddEntry(tree, point);
			}
		}
		while (expectedIterCount > 0)
		{
			int index = (m_RandomGenerator.RandFloatXY(0, m_aRectangles.Count() - 1));
			ForestGeneratorRectangle rectangle = m_aRectangles[index];
			GenerateTreeInsideRectangle(rectangle, bottomLevel, polygon, worldMat);
			expectedIterCount--;
		}
	}
	
	//-----------------------------------------------------------------------
	void GenerateTopTrees(array<float> polygon, AAB bbox, ForestGeneratorTopLevel topLevel)
	{
		if (topLevel == null || topLevel.m_aTreeGroups.Count() == 0)
			return;
		
		if (!GetIsAnyTreeValid(topLevel.m_aTreeGroups))
			return;
		
		array<float> groupProbas = new array<float>();
		array<float> groupCounts = new array<float>();
		groupCounts.Resize(topLevel.m_aTreeGroups.Count());
		
		vector worldMat[4];
		GetWorldTransform(worldMat);
		array<vector> points = new array<vector>();
		GetPolygonPoints(polygon, points);
		
		int expectedIterCount = (m_fArea / 10000) * topLevel.m_fDensity;
		int iterCount = 0;
		int rectCount = m_aRectangles.Count();
		for (int rectIdx = 0; rectIdx < rectCount; rectIdx++)
		{
			bool checkOutline = false;
			ForestGeneratorRectangle rectangle = m_aRectangles[rectIdx];
			if (rectangle.m_aLines.Count() > 0)
				checkOutline = true;
			float area = m_aRectangles[rectIdx].m_fArea / 10000;
			iterCount = topLevel.m_fDensity * area;
			for (int treeIdx = 0; treeIdx < iterCount; ++treeIdx)
			{
				// Generate random point inside the shape (polygon at first)
				vector pointLocal = GenerateRandomPointInRectangle(rectangle);
				expectedIterCount--;
				
				if (checkOutline)
				{
					if (!Math2D.IsPointInPolygon(polygon, pointLocal[0], pointLocal[2]))
						continue;
					if (IsInOutline(rectangle, pointLocal))
						continue;
				}
				
				vector point = pointLocal.Multiply4(worldMat);
	
				// See which trees are around - count the types
				groupProbas.Copy(topLevel.m_aGroupProbas);
				for (int i = 0; i < groupCounts.Count(); ++i)
					groupCounts[i] = 1;
	
				m_Grid.CountEntriesAround(point, topLevel.m_fClusterRadius, groupCounts);
	
				// Skew the probability of given groups based on counts
				float probaSumToNormalize = 0;
				for (int i = 0; i < groupProbas.Count(); ++i)
				{
					groupProbas[i] = groupProbas[i] * Math.Pow(groupCounts[i], topLevel.m_fClusterStrength);
					probaSumToNormalize += groupProbas[i];
				}
	
				for (int i = 0; i < groupProbas.Count(); ++i)
					groupProbas[i] = groupProbas[i] / probaSumToNormalize;
	
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
				
				ForestGeneratorTree tree = SelectTreeToSpawn(point, topLevel.m_aTreeGroups[groupIdx].m_aTrees);
				if (!IsEntryValid(tree, pointLocal))
					continue;
				
				tree.m_Type = TreeType.TOP;
				m_Grid.AddEntry(tree, point);
			}
		}
		while (expectedIterCount > 0)
		{
			int index = (m_RandomGenerator.RandFloatXY(0, m_aRectangles.Count() - 1));
			ForestGeneratorRectangle rectangle = m_aRectangles[index];
			GenerateTreeInsideRectangle(rectangle, topLevel, polygon, worldMat);
			expectedIterCount--;
		}
	}
	
	//-----------------------------------------------------------------------
	protected void GenerateTreeInsideRectangle(ForestGeneratorRectangle rectangle, ForestGeneratorLevel level, array<float> polygon, vector worldMat[4])
	{
		array<float> groupProbas = new array<float>();
		array<float> groupCounts = new array<float>();
		groupCounts.Resize(level.m_aTreeGroups.Count());
		// Generate random point inside the shape (polygon at first)
		vector pointLocal = GenerateRandomPointInRectangle(rectangle);
		
		if (rectangle.m_aLines.Count() > 0)
		{
			if (!Math2D.IsPointInPolygon(polygon, pointLocal[0], pointLocal[2]))
				return;
			if (IsInOutline(rectangle, pointLocal))
				return;
		}
		
		vector point = pointLocal.Multiply4(worldMat);

		// See which trees are around - count the types
		groupProbas.Copy(level.m_aGroupProbas);
		for (int i = 0; i < groupCounts.Count(); ++i)
			groupCounts[i] = 1;
		
		if (level.m_Type == ForestGeneratorLevelType.TOP)
		{
			auto topLevel = ForestGeneratorTopLevel.Cast(level);
			if (topLevel)
			{
				m_Grid.CountEntriesAround(point, topLevel.m_fClusterRadius, groupCounts);
				// Skew the probability of given groups based on counts
				float probaSumToNormalize = 0;
				for (int i = 0; i < groupProbas.Count(); ++i)
				{
					groupProbas[i] = groupProbas[i] * Math.Pow(groupCounts[i], topLevel.m_fClusterStrength);
					probaSumToNormalize += groupProbas[i];
				}
				
				for (int i = 0; i < groupProbas.Count(); ++i)
				groupProbas[i] = groupProbas[i] / probaSumToNormalize;
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
	
	//-----------------------------------------------------------------------
	private bool HasOutline(ForestGeneratorOutlineType type)
	{
		foreach (ForestGeneratorLevel level : m_aLevels)
		{
			ForestGeneratorOutline outline = ForestGeneratorOutline.Cast(level);
			if (outline)
			{
				if (outline.m_OutlineType == type)
					return true;
			}
		}
		return false;
	}
	
	//-----------------------------------------------------------------------
	private void LoadOutlines()
	{
		m_aOutlines.Clear();
		foreach (ForestGeneratorLevel level : m_aLevels)
		{
			ForestGeneratorOutline outline = ForestGeneratorOutline.Cast(level);
			if (outline)
				m_aOutlines.Insert(outline);
		}
	}

	//-----------------------------------------------------------------------
	ForestGeneratorTree SelectTreeToSpawn(vector point, array<ref ForestGeneratorTree> trees)
	{
		// Take a random tree type
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
		
		// See if it fits in given place, if not this type is not valid here
		bool isColiding = m_Grid.IsColliding(point, tree);
		if (isColiding)
			return null;
		/*if (fallenTree)
		{
			vector p1 = fallenTree.m_CapsuleStart + point;
			vector p2 = fallenTree.m_CapsuleEnd + point;
			Shape shape = Shape.Create(ShapeType.LINE, ARGB(255, 0, 0, 255), ShapeFlags.NOZBUFFER, p1, p2);
			m_aDebugShapes.Insert(shape);
			shape = Shape.CreateSphere(ARGB(255, 0, 255, 0), ShapeFlags.NOOUTLINE | ShapeFlags.NOZBUFFER, p1, 0.5);
			m_aDebugShapes.Insert(shape);
			shape = Shape.CreateSphere(ARGB(255, 255, 0, 0), ShapeFlags.NOOUTLINE | ShapeFlags.NOZBUFFER, p2, 0.5);
			m_aDebugShapes.Insert(shape);
		}*/
		
		m_aGridEntries.Insert(tree);

		return tree;
	}
	
	//-----------------------------------------------------------------------
	void SetObjectScale(SCR_ForestGeneratorTreeBase object)
	{
		object.m_fScale = SafeRandomFloat(object.m_fMinScale, object.m_fMaxScale);
		
		object.AdjustScale();
	}
	
	protected vector GetClosestPoint(vector A, vector B, vector C, vector P)
	{
		// A - closest point
		// B, C - point from A to left and right
		
	  	vector vectorAP = P - A;													//Vector from A to P
	  	vector vectorAB = B - A ;   												//Vector from A to B
	
	  	float magnitudeAB = vectorAB[0] * vectorAB[0] + vectorAB[2] * vectorAB[2]; 	//Magnitude of AB vector (it's length)		
	  	float ABAPproduct = vectorAB[0] * vectorAP[0] + vectorAB[2] * vectorAP[2]; 	//The product of a_to_p and a_to_b		
	  	float distanceABP = ABAPproduct / magnitudeAB;								//The normalized "distance" from a to your closest point
				
	  	vector vectorAC = C - A ;   												//Vector from A to C
	
	  	float magnitudeAC = vectorAC[0] * vectorAC[0] + vectorAC[2] * vectorAC[2]; 	//Magnitude of AC vector (it's length)		
	  	float ACAPproduct = vectorAC[0] * vectorAP[0] + vectorAC[2] * vectorAP[2]; 	//The product of a_to_p and a_to_c		
	  	float distanceACP = ACAPproduct / magnitudeAC;								//The normalized "distance" from a to your closest point
				
		vector v;
								
		// Not on A
		if (distanceABP < 0 || distanceABP > magnitudeAB)
		{
			// Not on C
			if (distanceACP < 0 || distanceACP > magnitudeAC)
			{
				v = A
			}
			// On C
			else
			{
				v[0] = A[0] + vectorAC[0] * distanceACP;
				v[1] = A[1];
				v[2] = A[2] + vectorAC[2] * distanceACP;
			}
		}
		// On A
		else
		{
			// Not On C
			if (distanceACP < 0 || distanceACP > magnitudeAC)
			{
				v[0] = A[0] + vectorAB[0] * distanceABP;
				v[1] = A[1];
				v[2] = A[2] + vectorAB[2] * distanceABP;		
			}
			// On A and C
			else
			{				
				if (distanceABP > distanceACP)
				{
					v[0] = A[0] + vectorAB[0] * distanceABP;
					v[1] = A[1];
					v[2] = A[2] + vectorAB[2] * distanceABP;
				}
				else
				{
					v[0] = A[0] + vectorAC[0] * distanceACP;
					v[1] = A[1];
					v[2] = A[2] + vectorAC[2] * distanceACP;
				}
			}
		}
		return v;
	}
	
	//-----------------------------------------------------------------------
	protected bool IsInOutline(ForestGeneratorRectangle rectangle, vector pointLocal, float aditionalDistance = 0)
	{
		foreach (ForestGeneratorLine line : rectangle.m_aLines)
		{
			float distance = Math3D.PointLineSegmentDistance(pointLocal, line.p1.m_vPos, line.p2.m_vPos);
			
			foreach (ForestGeneratorOutline outline: m_aOutlines)
			{
				if (!outline.m_bGenerate)
					continue;
				
				switch (outline.m_OutlineType)
				{
					case ForestGeneratorOutlineType.SMALL:
					{
						if (line.p1.m_bSmallOutline)
						{
							if (distance > outline.m_fMinDistance - aditionalDistance && distance < outline.m_fMaxDistance + aditionalDistance)
								return true;
						}
						break;
					}
					case ForestGeneratorOutlineType.MIDDLE:
					{
						if (line.p1.m_bMiddleOutline)
						{
							if (distance > outline.m_fMinDistance - aditionalDistance && distance < outline.m_fMaxDistance + aditionalDistance)
								return true;
						}
						break;
					}
				}
			}
		}
		return false;
	}
	
	//-----------------------------------------------------------------------
	float CalculateOutlineArea(ForestGeneratorOutline outline)
	{
		if (!outline)
			return 0;
		
		bool SmallOutline = false;
		bool MiddleOutline = false;
		array<ref ForestGeneratorLine> lines;
		if (outline.m_OutlineType == ForestGeneratorOutlineType.SMALL)
		{
			SmallOutline = true;
			lines = m_aSmallOutlineLines;
		}
		else
		{
			MiddleOutline = true;
			lines = m_aMiddleOutlineLines;
		}
		if (!lines)
			return 0;
		
		float area = 0;
		foreach (ForestGeneratorLine line : lines)
		{
			if (!line.p1.m_bSmallOutline && SmallOutline || !line.p1.m_bMiddleOutline && MiddleOutline)
				continue;
			area += (line.m_fLength * (outline.m_fMaxDistance - outline.m_fMinDistance));
		}
		
		array<ref ForestGeneratorPoint> points;
		if (outline.m_OutlineType == ForestGeneratorOutlineType.SMALL)
			points = m_aSmallOutlinePoints;
		else points = m_aMiddleOutlinePoints;
		if (!points)
			return area;
		
		foreach (ForestGeneratorPoint point : points)
		{
			if (!point.m_bSmallOutline && SmallOutline || !point.m_bMiddleOutline && MiddleOutline)
				continue;
			area += (point.m_fAngle / 360) * (outline.m_fMaxDistance - outline.m_fMinDistance);
		}
		
		return area;
	}
	
	//-----------------------------------------------------------------------
	private float CalculateAreaForOutline(ForestGeneratorLine line, ForestGeneratorOutline outline)
	{
		if (!line || !outline)
			return 0;
		
		float area = (line.m_fLength * (outline.m_fMaxDistance - outline.m_fMinDistance));
		
		return area;
	}
	
	//-----------------------------------------------------------------------
	private float CalculateAreaForOutline(ForestGeneratorPoint point, ForestGeneratorOutline outline)
	{
		if (!point || !outline)
			return 0;
		
		float areaBigger = Math.PI * Math.Pow(outline.m_fMaxDistance, 2);
		float areaSmaller = Math.PI * Math.Pow(outline.m_fMinDistance, 2);
		float area = (point.m_fAngle / 360) * (areaBigger - areaSmaller);
		
		return area;
	}
	
	//-----------------------------------------------------------------------
	private void ClockwiseCheck()
	{
		int count = m_aPoints.Count();
		
		if (count <= 2)
			return;
		
		int sum = 0;
		for (int i = 0; i < count; i++)
		{
			vector currentPoint = m_aPoints[i].m_vPos;
			vector nextPoint;
			if (i == count -1)
				nextPoint = m_aPoints[0].m_vPos;
			else
				nextPoint = m_aPoints[i+1].m_vPos;
			int currentEdge = (nextPoint[0] - currentPoint[0])*(nextPoint[2] + currentPoint[2]);
			sum += currentEdge;
		}
		if (sum < 0)
		{
			//COUNTER-CLOCKWISE
			int pointsCount = m_aPoints.Count();
			int iterNum = Math.Floor(pointsCount / 2);
			
			for (int i = 0; i < iterNum; i++)
			{
				auto genPoint = m_aPoints[i];
				m_aPoints[i] = m_aPoints[pointsCount - 1 - i];
				m_aPoints[pointsCount - 1 - i] = genPoint;
			}
		}
		else 
		{
			//CLOCKWISE
		}
	}
	
	//-----------------------------------------------------------------------
	protected override void OnIntersectingShapeChangedXZInternal(IEntitySource shapeEntitySrc, IEntitySource other, array<vector> mins, array<vector> maxes)
	{
		if (other)
		{
			int childrenCount = other.GetNumChildren();
			for (int i = 0; i < childrenCount; i++)
			{
				IEntitySource childEntitySource = other.GetChild(i);
				
				GeneratorBaseEntity generator;
				if (generator)
				{
					
				}
			}
		}
	}
};
