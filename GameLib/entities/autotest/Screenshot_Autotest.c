[EntityEditorProps(category: "GameLib/Scripted/Autotest", description:"Screenshot_autotest", dynamicBox: true)]
class Screenshot_AutotestClass: GenericEntityClass
{
}

[EntityEditorProps(category: "GameLib/Scripted/Autotest", description:"Screenshot_waypoint", dynamicBox: true)]
class Screenshot_WaypointClass: GenericEntityClass
{
}

//----------------------------------------------
/**
 * \defgroup Screenshot autotest
 * Autotest entity for taking screenshots. It sets specific camera transformation to values of waypoints which needs to be set as a child. The child needs to be Screenshot_Waypoint entity.
 * First it sets camera transform to the waypoint defined as a child, then it waits a step time on order to have everything loaded and stable FPS, then it takes a screenshots and since the screenshot is taken across several frames, it waits for this as well.
 * When the camera is transformed to the position and rotation of the waypoint entity, the EOnEnter method is called on the waypoint entity. Some custom code can be executed here.
 * Screenshots are save into $profile:ENTITY_NAME directory by default. This can be overriden by executable parameter screenshot-autotest-output-dir (DO NOT PLACE ENDING '/' AFTER THE DIR PATH)
 * Besides the screenshot itself, also a metafile is saved with addition information (position, orientation, fps). The name of the metafile is the same as the screenshot with .txt as a suffix and it is saved into the same location
 * Autotest can also store summary information, the it is stored in the summary.txt file.
 * @{
 */

class Screenshot_Autotest: GenericEntity
{
	[Attribute("Camera1", UIWidgets.EditBox, "Name of camera entity", "")]
	private string m_cameraEntityName; //< name of camera entity which will be used to take screenshots
	[Attribute("", UIWidgets.EditBox, "Autotest description", "")]
	private string m_description; //< description of the autotest

	[Attribute("1", UIWidgets.Slider, "Wait time in the waypoint before taking screenshot", "0 20 0.1")]
	private float m_stepWaitTime; //< this time is waited before taking screenshot
	[Attribute("1", UIWidgets.Slider, "Wait time for the screenshot to be made", "0 20 0.1")]
	private float m_screenshotWaitTime; //< this time is waited for the screenshot to be captured and saved on disk
	[Attribute("60", UIWidgets.Slider, "Which FPS is considered as minimal", "0 240 1")]
	private float m_FPSLimit; //< this time is waited for the screenshot to be captured and saved on disk
	[Attribute("true", UIWidgets.CheckBox, "Generate summary file")]
	private bool m_summary;

	protected float m_timer;
	protected float m_timeFromStart;

	private IEntity m_camera;
	private Screenshot_Waypoint m_waypoint;
	private float  m_timeFromScreenshot;
	private string m_directory;

	private TextWidget m_FPSWidget;

  void Screenshot_Autotest(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT | EntityEvent.FRAME);
		SetFlags(EntityFlags.ACTIVE, true);

		System.GetCLIParam("screenshot-autotest-output-dir", m_directory);

