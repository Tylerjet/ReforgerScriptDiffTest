class SCR_ParamEnumArray : ParamEnumArray
{
	protected static const string ENTRY_SEPARATOR = ";";
	protected static const string VALUE_SEPARATOR = ",";
	protected static const string ESCAPE_CHARACTER = "'";

	//------------------------------------------------------------------------------------------------
	//! Get ComboBox entries from string, format:
	//! - "text1;text2;text3"
	//! - "text1,desc1;text2,desc2;text3,desc3"
	//! - "value1,text1,desc1;value2,text2,desc2;value3,text3,desc3"
	//! If value is not defined, array index is used ("0", "1", "2", etc)
	//! \note commas and semicolons can be escaped using a single quote (" ', ", " '; ")
	//! \param[in] input format can be (entries are separated by semicolons ";"):
	//! - text
	//! - text,description
	//! - value,text,description (if there is no description, use a trailing comma to keep an empty description e.g "value,text,")
	//! \return the resulting ParamEnumArray - never null
	static ParamEnumArray FromString(string input)
	{
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(input))
			return { new ParamEnum("No options set", "0", "No options set - please edit the Attribute's SCR_ParamEnumArray.FromString string value") };

		array<string> entries = {};
		input.Split(ENTRY_SEPARATOR, entries, false);

		for (int i = entries.Count() - 1; i >= 1; --i)
		{
			if (entries[i - 1].EndsWith(ESCAPE_CHARACTER))
			{
				entries[i - 1] = entries[i - 1].Substring(0, entries[i - 1].Length() - 1) + ENTRY_SEPARATOR + entries[i];
				entries.RemoveOrdered(i);
			}
		}

		ParamEnumArray result = {};
		array<string> values = {};

		foreach (int i, string entry : entries)
		{
			entry.TrimInPlace();
			entry.Split(VALUE_SEPARATOR, values, false);

			int valuesCount = values.Count();
			if (valuesCount < 1) // cannot happen
				continue;

			string value = i.ToString();
			string text;
			string description;
			for (int j = values.Count() - 1; j >= 1; --j)
			{
				if (values[j - 1].EndsWith(ESCAPE_CHARACTER))
				{
					values[j - 1] = values[j - 1].Substring(0, values[j - 1].Length() - 1) + VALUE_SEPARATOR + values[j];
					values.RemoveOrdered(j);
					--valuesCount;
				}
			}

			if (valuesCount > 3)
			{
				for (int k = valuesCount - 1; k > 2; --k)
				{
					values[2] = values[2] + VALUE_SEPARATOR + values[k];
					values.Remove(k);
				}
			}

			foreach (int j, string value2 : values)
			{
				value2.TrimInPlace();
				values[j] = value2;
			}

			if (valuesCount < 1)
				continue;

			if (valuesCount == 1)		// 1 - text only
			{
				text = values[0];
				text.TrimInPlace();
			}
			else if (valuesCount == 2)	// 2 - text and description
			{
				text = values[0];
				description = values[1];

				text.TrimInPlace();
				description.TrimInPlace();
			}
			else						// 3 - VALUE, text and description - other splits are joined into description
			{
				value = values[0];
				text = values[1];
				description = values[2];

				// value.TrimInPlace();	// we keep spaces, they may eventually be useful
				text.TrimInPlace();
				description.TrimInPlace();
			}

			if (description)
				description += "\nValue: " + value;
			else
				description = "Value: " + value;

			result.Insert(new ParamEnum(text, value, description));
		}

		return result;
	}
}
