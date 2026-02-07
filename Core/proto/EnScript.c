/**
 * \defgroup Enforce Enforce script essentials
 * @{
 */
/**

\brief Marks method as obsolete. When is the method used, compiler just throw a compile-time warning, but method is called normally.
	@code
		[Obsolete("use diffetent method!")]
		void Hello()
		{
		}

		void Test()
		{
			Hello(); // throws compile warning on this line: 'Hello' is obsolete: use different method!
		}
	@endcode
	*/
class Obsolete: Managed
{
	string m_Msg;
	void Obsolete(string msg = "") 
	{
		m_Msg = msg;
	}
};

enum EAccessLevel
{
	ANY = 0,
	LEVEL_0, //!< core module
	LEVEL_1, //!< game lib module
	LEVEL_2, //!< game module
	LEVEL_3,
	LEVEL_4,
	LEVEL_5,
	LEVEL_6,
	/* stored in 4bits, max is 15 */
};

/**
\brief Limit acceess to method only to script modules within some access level. If user try to call this method from script module with impropper access level, compilation error is thrown
@code
	[Restrict(EAccessLevel.LEVEL_1, false)] // just throws warning
	void DangerousMethod1();

	[Restrict(EAccessLevel.LEVEL_1)] // throw an error and break compilation
	void DangerousMethod2();
@endcode
	*/
class Restrict
{
	private EAccessLevel m_Level;
	private bool m_IsError; //!< If true, throws compilation error, else it throw just warning
	
	void Restrict(EAccessLevel level, bool isError = true)
	{
		m_Level = level;
		m_IsError = isError;
	}
}

//! Module containing compiled scripts.
class ScriptModule
{
	private void ~ScriptModule();

	/*!Dynamic call of function
	when inst == NULL, it's global function call, otherwise it's method of class. If 'asynch' is true, creates new thread (so it's legal to use sleep/wait). Otherwise main thread is used and call is blocking.
	Return value of called method is returned via 'returnVal' (only when 'asynch' is false!)
	Returns true, when success.
  */
	proto volatile bool Call(Class inst, string function, bool asynch, out void returnVal, void param1 = NULL, void param2 = NULL, void param3 = NULL, void param4 = NULL, void param5 = NULL, void param6 = NULL, void param7 = NULL, void param8 = NULL, void param9 = NULL );
	
	proto native void Release();
	
	/**
	\brief Do load script and create ScriptModule for it
		\param parentModule Module
		\param scriptFile Script path
		\param listing ??
		\returns \p ScriptModule Loaded scripted module
		@code
			???
		@endcode
	*/
	static proto native ScriptModule LoadScript(ScriptModule parentModule, string scriptFile, bool listing);
};

//!Helper for printing out string expression. Example: PrintString("Hello " + var);
void PrintString(string s)
{
	Print(s);
}
 //@}
