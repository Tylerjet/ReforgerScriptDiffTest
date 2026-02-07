[EntityEditorProps(category: "GameScripted/Generators", description: "Scripted base class for generators.", visible: false)]
class SCR_GeneratorBaseEntityClass : GeneratorBaseEntityClass
{
	// prefab properties here
	[Attribute()]
	ref Color m_Color;
};

//------------------------------------------------------------------------------------------------
class SCR_GeneratorBaseEntity : GeneratorBaseEntity
{

#ifdef WORKBENCH
	static const ref Color BASE_GENERATOR_COLOR = Color.FromRGBA(255, 255, 255, 255);
	static const string VARNAME_LINE_COLOR = "LineColor";
	IEntitySource m_Source;
	
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
		
		if (!api.IsDoingEditAction())
		{
			api.BeginEntityAction();
			api.SetVariableValue(m_Source, null, "coords", "0 0 0");
			api.EndEntityAction();
		}
		else
			api.SetVariableValue(m_Source, null, "coords", "0 0 0");
	}
	
	//-----------------------------------------------------------------------
	static array<vector> GetPoints(IEntitySource shapeEntitySrc)
	{
		BaseContainerList points = shapeEntitySrc.GetObjectArray("Points");
		array<vector> result = new array<vector>();
		if (points == null)
			return result;

		for (int i = 0, count = points.Count(); i < count; ++i)
		{
			BaseContainer point = points.Get(i);
			vector pos;
			point.Get("Position", pos);
			result.Insert(pos);
		}

		return result;
	}
	
	//------------------------------------------------------------------------------------------------
	Color GetColor()
	{
		SCR_GeneratorBaseEntityClass prefabData = SCR_GeneratorBaseEntityClass.Cast(GetPrefabData());
		if (!prefabData)
			return BASE_GENERATOR_COLOR;
		else
			return prefabData.m_Color;
	}
	
	//------------------------------------------------------------------------------------------------
	override void _WB_OnCreate(IEntitySource src)
	{
		ColorShape();

		//When generator entity gets created by any edit activity (given by _WB_OnCreate event) then re-generate its generated content
		WorldEditorAPI api = _WB_GetEditorAPI();
		if (!api || api.UndoOrRedoIsRestoring())
			return;
		
		//re-generate content only if it's created with a parent
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
			api.SetVariableValue(src, null, "Flags", (flags | EntityFlags.EDITOR_ONLY).ToString());
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
		
		auto containerPath = new array<ref ContainerIdPathEntry>();
		
		Color color = GetColor();
		
		string colorString = "";
		colorString += color.R().ToString() + " ";
		colorString += color.G().ToString() + " ";
		colorString += color.B().ToString() + " ";
		colorString += color.A().ToString();
		
		if (api.IsDoingEditAction())
			api.SetVariableValue(parentSource, containerPath, VARNAME_LINE_COLOR, colorString);
		else
		{
			api.BeginEntityAction();
			api.SetVariableValue(parentSource, containerPath, VARNAME_LINE_COLOR, colorString);
			api.EndEntityAction();
		}
	}
#endif

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
	}

	//------------------------------------------------------------------------------------------------
	void SCR_GeneratorBaseEntity(IEntitySource src, IEntity parent)
	{
#ifdef WORKBENCH
		m_Source = src;
#endif
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_GeneratorBaseEntity()
	{
	}

};
