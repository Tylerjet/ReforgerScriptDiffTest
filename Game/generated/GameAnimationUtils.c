/*
===========================================
Do not modify, this script is generated
===========================================
*/

sealed class GameAnimationUtils
{
	private void GameAnimationUtils();
	private void ~GameAnimationUtils();
	
	//! Find or add AnimationEventID from provided string
	//! @Note: you should always cache and reuse returned id in order to reduce overhead
	//! return -1 if operation was unsuccessful
	static proto AnimationEventID RegisterAnimationEvent(string animationEventString);
	//! Find or add AnimationTagID from provided string
	//! @Note: you should always cache and reuse returned id in order to reduce overhead
	//! return -1 if operation was unsuccessful
	static proto AnimationTagID RegisterAnimationTag(string animationTagString);
};
