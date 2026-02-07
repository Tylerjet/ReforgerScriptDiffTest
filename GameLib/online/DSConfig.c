
class GenericJsonApiStruct: JsonApiStruct
{
	map<string, int> intFields;
	map<string, bool> boolFields;
	map<string, string> stringFields;
	map<string, float> floatFields;
	map<string, ref GenericJsonApiStruct> objectFields;
}


class DSGameConfig: JsonApiStruct
{
	string name;
	string scenarioId;
	int playerLimit
	string password;
	bool visible
	
	void DSGameConfig()
	{
		RegV("name");
		RegV("playerLimit");
		RegV("password");
		RegV("scenarioId");
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
	ref GenericJsonApiStruct scenario; 
}

class DSMod: JsonApiStruct
{
	string modId;
	string name;
	string version;
}

class DSConfig: JsonApiStruct
{
	string gameHostBindAddress;
	int gameHostBindPort;
	string gameHostRegisterBindAddress;
	int gameHostRegisterBindPort;
	string adminPassword;
	
	ref DSGameConfig game;
	ref DSGameProperties gameProperties;
	ref array<ref DSMod> mods;
	
	void DSConfig()
	{		
		RegV("gameHostBindAddress");
		RegV("gameHostBindPort");
		RegV("gameHostRegisterBindAddress");
		RegV("gameHostRegisterBindPort");
		RegV("adminPassword");
		
		RegV("game");
		RegV("gameProperties");
		RegV("mods");
	}
}