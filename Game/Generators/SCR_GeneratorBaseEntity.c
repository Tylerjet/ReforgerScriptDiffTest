//------------------------------------------------------------------------------------------------
[EntityEditorProps(category: "GameScripted/Generators", description: "Scripted base class for generators.", visible: false)]
class SCR_GeneratorBaseEntityClass : GeneratorBaseEntityClass
{
	[Attribute()]
	ref Color m_Color;
};

//------------------------------------------------------------------------------------------------
class SCR_GeneratorBaseEntity : GeneratorBaseEntity
{
	[Attribute(defvalue: "0", desc: "Avoid objects - the trace check is a 10cm cylinder (for trees mostly)", category: "Obstacles")]
	protected bool m_bAvoidObjects;

	[Attribute(defvalue: "0", desc: "Avoid roads, respecting their clearance setting", category: "Obstacles")]
	protected bool m_bAvoidRoads;

	[Attribute(defvalue: "0", desc: "Avoid rivers, respecting their clearance setting", category: "Obstacles")]
	protected bool m_bAvoidRivers;

	[Attribute(defvalue: "0", desc: "Avoid power lines, respecting their clearance setting", category: "Obstacles")]
	protected bool m_bAvoidPowerLines;

	[Attribute(defvalue: "0", desc: "Avoid lakes", category: "Obstacles")]
	protected bool m_bAvoidLakes;

#ifdef WORKBENCH
	protected IEntitySource m_Source;
	protected IEntitySource m_ParentShapeSource;

	protected static ref SCR_ObstacleDetector s_ObstacleDetector;
	protected static const ref Color BASE_GENERATOR_COLOR = Color.White;
	protected static const float BBOX_CHECK_HEIGHT = 100.0;
	protected static const float AVOID_OBJECTS_CHECK_RADIUS = 0.1;

	//------------------------------------------------------------------------------------------------
	override bool _WB_CanSelect(IEntitySource src)
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void OnShapeInitInternal(IEntitySource shapeEntitySrc, ShapeEntity shapeEntity)
	{
		WorldEditorAPI api = _WB_GetEditorAPI();
		if (!api || api.UndoOrRedoIsRestoring())
			return;

		if (api.IsDoingEditAction())
		{
			api.SetVariableValue(m_Source, null, "coords", "0 0 0");
		}
		else
		{
			api.BeginEntityAction();
			api.SetVariableValue(m_Source, null, "coords", "0 0 0");
			api.EndEntityAction();
		}
	}
#endif

	//------------------------------------------------------------------------------------------------
	//! \return 3D points relative to the provided shape source
	static array<vector> GetPoints(notnull IEntitySource shapeEntitySrc)
	{
		BaseContainerList points = shapeEntitySrc.GetObjectArray("Points");
		if (!points)
			return {};

		array<vector> result = {};
		vector pos;
		for (int i = 0, count = points.Count(); i < count; ++i)
		{
			points.Get(i).Get("Position", pos);
			result.Insert(pos);
		}

		return result;
	}

#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	void RefreshObstacles()
	{
		array<vector> vectorPoints = GetPoints(m_ParentShapeSource);
		AAB bbox = AAB.MakeFromPoints(vectorPoints);
		bbox.m_vMin[1] = BBOX_CHECK_HEIGHT * -0.5;
		bbox.m_vMax[1] = BBOX_CHECK_HEIGHT * 0.5;

		SetAvoidOptions();
		s_ObstacleDetector.RefreshObstaclesByAABB(CoordToParent(bbox.m_vMin), CoordToParent(bbox.m_vMax));
	}

