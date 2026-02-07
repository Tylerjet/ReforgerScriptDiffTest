[WorkbenchToolAttribute("Import objects", "Import data and create entities", "2", awesomeFontCode: 0xf0d0)]
class ObjectImportTool: WorldEditorTool
{
	[Attribute("", UIWidgets.ResourceNamePicker, "csv where space is the separator", "csv")]
	ResourceName DataPath;
	
	[Attribute("true", UIWidgets.CheckBox, "Makes Y coordinate relative to terrain vs absolute in world")]
	bool PosYRelativeToTerrain;
	
	[ButtonAttribute("Import objects")]
	void ImportData()
	{
		Print("Importing entities");
		ParseHandle parser = FileIO.BeginParse(DataPath.GetPath());
		if (parser == 0)
		{
			Print("Could not create parser from file: " + DataPath.GetPath(), LogLevel.ERROR);
			return;
		}

		int quatStart = 4;
		int posStart = 1;
		int tagStart = 0;
		int scaleStart = 8;
		array<string> tokens = new array<string>();
		if (m_API.BeginEntityAction())
		{
			for (int i = 0; true; ++i)
			{
				int numTokens = parser.ParseLine(i, tokens);
							
				if (numTokens == 0)
					break;
				
				if (numTokens != 9)
				{
					Print("Line " + i + ": Invalid data format expected 9 tokens got " + numTokens, LogLevel.ERROR);
					for (int j = 0; j < numTokens; ++j)
					{
						Print(tokens[j]);
					}
					continue;
				}
				
				float quat[4];
				for (int j = quatStart; j < quatStart + 4; ++j)
				{
					quat[j - quatStart] = tokens[j].ToFloat();
				}
				quat[2] = -quat[2]; // flip z
				quat[3] = -quat[3]; // flip rotation because of handiness
				Math3D.QuatNormalize(quat);
				
				vector pos;
				for (int j = posStart; j < posStart + 3; ++j)
				{
					pos[j - posStart] = tokens[j].ToFloat();
				}
				pos[2] = -pos[2]; // flip z
				
				if (PosYRelativeToTerrain)
				{
					pos[1] = pos[1] + m_API.GetTerrainSurfaceY(pos[0], pos[2]);
				}
							
				string resourceHash = tokens[tagStart];
				float scale = tokens[scaleStart].ToFloat();
				
				vector angles = Math3D.QuatToAngles(quat);
				// Convert angles from (yaw, pitch, roll) to (xRotate, yRotate, zRotate) as expected by `CreateEntity`
				float tmp = angles[0];
				angles[0] = angles[1];
				angles[1] = tmp;

				ResourceName entityResource = resourceHash;
				if (entityResource.GetPath() == "")
				{
					Print("Line " + i + ": Hash " + resourceHash + " does not refer to any resource", LogLevel.ERROR);
					continue;
				}
				
				//Print("Creating " + entityResource.GetPath() + " on: " + pos + " angles: " + angles + " scale: " + scale, LogLevel.VERBOSE);
				IEntitySource ent = m_API.CreateEntity(resourceHash, "", m_API.GetCurrentEntityLayerId(), null, pos, angles);
				if (!ent)
				{
					Print("Line " + i + ": Entity could not be created", LogLevel.ERROR);
					continue;
				}
				m_API.SetVariableValue(ent, null, "scale", scale.ToString());
			}
			m_API.EndEntityAction();
		}
		
		parser.EndParse();
		Print("Import done");
	}
}