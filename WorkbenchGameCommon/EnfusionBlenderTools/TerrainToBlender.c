#ifdef WORKBENCH

[WorkbenchToolAttribute(name: "Send Terrain To Blender", description: "Sends terrain selection to Blender for advanced terrain modifications.", wbModules: { "WorldEditor" }, shortcut: "Ctrl+P", awesomeFontCode: 0xf00a)]
class TerrainExportTool : WorldEditorTool
{
	[Attribute(defvalue: "1", desc: "Uniform scale for mouse selection", category: "Selection")]
	bool m_bRectangleSelection;
	[Attribute(defvalue: "0", UIWidgets.Coords, desc: "X width of rectangle from starting point", category: "Selection")]
	int m_iWidth;
	[Attribute(defvalue: "0", UIWidgets.Coords, desc: "Z height of rectangle from starting point", category: "Selection")]
	int m_iLength;
	[Attribute(defvalue: "0 0 0", desc: "Location of start corner. No need to fill Y-axis", category: "Selection")]
	vector m_vStartLocation;
	[Attribute(defvalue: "0 0 0", desc: "Location of end corner. No need to fill Y-axis", category: "Selection")]
	vector m_vEndLocation;


	ref DebugTextScreenSpace m_text;
	ref DebugTextScreenSpace m_crossHair;
	vector previousPoint3D;
	ref Shape polyline;
	vector currentPoint3D;
	vector line[8];
	vector startPosition;
	bool clicked = false;



	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Import To Blender")]
	protected void Execute()
	{
		if (!EBTConfigPlugin.HasBlenderRegistered())
			return;
		
		// getting coords from the selection shape
		float zMax = line[0][2];
		float xMin = line[0][0];
		float zMin = line[4][2];
		float xMax = line[4][0];
		// getting cellsize
		float cellSize = m_API.GetTerrainUnitScale();

		// corecting the Max and Mix values
		if (zMax < zMin)
		{
			zMin = zMin + zMax;
			zMax = zMin - zMax;
			zMin = zMin - zMax;
		}
		if (xMax < xMin)
		{
			xMin = xMin + xMax;
			xMax = xMin - xMax;
			xMin = xMin - xMax;
		}

		if (startPosition != "0 0 0")
		{
			// getting Y coords of the selection
			array<float> height = new array<float>;
			height = GetHeightArray(SnapToVertex(xMax, cellSize), SnapToVertex(zMax, cellSize), SnapToVertex(xMin, cellSize), SnapToVertex(zMin, cellSize), m_API, cellSize);
			string path;

			// creating temp bin file to pass the coords
			Workbench.GetAbsolutePath("$profile:", path);
			path = path + "/BlendTerrain.bin";

			// progress bar
			WorldEditor we = Workbench.GetModule(WorldEditor);
			WBProgressDialog progress = new WBProgressDialog("Processing", we);
			
			// writing the coords to the bin file
			FileHandle bin = FileIO.OpenFile(path, FileMode.WRITE);
			bin.WriteArray(height);
			bin.Close();

			// getting all necessary parameters for the blender
			string worldpath;
			m_API.GetWorldPath(worldpath);
			string coords = string.Format("%1|%2", SnapToVertex(xMin, cellSize), SnapToVertex(zMin, cellSize));
			int columns = Math.Round(((SnapToVertex(xMax, cellSize) - SnapToVertex(xMin, cellSize)) / cellSize));
			int rows = Math.Round(((SnapToVertex(zMax, cellSize) - SnapToVertex(zMin, cellSize)) / cellSize));

			string pathToExecutable;
			if (!EBTConfigPlugin.GetDefaultBlenderPath(pathToExecutable))
				return;
			
			// run Blender via CMD with the parameters and the path to the bin temp
			Workbench.RunProcess(string.Format("\"%1\" --python-expr \"import bpy;bpy.ops.scene.ebt_import_terrain()\" -- -binPath \"%2\" -count %3 -rows %4 -columns %5 -cellsize %6 -coords \" %7\" -worldpath \"%8\"",
			pathToExecutable, path, height.Count(), rows, columns, cellSize, coords, worldpath));
			Print("Blender is starting");
		}
		else
		{
			Print("No area selected!");
		}
	}


	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Use Coords")]
	protected void CoordsReload()
	{
		// setting lines points to the coords set by user
		line[0] = m_vStartLocation;
		line[1] = Vector(m_vStartLocation[0], m_vStartLocation[1], m_vEndLocation[2]);
		line[2] = line[1];
		line[3] = Vector(m_vEndLocation[0], m_vStartLocation[1], m_vEndLocation[2]);
		line[4] = line[3];
		line[5] = Vector(m_vEndLocation[0], m_vStartLocation[1], m_vStartLocation[2]);
		line[6] = line[5];
		line[7] = line[0];

		// creating the polyline
		polyline = Shape.CreateLines(ARGB(255, 255, 255, 255), ShapeFlags.NOZBUFFER|ShapeFlags.TRANSP, line, 8);
	}
	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Use Scale")]
	protected void ScaleReload()
	{
		if (startPosition == ("0 0 0"))
		{
			Print("No start position selected!");
		}
		else
		{
			// calculating the lines points from the startPosition with the scale
			line[0] = startPosition;
			line[1] = Vector(startPosition[0], startPosition[1], startPosition[2] + m_iLength);
			line[2] = line[1];
			line[3] = Vector(startPosition[0] + m_iWidth, startPosition[1], startPosition[2] + m_iLength);
			line[4] = line[3];
			line[5] = Vector(startPosition[0] + m_iWidth, startPosition[1], startPosition[2]);
			line[6] = line[5];
			line[7] = line[0];

			polyline = Shape.CreateLines(ARGB(255, 255, 255, 255), ShapeFlags.NOZBUFFER|ShapeFlags.TRANSP, line, 8);
		}

	}
	//------------------------------------------------------------------------------------------------

