/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Types
\{
*/

sealed class typename
{
	private void typename();
	private void ~typename();

	const static typename Empty;

	//!Returns type name of variable as string
	proto external string ToString();
	/*!
	Returns true when type is the same as 'baseType', or inherited one.
	\param baseType typename
	\returns \p bool true when type is the same as 'baseType', or inherited one.
	\code
		???
	\endcode
	*/
	proto external bool IsInherited(typename baseType);
	/*!
	Dynamic variant to `new` keyword. It creates new instance of class.
	\returns \p instance of class
	\code
		???
	\endcode
	*/
	proto external ref Managed Spawn();
	proto external int GetVariableCount();
	proto external owned string GetVariableName(int vIdx);
	proto external typename GetVariableType(int vIdx);
	proto external bool GetVariableValue(Class var, int vIdx, out void val);
	/*!
	Return string name of enum value.
	\code
		DialogPriority prio = DialogPriority.WARNING;
		Print( typename.EnumToString(DialogPriority, prio) );
	\endcode
	*/
	static proto string EnumToString(typename e, int enumValue);
	/*!
	Return enum value from string name.
	\code
		Print( typename.StringToEnum(DialogPriority, "WARNING") );
	\endcode
	*/
	static proto int StringToEnum(typename e, string enumName);
}

/*!
\}
*/
