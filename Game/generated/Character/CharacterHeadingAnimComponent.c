/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Character
\{
*/

class CharacterHeadingAnimComponentClass: GenericComponentClass
{
}

//!
class CharacterHeadingAnimComponent: GenericComponent
{
	//! tries to align myPredictedPos so its aligned with real pos
	//! tries to rotate internally so pMyPredictedDir will align with pRealDir
	proto external void AlignPosDirWS(vector pPredictedPos, vector pPredictedDir, vector pTargetPos, vector pTargetDir);
	proto external void AlignPosRotWS(vector pPredictedPos, float pPredictedRot[4], vector pTargetPos, float pTargetRot[4]);
	proto external void ResetAligning();
	proto external bool IsAligning();
}

/*!
\}
*/
