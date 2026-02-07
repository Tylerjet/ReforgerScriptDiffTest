enum EMapDataExportType
{
	Geometry2D,
	Soundmap,
	Rasterization
};

/*!
Exports data of the current map based on selected type.
*/

[WorkbenchToolAttribute("Export Map data", "Export data of various types from the current world", "", awesomeFontCode: 0xf0ac)]
class WorldMapExportTool: WorldEditorTool
{
	[Attribute(defvalue: "", uiwidget: UIWidgets.FileNamePicker, desc: "Folder where the file will be saved", params: "unregFolders", enums: NULL, category: "Export file")]
	ResourceName destinationPath;
	string exportPath;
	
	[Attribute("0", UIWidgets.ComboBox, "Export type", "", ParamEnumArray.FromEnum(EMapDataExportType), category: "Export file")]
	int type;

	[Attribute("1", UIWidgets.CheckBox, "2D Geometry: Generation of Roads' data", category: "Geometry 2D")]
	bool generateRoads;
	[Attribute("1", UIWidgets.CheckBox, "2D Geometry: Save Roads' data to exported file", category: "Geometry 2D")]
	bool saveRoads;
	
	[Attribute("1", UIWidgets.CheckBox, "2D Geometry: Generation of Powerlines' data", category: "Geometry 2D")]
	bool generatePowerlines;
	[Attribute("1", UIWidgets.CheckBox, "2D Geometry: Save Powerlines' data to exported file", category: "Geometry 2D")]
	bool savePowerlines;
	
	[Attribute("1", UIWidgets.CheckBox, "2D Geometry: Generation of Buildings' data", category: "Geometry 2D")]
	bool generateBuildings;
	[Attribute("1", UIWidgets.CheckBox, "2D Geometry: Save Buildings' data to exported file", category: "Geometry 2D")]
	bool saveBuildings;
	
	[Attribute("1", UIWidgets.CheckBox, "2D Geometry: Generation of Areas' data", category: "Geometry 2D")]
	bool generateAreas;
	[Attribute("1", UIWidgets.CheckBox, "2D Geometry: Save Areas' data to exported file", category: "Geometry 2D")]
	bool saveAreas;
	
	[Attribute("1", UIWidgets.CheckBox, "2D Geometry: Generation of Water Bodies' data", category: "Geometry 2D")]
	bool generateWaterBodies;
	[Attribute("1", UIWidgets.CheckBox, "2D Geometry: Save Water Bodies' data to exported file", category: "Geometry 2D")]
	bool saveWaterBodies;
	
	[Attribute("1", UIWidgets.CheckBox, "2D Geometry: Generation of Hills' data", category: "Geometry 2D")]
	bool generateHills;
	[Attribute("50.0", UIWidgets.Slider, "Minimum hill height", "30 155 5", category: "Geometry 2D")]
	float hillMinimumHeight;
	[Attribute("1", UIWidgets.CheckBox, "2D Geometry: Save Hills' data to exported file", category: "Geometry 2D")]
	bool saveHills;

	[Attribute("0.933 0.933 0.894 1", UIWidgets.ColorPicker, "Rasterization: Land color", category: "Rasterization")]
	ref Color landColor;
	[Attribute("0.463 0.709 0.773 1", UIWidgets.ColorPicker, "Rasterization: Ocean color", category: "Rasterization")]
	ref Color oceanColor;
	[Attribute("1.0", UIWidgets.Slider, "Rasterization: Land scale factor", "0 10 0.1", category: "Rasterization")]
	float scaleLand;
	[Attribute("1.0", UIWidgets.Slider, "Rasterization: Ocean scale factor", "0 10 0.1", category: "Rasterization")]
	float scaleOcean;
	[Attribute("360.0", UIWidgets.Slider, "Rasterization: Height scale factor", "-1000 1000 1.0", category: "Rasterization")]
	float heightScale;
	[Attribute("30.0", UIWidgets.Slider, "Rasterization: Depth scale factor", "-1000 1000 1.0", category: "Rasterization")]
	float depthScale;
	[Attribute("-10.0", UIWidgets.Slider, "Rasterization: Depth Lerp meters", "-1000 1000 1.0", category: "Rasterization")]
	float depthLerpMeters;
	[Attribute("1.33", UIWidgets.Slider, "Rasterization: Shade intensity factor", "0.5 15 0.01", category: "Rasterization")]
	float shadeIntensity;
	[Attribute("1.5", UIWidgets.Slider, "Rasterization: Height intensity factor", "0.5 15 0.01", category: "Rasterization")]
	float heightIntensity;

	[ButtonAttribute("Export")]
	void Execute()
	{
		Export();
	}
	
