/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components_Character
\{
*/

sealed class CharacterAnimGraphComponent: GenericComponent
{
	//! Switch the active animation set instance to a new one.
	//! \param	resNameAnimSetInstance		Resource name of a new anim set instance which must match animation graph.
	//!																		If ResourceName::Null is passed, controller resets to default anim set instance.
	//! \param	blendTime									optional, blend time in seconds
	//! \return														returns true on successful change, false if anim instance does not match the graph
	proto external bool SetAnimSetInstance(ResourceName resNameAnimSetInstance, float blendTime);
	//! Attach a different graph to the named attachment. Returns false on failure.
	//! \param	bindingName								binding name recognized by the main graph (where to attach)
	//! \param resNameAttachedGraph				resource name of the attached graph
	//! \param resNameAttachedInst				resource name of the anim set instance used for the attached graph
	//! \param attachedNodeName						name of the starting node in the attached graph
	proto external bool SetAttachment(string bindingName, ResourceName resNameAttachedGraph, ResourceName resNameAttachedInst, string attachedNodeName);
	//! Remove graph attachment. Returns false when there is no attachment bound under given name.
	proto external bool RemoveAttachment(string bindingName);
	proto external void SetExternalIKPose(ResourceName ikPoseResource);
	//! Binds anim command and returns it's ID
	proto external int BindCommand(string commandName);
	proto external void CallCommand(int cmdID, int intParam, float floatParam);
	proto external void CallCommand4I(int cmdID, int intParam1, int intParam2, int intParam3, int intParam4, float floatParam);
	//! Binds integer variable and returns it's ID
	proto external int BindIntVariable(string varName);
	proto external void SetIntVariable(int varId, int value);
	proto external int GetIntVariable(int varId);
	//! Binds float variable and returns it's ID
	proto external int BindFloatVariable(string varName);
	proto external void SetFloatVariable(int varId, float value);
	proto external float GetFloatVariable(int varId);
	//! Binds bool variable and returns it's ID
	proto external int BindBoolVariable(string varName);
	proto external void SetBoolVariable(int varId, bool value);
	proto external bool GetBoolVariable(int varId);
	proto external int BindTag(string tagName);
	proto external bool IsTag(int tagId);
	proto external bool IsSecondaryTag(int tagId);
	proto external bool IsTagOrSecondaryTag(int tagId);
	proto external int BindEvent(string eventName);
	proto external bool IsEvent(int eventId);
	//! Get event completion if eventId is present.
	proto bool GetEventCompletion(int eventId, float timeSlice, out float completion);
	//! Get event user data if eventId is present.
	proto bool GetEventUserData(int eventId, out string eventUserString, out int eventUserInt);
	proto external int BindIKTarget(string iKTargetName);
	proto external bool SetIKTarget(int targetId, vector position, vector rotationRad);
	proto external bool RemoveIKTarget(int targetId);
	proto external int BindPrediction(string predictionName);
	//! Get prediction data if predictionId is present.
	proto bool GetPredictionData(int predictionId, out vector translation, out float rotation[4], out float time, out float normTime);
	proto external vector GetRootMotionTranslation();
	proto external vector OverrideRootMotionTranslation(vector translation);
	proto void GetRootMotionRotation(out float rotation[4]);
	proto void OverrideRootMotionRotation(float rotation[4]);
	proto external void SetForceEvaluateMotion(bool state);
}

/*!
\}
*/
