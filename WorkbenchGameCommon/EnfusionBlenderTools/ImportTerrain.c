class ImportTerrainRequest: JsonApiStruct
{
	float zMax;
	float xMin;
	float zMin;
	float xMax;
	float zMaxNew;
	
	void ImportTerrainRequest()
	{
		RegV("zMax");
		RegV("xMin");
		RegV("zMin");
		RegV("xMax");
		RegV("zMaxNew");

	}
};

class ImportTerrainResponse: JsonApiStruct
{
	int columns;
	int rows;
	string coords;
	ref array<float> Ycoords = new array<float>;
	float cellSize;
	int arraySize;
	float zMaxNew;
	string worldPath;

	void ImportTerrainResponse()
	{
		RegV("columns");
		RegV("rows");
		RegV("coords");
		RegV("Ycoords");
		RegV("cellSize");
		RegV("arraySize");
		RegV("zMaxNew");
		RegV("worldPath");
	}
};

class ImportTerrainUtils
{
	// gets Y coords from the selection
	array<float> GetHeightArray(float xMax, float zMax, float xMin, float zMin, WorldEditorAPI api, float cellSize, ImportTerrainResponse response)
	{	
		//Floats now Works
		//Have to fix that buffer size
		// Can use SIZE*cellsize which would be like 10000 mby
		// but wait for peter
		array<float> coords = new array<float>;
		for(int z = zMax*100; z >= zMin*100; z -= cellSize*100)
		{
			if(coords.Count() >= 40000)
			{
				response.zMaxNew = z/100;
				return coords;
			}
			for(int x = xMax*100; x >= xMin*100; x -= cellSize*100)
			{
				coords.Insert(api.GetTerrainSurfaceY(x/100,z/100));
			}
		}
		return coords;
	}
	
	// when coords are floats then I have to snap them to the closest vertex
	float SnapToVertex(float coord, float cellSize)
	{
		float vertex_coord = Math.Round(coord / cellSize) * cellSize;
    	return vertex_coord;
	}
	
};

class ImportTerrain: NetApiHandler
{
	override JsonApiStruct GetRequest()
	{
		return new ImportTerrainRequest();
	}
	
	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		ImportTerrainRequest req = ImportTerrainRequest.Cast(request);
		ImportTerrainResponse response = new ImportTerrainResponse();
		ImportTerrainUtils utils = new ImportTerrainUtils();
		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		WorldEditorAPI api = worldEditor.GetApi();
		
		if(!api)
		{
			Print("World Editor couldn't initialize!");
			return response;
		}
		float cellSize = api.GetTerrainUnitScale();
		// because of loop I have to get the zCoord where it ended
		float zCoord = 0;
		if(req.zMaxNew != req.zMax)
		{
			zCoord = req.zMaxNew;
		}
		else
		{
			zCoord = req.zMax;
		}
		response.Ycoords = utils.GetHeightArray(utils.SnapToVertex(req.xMax, cellSize), 
												utils.SnapToVertex(zCoord, cellSize), 
												utils.SnapToVertex(req.xMin, cellSize), 
												utils.SnapToVertex(req.zMin, cellSize), 
												api, cellSize, response);
		
		
		// offset coords
		response.coords = string.Format("%1 %2",utils.SnapToVertex(req.xMin, cellSize), utils.SnapToVertex(req.zMin, cellSize));
		response.columns = Math.Round(((utils.SnapToVertex(req.xMax, cellSize) - utils.SnapToVertex(req.xMin, cellSize)) / cellSize));
		response.rows = Math.Round(((utils.SnapToVertex(req.zMax, cellSize) - utils.SnapToVertex(req.zMin, cellSize)) / cellSize));
		response.arraySize = ((response.columns + 1) * (response.rows + 1));
		response.cellSize = cellSize;
		api.GetWorldPath(response.worldPath);

		// I can't have progress bar so I have atleast this
		if(zCoord != 0 && response.Ycoords.Count() != response.arraySize)
		{
			Print(Math.Round((req.zMin / zCoord)*100).ToString() + "\%");
		}
		return response;
	}
}