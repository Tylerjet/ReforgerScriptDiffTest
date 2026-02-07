/*
===========================================
Do not modify, this script is generated
===========================================
*/

#ifdef WORKBENCH

class TxaExporter
{
	proto external TxaErrCode LoadProfiles(string pProfilesPath);
	proto external int GetNumProfiles();
	proto external int GetProfileIndex(string profileName);
	proto external string GetProfileName(int profileIndex);
	proto external int GetProfileChannelCount(int profileIndex);
	proto external string GetProfileChannelName(int profileIndex, int trackIndex);
	proto external string GetProfileChannelGenFn(int profileIndex, int trackIndex);
	proto external TxaErrCode TrackReset(int	profileIndex, int	nKeyframes, int	fps, string	sourceFile, string targetFile);
	proto external TxaErrCode TrackSetChannels(int	nChannels, int	nKeyframes, notnull array<float> pKeyframeData, notnull array<float> pDiffTgData);
	proto external TxaErrCode TrackExport();
	proto external TxaErrCode ErrCode();
	proto external string ErrMsg();
}

#endif // WORKBENCH
