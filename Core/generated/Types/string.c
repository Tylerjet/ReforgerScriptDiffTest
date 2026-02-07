/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Types
* @{
*/

sealed class string
{
	private void string();
	private void ~string();
	
	static const string Empty;
	
	/**
	\brief Converts string to integer
	\return \p int - Converted \p string.
	@code
	string str = "56";
	int i = str.ToInt();
	Print(i);
	
	>> i = 56
	@endcode
	*/
	proto external int ToAscii();
	/**
	\brief Converts string to float
	\return \p float - Converted \p string \p in float.
	@code
	string str = "56.6";
	float f = str.ToFloat();
	Print(f);
	
	>> f = 56.6
	@endcode
	*/
	proto external float ToFloat();
	/**
	\brief Converts string to integer
	\return \p int - Converted \p string.
	@code
	string str = "56";
	int i = str.ToInt();
	Print(i);
	
	>> i = 56
	@endcode
	*/
	proto external int ToInt();
	/**
	\brief Returns a vector from a string
	\return \p vector Converted s as vector
	@code
	string str = "1 0 1";
	vector v = str.ToVector();
	Print(v);
	
	>> v = <1,0,1>
	@endcode
	*/
	proto external vector ToVector();
	/**
	\brief Substring of 'str' from 'start' position 'len' number of characters
	\param start Position in \p str
	\param len Count of characters
	\return \p string - Substring of \p str
	@code
	string str = "Hello World";
	string strSub = str.Substring(2, 5);
	Print(strSub);
	
	>> strSub = llo W
	@endcode
	*/
	proto external string Substring(int start, int len);
	/**
	\brief Returns trimmed string with removed leading and trailing whitespaces
	\return \p string - Trimmed string
	@code
	string str = " Hello World "
	Print( str );
	Print( str.Trim() );
	
	>> ' Hello World '
	>> 'Hello World'
	@endcode
	
	*/
	proto external string Trim();
	/**
	\brief Gets n-th character from string
	\param index character index
	\return \p string character on index-th position in string
	@code
	int a = 5;
	float b = 5.99;
	string c = "beta";
	string 	test = string.Format("Ahoj %1 = %3 , %2", a, b, c);
	Print(test);
	>> 'Ahoj 5 = 'beta' , 5.99'
	@endcode
	*/
	static proto string Format(string fmt, void param1 = NULL, void param2 = NULL, void param3 = NULL, void param4 = NULL, void param5 = NULL, void param6 = NULL, void param7 = NULL, void param8 = NULL, void param9 = NULL);
	/**
	\brief Removes leading and trailing whitespaces in string. Returns length
	\return \p int - Count of chars
	@code
	string str = " Hello World ";
	int i = str.TrimInPlace();
	Print(str);
	Print(i);
	
	>> str = 'Hello World'
	>> i = 11
	@endcode
	*/
	proto external int TrimInPlace();
	/**
	\brief Returns length of string
	\return \p int - Length of string
	@code
	string str = "Hello World";
	int i = str.Length();
	Print(i);
	
	>> i = 11
	@endcode
	*/
	proto external int Length();
	/**
	\brief      Determines if string is empty
	
	\return     True if empty, False otherwise.
	*/
	proto external bool IsEmpty();
	/**
	\brief Returns hash of string
	\return \p int - Hash of string
	@code
	string str = "Hello World";
	int hash = str.Hash();
	Print(hash);
	@endcode
	*/
	proto external int Hash();
	/**
	\brief Finds 'sample' in 'str'. Returns -1 when not found
	\param sample \p string Finding string
	\return \p int - Returns position where \p sample starts, or -1 when \p sample not found
	@code
	string str = "Hello World";
	Print( str.IndexOf( "H" ) );
	Print( str.IndexOf( "W" ) );
	Print( str.IndexOf( "Q" ) );
	
	>> 0
	>> 6
	>> -1
	@endcode
	*/
	proto external int IndexOf(string sample);
	/**
	\brief Finds last 'sample' in 'str'. Returns -1 when not found
	\param sample \p string Finding string
	\return \p int - Returns position where \p sample starts, or -1 when \p sample not found
	@code
	string str = "Hello World";
	Print( str.IndexOf( "l" ) );
	
	>> 9
	@endcode
	*/
	proto external int LastIndexOf(string sample);
	/**
	\brief Finds 'sample' in 'str' from 'start' position. Returns -1 when not found
	\param start \p int Start from position
	\param sample \p string Finding string expression
	\return \p int - Length of string \p s
	@code
	string str = "Hello World";
	Print( str.IndexOfFrom( 3, "H" ) );
	Print( str.IndexOfFrom( 3, "W" ) );
	Print( str.IndexOfFrom( 3, "Q" ) );
	
	>> -1
	>> 6
	>> -1
	@endcode
	*/
	proto external int IndexOfFrom(int start, string sample);
	/**
	\brief Retunrs true if sample is substring of string
	\param sample \p string Finding string expression
	\return \p bool true if sample is substring of string
	@code
	string str = "Hello World";
	Print( str.Contains("Hello") );
	Print( str.Contains("Mexico") );
	
	>> true
	>> false
	@endcode
	*/
	proto external bool Contains(string sample);
	/**
	\brief Retunrs true if string starts with sample, otherwise return false
	\param sample \p string Finding string expression
	\return \p bool true if string starts with sample
	@code
	string str = "Hello World";
	Print( str.StartsWith("Hello") );
	Print( str.StartsWith("World") );
	
	>> true
	>> false
	@endcode
	*/
	proto external bool StartsWith(string sample);
	/**
	\brief Retunrs true if string ends with sample, otherwise return false
	\param sample \p string Finding string expression
	\return \p bool true if string ends with sample
	@code
	string str = "Hello World";
	Print( str.StartsWith("Hello") );
	Print( str.StartsWith("World") );
	
	>> false
	>> true
	@endcode
	*/
	proto external bool EndsWith(string sample);
	/**
	\brief Compares with sample and returns an integer less than, equal to, or greater than zero if string is less than, equal to, or greater than sample.
	\param sample \p string to campare with
	\return \p bool less than, equal to, or greater than zero if string is less than, equal to, or greater than sample
	@code
	string str = "Hello";
	Print( str.Compare("Hello") );
	Print( str.Compare("heLLo") );
	Print( str.Compare("heLLo", false) );
	
	>> 0
	>> -1
	>> 0
	@endcode
	*/
	proto external int Compare(string sample, bool caseSensitive = true);
	/**
	\brief Replace all occurrances of 'sample' in 'str' by 'replace'
	\param sample string to search in \p str
	\param replace string which replace \p sample in \p str
	\return \p int - number of occurrances of 'sample' in 'str'
	@code
	string test = "If the length of the C string in source is less than num, only the content up to the terminating null-character is copied.";
	Print(test);
	int count = test.Replace("the", "*");
	Print(count);
	Print(test);
	
	>> string test = 'If the length of the C string in source is less than num, only the content up to the terminating null-character is copied.';
	>> int count =   4
	>> string test = 'If * length of * C string in source is less than num, only * content up to * terminating null-character is copied.'
	@endcode
	*/
	proto external int Replace(string sample, string replace);
	/**
	\brief Changes string to lowercase. Returns length. Works with just ASCII characters
	\return \p int - Length of changed string
	@code
	string str = "Hello World";
	int i = str.ToLower();
	Print(str);
	Print(i);
	
	>> str = hello world
	>> i = 11
	@endcode
	*/
	proto external int ToLower();
	/**
	\brief Changes string to uppercase. Returns length. Works with just ASCII characters
	\return \p int - Length of changed string
	@code
	string str = "Hello World";
	int i = str.ToUpper();
	Print(str);
	Print(i);
	
	>> str = HELLO WORLD
	>> i = 11
	@endcode
	*/
	proto external int ToUpper();
	/**
	\brief Splits string into array of strings separated by 'delimiter'
	\param delimiter \p string Strings separator
	\param removeEmptyEntries If true removes empty strings from outTokens array
	\return \p TStringArray Array with strings
	@code
	TStringArray strs = new TStringArray;
	EnString.Split("The;quick;brown;fox;jumps;over;the;;dog;", ";", strs);
	
	for ( int i = 0; i < strs.Count(); i++ )
	{
	Print("_" + strs.Get(i) + "_");
	}
	
	>> '_The_'
	>> '_quick_'
	>> '_brown_'
	>> '_fox_'
	>> '_jumps_'
	>> '_over_'
	>> '_the_'
	>> '__'		// This one is an empty string
	>> '_dog_'
	>> '__'		// This one is an empty string
	@endcode
	*/
	proto external void Split(string delimiter, out array<string> outTokens, bool removeEmptyEntries);
	static proto string ToString(void var, bool type = false, bool name = false, bool quotes = true);
	/**
	\brief Gets n-th character from string
	\param index character index
	\return \p string character on index-th position in string
	@code
	string str = "Hello World";
	Print( str[4] ); // Print( str.Get(4) );
	
	>> 'o'
	@endcode
	*/
	proto external string Get(int index);
	/**
	\brief Returns internal type representation. Can be used in runtime, or cached in variables and used for faster inheritance checking
	\returns \p typename Type of class
	@code
	???
	@endcode
	*/
	proto external typename ToType();
};

/** @}*/
