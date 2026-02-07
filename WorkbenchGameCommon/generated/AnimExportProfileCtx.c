/*
===========================================
Do not modify, this script is generated
===========================================
*/

sealed class AnimExportProfileCtx: Managed
{
	private void AnimExportProfileCtx();
	
	static proto ref AnimExportProfileCtx LoadProfile(string exportModulesBasePath);
	proto external int GetNumProfiles();
	proto external string GetProfileName(int index);
	proto external bool HasProfileDiffBones(string exportModulesBasePath);
	proto external string GetErrorString();
};
