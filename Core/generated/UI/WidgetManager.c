/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup UI
* @{
*/

/*!
Class containing global and util functions regarding widgets
*/
sealed class WidgetManager
{
	private void WidgetManager();
	private void ~WidgetManager();
	
	/*!
	Gets all widgets at given position returning them in array from the closest to the farthest
	Widgets with DISABLED or IGNORE_CURSOR or without VISIBLE flags are ignored.
	
	\param x X coordinate on screen in NATIVE resolution
	\param y Y coordinate on screen in NATIVE resolution
	\param rootWidget Where in the hierarchy should the tracing start. Is included in tracing.
	\param outWidgets Array filled with traced widgets sorted from the closest to the farthest
	*/
	static proto void TraceWidgets(int x, int y, notnull Widget rootWidget, notnull array<ref Widget> outWidgets);
	static proto void SetLanguage(string languageCode);
	static proto void GetLanguage(out string languageCode);
	static proto void SetCursor(int cursorIndex);
	static proto void ReportMouse(int mousex, int mousey, notnull Widget rootWidget);
	/*!
	Gets reference resolution used for DPI scaling
	To work with DPI scaling use WorkspaceWidget.DPIScale and WorkspaceWidget.DPIUnscale
	*/
	static proto void GetReferenceScreenSize(out int width, out int height);
	//! Gets current mouse position in native (current) resolution (not reference)
	static proto void GetMousePos(out int x, out int y);
	static proto Widget GetWidgetUnderCursor();
	//! Returns how long is TextCursor shown/hidden for. Time in milliseconds
	static proto int GetTextCursorBlinkTime();
	//! \see GetTextCursorBlinkTime
	static proto void SetTextCursorBlinkTime(int milliseconds);
	//! Convert given text to localized version (multibyte UTF-8) using text ID's and localization table
	static proto string Translate(string text, void param1 = NULL, void param2 = NULL, void param3 = NULL, void param4 = NULL, void param5 = NULL, void param6 = NULL, void param7 = NULL, void param8 = NULL, void param9 = NULL);
	//! Return indices of IDs which contains searched text. Search is case insensitive.
	static proto int SearchLocalized(string text, notnull array<string> IDs, notnull out array<int> outIndices);
};

/** @}*/
