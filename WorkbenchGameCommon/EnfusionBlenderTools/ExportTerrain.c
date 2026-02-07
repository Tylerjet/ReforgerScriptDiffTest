class ExportTerrainRequest: JsonApiStruct
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
};

class ExportTerrainResponse: JsonApiStruct
{
	bool Output;
	
	void ExportTerrainResponse()
	{
		RegV("Output");
	}
};


class ExportTerrain: NetApiHandler
{
	override JsonApiStruct GetRequest()
	{
		return new ExportTerrainRequest();
	}
	
	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		ExportTerrainRequest req = ExportTerrainRequest.Cast(request);	
		ExportTerrainResponse response = new ExportTerrainResponse();
		
		WorldEditor wm = Workbench.GetModule(WorldEditor);
		WorldEditorAPI api = wm.GetApi();
		string openedWorldPath;
		// if WorldEditor is not opened, open it and load map
		if(!api)
		{	
			Workbench.OpenModule(WorldEditor);
			api = wm.GetApi();
			wm.SetOpenedResource(req.worldPath);
		}
		api.GetWorldPath(openedWorldPath);
		if(openedWorldPath != req.worldPath)
		{
			wm.SetOpenedResource(req.worldPath);
		}
		
		array<float> coords = CoordsFromBin(req.path, req.count);
		ModifyTerrain(api, coords);
		response.Output = true;
		return response;
	}
	
	
	private array<float> CoordsFromBin(string path, int size)
	{
		FileHandle bin = FileIO.OpenFile(path, FileMode.READ);
		float x,y,z;
		array<float> coords = new array<float>;
		
		// read and insert coords from bin
		for(int i = 0; i < size; i+=3)
		{
			bin.Read(x, 4);
			bin.Read(z, 4);
			bin.Read(y, 4);
			coords.Insert(x);
			coords.Insert(z);
			coords.Insert(y);
		}
		bin.Close();
		FileIO.DeleteFile(path);
		return coords;
	}
	
	// coords[X,Z,Y]
	private void ModifyTerrain(WorldEditorAPI api, array<float> coords)
	{
		api.BeginTerrainAction(TerrainToolType.TTT_HEIGHT_EXACT);
		TerrainToolDesc_HeightExact tool = new TerrainToolDesc_HeightExact();	
		
		for(int i = 0; i < coords.Count(); i+=3)
		{
			tool.fExactHeight = coords[i+2];
			api.ModifyHeightMap(coords[i+0], coords[i+1],FilterMorphOperation.MORPH_EXACT, tool, FilterMorphShape.SHAPE_SQUARE, FilterMorphLerpFunc.FUNC_LINEAR);
		}
		api.EndTerrainAction();
	}
}