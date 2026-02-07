[WorkbenchToolAttribute("Coords Tool", "Tool that allows you to navigate to coordinates or copy current ones easily.\n\nThis tool provides two options of input:\n1) Enter coordinates into Position and Rotation field manually as\n X,Y,Z and Pitch,Yaw,Roll in degrees then click on the Goto Coords button.\n\n2) Enter enfusion protocol link or string in the following format\n \"x,y,z;pitch,yaw,roll\" into the String field and press the Goto StringLink button.\nNote: This does NOT change current world to one in link if provided.\n\n\nYou can always copy current coordinates by pressing the Copy Link button.", awesomeFontCode : 0xf5a0)]
class CoordsTool : WorldEditorTool
{
	[Attribute("0 0 0", UIWidgets.Auto, "VectorCoordinates: Position in world space to set the camera position to.", category: "VectorCoordinates")]
	protected vector m_vPosition;

	[Attribute("0 0 0", UIWidgets.Auto, "VectorCoordinates: Angles as pitch, yaw, roll to set the camera rotation to.", category: "VectorCoordinates")]
	protected vector m_vRotation;

	[Attribute("", UIWidgets.EditBox, "StringCoordinates: *Enfusion protocol link* or coordinates string as \"x,y,z;pitch,yaw,roll\".", category: "StringCoordinates")]
	protected string m_sString;

	//------------------------------------------------------------------------------------------------
	/*!
		Return enfusion protocol link for world editor.
		\param transformation The transformation to create the link at.
	*/
	static string GetWorldEditorLink(vector transformation[4])
	{
		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		if (!worldEditor)
			return string.Empty;

		WorldEditorAPI editorApi = worldEditor.GetApi();
		if (!editorApi)
			return string.Empty;

		string fullLink;
		editorApi.GetWorldPath(fullLink);
		if (!fullLink || fullLink.IsEmpty())
			return string.Empty;

		// Fetch position
		vector position = transformation[3];

		// Fetch angles
		vector angles = Math3D.MatrixToAngles(transformation);

		// We want to substring only /worlds/...
		// to prevent exposing local folders, etc.
		int begin = fullLink.IndexOf("worlds\\");
		if (begin == -1)
			begin = fullLink.IndexOf("worlds/");
		if (begin == -1)
			return string.Empty;
		string worldPath = fullLink.Substring(begin, fullLink.Length() - begin);

		// Create link
		string link = string.Format(
		"enfusion://WorldEditor/%1;%2,%3,%4;%5,%6,%7",
		worldPath,
		position[0],
		position[1],
		position[2],
		angles[1],
		angles[0],
		angles[2]);

		// Have consistent link
		link.Replace("\\", "/");

		// Print it to console
		return link;
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Sets world editor camera position and rotation.
		\param pos Position in world space.
		\param rot Rotation as pitch, yaw, roll in degrees.
	*/
	private void SetCamera(vector pos, vector rot)
	{
		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		if (!worldEditor)
			return;

		WorldEditorAPI editorApi = worldEditor.GetApi();
		if (!editorApi)
			return;

		vector yawPitchRoll;
		yawPitchRoll[0] = rot[1];
		yawPitchRoll[1] = rot[0];
		yawPitchRoll[2] = rot[2];

		vector lookDirection = yawPitchRoll.AnglesToVector();
		editorApi.SetCamera(pos, lookDirection);
		Print(string.Format("Camera set to:\nPosition=<%1,%2,%3>\nRotation=<%4,%5,%6>",
		pos[0], pos[1], pos[2],
		rot[0], rot[1], rot[2]));
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Sets world editor camera position and rotation.
		\param pos Position in world space.
		\param rot Rotation as pitch, yaw, roll in degrees.
	*/
	private bool GetCamera(out vector transform[4])
	{
		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		if (!worldEditor)
			return false;

		WorldEditorAPI editorApi = worldEditor.GetApi();
		if (!editorApi)
			return false;

		BaseWorld world = editorApi.GetWorld();
		if (!world)
			return false;

		world.GetCurrentCamera(transform);
		return true;
	}


	//------------------------------------------------------------------------------------------------
	/*!
		Parses string for coordinates and returns true in case of success, false otherwise.
	*/
	private bool ParseString(string str, out vector pos, out vector rot)
	{
		if (!str)
			return false;

		if (str.IsEmpty())
			return false;


		string coordsLink;
		// Try parse as enflink to get coords
		// enf link is world;pos;rot
		if (str.Contains(";") && str.StartsWith("enfusion"))
		{
			int begin = str.IndexOf(";");
			coordsLink = str.Substring(begin+1, str.Length() - begin-1);
		}
		else
			coordsLink = str;

		// split coords into two
		array<string> values = new array<string>();
		coordsLink.Split(";", values, true);

		// check validity
		int length = values.Count();
		if (length < 1)
			return false;

		// parse values
		string posString = values[0];
		posString.Replace(",", " ");
		if (posString.IsEmpty())
			return false;
		vector tempPos = posString.ToVector();
		vector tempRot = vector.Zero;

		if (length > 1)
		{
			string rotString = values[1];
			if (!rotString.IsEmpty())
			{
				rotString.Replace(",", " ");
				tempRot = rotString.ToVector();
			}
		}

		pos = tempPos;
		rot = tempRot;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Copies current transform and generates enfusion link, which is copied to clipboard.
	*/
	[ButtonAttribute("Copy link")]
	protected void CopyLink()
	{
		vector transform[4];
		if (GetCamera(transform))
		{
			string link = GetWorldEditorLink(transform);
			if (!link.IsEmpty())
			{
				System.ExportToClipboard(link);
				Print("Link copied to clipboard:\n"+link);
				return;
			}
		}


		Print("Link couldn't be copied, an error occurred.", LogLevel.WARNING);
	}

	//------------------------------------------------------------------------------------------------
	/*!
		When clicked by the user, sets the camera to desired position.
	*/
	[ButtonAttribute("Goto Coords")]
	protected void NavigateToCoords()
	{
		SetCamera(m_vPosition, m_vRotation);
	}

	[ButtonAttribute("Goto StringLink")]
	protected void NavigateToStringLink()
	{
		vector pos;
		vector rot;
		if (ParseString(m_sString, pos, rot))
			SetCamera(pos, rot);
		else
			Print("String is in invalid format. Expected enfusion protocol link or in following format: \"x,y,z;pitch,yaw,roll\"!");
	}


	//------------------------------------------------------------------------------------------------
	void CoordsTool(IEntitySource src, IEntity parent)
	{
	}

	//------------------------------------------------------------------------------------------------
	void ~CoordsTool()
	{
	}
};