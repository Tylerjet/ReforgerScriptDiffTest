//------------------------------------------------------------------------------------------------
enum EOperationType
{
	Equal = 0,
	Smaller = 1,
	Smaller_or_Equal = 2
};

class SCR_AIDecoCompareVariables : DecoratorScripted
{
	[Attribute("0", UIWidgets.ComboBox, "Comparison type", "", ParamEnumArray.FromEnum(EOperationType) )]
	protected int m_comparisonType;
		
	protected override bool TestFunction(AIAgent owner)
	{
		if (GetVariableType(true, "value1") == GetVariableType(true, "value2"))
		{
			if (GetVariableType(true, "value1") == int)
			{
				int value1 = 0;	
				int value2 = 0;	
				GetVariableIn("value1", value1);
				GetVariableIn("value2", value2);
				
				switch (m_comparisonType)
				{
					case EOperationType.Equal 				: { return value1 == value2; }
					case EOperationType.Smaller 			: { return value1 < value2; }
					case EOperationType.Smaller_or_Equal 	: { return value1 <= value2; }
				};				
			}
			else if (GetVariableType(true, "value1") == float)
			{
				float val1 = 0;	
				float val2 = 0;	
				GetVariableIn("value1", val1);
				GetVariableIn("value2", val2);
				
				switch (m_comparisonType)
				{
					case EOperationType.Equal 				: { return float.AlmostEqual(val1, val2); }
					case EOperationType.Smaller 			: { return val1 < val2; }
					case EOperationType.Smaller_or_Equal 	: { return val1 < val2 || float.AlmostEqual(val1, val2); }
				};				
			}
			else if (GetVariableType(true, "value1").IsInherited(Managed))
			{
				Managed val1 = null; 
				Managed val2 = null; 
				GetVariableIn("value1", val1);
				GetVariableIn("value2", val2);
				
				return val1 == val2;
			}
			else if (GetVariableType(true, "value1") == bool)
			{
				bool val1; 
				bool val2; 
				GetVariableIn("value1", val1);
				GetVariableIn("value2", val2);				
								
				return val1 == val2;
			}
		}
		
		return false;
	}
	
	protected override bool VisibleInPalette()
	{
		return true;
	}	
	
	protected override string GetOnHoverDescription()
	{
		return "SCR_AIDecoCompareVariables: Compares variables on input by given criterion. Supports int-int, float-float, bool - bool and children of Managed comparison";
	}
	
	protected static ref TStringArray s_aVarsIn = {
		"value1",
		"value2"
	};
	protected override TStringArray GetVariablesIn()
	{
		return s_aVarsIn;
	}
	
	/*protected string GetNodeMiddleText()
	{
		string enumToString;
		switch (m_comparisonType)
		{
			case EOperationType.Equal 				: {enumToString = " == "; break;}
			case EOperationType.Smaller 			: {enumToString = " <  "; break;}
			case EOperationType.Smaller_or_Equal 	: {enumToString = " <= "; break;}			
		}		
		return "value1" + enumToString + "value2";
	}*/
};
