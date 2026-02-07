class SCR_LatestSaveSettings: ModuleGameSettings
{
	[Attribute()]
	ref array<ref SCR_LatestSaveEntry> m_aEntries;
	
	//----------------------------------------------------------------------------------------
	/*!
	Set the latest save file name for given mission.
	\param missionFileName Mission save file name
	\param saveFileName Latest save file name
	*/
	void SetFileName(string missionFileName, string saveFileName)
	{
		SCR_LatestSaveEntry entry;
		foreach (SCR_LatestSaveEntry entryCandidate: m_aEntries)
		{
			if (entryCandidate.m_sMissionFileName == missionFileName)
			{
				entry = entryCandidate;
				break;
			}
		}
		if (!entry)
		{
			entry = new SCR_LatestSaveEntry();
			entry.m_sMissionFileName = missionFileName;
			m_aEntries.Insert(entry);
		}
		entry.m_sSaveFileName = saveFileName;
		
		Print(string.Format("SCR_LatestSaveSettings: Latest save for %1 is now %2", missionFileName, saveFileName), LogLevel.VERBOSE);
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Remove the latest save file name for given mission.
	\param missionFileName Mission save file name
	*/
	void RemoveFileName(string missionFileName)
	{
		for (int i = m_aEntries.Count() - 1; i >= 0; i--)
		{
			if (m_aEntries[i].m_sMissionFileName == missionFileName)
			{
				m_aEntries.Remove(i);
		
				Print(string.Format("SCR_LatestSaveSettings: Latest save for %1 removed", missionFileName), LogLevel.VERBOSE);
				break;
			}
		}
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Find the latest save file name for given mission.
	\param missionFileName Mission save file name
	\param[out] outSaveFileName String to be filled with the latest save file name
	\return True if the latest save file name was found
	*/
	bool FindFileName(string missionFileName, out string outSaveFileName)
	{
		for (int i = m_aEntries.Count() - 1; i >= 0; i--)
		{
			if (m_aEntries[i].m_sMissionFileName == missionFileName)
			{
				outSaveFileName = m_aEntries[i].m_sSaveFileName;
				return true;
			}
		}
		return false;
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Print out all latest saves kept in the memory.
	*/
	void Log()
	{
		int count = m_aEntries.Count();
		PrintFormat("Latest saves: %1", count);
		for (int i = count - 1; i >= 0; i--)
		{
			PrintFormat("  %1: %2", m_aEntries[i].m_sMissionFileName, m_aEntries[i].m_sSaveFileName);
		}
	}
};

[BaseContainerProps(), BaseContainerCustomTitleField("m_sMissionFileName")]
class SCR_LatestSaveEntry: JsonApiStruct
{
	[Attribute()]
	string m_sMissionFileName;
	
	[Attribute()]
	string m_sSaveFileName;
};