	//----------------------------------------------------------------------------------------------
	bool Export()
	{
		WorldEditor we = Workbench.GetModule(WorldEditor);
		WorldEditorAPI api = we.GetApi();
		string worldPath;
		api.GetWorldPath(worldPath);

		WBProgressDialog progress = new WBProgressDialog("Processing", we);


		exportPath = destinationPath.GetPath();

		if (type == EMapDataExportType.Geometry2D)
		{
			ExcludeGenerateFlags generateFlags = ExcludeGenerateFlags.ExcludeGenerateNone;
			if(!generateRoads) generateFlags = generateFlags | ExcludeGenerateFlags.ExcludeGenerateRoads;
			if(!generatePowerlines) generateFlags = generateFlags | ExcludeGenerateFlags.ExcludeGeneratePowerlines;
			if(!generateBuildings) generateFlags = generateFlags | ExcludeGenerateFlags.ExcludeGenerateBuildings;
			if(!generateAreas) generateFlags = generateFlags | ExcludeGenerateFlags.ExcludeGenerateAreas;
			if(!generateWaterBodies) generateFlags = generateFlags | ExcludeGenerateFlags.ExcludeGenerateWaterBodies;
			if(!generateHills) generateFlags = generateFlags | ExcludeGenerateFlags.ExcludeGenerateHills;
			ExcludeSaveFlags saveFlags = ExcludeSaveFlags.ExcludeSaveNone;
			if(!saveRoads) saveFlags = saveFlags | ExcludeSaveFlags.ExcludeSaveRoads;
			if(!savePowerlines) saveFlags = saveFlags | ExcludeSaveFlags.ExcludeSavePowerlines;
			if(!saveBuildings) saveFlags = saveFlags | ExcludeSaveFlags.ExcludeSaveBuildings;
			if(!saveAreas) saveFlags = saveFlags | ExcludeSaveFlags.ExcludeSaveAreas;
			if(!saveWaterBodies) saveFlags = saveFlags | ExcludeSaveFlags.ExcludeSaveWaterBodies;
			if(!saveHills) saveFlags = saveFlags | ExcludeSaveFlags.ExcludeSaveHills;
			if(!MapDataExporter.ExportData(EMapDataType.Geometry2D, exportPath, worldPath, hillMinimumHeight, generateFlags, saveFlags))
			{
				Print("Could not export 2D map data.", LogLevel.ERROR);
				return false;
			}
		}
		else if (type == EMapDataExportType.Soundmap)
		{
			if(!MapDataExporter.ExportData(EMapDataType.Soundmap, exportPath, worldPath))
			{
				Print("Could not export Soundmap data.", LogLevel.ERROR);
				return false;
			}
		}
		else if (type == EMapDataExportType.Rasterization)
		{
			if(!MapDataExporter.ExportRasterization(exportPath, worldPath, landColor, oceanColor, scaleLand, scaleOcean, heightScale, depthScale, depthLerpMeters, shadeIntensity, heightIntensity))
			{
				Print("Could not export map rasterization.", LogLevel.ERROR);
				return false;
			}
		}
		return true;
	}
};

[WorkbenchPluginAttribute("World Data Export Plugin", "Open world and export defined data type")]
class WorldDataExport: WorkbenchPlugin
{
	override void RunCommandline()
	{
		ResourceManager rb = Workbench.GetModule(ResourceManager);
		string path;
		rb.GetCmdLine("-world", path);
		string type;
		rb.GetCmdLine("-type", type);
		string dir;
		rb.GetCmdLine("-saveDir", dir);
		
		if (ExportWorldData(path, type, dir))
			Workbench.Exit(0);
		else
			Workbench.Exit(-1);
	}

	bool ExportWorldData(string path, string type, string dir)
	{
		bool success = false;
		Print("WorldDataExport: opening world "+ path, LogLevel.VERBOSE);

		Workbench.OpenModule(WorldEditor);
		WorldEditor we = Workbench.GetModule(WorldEditor);
		if (we.SetOpenedResource(path))
		{
			Sleep(300);

			bool exportMap = true;
			bool exportSound = true;
			bool exportRasterization = true;
			if (type.Length() > 0)
			{
				if (type.StartsWith("2DMap"))
				{
					exportSound = false;
					exportRasterization = false;
				}
				else if (type.StartsWith("SoundMap"))
				{
					exportMap = false;
					exportRasterization = false;
				}
				else if (type.StartsWith("2DRasterization"))
				{
					exportSound = false;
					exportMap = false;
				}
			}

			WorldEditorAPI api = we.GetApi();
			string worldPath;
			api.GetWorldPath(worldPath);

			WBProgressDialog progress = new WBProgressDialog("Processing", we);
			success = true;
			if (exportMap)
			{
				if(!MapDataExporter.ExportData(EMapDataType.Geometry2D, dir, worldPath, 50, ExcludeGenerateFlags.ExcludeGenerateNone, ExcludeSaveFlags.ExcludeSaveNone))
				{
					Print("WorldDataExport: 2DMap export failed!", LogLevel.ERROR);
					success = false;
				}
			}
			if (exportSound)
			{
				if(!MapDataExporter.ExportData(EMapDataType.Soundmap, dir, worldPath))
				{
					Print("WorldDataExport: SoundMap export failed!", LogLevel.ERROR);
					success = false;
				}
			}
			if (exportRasterization)
			{
				//TODO: change to user defined values (along with WorldMapExportTool)
				Color landColor = new Color(0.933, 0.933, 0.894, 1);
				Color oceanColor = new Color(0.463, 0.709, 0.773, 1.0);
				float scaleLand = 1.0;
				float scaleOcean = 1.0;
				float heightScale = 360.0;
				float depthScale = 30.0;
				float depthLerpMeters = -10.0;
				float shadeIntensity = 1.33;
				float heightIntensity = 1.5;

				if(!MapDataExporter.ExportRasterization(dir, worldPath, landColor, oceanColor, scaleLand, scaleOcean, heightScale, depthScale, depthLerpMeters, shadeIntensity, heightIntensity))
				{
					Print("WorldDataExport: Rasterization export failed!", LogLevel.ERROR);
					success = false;
				}
			}

			if (success)
				Print("WorldDataExport: export successful!", LogLevel.VERBOSE);

			Sleep(100);
			we.Close();
		}

		return success;
	}
};