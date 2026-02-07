// *************************************************************************************
// ! CharacterCamera3rdPersonErc - 3rd person camera in erected stance
// *************************************************************************************
class CharacterCamera3rdPersonUnconscious extends CharacterCamera3rdPersonBase
{
	//-----------------------------------------------------------------------------
	void CharacterCamera3rdPersonUnconscious(CameraHandlerComponent pCameraHandler)
	{
		m_iBoneIndex		= m_OwnerCharacter.GetAnimation().GetBoneIndex("Neck1");
		m_CameraOffsetLS	= "0.0 0.0 0.0";
		m_CameraOffsetMS	= "0.0 0.0 0.0";
		m_fDistance 		= 1.6;
	}
};