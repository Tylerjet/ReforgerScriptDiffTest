/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Map
* @{
*/

class MapDataExporter
{
	static proto DataExportErrorType ExportData(EMapDataType type, string exportPath, string worldPath, float minimumHillHeight = 50, ExcludeGenerateFlags generateFlags = ExcludeGenerateFlags.ExcludeGenerateNone, ExcludeSaveFlags saveFlags = ExcludeSaveFlags.ExcludeSaveNone);
	static proto DataExportErrorType ExportRasterization(string exportPath, string worldPath, notnull Color landColor, notnull Color oceanColor, float scaleLand, float scaleOcean, float heightScale, float depthScale, float depthLerpMeters, float shadeIntensity, float heightIntensity);
};

/** @}*/
