/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Debug
* @{
*/

class DebugTextScreenSpace: DebugText
{
	//! Instantiate using the Create method
	private void DebugTextScreenSpace();
	void ~DebugTextScreenSpace();
	
	proto external void SetPosition(float x, float y);
	proto external vector GetPosition();
	/*!
	\brief Creates a text instance, should be saved to ref, otherwise is impossible to remove from screen (if not ONCE see flags)
	\param world In which world should be the text drawn
	\param text Text to draw on screen
	\param flags DebugTextFlags
	\param x X coordinate of the top left corner of the text
	\param y Y coordinate of the top left corner of the text
	\param size Size of the font in pixels
	\param color Color of the text
	\param drawBg Flag which determines whether a background will be drawn behind the text
	\param priority Z order priority. Note: texts with background are always rendered under texts without background.
	\return DebugText Instance of debug text with given properties set, if flags contains DebugTextFlags.ONCE null is returned
	*/
	static proto DebugTextScreenSpace Create(BaseWorld world, string text, DebugTextFlags flags, float x, float y, float size = 20.0, int color = 0xFFFFFFFF, int bgColor = 0x00000000, int priority = 1000);
};

/** @}*/