		if (m_directory.Length() == 0)
		{
			m_directory = "$profile:" + GetName();
		}
  }

	void ~Screenshot_Autotest()
	{
		delete m_FPSWidget;
	}

	override void EOnInit(IEntity owner) //!EntityEvent.INIT
	{
		m_camera = g_Game.FindEntity(m_cameraEntityName);
		m_waypoint = Screenshot_Waypoint.Cast(GetChildren());

		if (!m_camera)
		{
			Print("Camera not found!", LogLevel.ERROR);
		}

		if (!m_waypoint)
		{
			Print("No waypoints for screenshots!", LogLevel.WARNING);
		}

		TransformCameraToWaypoint();

		m_FPSWidget = TextWidget.Cast(g_Game.GetWorkspace().CreateWidgetInWorkspace(WidgetType.TextWidgetTypeID, 16, 16, 512, 128, WidgetFlags.VISIBLE, new Color(0.0, 0.0, 0.0, 1.0), 1024));
		m_FPSWidget.SetExactFontSize(64);
	}

	override void EOnFrame(IEntity owner, float timeSlice) //!EntityEvent.FRAME
	{
		if (!m_camera)
		{
			Print("Exiting the game", LogLevel.WARNING);
			g_Game.RequestClose();
		}

		if (!m_waypoint)
		{
			Print("No waypoints for next screenshot");
			if (m_summary)
			{
				string summeryFilename = string.Format("%1/%2", m_directory, "summary.txt");
				MakeSummeryFile(summeryFilename);
			}
			g_Game.RequestClose();
		}

		int remaining = g_Game.WaitPreload(100);
		if (remaining > 0)
		{
			m_FPSWidget.SetColor(new Color(1.0, 1.0, 1.0, 1.0));
			m_FPSWidget.SetText("Preload: remainig " + remaining + " resources");
			return;
		}
		
		int fps = System.GetFPS();
		m_FPSWidget.SetText("FPS " + fps);
		if (fps < m_FPSLimit)
			m_FPSWidget.SetColor(new Color(1.0, 0.0, 0.0, 1.0));
		else
			m_FPSWidget.SetColor(new Color(0.0, 1.0, 0.0, 1.0));

		if (m_timer > m_stepWaitTime && m_timeFromScreenshot == 0)
		{
			m_timer = 0;

			string screenshotFilename = string.Format("%1/%2.bmp", m_directory, m_waypoint.GetName());
			string metadataFilename = screenshotFilename + ".txt";

			m_timeFromScreenshot = m_timeFromStart;

			FileIO.MakeDirectory(m_directory); // no need to create it for every screenshot, but better then have it in constructor or in init, because directory would be created when placing entity into the world in editor which might not be intended
			MakeScreenshotMetafile(metadataFilename);
			System.MakeScreenshot(screenshotFilename);
		}

		if (m_timeFromScreenshot == 0)
		{
			m_timer += timeSlice; // increment timer only when we are not taking screenshots
		}
		else if (m_timeFromScreenshot + m_screenshotWaitTime < m_timeFromStart)
		{
			m_timeFromScreenshot = 0; // screenshot wait time is up, go to the next waypoint
			if(m_waypoint) m_waypoint.EOnExit();
			m_waypoint = Screenshot_Waypoint.Cast(m_waypoint.GetSibling());
			TransformCameraToWaypoint();
		}

		m_timeFromStart += timeSlice;
	}

	private void MakeSummeryFile(string filename)
	{
		FileHandle descrFile = FileIO.OpenFile(filename, FileMode.WRITE);

		if(descrFile != 0)
		{
			if (m_description.Length() > 0)
				descrFile.FPrintln(string.Format("%1", m_description));
			int sizeX = g_Game.GetWorkspace().GetWidth();
			int sizeY = g_Game.GetWorkspace().GetHeight();
			
			descrFile.FPrintln(string.Format("Resolution: %1 x %2 px", sizeX, sizeY));
#ifdef WORKBENCH
			descrFile.FPrintln(string.Format("Entering playmode time: %1 s", g_Game.GetLoadTime() / 1000));
#else
			descrFile.FPrintln(string.Format("Load time: %1 s", g_Game.GetLoadTime() / 1000));
#endif
			descrFile.FPrintln(string.Format("Memory: %1 MB", System.MemoryAllocationKB() / 1024));
			descrFile.FPrintln(string.Format("Allocations: %1", System.MemoryAllocationCount()));
			descrFile.FPrintln(string.Format("Duration: %1 s", m_timeFromStart));
			descrFile.CloseFile();
			Print("Summary file successfully saved into " + filename);
		}
	}

	private void MakeScreenshotMetafile(string filename)
	{
		string content;
		FileHandle descrFile = FileIO.OpenFile(filename, FileMode.WRITE);

		vector position = m_camera.GetOrigin();
		vector orientation = m_camera.GetYawPitchRoll();

		if(descrFile != 0)
		{
			string description = m_waypoint.GetDescription();
			if (description.Length() > 0)
				descrFile.FPrintln(string.Format("%1", description));
			descrFile.FPrintln(string.Format("FPS: %1", System.GetFPS()));
			descrFile.FPrintln(string.Format("Frame time (ms): %1", 1000.0 * System.GetFrameTimeS()));
			descrFile.FPrintln(string.Format("Position: [%1, %2, %3]", position[0], position[1], position[2]));
			descrFile.FPrintln(string.Format("XYZ Rotation: [%1, %2, %3]", orientation[1], orientation[0], orientation[2]));
			
			string link = string.Format("enfusion://WorldEditor/%1;%2,%3,%4;%5,%6,%7", g_Game.GetWorldFile(), position[0], position[1], position[2], orientation[1], orientation[0], orientation[2]);
			descrFile.FPrintln(string.Format("<a href=\"%1\">Link to World Editor</a>", link));
			
			descrFile.CloseFile();
			Print("Screenshot metafile successfully saved into " + filename);
		}
	}

	private void TransformCameraToWaypoint()
	{
		if (m_waypoint && m_camera)
		{
			vector mat[4];
			m_waypoint.GetTransform(mat);
			m_camera.SetTransform(mat);
			m_waypoint.EOnEnter();
			g_Game.BeginPreload(GetWorld(), mat[3], 500);
		}
	}
}

//----------------------------------------------

class Screenshot_Waypoint: GenericEntity
{
	[Attribute("", UIWidgets.EditBox, "Waypoint description", "")]
	private string m_description; //< description of the autotest

	 void Screenshot_Waypoint(IEntitySource src, IEntity parent)
	{
		if (parent)
		{
			SetFlags(EntityFlags.ACTIVE, true);

			vector position;
			float angleX, angleY, angleZ;
			vector rotation; // we cannot supply array as a out param, so we need the floats and then assign them to vector
			src.Get("coords", position);
			src.Get("angleX", angleX);
			src.Get("angleY", angleY);
			src.Get("angleZ", angleZ);

			rotation[1] = angleX;
			rotation[0] = angleY;
			rotation[2] = angleZ;

			vector mat[4];
			Math3D.AnglesToMatrix(rotation, mat);

			mat[3] = position;

			SetTransform(mat); // needs to be done because addchild is counting with the transform of parent which was already applied so we need to set local position and transform

			parent.AddChild(this, -1);
		}
	  }

	void ~Screenshot_Waypoint()
	{

	}

	/**
	\brief This method is called when the waypoint is entered
	*/
	event void EOnEnter()
	{
	}

	event void EOnExit()
	{
	}

	string GetDescription()
	{
		return m_description;
	}
}
//@}