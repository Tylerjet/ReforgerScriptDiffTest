/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Types
\{
*/

sealed class int
{
	private void int();
	private void ~int();

	const int MAX = 2147483647;
	const int MIN = -2147483648;

	/*!
	Converts ASCII code to string.
	\return `string` - Converted `int`.
	\code
		int ascii_code = 77;
		string str = ascii_code.AsciiToString();
		Print(str);

		>> str = 'M'
	\endcode
	*/
	proto external string AsciiToString();
	/*!
	Integer to string with fixed length, padded with zeroes.
	\param len \p int fixed length
	\code
		int num = 123;
		string s1 = num.ToString(5);
		string s2 = num.ToString();
		Print(s1);
		Print(s2);
		>> s1 = '00123'
		>> s2 = '123'
	\endcode
	*/
	proto external string ToString(int len = -1);
	/*!
	Returns an integer that indicates the sign of a number.
	- For `intNumber.Sign() < 0` result is -1.
	- For `intNumber.Sign() == 0` result is 0.
	- For `intNumber.Sign() > 0` result  is 1.
	*/
	proto external int Sign();
}

/*!
\}
*/
