class DSGameConfig: JsonApiStruct
{
	string name;
	string scenarioId;
	string hostedScenarioModId;
	int playerCountLimit
	string password;
	bool visible
	
	ref DSGameProperties gameProperties;
	ref array<ref DSMod> mods;
	
	void DSGameConfig()
	{
		RegV("name");
		RegV("playerCountLimit");
		RegV("password");
		RegV("scenarioId");
		RegV("hostedScenarioModId");	
		RegV("gameProperties");
		RegV("mods");
	}
	
	override void OnPack()
	{
		UnregV("visible");
		StoreBoolean("visible", visible);
	}
	
	override void OnExpand()
	{
		RegV("visible");
	}
}

class DSGameProperties: JsonApiStruct
{
	bool battlEye;
		
	override void OnPack()
	{
		UnregV("battlEye");
		StoreBoolean("battlEye", battlEye);
	}
	
	override void OnExpand()
	{
		RegV("battlEye");
	}
}

class DSMod: JsonApiStruct
{
	string modId;
	string name;
	string version;

	void DSMod()
	{
		RegV("modId");
		RegV("name");
		RegV("version");
	}
}

class DSConfig: JsonApiStruct
{
	string gameHostBindAddress;
	int gameHostBindPort;
	string gameHostRegisterBindAddress;
	int gameHostRegisterPort;
	string adminPassword;
	
	ref DSGameConfig game;
	
	bool crossPlatform;
	
	void DSConfig()
	{		
		RegV("gameHostBindAddress");
		RegV("gameHostBindPort");
		RegV("gameHostRegisterBindAddress");
		RegV("gameHostRegisterPort");
		RegV("adminPassword");
		
		RegV("game");
	}

	override void OnPack()
	{
		UnregV("crossPlatform");
		StoreBoolean("crossPlatform", crossPlatform);
	}
	
	override void OnExpand()
	{
		RegV("crossPlatform");
	}
}