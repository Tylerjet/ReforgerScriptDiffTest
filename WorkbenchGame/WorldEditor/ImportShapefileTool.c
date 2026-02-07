[WorkbenchToolAttribute("Import Shapefile", "Import vector data from SHP file", "2", awesomeFontCode: 0xf56f)]
class ImportShapefileTool: WorldEditorTool
{
	[Attribute("", UIWidgets.ResourceNamePicker, "SHP File", "shp")]
	ResourceName shpPath;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Gives this prefab to all imported shapes as a child. Use for example to assign the same generator too all imported shapes.", "et")]
	ResourceName childPrefab;
	
	[Attribute()]
	bool m_bChildrenFromList;
	
	[Attribute(params: "conf")]
	ResourceName m_PrefabsListPath;
	
	[Attribute(defvalue: "id", desc: "Name of the column with IDS")]
	string m_sIDColumnName;
	
	[Attribute("0", UIWidgets.CheckBox, "Creates a spline from the input vectors if ticked, otherwise creates a polyline.")]
	bool m_bSpline;
	
	[Attribute("0.0", UIWidgets.EditBox)]
	float x_shift;
	
	[Attribute("0.0", UIWidgets.EditBox)]
	float y_shift;
	
	[Attribute("0.93 0.13 0.33 1", UIWidgets.ColorPicker)]
	vector ShapeColor;
	
	[Attribute()]
	bool m_bGenerateForestGeneratorPointData;
	
	private ref array<ref ForestGeneratorShapeImportData> m_aShapesData;
	private ref SCR_PrefabsList m_PrefabsList;
	private int checks = 0;
	
	//-----------------------------------------------------------------------
	[ButtonAttribute("Import SHP")]
	void Execute()
	{
		m_aShapesData = new ref array<ref ForestGeneratorShapeImportData>();
		WorldEditor we = Workbench.GetModule(WorldEditor);
		Debug.BeginTimeMeasure();
		WBProgressDialog progress = new WBProgressDialog("Importing geometries...", we);
		LoadPrefabsList();
		
		if (!m_API.IsDoingEditAction() && !m_API.UndoOrRedoIsRestoring())
			m_API.BeginEntityAction();
		
		//---------------------------Shapes--------------------------
		GeoShapeCollection shapes = GeoShapeLoader.LoadShapeFile(shpPath.GetPath());
		if (shapes == null)
		{
			Print("Shapefile failed to load", LogLevel.ERROR);
			m_API.EndEntityAction();
			return;
		}
		
		for (int i = 0, count = shapes.Count(); i < count; i++)
		{
			GeoShape shape = shapes[i];
			switch (shape.GetType())
			{
				case GeoShapeType.POINT:
					Print("Point data not supported yet!");
					break;
				case GeoShapeType.POLYLINE:
					Load_polylines(shape);
					break;
				case GeoShapeType.POLYGON:
					Load_polygons(shape);
					break;
			}
			progress.SetProgress(i / count);
		}
		
		Debug.BeginTimeMeasure();
		if (m_bGenerateForestGeneratorPointData)
			GenerateForestGeneratorPointData();
		Debug.EndTimeMeasure("GenerateForestGeneratorPointData: ");
		
		WBProgressDialog childrenAttachProgress = new WBProgressDialog("Attaching children...", we);
		for (int i = 0, count = m_aShapesData.Count(); i < count; i++)
		{
			AttachChild(m_aShapesData[i]);
			childrenAttachProgress.SetProgress(i / count);
		}
		
		m_aShapesData.Clear();
		m_aShapesData = null;
		
		m_API.EndEntityAction();
		Debug.EndTimeMeasure("Importing finished: ");
	}
	
