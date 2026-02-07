class GamematInfoRequest : JsonApiStruct
{
	string Input;

	void GamematInfoRequest()
	{
		RegV("Input");
	}
}

class GamematInfoResponse : JsonApiStruct
{
	ref map<string, string> gamemats;

	void GamematInfoResponse(map<string, string> materials)
	{
		gamemats = materials;
	}
	
	override void OnPack()
	{
		
		foreach (string key, string value : gamemats)
		{
			StoreString(key,value);
		}
		
	}
}


static map<string, string> GetGamemats()
{
	map<string, string> result = new map<string, string>();		

	SearchResourcesFilter filter = new SearchResourcesFilter();
	filter.fileExtensions = {"gamemat"};
	
	array<ResourceName> gamemats = {};
	ResourceDatabase.SearchResources(filter, gamemats.Insert);
	
	string firstLetter;
	array<string> splitStrings = new array<string>();
	string guid;
	string name;
	string key;
	
	for (int i = 0; i < gamemats.Count(); i++)
	{
		// getting name
		if (gamemats[i].Contains("/"))
		{
			name = gamemats[i].Substring(gamemats[i].LastIndexOf("/") + 1, gamemats[i].IndexOf(".") - gamemats[i].LastIndexOf("/") - 1);
		}
		else
		{
			name = gamemats[i].Substring(gamemats[i].IndexOf("}") + 1, gamemats[i].IndexOf(".") - gamemats[i].IndexOf("}") - 1);
		}
		
		guid = gamemats[i].Substring(gamemats[i].IndexOf("{") + 1, gamemats[i].IndexOf("}") - 1);
		
		key = "";
		name.Split("_", splitStrings, false);
		
		
		foreach (string item : splitStrings)
		{
			firstLetter = item[0];
			firstLetter.ToUpper();
			item = firstLetter + item.Substring(1, item.Length()-1);
			
			if (key.IsEmpty())
				key = item;
			else
				key = key + " " + item;
		}
		
		
		result.Set(key, name + "_" + guid);
		
	}
	
	return result;
}


class GamematInfo : NetApiHandler
{
	override JsonApiStruct GetRequest()
	{
		return new GamematInfoRequest();
	}

	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		GamematInfoRequest req = GamematInfoRequest.Cast(request);
		
		GamematInfoResponse response = new GamematInfoResponse(GetGamemats());

		return response;
	}
}
