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
	ref TargetObject target;
	string targetName;
	string scope;
	string type;
	string reason;
	string issuer;
	int createdAt;
	int expiresAt;

	void BanObject()
	{
		RegV("target");
		RegV("targetName");
		RegV("scope");
		RegV("type");
		RegV("reason");
		RegV("issuer");
		RegV("createdAt");
		RegV("expiresAt");
	}
}

class TargetObject: JsonApiStruct
{
	string type;
	string value;

	void BanObject()
	{
		RegV("type");
		RegV("value");
	}
}