/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup UserAction
* @{
*/

class ScriptedSignalUserAction: ScriptedUserAction
{
	void PerformScriptedContinuousAction(IEntity pOwnerEntity, IEntity pUserEntity, float timeSlice);
	
	proto external float GetMinimumValue();
	proto external float GetMaximumValue();
	proto external float GetCurrentValue();
	proto external void SetSignalValue(float newValue);
};

/** @}*/
