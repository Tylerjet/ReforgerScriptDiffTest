[BaseContainerProps()]
class SCR_MetaStruct: SCR_JsonApiStruct
{
	protected string bV; //--- Build version
	protected int cT; //--- Creation time
	protected string hR; //--- Header resource GUID
	
	//----------------------------------------------------------------------------------------
	string GetHeaderResource()
	{
		return hR;
	}
	
	//----------------------------------------------------------------------------------------
	void GetDateAndTime(out int year, out int month, out int day, out int hour, out int minute)
	{
		SCR_DateTimeHelper.ConvertMinutesIntoDate(cT, year, month, day, hour, minute);
	}
	
	//----------------------------------------------------------------------------------------
	override bool Serialize()
	{
		//--- Build version (ToDo)
		bV = GetGame().GetBuildVersion();
		
		//--- Creation time
		int y, m, d, hh, mm, ss;
		System.GetYearMonthDay(y, m, d);
		System.GetHourMinuteSecond(hh, mm, ss);
		cT = SCR_DateTimeHelper.ConvertDateIntoMinutes(y, m, d, hh, mm);
		
		//--- Header resource GUID
		MissionHeader header = GetGame().GetMissionHeader();
		if (header)
			hR = SCR_ConfigHelper.GetGUID(header.GetHeaderResourceName());
		else if (SCR_SaveLoadComponent.GetInstance())
			hR = SCR_ConfigHelper.GetGUID(SCR_SaveLoadComponent.GetInstance().GetDebugHeaderResourceName());
		
		return true;
	}
	
	//----------------------------------------------------------------------------------------
	override bool Deserialize()
	{
		return true;
	}
	
	//----------------------------------------------------------------------------------------
	override void Log()
	{
		int y, m, d, hh, mm;
		SCR_DateTimeHelper.ConvertMinutesIntoDate(cT, y, m, d, hh, mm);
		
		Print("--- SCR_MetaStruct --------------------------");
		PrintFormat("Build version = %1", bV);
		PrintFormat("Creation time = %1", SCR_FormatHelper.FormatDateTime(y, m, d, hh, mm, 0));
		PrintFormat("Header resource GUID = %1", hR);
		Print("---------------------------------------------");
	}
	
	//----------------------------------------------------------------------------------------
	void SCR_MetaStruct()
	{
		RegAll();
	}
}