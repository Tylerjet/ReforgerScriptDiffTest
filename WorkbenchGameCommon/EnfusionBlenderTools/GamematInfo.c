class GamematInfoRequest: JsonApiStruct
{
	string Input;
	
	void GamematInfoRequest()
	{
		RegV("Input");
	}
};

class GamematInfoResponse: JsonApiStruct
{
	string Gamemat;
	
	void GamematInfoResponse()
	{
		RegV("Gamemat");
	}
};

class GamematInfoUtils
{
	void GetGamemat(GamematInfoResponse response)
	{
		// Getting gamemats from rdb
		array<ResourceName> gamemats = {};
		string guid;
		string name;
		Workbench.SearchResources(gamemats.Insert, {"gamemat"});
		for(int i = 0; i < gamemats.Count(); i++)
		{
			if(gamemats[i].Contains("/"))
			{
				name = gamemats[i].Substring(gamemats[i].LastIndexOf("/") + 1, gamemats[i].IndexOf(".") - gamemats[i].LastIndexOf("/") - 1);
			}
			else
			{
				name = gamemats[i].Substring(gamemats[i].IndexOf("}") + 1, gamemats[i].IndexOf(".") - gamemats[i].IndexOf("}") - 1);
			}
			guid = gamemats[i].Substring(gamemats[i].IndexOf("{") + 1, gamemats[i].IndexOf("}") - 1);
			if(i + 1 == gamemats.Count())
			{
				response.Gamemat += name + "_" + guid;
			}
			else
			{
				response.Gamemat += name + "_" + guid + " ";
			}
		}
		return;
	}
}


class GamematInfo: NetApiHandler
{
	override JsonApiStruct GetRequest()
	{
		return new GamematInfoRequest();
	}
	
	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		GamematInfoRequest req = GamematInfoRequest.Cast(request);
		GamematInfoResponse response = new GamematInfoResponse();
		GamematInfoUtils utils = new GamematInfoUtils();
		
		ResourceName input = req.Input;
		
		utils.GetGamemat(response);

		return response;
	}
}