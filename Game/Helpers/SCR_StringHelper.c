class SCR_StringHelper
{
	protected static const string LOWERCASE = "abcdefghijklmnopqrstuvwxyz";
	protected static const string UPPERCASE = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	protected static const string DIGITS = "0123456789";
	protected static const string TRANSLATION_KEY_CHARS = "_-";

	//------------------------------------------------------------------------------------------------
	static bool ContainsDigit(string input)
	{
		int asciiValue;
		for (int i, len = input.Length(); i < len; i++)
		{
			asciiValue = input[i].ToAscii();
			if (asciiValue >= 48 && asciiValue <= 57)
				return true;
		}
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Gets the amount of times a needle is found in the haystack.
	//! e.g looking for "AA" in "AAAAA" will find it twice.
	//! \param haystack the string in which to search
	//! \param needle the string to find
	//! \param caseInsensitive if set to true, the search will be case-insensitive (e.g "A" will match "a" and vice-versa)
	//! \return the amount of needle occurences in haystack, 0 if haystack or needle is empty
	static int CountOccurences(string haystack, string needle, bool caseInsensitive = false)
	{
		if (needle.IsEmpty() || haystack.IsEmpty())
			return 0;

		if (caseInsensitive)
		{
			needle.ToLower();
			haystack.ToLower();
		}

		int needleLength = needle.Length();
		int haystackLength = haystack.Length();

		if (needleLength > haystackLength)
			return 0;

		int result;
		int searchIndex;
		int resultIndex;
		while (searchIndex < haystackLength)
		{
			resultIndex = haystack.IndexOfFrom(searchIndex, needle);
			if (resultIndex < 0)
				break;

			result++;
			searchIndex = resultIndex + needleLength;
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param format
	//! \param input must be ASCII
	static bool IsFormat(EStringFormat format, string input)
	{
		switch (format)
		{
			case EStringFormat.ALPHABETICAL_UC:		return CheckCharacters(input, false, true, false);
			case EStringFormat.ALPHABETICAL_LC:		return CheckCharacters(input, true, false, false);
			case EStringFormat.ALPHABETICAL_I:		return CheckCharacters(input, true, true, false);
			case EStringFormat.ALPHANUMERICAL_UC:	return CheckCharacters(input, false, true, true);
			case EStringFormat.ALPHANUMERICAL_LC:	return CheckCharacters(input, true, false, true);
			case EStringFormat.ALPHANUMERICAL_I:	return CheckCharacters(input, true, true, true);
			case EStringFormat.DIGITS_ONLY:			return CheckCharacters(input, false, false, true);
		}
		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected static bool CheckCharacters(string input, bool allowLC, bool allowUC, bool allowDigits)
	{
		if (input.IsEmpty())
			return false;

		int asciiValue;
		for (int i, len = input.Length(); i < len; i++)
		{
			asciiValue = input[i].ToAscii();
			if (!(
				(allowLC && asciiValue >= 97 && asciiValue <= 122) ||
				(allowUC && asciiValue >= 65 && asciiValue <= 90) ||
				(allowDigits && asciiValue >= 48 && asciiValue <= 57)
			))
				return false;
		}

		return true;
	}

	/*
	//------------------------------------------------------------------------------------------------
	//! old method, 4-5× slower but allows for non-ASCII values, not useful for now
	protected static bool CheckCharactersOld(string input, bool allowLC, bool allowUC, bool allowDigits)
	{
		string filter;
		if (allowLC)
			filter += LOWERCASE;
		if (allowUC)
			filter += S_UPPERCASE;
		if (allowDigits)
			filter += S_DIGITS;

		if (filter.IsEmpty())
			return false;

		for (int i, len = input.Length(); i < len; i++)
		{
			if (!filter.Contains(input[i]))
				return false;
		}

		return true;
	}
	// */

	//------------------------------------------------------------------------------------------------
	//! Gets int values from a string (e.g { 0, 3, 5, 7, 9 } from "0 3 5 abc 7 9)
	//! numbers should be separated by spaces
	static array<int> GetIntsFromString(string input)
	{
		array<int> result = {};
		array<string> splits = {};
		input.Split(" ", splits, true);

		int value;
		foreach (string split : splits)
		{
			value = split.ToInt();
			if (value != 0 || split == "0")
				result.Insert(value);
		}
		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Gets float values from a string (e.g { 0.3, 5.0, 7.9 } from "0.3 5.0 abc 7.9")
	//! numbers should be separated by spaces
	static array<float> GetFloatsFromString(string input)
	{
		array<float> result = {};
		array<string> splits = {};
		input.Split(" ", splits, true);

		float value;
		foreach (string split : splits)
		{
			value = split.ToFloat();
			if (value != 0 || split.StartsWith("0"))
				result.Insert(value);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Insert a string into another string
	//! \return a new string instance with the result
	static string InsertAt(string input, string insertion, int insertionIndex = 0)
	{
		if (input.IsEmpty() || insertion.IsEmpty() || insertionIndex < 0 || insertionIndex > input.Length())
			return input;

		if (insertionIndex == 0)
			return insertion + input;

		return input.Substring(0, insertionIndex) + insertion + input.Substring(insertionIndex, input.Length() - insertionIndex);
	}

	//------------------------------------------------------------------------------------------------
	//! Returns true if input is empty or only made of spaces or tabs
	static bool IsEmptyOrWhiteSpace(string input)
	{
		return input.Trim().IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	//! \return true if input is to translation key format (e.g #AR-Translation_Value_3) - pound sign (#) must be present!
	static bool IsTranslationKey(string input)
	{
		if (IsEmptyOrWhiteSpace(input))
			return false;

		if (input != input.Trim())
			return false;

		if (input.Length() < 2 || !input.StartsWith("#"))
			return false;

		for (int i, count = TRANSLATION_KEY_CHARS.Length(); i < count; i++)
		{
			if (input.EndsWith(TRANSLATION_KEY_CHARS[i]))
				return false;
		}

		string filter = LOWERCASE + UPPERCASE;
		if (!filter.Contains(input[1])) // \#[a-zA-Z].*
			return false;

		filter += DIGITS + TRANSLATION_KEY_CHARS;
		for (int i, len = input.Length(); i < len; i++)
		{
			if (!filter.Contains(input[i]))
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Joins strings together (reverse operation of string.Split)
	static string Join(string separator, notnull array<string> pieces, bool joinEmptyEntries)
	{
		if (pieces.IsEmpty())
			return string.Empty;

		string result = pieces[0];
		for (int i = 1, cnt = pieces.Count(); i < cnt; i++)
		{
			if (joinEmptyEntries || !pieces[i].IsEmpty())
				result += separator + pieces[i];
		}
		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Pad left
	static string PadLeft(string input, int length, string padding = " ")
	{
		while (input.Length() < length)
		{
			input = padding + input;
		}

		return input;
	}

	//------------------------------------------------------------------------------------------------
	//! Pad right
	static string PadRight(string input, int length, string padding = " ")
	{
		while (input.Length() < length)
		{
			input += padding;
		}

		return input;
	}

	//------------------------------------------------------------------------------------------------
	//! Replace X times a string from within a string from left to right
	//! @code
	//! ReplaceTimes("Hello Hallo Yellow", "llo", "ya", 2); // returns Heya Haya Hollow"
	//! @endcode
	//! \param input
	//! \param sample
	//! \param replace
	//! \param howMany
	//! \return input with 'sample' replaced by 'replace' 'howMany' times
	static string ReplaceTimes(string input, string sample, string replace, int howMany = 1)
	{
		if (howMany < 1 || input.IsEmpty() || sample.IsEmpty() || sample == replace)
			return input;

		int length;
		int index;
		int sampleLength = sample.Length();

		while (howMany > 0)
		{
			index = input.IndexOfFrom(index, sample);
			if (index < 0)
				break;

			if (index == 0)
				input = replace + input.Substring(sampleLength, input.Length() - sampleLength);
			else
				input = input.Substring(0, index) + replace + input.Substring(index + sampleLength, input.Length() - (index + sampleLength));

			howMany--;
		}

		return input;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns the provided input string reversed
	static string Reverse(string input)
	{
		string result;
		for (int i = input.Length() - 1; i >= 0; i--)
		{
			result += input[i];
		}
		return result;
	}

	//------------------------------------------------------------------------------------------------
	static bool ContainsAny(string input, array<string> needles)
	{
		if (input.IsEmpty() || needles.IsEmpty())
			return false;

		foreach (string needle : needles)
		{
			if (input.Contains(needle))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	static bool StartsWithAny(string input, array<string> lineStarts)
	{
		if (input.IsEmpty())
			return false;

		foreach (string lineStart : lineStarts)
		{
			if (input.StartsWith(lineStart))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	static bool EndsWithAny(string input, array<string> lineEnds)
	{
		if (input.IsEmpty())
			return false;

		foreach (string lineEnd : lineEnds)
		{
			if (input.EndsWith(lineEnd))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Get the actual translation from the translation key
	//! If not a translation key, the provided input is returned
	//! It is NOT recommended to manipulate a potentially non-ASCII string (multibyte UTF-8), use at your own risk!
	//! \return translated string, multibyte UTF-8 format
	static string Translate(
		string input,
		string param1 = string.Empty,
		string param2 = string.Empty,
		string param3 = string.Empty,
		string param4 = string.Empty,
		string param5 = string.Empty,
		string param6 = string.Empty,
		string param7 = string.Empty,
		string param8 = string.Empty,
		string param9 = string.Empty)
	{
		return WidgetManager.Translate(input, param1, param2, param3, param4, param5, param6, param7, param8, param9);
	}
};

enum EStringFormat
{
	ALPHABETICAL_UC,	//!< [A-Z]+
	ALPHABETICAL_LC,	//!< [a-z]+
	ALPHABETICAL_I,		//!< [a-zA-Z]+ (I = insensitive)
	ALPHANUMERICAL_UC,	//!< [A-Z0-9]+
	ALPHANUMERICAL_LC,	//!< [a-z0-9]+
	ALPHANUMERICAL_I,	//!< [a-zA-Z0-9]+ (I = insensitive)
	DIGITS_ONLY,		//!< [0-9]+
	// FLOATING_POINT,		//!< [0-9][0-9\.]+[0-9]
};

// typedef?
class SCR_StringArray : array<string>
{
	//------------------------------------------------------------------------------------------------
	int CountEmptyEntries()
	{
		int result;
		for (int i = Count() - 1; i >= 0; i--)
		{
			if (this[i].IsEmpty())
				result++;
		}
		return result;
	}

	//------------------------------------------------------------------------------------------------
	int CountEmptyOrWhiteSpaceEntries()
	{
		int result;
		for (int i = Count() - 1; i >= 0; i--)
		{
			if (SCR_StringHelper.IsEmptyOrWhiteSpace(this[i]))
				result++;
		}
		return result;
	}

	//------------------------------------------------------------------------------------------------
	string Join(string separator = string.Empty, bool joinEmptyEntries = true)
	{
		return SCR_StringHelper.Join(separator, this, joinEmptyEntries);
	}

	//------------------------------------------------------------------------------------------------
	bool HasEmptyEntry()
	{
		for (int i = Count() - 1; i >= 0; i--)
		{
			if (this[i].IsEmpty())
				return true;
		}
		return false;
	}

	//------------------------------------------------------------------------------------------------
	bool HasEmptyOrWhiteSpaceEntry()
	{
		for (int i = Count() - 1; i >= 0; i--)
		{
			if (SCR_StringHelper.IsEmptyOrWhiteSpace(this[i]))
				return true;
		}
		return false;
	}

	//------------------------------------------------------------------------------------------------
	void RemoveEmptyEntries()
	{
		for (int i = Count() - 1; i >= 0; i--)
		{
			if (this[i].IsEmpty())
				RemoveOrdered(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	void RemoveEmptyOrWhiteSpaceEntries()
	{
		for (int i = Count() - 1; i >= 0; i--)
		{
			if (SCR_StringHelper.IsEmptyOrWhiteSpace(this[i]))
				RemoveOrdered(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	void ToLower()
	{
		string tmp;
		for (int i = Count() - 1; i >= 0; i--)
		{
			tmp = this[i];
			tmp.ToLower();
			this[i] = tmp;
		}
	}

	//------------------------------------------------------------------------------------------------
	void ToUpper()
	{
		string tmp;
		for (int i = Count() - 1; i >= 0; i--)
		{
			tmp = this[i];
			tmp.ToUpper();
			this[i] = tmp;
		}
	}

	//------------------------------------------------------------------------------------------------
	void Trim()
	{
		for (int i = Count(); i >= 0; i--)
		{
			this[i] = this[i].Trim();
		}
	}
};