	override void OnMouseMoveEvent(float x, float y)
	{
		vector traceStart;
		vector traceEnd;
		vector traceDir;

		// setting visual parameters
		m_crossHair.SetTextColor(ARGBF(1, 1.0, 1.0, 1.0));
		m_text.SetTextColor(ARGBF(1, 1.0, 1.0, 1.0));
		m_crossHair.SetPosition(x - 9, y - 16);
		m_crossHair.SetText("+");
		m_text.SetPosition(x + 15, y);


		if (m_API.TraceWorldPos(x, y, TraceFlags.WORLD, traceStart, traceEnd, traceDir))
		{
			currentPoint3D = traceEnd;
			if (clicked)
			{
				// calculating the rectangle from the startPoint and mousePosition
				if (m_bRectangleSelection)
				{
					float zLength = Math.Max(Math.AbsFloat(startPosition[0] - traceEnd[0]), Math.AbsFloat(startPosition[2] - traceEnd[2]));
					float xLength = Math.Max(Math.AbsFloat(startPosition[0] - traceEnd[0]), Math.AbsFloat(startPosition[2] - traceEnd[2]));

					//Flipping Selection box
					if (traceEnd[0] < startPosition[0] && traceEnd[2] < startPosition[2])
					{
						xLength = -xLength;
						zLength = -zLength;
					}
					else if (traceEnd[2] < startPosition[2])
					{
						zLength = -zLength;
					}
					else if (traceEnd[0] < startPosition[0])
					{
						xLength = -xLength;
					}
					line[0] = startPosition;
					line[1] = Vector(startPosition[0], startPosition[1], startPosition[2] + zLength);
					line[2] = line[1];
					line[3] = Vector(startPosition[0] + xLength, startPosition[1], startPosition[2] + zLength);
					line[4] = line[3];
					line[5] = Vector(startPosition[0] + xLength, startPosition[1], startPosition[2]);
					line[6] = line[5];
					line[7] = line[0];
				}
				// calculating square
				else
				{
					line[0] = startPosition;
					line[1] = Vector(startPosition[0], startPosition[1], traceEnd[2]);
					line[2] = line[1];
					line[3] = Vector(traceEnd[0], startPosition[1], traceEnd[2]);
					line[4] = line[3];
					line[5] = Vector(traceEnd[0], startPosition[1], startPosition[2]);
					line[6] = line[5];
					line[7] = line[0];
				}

				polyline = Shape.CreateLines(ARGB(255, 255, 255, 255), ShapeFlags.NOZBUFFER|ShapeFlags.TRANSP, line, 8);
			}
		}
	}

