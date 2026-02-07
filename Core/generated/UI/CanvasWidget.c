/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup UI
* @{
*/

sealed class CanvasWidget: CanvasWidgetBase
{
	/*!
	Commands in the array will be used to draw from now on
	The caller needs to keep the array alive - the callee takes just a pointer to it.
	Commands in the array are batched by the widget if possible. However, to allow batching
	some rules need to be followed.
	Consecutive commands of different type always start a new batch.
	For consecutive commands of the same type, using a different texture starts a new batch.
	ImageDrawCommand is not batched.
	TextDrawCommands with different rotations start a new batch.
	*/
	proto external void SetDrawCommands(array<ref CanvasWidgetCommand> drawCommands);
	//! Loads a texture which can be then used in draw commands
	static proto SharedItemRef LoadTexture(ResourceName resource);
};

/** @}*/
