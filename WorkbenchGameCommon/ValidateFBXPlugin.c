[WorkbenchPluginAttribute(name: "Validate FBXs", wbModules: {"ResourceManager"},resourceTypes: {"fbx"} , awesomeFontCode: 0xf0ad, description:"Validates checks on FBX on various issues that can be tedious and time-consuming to verify manually", category:"EBT Validation")]
class ValidateFBXPlugin: WorkbenchPlugin
{	
	[Attribute("true", UIWidgets.CheckBox, "Report missing usage property on a colliders")]
	bool UsageProperty;
	[Attribute("true", UIWidgets.CheckBox, "Report invalid material names, that have invalid characters")]
	bool MaterialNames;
	[Attribute("true", UIWidgets.CheckBox, "Report invalid gamemat names, that are not using coll_friendlyname_GUID convention")]
	bool GamematNames;
	[Attribute("true", UIWidgets.CheckBox, "Report invalid bone names, bones that ends with \"_end\" suffix.")]
	bool BoneNames;	
	[Attribute("true", UIWidgets.CheckBox, "Report invalid texture/emat GUIDs and missing links in xob")]
	bool TextureGUIDs;
	[Attribute("true", UIWidgets.CheckBox, "Report objects that have Vertex Colors")]
	bool VertexColors;
	
	
	static ref array<string> allFbxs = new array<string>;
	
	// Tool is located in the root folder
	private void GetToolPath(out string toolPath)
	{
		const string TOOL_NAME = "/ValidateResource.exe";
		string rootPath;
		Workbench.GetCwd(rootPath);
		toolPath = rootPath + TOOL_NAME; 
		return;
	}
	
	override void OnResourceContextMenu(notnull array<ResourceName> resources) 
	{
		if (Workbench.ScriptDialog("Validate FBXs", "Select one or multiple FBX files you want to validate, also possible to select whole folder", this))
		{
			allFbxs.Clear();
			// get path to Validate.exe as parameter
			string toolPath;
			GetToolPath(toolPath);
			
			// get selected files
			int selected = 0;
			
			// filter only FBX files
			foreach(ResourceName asset: resources)
			{
				string lower = asset;
				lower.ToLower();
				string path;
				Workbench.GetAbsolutePath(asset.GetPath(),path);
				selected += 1;
				// prepare parameter
				allFbxs.Insert(path);
			}
			
			// config, which checks are selected
			string config = "";
			if(UsageProperty)
			{
				config += " -usage";
			}
			if(MaterialNames)
			{
				config += " -mname";
			}
			if(GamematNames)
			{
				config += " -gname";
			}
			if(BoneNames)
			{
				config += " -bname";
			}
			if(TextureGUIDs)
			{
				config += " -guid";
			}
			if(VertexColors)
			{
				config += " -vc";
			}
			if(selected == 0)
			{
				Print("No fbx file selected. Please select at least one fbx file in the resource browser.",LogLevel.WARNING);
			}
			// caling the ValidateFBX.exe with the right parameters
			else
			{
				string command = "\"" + toolPath + "\"" + config + " -pl FBX";
				Workbench.RunProcess(command);
				Print("Validating " + selected + " fbx file/s...");
			}
		}
	}
	
	override void Run()
	{
		if (Workbench.ScriptDialog("Validate FBXs", "Select one or multiple FBX files you want to validate, also possible to select whole folder", this))
		{
			allFbxs.Clear();
			// get path to Validate.exe as parameter
			string toolPath;
			GetToolPath(toolPath);
			
			// get selected files
			int selected = 0;
			ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
			array<ResourceName> selection = {};
			resourceManager.GetResourceBrowserSelection(selection.Insert, true);
			
			// filter only FBX files
			foreach(ResourceName asset: selection)
			{
				string lower = asset;
				lower.ToLower();
				if(lower.EndsWith(".fbx"))
				{
					string path;
					Workbench.GetAbsolutePath(asset.GetPath(),path);
					selected += 1;
					allFbxs.Insert(path);
					// prepare parameter
				}
			}
			
			// config, which checks are selected
			string config = "";
			if(UsageProperty)
			{
				config += " -usage";
			}
			if(MaterialNames)
			{
				config += " -mname";
			}
			if(GamematNames)
			{
				config += " -gname";
			}
			if(BoneNames)
			{
				config += " -bname";
			}
			if(TextureGUIDs)
			{
				config += " -guid";
			}
			if(VertexColors)
			{
				config += " -vc";
			}
			
			if(selected == 0)
			{
				Print("No fbx file selected. Please select at least one fbx file in the resource browser.",LogLevel.WARNING);
			}
			// caling the ValidateFBX.exe with the right parameters
			else
			{
				string command = "\"" + toolPath + "\"" + config + " -pl FBX";
				Workbench.RunProcess(command);
				Print("Validating " + selected + " fbx file/s...");
			}
		}
	}
	
