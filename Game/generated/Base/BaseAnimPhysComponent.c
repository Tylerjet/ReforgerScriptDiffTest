/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Base
* @{
*/

class BaseAnimPhysComponentClass: GameComponentClass
{
};

class BaseAnimPhysComponent: GameComponent
{
	/**
	-----------------------------------------------------------------------------
	 commands
	-----------------------------------------------------------------------------
	*/
	proto external void SetCurrentCommand(AnimPhysCommandScripted pCommand);
	proto external AnimPhysCommandScripted GetCommandScripted();
	/**
	-----------------------------------------------------------------------------
	 script binding
	-----------------------------------------------------------------------------
	*/
	proto external TAnimGraphCommand	BindCommand(string pCommandName);
	//! binds variable
	proto external TAnimGraphVariable	BindVariableFloat(string pVariableName);
	proto external TAnimGraphVariable	BindVariableInt(string pVariableName);
	proto external TAnimGraphVariable	BindVariableBool(string pVariableName);
	//! binds tag
	proto external TAnimGraphTag		BindTag(string pTagName);
	//! binds event
	proto external TAnimGraphEvent		BindEvent(string pEventName);
	//! binds prediction
	proto external TAnimGraphPrediction	BindPrediction(string pPredictionName);
	//! returns true when character is physically falling
	proto external bool PhysicsIsFalling();
	//! enables physics
	proto external void PhysicsEnableGravity(bool pState);
};

/** @}*/
