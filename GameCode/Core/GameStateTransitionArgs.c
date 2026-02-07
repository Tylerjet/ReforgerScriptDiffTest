//------------------------------------------------------------------------------------------------
class ServerBrowserParams
{
	SBServerFlag GetFlags() { return SBServerFlag.JOINABLE; }
	string GetName();
	string GetScenarioName();
	string GetScenarioModId();
	string GetHostModId();
	string GetDescription();
	string GetClientVersion() { return "Client version"; }
	int GetGameType();
	string GetGameVersion() { return "EnfusionGame"; }
	int GetGameMode();
	SBRegion GetRegion() { return SBRegion.EUROPE; } //cannot be ANY
	SBPlatform GetPlatform() { return SBPlatform.PC; } //cannot be ANY
	int GetMaxPlayers() { return 16; }
	string GetJsonMetadata(); //set to empty string, not sure if it is working properly
	string GetHostIp()
	{
		// This is hack! This value should almost always come from config for real-world servers.
		// Ideally, the default implementation would return nothing and leave IP address determination
		// up to online services, but that doesn't seem to work very well for development, so using this
		// instead.
		return RplSession.TryGetListenAddress();
	}
	int GetHostPort() { return 2001; }
};
