class GetPortalMatRequest: JsonApiStruct
{
	
	ref array<int> matHeights = new array<int>;
	ref array<int> matWidths = new array<int>;
	ref array<string> socketNames = new array<string>;

	void GetPortalMatRequest()
	{
		RegV("matHeights");
		RegV("matWidths");
		RegV("socketNames");
	}
}

class GetPortalMatResponse: JsonApiStruct
{
	ref array<string> matNames = new array<string>;
	
	void GetPortalMatResponse()
	{
		RegV("matNames");
	}
}

class GetPortalMat: NetApiHandler
{
	override JsonApiStruct GetRequest()
	{
		return new GetPortalMatRequest();
	}
	
	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		GetPortalMatRequest req = GetPortalMatRequest.Cast(request);		
		GetPortalMatResponse response = new GetPortalMatResponse();
		
		// no possibility to get door portal mats from name so far
		
		
		// Get folder with all PRT materials
		string absPath;
		string relPath = "Assets/_SharedMaterials/Portals";
		Workbench.GetAbsolutePath(relPath, absPath);
		array<string> sharedPortals = new array<string>();		
		FileIO.FindFiles(sharedPortals.Insert, absPath, ".emat");

		for(int i = 0; i < req.socketNames.Count(); i++)
		{
			// creating name with defined sizes
			string name = "PRT_" + req.matWidths[i] + "x" + req.matHeights[i];
			foreach(string portal : sharedPortals)
			{
				// if the name with desired sizes was found add the matName to array
				if(FilePath.StripPath(portal).Contains(name))
				{
					
					ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
					MetaFile meta = resourceManager.GetMetaFile(portal);
					string guid = meta.GetResourceID().Substring(1,16);
					string matName = FilePath.StripExtension(FilePath.StripPath(portal)) + "_" + guid;
					response.matNames.Insert(matName);
				}
			}

			// if not add null
			if(i >= response.matNames.Count())
			{
				response.matNames.Insert("");
			}
		} 	
		return response;
	}
}