	//------------------------------------------------------------------------------------------------
	// overridable for each generator to have their own options (e.g Forest Generator to not have an "Avoid Forests" option)
	void SetAvoidOptions()
	{
		s_ObstacleDetector.SetAvoidObjects(m_bAvoidObjects);
		s_ObstacleDetector.SetAvoidObjectsDetectionRadius(AVOID_OBJECTS_CHECK_RADIUS);
		s_ObstacleDetector.SetAvoidObjectsDetectionHeight(BBOX_CHECK_HEIGHT);
		s_ObstacleDetector.SetAvoidRoads(m_bAvoidRoads);
		s_ObstacleDetector.SetAvoidRivers(m_bAvoidRivers);
		s_ObstacleDetector.SetAvoidLakes(m_bAvoidLakes);
	}

	//------------------------------------------------------------------------------------------------
	bool HasObstacle(vector worldPos, array<IEntity> exclusionList = null)
	{
		if (!s_ObstacleDetector)
		{
			Print("HasObstacle() method requires obstacles info through RefreshObstacle() method first", LogLevel.ERROR);
			return true; // prevent placement by default
		}
		return s_ObstacleDetector.HasObstacle(worldPos, exclusionList);
	}

	//------------------------------------------------------------------------------------------------
	void ClearObstacles()
	{
		if (s_ObstacleDetector)
			s_ObstacleDetector.ClearObstacles();
	}

	//------------------------------------------------------------------------------------------------
	Color GetColor()
	{
		SCR_GeneratorBaseEntityClass prefabData = SCR_GeneratorBaseEntityClass.Cast(GetPrefabData());
		if (prefabData)
			return prefabData.m_Color;

		return BASE_GENERATOR_COLOR;
	}

	//------------------------------------------------------------------------------------------------
	override void _WB_OnCreate(IEntitySource src)
	{
		ColorShape();

		// when generator entity gets created by any edit activity (given by _WB_OnCreate event) then re-generate its generated content
		WorldEditorAPI api = _WB_GetEditorAPI();
		if (!api || api.UndoOrRedoIsRestoring())
			return;

		// re-generate content only if it's created with a parent
		IEntitySource shapeEntitySrc = src.GetParent();
		if (shapeEntitySrc)
		{
			ShapeEntity shapeEntity = ShapeEntity.Cast(api.SourceToEntity(shapeEntitySrc));
			if (shapeEntity)
				OnShapeInit(shapeEntitySrc, shapeEntity);
		}

		EntityFlags flags;
		src.Get("Flags", flags);
		if (api.IsDoingEditAction())
		{
			api.SetVariableValue(src, null, "Flags", (flags | EntityFlags.EDITOR_ONLY).ToString());
		}
		else
		{
			api.BeginEntityAction();
			api.SetVariableValue(src, null, "Flags", (flags | EntityFlags.EDITOR_ONLY).ToString());
			api.EndEntityAction();
		}
	}

	//------------------------------------------------------------------------------------------------
	void ColorShape()
	{
		if (!m_Source)
			return;

		WorldEditorAPI api = _WB_GetEditorAPI();
		if (!api || api.UndoOrRedoIsRestoring())
			return;

		IEntitySource parentSource = m_Source.GetParent();
		if (!parentSource)
			return;

		array<ref ContainerIdPathEntry> containerPath = {};

		Color color = GetColor();
		string colorString = string.Format("%1 %2 %3 %4", color.R(), color.G(), color.B(), color.A());

		if (api.IsDoingEditAction())
		{
			api.SetVariableValue(parentSource, containerPath, "LineColor", colorString);
		}
		else
		{
			api.BeginEntityAction();
			api.SetVariableValue(parentSource, containerPath, "LineColor", colorString);
			api.EndEntityAction();
		}
	}
#endif

	//------------------------------------------------------------------------------------------------
	void SCR_GeneratorBaseEntity(IEntitySource src, IEntity parent)
	{
#ifdef WORKBENCH
		m_Source = src;
		m_ParentShapeSource = m_Source.GetParent();
		if (!s_ObstacleDetector || !s_ObstacleDetector.IsValid())
			s_ObstacleDetector = new SCR_ObstacleDetector(_WB_GetEditorAPI());
#endif
	}
};
