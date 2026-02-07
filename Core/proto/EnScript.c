/*!
\defgroup Enforce Enforce script essentials
\{
*/

/*!
Marks method as obsolete. When is the method used, compiler just throw a compile-time
warning, but method is called normally.
\code
	[Obsolete("Use different method!")]
	void Hello()
	{
	}

	void Test()
	{
		Hello(); // throws compile warning on this line: 'Hello' is obsolete: use different method!
	}
\endcode
*/
class Obsolete: Managed
{
	string m_Msg;
	void Obsolete(string msg = "")
	{
		m_Msg = msg;
	}
}

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
}

/*!
Limit access to method only to script modules within some access level. If user
tries to call this method from script module with improper access level, compilation
error is thrown.
\code
	[Restrict(EAccessLevel.LEVEL_1, false)] // just throws warning
	void DangerousMethod1();

	[Restrict(EAccessLevel.LEVEL_1)] // throw an error and break compilation
	void DangerousMethod2();
\endcode
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

//!Helper for printing out string expression. Example: PrintString("Hello " + var);
void PrintString(string s)
{
	Print(s);
}

/*!
\}
*/
