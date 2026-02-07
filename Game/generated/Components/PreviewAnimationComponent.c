/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Components
* @{
*/

class PreviewAnimationComponentClass: AnimationControllerComponentClass
{
};

class PreviewAnimationComponent: AnimationControllerComponent
{
	//! Performs manual graph step (might be needed when owner/component is controlled outside of world simulation and visual state update of skeleton is required)
	proto external void UpdateFrameStep(IEntity owner, float ts);
	//! Changes used anim graph
	proto external void SetGraphResource(IEntity owner, ResourceName graphResource, ResourceName animInstanceResource, string startNode);
	//! Changes used anim instance
	proto external void UpdateAnimInstance(IEntity owner, ResourceName instanceResource);
	//! Changes used ik pose resource
	proto external void SetHandsIKPose(IEntity owner, ResourceName ikPoseResource);
	//! Changes IK state for weapon IKVariable (usually state set once upon creation)
	proto external void SetIkState(bool leftHand, bool rightHand);
	//! Rebinding of entity is necessary in case of changed/modified underlying MeshObject
	proto external void RebindEntity(IEntity owner);
	//! Binds anim command and returns it's ID
	proto external int BindCommand(string commandName);
	proto external void CallCommand(int cmdID, int intParam, float floatParam);
	//! Binds integer variable and returns it's ID
	proto external int BindIntVariable(string varName);
	proto external void SetIntVariable(int varId, int value);
	//! Binds float variable and returns it's ID
	proto external int BindFloatVariable(string varName);
	proto external void SetFloatVariable(int varId, float value);
	//! Binds bool variable and returns it's ID
	proto external int BindBoolVariable(string varName);
	proto external void SetBoolVariable(int varId, bool value);
};

/** @}*/