	override void OnMousePressEvent(float x, float y, WETMouseButtonFlag buttons)
	{
		vector traceStart;
		vector traceEnd;
		vector traceDir;
		if (m_API.TraceWorldPos(x, y, TraceFlags.WORLD, traceStart, traceEnd, traceDir))
			{
				if (!clicked)
				{
					// setting start position right before the user clicks
					currentPoint3D = traceEnd;
					startPosition = currentPoint3D;
					previousPoint3D = currentPoint3D;
					clicked = true;
				}
				else
				{
					// start creating the polyline
					polyline = Shape.CreateLines(ARGB(255, 255, 255, 255), ShapeFlags.NOZBUFFER|ShapeFlags.TRANSP, line, 8);
					clicked = false;
				}
			}
	}
	override void OnKeyPressEvent(KeyCode key, bool isAutoRepeat)
	{
		if (key == KeyCode.KC_ESCAPE && isAutoRepeat == false)
		{
			// reseting all positions
			previousPoint3D = "0 0 0";
			startPosition = "0 0 0";
			clicked = false;
			polyline = Shape.CreateLines(ARGB(255, 255, 0, 0), ShapeFlags.NOZBUFFER|ShapeFlags.TRANSP, line, 1);
		}
	}
	override void OnActivate()
	{
		EBTConfigPlugin.UpdateRegisteredBlenderExecutables();
		// showing cross
		m_text = DebugTextScreenSpace.Create(m_API.GetWorld(), "", 0, 100, 100, 14, ARGBF(1, 1, 1, 1), 0x00000000);
		m_crossHair = DebugTextScreenSpace.Create(m_API.GetWorld(), "", 0, 0, 0, 30, ARGBF(1, 1, 1, 1), 0x00000000);
	}
	override void OnDeActivate()
	{
		// reseting
		m_text = null;
		m_crossHair = null;
		polyline = null;
		startPosition = "0 0 0";
	}

	// gets Y coords from the selection
	array<float> GetHeightArray(float xMax, float zMax, float xMin, float zMin, WorldEditorAPI api, float cellSize)
	{
		WorldEditor we = Workbench.GetModule(WorldEditor);
		WBProgressDialog progress = new WBProgressDialog("Processing...", we);
		int diff = (zMax - zMin) / cellSize;
		int count = 0;
		// setting constant to fix Floating Point Error when iterating with floats
		// cellsize can't be lower than 0.1 so 100 should be enough to convert it to int
		const int FLOAT_FIX = 100;
		array<float> coords = new array<float>;
		// rows from max to min
		for (int z = zMax*FLOAT_FIX; z >= zMin*FLOAT_FIX; z -= cellSize*FLOAT_FIX)
		{
			// columns from max to min
			progress.SetProgress(count / diff);
			count += 1;
			for (int x = xMax*FLOAT_FIX; x >= xMin*FLOAT_FIX; x -= cellSize*FLOAT_FIX)
			{
				coords.Insert(api.GetTerrainSurfaceY(x/FLOAT_FIX, z/FLOAT_FIX));
			}
		}
		return coords;
	}

	// snapping coords to the nearest vertex using cellSize
	float SnapToVertex(float coord, float cellSize)
	{
		float vertex_coord = Math.Round(coord / cellSize) * cellSize;
		return vertex_coord;
	}
}

#endif 

