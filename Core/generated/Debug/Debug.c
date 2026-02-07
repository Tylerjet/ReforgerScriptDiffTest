/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Debug
* @{
*/

sealed class Debug
{
	private void Debug();
	private void ~Debug();
	
	const int MB_PRESSED_MASK = 0x80000000;
	
	/**
	\brief Prints current call stack (stack trace)
	\return \p void
	@code
	DumpStack();
	
	@endcode
	
	\verbatim
	Output:
	-- Stack trace --
	SaveFile() Scripts\Entities\Modules\ModuleBase\ModuleFileHandler.c : 51
	SaveConfigToFile() Scripts\Entities\Modules\ModuleBase\ModuleFileHandler\ModuleLocalProfile.c : 114
	SaveParameterArray() Scripts\Entities\Modules\ModuleBase\ModuleFileHandler\ModuleLocalProfile.c : 133
	SetParameterArray() Scripts\Entities\Modules\ModuleBase\ModuleFileHandler\ModuleLocalProfile.c : 231
	PresetAdd() Scripts\Entities\Modules\ModuleBase\ModuleFileHandler\ModuleLocalProfile\ModuleLocalProfileUI.h : 46
	OnKeyPress() Scripts/mission/missionGameplay.c : 215
	OnKeyPress() Scripts/DayZGame.c : 334
	-----------------
	\endverbatim
	*/
	static proto void DumpStack();
	//! Dump all allocated script objects with callstack of its allocation into output/log, can be used only together with -checkInstance CLI param
	static proto void DumpInstances(bool csvFormatting);
	//!Messagebox with error message
	static proto void Error2(string title, string err);
	//!Messagebox with error message
	static proto void Error(string err);
	//! Starts measuring time until EndTimeMeasure is called
	static proto void BeginTimeMeasure();
	//! Ends time measurement which began with last BeginTimeMeasure call
	static proto void EndTimeMeasure(string title);
	//!Prints content of variable to console/log. Should be used for critical messages so it will appear in debug log
	static proto void DPrint(string var);
	//!Trigger C++ breakpoint when running from C++ debugger
	static proto void Break(bool condition = true, void param1 = NULL, void param2 = NULL, void param3 = NULL, void param4 = NULL, void param5 = NULL, void param6 = NULL, void param7 = NULL, void param8 = NULL, void param9 = NULL);
	//! Triggers breakpoint in C++ in compile time(when app is running in debug enviroment)
	static proto void CompileBreak();
	/*!
	Gets key state
	\param key	Key code
	\returns 0 when not pressed, 15. bit set when pressed, 0.-14. bit pressed count
	*/
	static proto int KeyState(KeyCode key);
	/*!
	Returns state of mouse button. It's combination of number of release/pressed edges and mask Debug.MB_PRESSED_MASK
	that is set when button is pressed.
	If you want just to check if button is pressed, use this: if(GetMouseState(MouseState.LEFT) & Debug.MB_PRESSED_MASK)) Print("left button pressed");
	*/
	static proto int GetMouseState(MouseState index);
	/*!
	Clears the key state.
	Call this function if you want to overcome autorepeating in reporting key state. If called, the KeyState returns pressed only after the key is released and pressed again.
	*/
	static proto void ClearKey(KeyCode key);
};

/** @}*/