	[ButtonAttribute("Validate")]
	bool OK()
	{
		string toolPath;
		GetToolPath(toolPath);
		if(!FileIO.FileExists(toolPath))
		{
			Print(toolPath + " doesn't exist, please reinstall or update your Workbench version", LogLevel.ERROR);
			return false;
		}
		return true;
	}
	
	[ButtonAttribute("Cancel")]
	bool Cancel()
	{
		return false;
	}

};

class FBXReportToolRequest: JsonApiStruct
{
	ref array<string> arrayMessage = new array<string>;
	string reportMessage;
	bool noError;
	ref array<bool> cleanFbxs = new array<bool>;
	
	void FBXReportToolRequest()
	{
		RegV("arrayMessage");
		RegV("reportMessage");
		RegV("noError");
		RegV("cleanFbxs");
	}
};

class FBXReportToolResponse: JsonApiStruct
{
	string Output;
	
	void FBXReportToolResponse()
	{
		RegV("Output");
	}
};


class FBXReportToolMessage: NetApiHandler
{
	override JsonApiStruct GetRequest()
	{
		return new FBXReportToolRequest();
	}
	
	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		FBXReportToolRequest req = FBXReportToolRequest.Cast(request);
		FBXReportToolResponse response = new FBXReportToolResponse();
		LogLevel level = LogLevel.NORMAL;
		array<string> messages = new array<string>;
		for(int i = 0; i < req.arrayMessage.Count(); i++)
		{
			// print header
			if(i == 0)
			{
				Print(string.ToString(req.arrayMessage[i].Substring(0,req.arrayMessage[i].Length()-2),false,false,false),LogLevel.NORMAL);
			}
			// print everything else
			else
			{
				// need to separate it per one option, because the message was too long
				req.arrayMessage[i].Split("||",messages,true);
				Print(string.ToString("\n" + messages[0],false,false,false),LogLevel.DEBUG);
				
				for(int j = 1; j < messages.Count(); j++)
				{
					if(req.cleanFbxs[i-1])
					{
						level = LogLevel.NORMAL;
					}
					else
					{
						level = LogLevel.WARNING;
					}
					Print(string.ToString("\n" + messages[j],false,false,false),level);
				}

			}
		}
		
		return response;
	}
}




class ValidateFBXUtilsReq: JsonApiStruct
{
	int index;

	void ValidateFBXUtilsReq()
	{
		RegV("index");
	}
};

class ValidateFBXUtilsRes: JsonApiStruct
{
	ref array<ResourceName> fbxs = new array<ResourceName>();
	int count;
	
	void ValidateFBXUtilsRes()
	{
		RegV("fbxs");
		RegV("count");
	}
};


class ValidateFBXUtils: NetApiHandler
{
	override JsonApiStruct GetRequest()
	{
		return new ValidateFBXUtilsReq();
	}
	
	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		ValidateFBXUtilsReq req = ValidateFBXUtilsReq.Cast(request);
		ValidateFBXUtilsRes response = new ValidateFBXUtilsRes();
		const int MAX_FBX_SEND = 4500;
		response.count = ValidateFBXPlugin.allFbxs.Count();
		if(ValidateFBXPlugin.allFbxs.Count() != 0)
		{
			for(int i = req.index-1; i < ValidateFBXPlugin.allFbxs.Count(); i++)
			{
				response.fbxs.Insert(ValidateFBXPlugin.allFbxs[i]);	
				if(response.fbxs.Count() >= MAX_FBX_SEND)
				{
					return response;
				}
			}
		}
		return response;
	}
}
