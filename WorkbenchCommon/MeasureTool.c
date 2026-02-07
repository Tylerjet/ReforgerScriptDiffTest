[WorkbenchToolAttribute("Measure Tool", "Measure lenght along line in 3D\n(works only on a terrain entity)\n\nLine segment lenght rendered at the center of segment.\nTotal lenght rendered next to the cursor.\n\nLMB - Start measuring\nESC - Cancel measuring", "M", awesomeFontCode: 0xf545)]
class MeasureTool: WorldEditorTool
{
	ref DebugTextScreenSpace m_text;
	ref DebugTextScreenSpace m_crossHair;
	ref array<ref DebugTextWorldSpace> m_segments_lenght;
	ref Shape polyline;

	vector previousPoint3D;
	vector currentPoint3D;
	float distancePrevious;
	float distanceCurrent;
	vector linePts[1000];
	int linePtsCount = 0;
	float last_x;
	float last_y;

	override void OnMouseMoveEvent(float x, float y)
	{
		vector traceStart;
		vector traceEnd;
		vector traceDir;

		m_crossHair.SetTextColor(ARGBF(1, 1.0, 1.0, 1.0));
		m_text.SetTextColor(ARGBF(1, 1.0, 1.0, 1.0));
		m_crossHair.SetPosition(x - 9, y - 16);
		m_crossHair.SetText("+");
		m_text.SetPosition(x + 15, y);

		if (m_API.TraceWorldPos(x,y, TraceFlags.WORLD, traceStart, traceEnd, traceDir))
		{
			currentPoint3D = traceEnd;

			if (linePtsCount >= 1)
			{
				distanceCurrent = (previousPoint3D - currentPoint3D).Length();
				m_text.SetText((distanceCurrent + distancePrevious).ToString() + " m");
				linePts[linePtsCount] = traceEnd;
				polyline = Shape.CreateLines(ARGB(255, 255, 255, 255), ShapeFlags.NOZBUFFER|ShapeFlags.TRANSP, linePts, linePtsCount + 1);
				last_x = x;
				last_y = y;
			}
		}
		else
		{
			// line is being drawn and cursor is out of terrain bounds, stick cursor and lenght info to terrain's edge
			if (linePtsCount >= 1)
			{
				m_crossHair.SetPosition(last_x - 9, last_y - 16);
				m_text.SetPosition(last_x + 15, last_y);
			}
			else
			// line is not being drawn and cursor is out of terrain bounds
			{
				m_crossHair.SetText("");
			}
		}
	}

	override void OnMousePressEvent(float x, float y, WETMouseButtonFlag buttons)
	{
		vector traceStart;
		vector traceEnd;
		vector traceDir;
		vector text_position;

		if (linePtsCount == 0)
		{
			previousPoint3D = currentPoint3D;
		}

		if (m_API.TraceWorldPos(x,y, TraceFlags.WORLD, traceStart, traceEnd, traceDir))
		{
			currentPoint3D = traceEnd;
			distancePrevious += distanceCurrent;

			//Render line segments lenght
			if (linePtsCount != 0)
			{
				text_position = previousPoint3D + ((currentPoint3D - previousPoint3D).Normalized() * (previousPoint3D - currentPoint3D).Length() / 2);
				m_segments_lenght.Insert(DebugTextWorldSpace.Create(m_API.GetWorld(), (previousPoint3D - currentPoint3D).Length().ToString() + " m", 0, text_position[0], text_position[1], text_position[2], 15, ARGBF(1, 1, 1, 1), 0x00000000));
			}

			linePts[linePtsCount] = currentPoint3D;
			//Print(linePts[linePtsCount]);
			linePtsCount++;
			//Print(linePtsCount);
			previousPoint3D = currentPoint3D;
		}
	}

	override void OnKeyPressEvent(KeyCode key, bool isAutoRepeat)
	{
		//Cancel measuring
		if (key == KeyCode.KC_ESCAPE && isAutoRepeat == false)
		{
			m_text.SetText("");
			m_segments_lenght.Clear();
			distancePrevious = 0.0;
			distanceCurrent = 0.0;
			previousPoint3D = "0 0 0";
			linePtsCount = 0;
			polyline = Shape.CreateLines(ARGB(255, 255, 0, 0), ShapeFlags.NOZBUFFER|ShapeFlags.TRANSP, linePts, linePtsCount + 1);
		}
	}

	override void OnActivate()
	{
		m_text = DebugTextScreenSpace.Create(m_API.GetWorld(), "", 0, 100, 100, 14, ARGBF(1, 1, 1, 1), 0x00000000);
		m_crossHair = DebugTextScreenSpace.Create(m_API.GetWorld(), "", 0, 0, 0, 30, ARGBF(1, 1, 1, 1), 0x00000000);
		distancePrevious = 0.0;
		m_segments_lenght = new array<ref DebugTextWorldSpace>;
	}

	override void OnDeActivate()
	{
		m_text = null;
		m_crossHair = null;
		m_segments_lenght.Clear();
		distancePrevious = 0.0;
		distanceCurrent = 0.0;
		previousPoint3D = "0 0 0";
		linePtsCount = 0;
		polyline = null;
	}
}
