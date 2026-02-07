class ArmaReforgerServerParams : ServerBrowserParams
{
	// This place can combine values obtained from mission header and server
	// config, or wherever else they would come from.
	ref SCR_MissionHeader m_MissionHeader;
	string m_sServerName = "Arma Reforger Prototype Server";
	string m_sServerIP = "";
	string m_sClientConnectIP = "";
	int m_iClientConnectPort = 0;

	//------------------------------------------------------------------------------------------------
	void ArmaReforgerServerParams(SCR_MissionHeader header)
	{
		m_MissionHeader = header;
		m_sServerName = string.Format("%1 (%2)", header.m_sName, SCR_Global.GetProfileName());
	}

	// Implement accessors to data. Unimplemented methods will still provide default values.

	override string GetName() { return m_sServerName; }
	override string GetScenarioName() { return m_MissionHeader.m_sName; }
	override string GetScenarioModId() { return m_MissionHeader.m_sOwner; }
	override string GetHostModId() { return m_MissionHeader.m_sOwner; }
	override string GetDescription() { return m_MissionHeader.m_sDescription; }
	override int GetMaxPlayers() { return m_MissionHeader.m_iPlayerCount; }

	override string GetHostIp()
	{
		string clientConnectIp = m_sClientConnectIP;
		if (clientConnectIp != string.Empty)
			return clientConnectIp;

		return super.GetHostIp();
	}

	override int GetHostPort()
	{
		int clientConnectPort = m_iClientConnectPort;
		if (clientConnectPort != 0)
			return clientConnectPort;

		return super.GetHostPort();
	}
};

class HeaderFileCallback
{
	array<ref SCR_MissionHeader> m_aHeaders;
	ref array<string> m_aPaths = {};
	WorkshopApi api;
	
	void FindFilesCallback(string fileName, FileAttribute attributes = 0, string filesystem = string.Empty)
	{
		ref SCR_MissionHeader header = SCR_MissionHeader.Cast(MissionHeader.ReadMissionHeader( fileName ));
		header.m_sOwner = filesystem;
		m_aHeaders.Insert(header);
		m_aPaths.Insert(string.Format(fileName));
	}
};

class HeaderSearchCallback
{
	string addon;
	string scenario;
	WorkshopApi api;
	ref SCR_MissionHeader output;
	
	void FindFilesCallback(string fileName, FileAttribute attributes = 0, string filesystem = string.Empty)
	{
		if (filesystem == addon)
		{
			ref SCR_MissionHeader header = SCR_MissionHeader.Cast(MissionHeader.ReadMissionHeader( fileName ));
			if (header.m_sName == scenario)
				output = header;
		}
	}
};
