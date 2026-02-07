class SCR_StringHelper
{
	protected static const ref array<string> S_LOWERCASE = { "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z" };
	protected static const ref array<string> S_UPPERCASE = { "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z" };
	protected static const ref array<string> S_DIGITS = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9" };
	protected static const ref array<string> S_TRANSLATION_KEY_CHARS = { "_", "-" };

	//------------------------------------------------------------------------------------------------
	static bool ContainsDigit(string input)
	{
		for (int i, len = input.Length(); i < len; i++)
		{
			if (S_DIGITS.Contains(input[i]))
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
	//! \return the amount of needle occurences in haystack
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
		array<string> filter = {};
		if (allowLC)
			filter.InsertAll(S_LOWERCASE);
		if (allowUC)
			filter.InsertAll(S_UPPERCASE);
		if (allowDigits)
			filter.InsertAll(S_DIGITS);

		for (int i, len = input.Length(); i < len; i++)
		{
			if (!filter.Contains(input[i]))
				return false;
		}
		return true;
	}

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
	//! Gets float values from a string (e.g { 0.3, 5.0, 7.9 } from "0.3 5.0 abc 7.9)
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
	//! Returns true if input is empty or only made of spaces or tabs
	static bool IsEmptyOrWhiteSpace(string input)
	{
		return input.Trim().IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	//! Returns true if input is to translation key format (e.g #AR-Translation_Value_3)
	static bool IsTranslationKey(string input)
	{
		if (input.Trim().IsEmpty())
			return false;

		if (input.Trim() != input)
			return false;

		if (input.Length() < 2 || !input.StartsWith("#"))
			return false;

		foreach (string char : S_TRANSLATION_KEY_CHARS)
		{
			if (input.EndsWith(char))
				return false;
		}

		array<string> filter = {};
		filter.InsertAll(S_LOWERCASE);
		filter.InsertAll(S_UPPERCASE);

		if (!filter.Contains(input[1])) // \#[a-zA-Z].*
			return false;

		filter.InsertAll(S_DIGITS);
		filter.InsertAll(S_TRANSLATION_KEY_CHARS);

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
			if (!pieces[i].IsEmpty() || joinEmptyEntries)
				result += separator + pieces[i];
		}
		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Reverses the provided input string
	static string Reverse(string input)
	{
		string result;
		for (int i, len = input.Length(); i < len; i++)
		{
			result += input[len - i -1];
		}
		return result;
	}
};

enum EStringFormat
{
	ALPHABETICAL_UC,	//< [A-Z]+
	ALPHABETICAL_LC,	//< [a-z]+
	ALPHABETICAL_I,		//< [a-zA-Z]+ (I = insensitive)
	ALPHANUMERICAL_UC,	//< [A-Z0-9]+
	ALPHANUMERICAL_LC,	//< [a-z0-9]+
	ALPHANUMERICAL_I,	//< [a-zA-Z0-9]+ (I = insensitive)
	DIGITS_ONLY,		//< [0-9]+
	// FLOATING_POINT,		//< [0-9][0-9\.]+[0-9]
};
