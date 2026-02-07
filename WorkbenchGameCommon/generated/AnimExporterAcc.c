/*
===========================================
Do not modify, this script is generated
===========================================
*/

sealed class AnimExporterAcc: Managed
{
	proto external void SetProjectName(string filename);
	proto external int FillTakeInfo(string takeName);
	proto external void AddExportForTake(int takeID, string profile, string exportFile, int startFrame, int endFrame, string diffPose);
	proto external int AddBone(int parent, string name);
	proto external void AddTrackKey(int boneID, float pKey[16], int takeID);
	proto external void ExportTake(AnimExportProfileCtx pProfile, string pExportPath);
}
