class ActiveBansObject: JsonApiStruct
{
	ref array<ref BanObject> activeBans;

	void ActiveBansObject()
	{
		activeBans = {};
		RegV("activeBans");
	}
}

class BanObject: JsonApiStruct
{
	string scope;
	string type;
	string reason;
	string issuer;
	int createdAt;
	int expiresAt;

	void BanObject()
	{
		RegV("scope");
		RegV("type");
		RegV("reason");
		RegV("issuer");
		RegV("createdAt");
		RegV("expiresAt");
	}
}