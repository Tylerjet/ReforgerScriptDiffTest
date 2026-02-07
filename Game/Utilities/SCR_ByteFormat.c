/*
Useful functions to handle byte size formatting
*/

class SCR_ByteFormat
{
	const static string FORMAT_B ="#AR-DataSize_B";
	const static string FORMAT_KB ="#AR-DataSize_kB";
	const static string FORMAT_MB ="#AR-DataSize_MB";
	const static string FORMAT_GB = "#AR-DataSize_GB";
	
	//------------------------------------------------------------------------------------------------
	//! Return byte size in 3 digid max format with included size name up to GB
	static string ReadableSizeFromBytes(out float value)
	{
		int safeCount = 4;
		int count = 0;
		float t = 1024.0;
		
		float valCheck = value;
		
		while (value > t && count < safeCount)
		{
			value /= t;
			count++;
		}
		
		// Cut last 1000 
		if (value / 1000 > 1)
		{
			value /= 1000;
			count++;
		}
		
		valCheck = value;
		
		// First decimal place
		float valueMult10 = Math.Ceil(value * 10.0);
		
		if (valueMult10 <= 10)
			value = valueMult10 / 10;
		else		
			value = valueMult10 / 10;
		
		// Add proper metrics 
		switch (count)
		{
			// byte
			case 0: return FORMAT_B; break;
			
			// kilo
			case 1: return FORMAT_KB; break;
			
			// mega
			case 2: return FORMAT_MB; break;
			
			// giga
			case 3: return FORMAT_GB; break;	
		}
		
		return "";
	}
	
	
	//------------------------------------------------------------------------------------------------
	//! Return byte size in 3 digid max format with included size name up to GB:
	//! 123 B, 123 kB, 123 MB, 123 GB
	static string GetReadableSize(float value)
	{
		string unit = SCR_ByteFormat.ReadableSizeFromBytes(value); // format %1 unit	
		return WidgetManager.Translate(unit, value);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns byte size in megabytes, rounded to nearest MB.
	//! If size is lower than 1 MB, it is rounded to nearest 0.1 MB: 0.6 MB.
	static string GetReadableSizeMb(float value)
	{
		float mb = 1024.0*1024.0;
		float sizeMb = value / mb;
		
		if (sizeMb < 1.0)
		{
			int sizeMbMult10 = Math.Ceil(sizeMb * 10.0);
			
			if (sizeMbMult10 <= 10)
				sizeMb = sizeMbMult10 / 10;
			
			return WidgetManager.Translate(FORMAT_MB, sizeMb);
		}
		else
			return  WidgetManager.Translate(FORMAT_MB, Math.Round(sizeMb));
	}
};