	//------------------------------------------------------------------------------------------------
	void Load_polygons(GeoShape shape)
	{
		float traceX;
		float traceZ;
		float pntY;
		GeoPolygon polygon = GeoPolygon.Cast(shape);
		//Polygon's parts(in case polygon has holes)
		for (int j=0; j < polygon.PartsCount(); j++)
		{
			int polygonPointsCount = polygon.GetPart(j).Count();
			
			traceX = polygon.GetPart(j)[0][0] + x_shift;
			traceZ = polygon.GetPart(j)[0][2] + y_shift;
			pntY = m_API.GetTerrainSurfaceY(traceX, traceZ);
			vector bb3Dmin;
			vector bb3dmax;
			bb3Dmin[0] = traceX;
			bb3Dmin[1] = pntY;
			bb3Dmin[2] = traceZ;
			
			bb3dmax[0] = traceX;
			bb3dmax[1] = pntY;
			bb3dmax[2] = traceZ;
		
			//Part's points - calculate polygon's 3D Bbox
			for (int k=0; k < polygonPointsCount; k++)
			{
				traceX = polygon.GetPart(j)[k][0] + x_shift;
				traceZ = polygon.GetPart(j)[k][2] + y_shift;
				pntY = m_API.GetTerrainSurfaceY(traceX, traceZ);
				
				vector pnt;
				pnt[0] = traceX;
				pnt[1] = pntY;
				pnt[2] = traceZ;
				
				for (int l = 0; l < 3; l++)
				{
					float val = pnt[l];
					if (val < bb3Dmin[l])
						bb3Dmin[l] = val;
					if (val > bb3dmax[l])
						bb3dmax[l] = val;
				}
			}
			
			//calculate polygon's origin(as center of 3D Bbox)
			vector polygonOrigin;
			polygonOrigin = ((bb3dmax - bb3Dmin) / 2) + bb3Dmin;
	
			vector orienatation = "0 0 0";
			IEntity obj = m_API.CreateEntity("PolylineShapeEntity", "", m_API.GetCurrentEntityLayerId(), null, polygonOrigin, orienatation);
			IEntitySource entSrc = m_API.EntityToSource(obj);
			m_API.ModifyEntityKey(obj, "IsClosed", "1");
	
			ForestGeneratorShapeImportData data = new ForestGeneratorShapeImportData();
			
			//Part's points - calculate point's local coords(center of 3D Bbox is the origin)
			for (int k = 0; k < polygonPointsCount - 1; k++)  // polygonPointsCount - 1 because the last points duplicates the first one
			{
				vector worldCoords;
				worldCoords[0] = polygon.GetPart(j)[k][0] + x_shift;
				worldCoords[2] = polygon.GetPart(j)[k][2] + y_shift;
				worldCoords[1] = m_API.GetTerrainSurfaceY(worldCoords[0], worldCoords[2]);
				data.points.Insert(worldCoords);
				
				string pointPos = "";
				pointPos += (worldCoords[0] - polygonOrigin[0]).ToString();
				pointPos += " ";
				pointPos += (worldCoords[1] - polygonOrigin[1]).ToString();
				pointPos += " ";
				pointPos += (worldCoords[2] - polygonOrigin[2]).ToString();
	
				m_API.CreateObjectArrayVariableMember(entSrc, null, "Points", "ShapePoint", k);
	
				auto containerPath = new array<ref ContainerIdPathEntry>();
				auto entry = new ContainerIdPathEntry("Points", k);
				containerPath.Insert(entry);
				m_API.SetVariableValue(entSrc, containerPath, "Position", pointPos);
			}
	
			// setup polygons colors
			float finalR = ShapeColor[0];
			float finalG = ShapeColor[1];
			float finalB = ShapeColor[2];
			
			if (j != 0)
			{
				//invert default color to better distinguish inner polygons(holes) from outer(main) polygon
				finalR = Math.AbsFloat(finalR - 1);
				finalG = Math.AbsFloat(finalG - 1);
				finalB = Math.AbsFloat(finalB - 1);
			}
			
			obj = m_API.SourceToEntity(entSrc);
			m_API.ModifyEntityKey(obj, "LineColor", finalR.ToString() + " " + finalG.ToString() + " " + finalB.ToString() + " 1");
			
			obj = m_API.SourceToEntity(entSrc);
			data.source = entSrc;
			data.entity = obj;
			data.GenerateAAB();
			
			if (shape.GetAttributes().HasAttrib(m_sIDColumnName))
				data.id = shape.GetAttributes().GetIntByName(m_sIDColumnName);
			else
				data.id = -1;
			
			m_aShapesData.Insert(data);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void Load_polylines(GeoShape shape)
	{
		float traceX;
		float traceZ;
		float pntY;
		
		GeoPolyline polyline = GeoPolyline.Cast(shape);
		int polylinePointsCount = polyline.GetVertices().Count();
	
		traceX = polyline.GetVertices()[0][0] + x_shift;
		traceZ = polyline.GetVertices()[0][2] + y_shift;
		pntY = m_API.GetTerrainSurfaceY(traceX, traceZ);
		vector bb3Dmin;
		vector bb3dmax;
		bb3Dmin[0] = traceX;
		bb3Dmin[1] = pntY;
		bb3Dmin[2] = traceZ;
		
		bb3dmax[0] = traceX;
		bb3dmax[1] = pntY;
		bb3dmax[2] = traceZ;
		
		//polyline's points - calculate polyline's 3D Bbox
		for (int k = 0; k < polylinePointsCount; k++)
		{
			traceX = polyline.GetVertices()[k][0] + x_shift;
			traceZ = polyline.GetVertices()[k][2] + y_shift;
			pntY = m_API.GetTerrainSurfaceY(traceX, traceZ);
			
			vector pnt;
			pnt[0] = traceX;
			pnt[1] = pntY;
			pnt[2] = traceZ;
			
			for (int l = 0; l < 3; l++)
			{
				float val = pnt[l];
				if (val < bb3Dmin[l])
					bb3Dmin[l] = val;
				if (val > bb3dmax[l])
					bb3dmax[l] = val;
			}
		}
		
		//calculate polygon's origin(as center of 3D Bbox)
		vector polylineOrigin;
		polylineOrigin = ((bb3dmax - bb3Dmin) / 2) + bb3Dmin;
	
		// create PolylineEntity
		vector orienatation = "0 0 0";
		IEntity obj;
		
		if (m_bSpline)
			obj = m_API.CreateEntity("SplineShapeEntity", "", m_API.GetCurrentEntityLayerId(), null, polylineOrigin, orienatation);
		else
			obj = m_API.CreateEntity("PolylineShapeEntity", "", m_API.GetCurrentEntityLayerId(), null, polylineOrigin, orienatation);
	
		IEntitySource entSrc = m_API.EntityToSource(obj);
		ForestGeneratorShapeImportData data = new ForestGeneratorShapeImportData();
		
		//polyline's points - calculate point's local coords(center of 3D Bbox is the origin)
		for (int k = 0; k < polylinePointsCount; k++)
		{
			vector worldCoords;
			worldCoords[0] = polyline.GetVertices()[k][0] + x_shift;
			worldCoords[2] = polyline.GetVertices()[k][2] + y_shift;
			worldCoords[1] = m_API.GetTerrainSurfaceY(worldCoords[0], worldCoords[2]);
			data.points.Insert(worldCoords);
			data.GenerateAAB();
			
			string pointPos = "";
			pointPos += (worldCoords[0] - polylineOrigin[0]).ToString();
			pointPos += " ";
			pointPos += (worldCoords[1] - polylineOrigin[1]).ToString();
			pointPos += " ";
			pointPos += (worldCoords[2] - polylineOrigin[2]).ToString();
	
			m_API.CreateObjectArrayVariableMember(entSrc, null, "Points", "ShapePoint", k);
	
			auto containerPath = new array<ref ContainerIdPathEntry>();
			auto entry = new ContainerIdPathEntry("Points", k);
			containerPath.Insert(entry);
			m_API.SetVariableValue(entSrc, containerPath, "Position", pointPos);
		}
		
		m_API.ModifyEntityKey(obj, "LineColor", ShapeColor[0].ToString() + " " + ShapeColor[1].ToString() + " " + ShapeColor[2].ToString() + " 1");
		
		data.source = entSrc;
		data.entity = obj;
		data.GenerateAAB();
		
		if (shape.GetAttributes().HasAttrib(m_sIDColumnName))
			data.id = shape.GetAttributes().GetIntByName(m_sIDColumnName);
		else
			data.id = -1;
		
		m_aShapesData.Insert(data);
	}
	
	//------------------------------------------------------------------------------------------------
	void LoadPrefabsList()
	{
		if (m_PrefabsListPath == string.Empty)
			return;
		
		SCR_PrefabsList prefabsList;
		
		Resource holder = BaseContainerTools.LoadContainer(m_PrefabsListPath);
		if (!holder)
			return;
		
		prefabsList = SCR_PrefabsList.Cast(BaseContainerTools.CreateInstanceFromContainer(holder.GetResource().ToBaseContainer()));
		if (!prefabsList)
			return;
		
		prefabsList.SetConfigPath(m_PrefabsListPath);
		
		m_PrefabsList = prefabsList;
	}
	
	//------------------------------------------------------------------------------------------------
	void AttachChild(ForestGeneratorShapeImportData data)
	{
		IEntitySource source = data.source;
		IEntity shape = data.entity;
		ResourceName chosenPrefab = childPrefab;
		
		if (m_bChildrenFromList)
		{
			if (!m_PrefabsList)
				Print("No prefab list selected!", LogLevel.ERROR);
			else
			{
				foreach (SCR_PrefabListObject prefab : m_PrefabsList.m_Prefabs)
				{
					if (data.id == prefab.m_iID)
						chosenPrefab = prefab.m_Prefab;
				}
			}
		}
		
		if (chosenPrefab.GetPath() != "")
			IEntity child = m_API.CreateEntity(chosenPrefab, "", m_API.GetCurrentEntityLayerId(), source, vector.Zero, vector.Zero);
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsPointDuplicate(vector point, array<vector> points)
	{
		for (int i = 0, count = points.Count(); i < count; i++)
		{
			checks++;
			if (vector.Distance(point, points[i]) < 0.1)
			{
				return true;
			}
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void GenerateForestGeneratorPointData()
	{
		WorldEditor we = Workbench.GetModule(WorldEditor);
		//WBProgressDialog progress = new WBProgressDialog("Generating forest generator points data...", we);
		for (int i = 0, count = m_aShapesData.Count(); i < count; i++)
		{
			BaseContainerList points = m_aShapesData[i].source.GetObjectArray("Points");
			array<ForestGeneratorShapeImportData> collidedShapes = new array<ForestGeneratorShapeImportData>();
			
			for (int y = 0; y < count; y++)
			{
				if (y == i)
					continue;
				if (!m_aShapesData[i].bbox.DetectCollision2D(m_aShapesData[y].bbox))
					continue;
				collidedShapes.Insert(m_aShapesData[y]);
			}
			
			bool wasPreviousDuplicate = false;
			
			m_aShapesData[i].points.Insert(m_aShapesData[i].points[0]); // Duplicate first point
			
			for (int p = 0, countPoints = m_aShapesData[i].points.Count(); p < countPoints; p++)
			{
				bool isDuplicate = false;
				for (int y = 0, otherCount = collidedShapes.Count(); y < otherCount; y++)
				{
					if (IsPointDuplicate(m_aShapesData[i].points[p], collidedShapes[y].points))
					{
						isDuplicate = true;
						if (wasPreviousDuplicate)
						{
							m_API.CreateObjectArrayVariableMember(points[p - 1], null, "Data", "ForestGeneratorPointData", 0);
							
							BaseContainerList dataArr = points[p - 1].GetObjectArray("Data");
							int dataCount = dataArr.Count();
							for (int j = 0; j < dataCount; ++j)
							{
								BaseContainer data = dataArr.Get(j);
								if (data.GetClassName() == "ForestGeneratorPointData")
								{
									auto containerPath = new array<ref ContainerIdPathEntry>();
									auto entry1 = new ContainerIdPathEntry("Points", p - 1);
									containerPath.Insert(entry1);
									auto entry2 = new ContainerIdPathEntry("Data", j);
									containerPath.Insert(entry2);
									m_API.SetVariableValue(m_aShapesData[i].source, containerPath, "m_bSmallOutline", "false");
									m_API.SetVariableValue(m_aShapesData[i].source, containerPath, "m_bMiddleOutline", "false");
									break;
								}
							}
						}
						wasPreviousDuplicate = true;
						break;
					}
				}
				if (isDuplicate)
					continue;
				wasPreviousDuplicate = false;
			}
			
			m_aShapesData[i].points.Remove(m_aShapesData[i].points.Count() - 1); // Remove the duplicate point
		}
	}
};