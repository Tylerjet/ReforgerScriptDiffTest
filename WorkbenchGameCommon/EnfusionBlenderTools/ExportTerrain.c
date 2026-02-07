class ExportTerrainRequest : JsonApiStruct
{
	ref array<float> newcoords = new array<float>;
	string worldPath;
	string path;
	int count;

	void ExportTerrainRequest()
	{
		RegV("newcoords");
		RegV("worldPath");
		RegV("path");
		RegV("count");
	}
}

class ExportTerrainResponse : JsonApiStruct
{
	bool Output;

	void ExportTerrainResponse()
	{
		RegV("Output");
	}
}


class ExportTerrain : NetApiHandler
{
	override JsonApiStruct GetRequest()
	{
		return new ExportTerrainRequest();
	}

	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		ExportTerrainRequest req = ExportTerrainRequest.Cast(request);
		ExportTerrainResponse response = new ExportTerrainResponse();

		// getting WorldEditor API
		WorldEditor wm = Workbench.GetModule(WorldEditor);
		WorldEditorAPI api = wm.GetApi();
		string openedWorldPath;

		// if WorldEditor is not opened, open it and load correct map
		if (!api)
		{
			Workbench.OpenModule(WorldEditor);
			api = wm.GetApi();
			wm.SetOpenedResource(req.worldPath);
		}

		// if WE is opened check if with a correct map if not open it
		api.GetWorldPath(openedWorldPath);
		if (openedWorldPath != req.worldPath)
		{
			wm.SetOpenedResource(req.worldPath);
		}

		// read the Y coords from Binary temp file
		array<float> coords = CoordsFromBin(req.path, req.count);
		// modify the terrain coresponding to the new coords
		ModifyTerrain(api, coords);

		response.Output = true;
		return response;
	}


	private array<float> CoordsFromBin(string path, int size)
	{
		// open temp bin
		FileHandle bin = FileIO.OpenFile(path, FileMode.READ);
		float x, y, z;
		array<float> coords = new array<float>;

		// read the new coords and remove the file
		bin.ReadArray(coords, 4, size);
		bin.Close();
		FileIO.DeleteFile(path);
		return coords;
	}

	// coords[X,Z,Y]
	private void ModifyTerrain(WorldEditorAPI api, array<float> coords)
	{
		// init tool
		api.BeginTerrainAction(TerrainToolType.HEIGHT_EXACT);
		TerrainToolDesc_HeightExact tool = new TerrainToolDesc_HeightExact();

		// set all new coords
		for (int i = 0; i < coords.Count(); i+=3)
		{
			// the new Y coords
			tool.fExactHeight = coords[i+ 2];
			api.ModifyHeightMap(coords[i+ 0], coords[i+ 1], FilterMorphOperation.EXACT, tool, FilterMorphShape.SQUARE, FilterMorphLerpFunc.LINEAR);
		}
		api.EndTerrainAction();
	}
}
