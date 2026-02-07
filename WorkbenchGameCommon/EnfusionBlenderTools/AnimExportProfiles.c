class AnimExportProfilesRequest: JsonApiStruct
{
	string ExportProfileAbsPath;
	
	void AnimExportProfilesRequest()
	{
		RegV("ExportProfileAbsPath");
	}
}

class AnimExportProfilesResponse: JsonApiStruct
{
	string ProfileNames;
	string PathToExportProfiles;
	
	void AnimExportProfilesResponse()
	{
		RegV("ProfileNames");
		RegV("PathToExportProfiles");
	}
}


class AnimExportProfilesUtils{
	
	string createProfileNamesCache(AnimExportProfileCtx ctx)
	{
		string profileNames = "[";		
		for(int i = 0; i < ctx.GetNumProfiles(); i++)
		{
			string profileFormat = "'" + ctx.GetProfileName(i) + "', ";
			if(i + 1 != ctx.GetNumProfiles())
			{
				profileNames += "(" + profileFormat + profileFormat + "''), ";
			}
			else
			{
				profileNames += "(" + profileFormat + profileFormat + "'')";
			}
		}
		profileNames += "]";
		return profileNames;
	}
}


class AnimExportProfiles: NetApiHandler
{
	override JsonApiStruct GetRequest()
	{
		return new AnimExportProfilesRequest();
	}
	
	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		AnimExportProfilesRequest req = AnimExportProfilesRequest.Cast(request);
		AnimExportProfilesResponse response = new AnimExportProfilesResponse();
		AnimExportProfilesUtils utils = new AnimExportProfilesUtils();
		AnimExportProfileCtx ctx = AnimExportProfileCtx.LoadProfile(req.ExportProfileAbsPath);
		
		response.ProfileNames = utils.createProfileNamesCache(ctx);
		response.PathToExportProfiles = req.ExportProfileAbsPath;
		return response;
	}

}