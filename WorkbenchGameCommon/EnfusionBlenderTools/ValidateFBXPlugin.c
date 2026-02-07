[WorkbenchPluginAttribute(name: "Validate FBXs", wbModules: {"ResourceManager"}, awesomeFontCode: 0xf0ad)]
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
	
	
	
	override void Run()
	{
		if (Workbench.ScriptDialog("Validate FBXs", "Select one or multiple FBX files you want to validate, also possible to select whole folder", this))
		{
			// get path to Validate.exe
			ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
			string toolPath;
			Workbench.GetCwd(toolPath);
			toolPath += "/ValidateFBX/ValidateFBXs.exe";
			string exePath = toolPath + " -p";
			
			// get selected files
			int selected = 0;
			array<ResourceName> selection = {};
			resourceManager.GetResourceBrowserSelection(selection.Insert, true);
			string fbxs = string.Empty;
			foreach(ResourceName asset: selection)
			{
				string lower = asset;
				lower.ToLower();
				// filter only FBX files
				if(lower.EndsWith(".fbx"))
				{
					string path;
					Workbench.GetAbsolutePath(asset.GetPath(),path);
					selected += 1;
					fbxs += " " + "\"" + path + "\"";
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
				config += " -guids";
			}
			
			if(fbxs == string.Empty)
			{
				Print("No fbx file selected. Please select at least one fbx file in the resource browser.");
			}
			// caling the ValidateFBX.exe with the right parameters
			else
			{
				string command = exePath + fbxs + config;
				Workbench.RunProcess(command);
				Print("Validating " + selected + " fbx file/s...");
			}
		}
	}
	
	[ButtonAttribute("Validate")]
	bool OK()
	{
		string toolPath;
		Workbench.GetCwd(toolPath);
		toolPath += "/ValidateFBX/ValidateFBXs.exe";
		
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
		// change LogLevel when there is an error


		array<string> messages = new array<string>;
		for(int i = 0; i < req.arrayMessage.Count(); i++)
		{
			// print header
			if(i == 0)
			{
				Print(string.ToString(req.arrayMessage[i].Substring(0,req.arrayMessage[i].Length()-2)),LogLevel.NORMAL);
			}
			// print everything else
			else
			{
				// need to separate it per one option, because the message was too long
				req.arrayMessage[i].Split("||",messages,true);
				foreach(string message: messages)
				{
					if(req.cleanFbxs[i-1])
					{
						level = LogLevel.NORMAL;
					}
					else
					{
						level = LogLevel.WARNING;
					}
					Print(string.ToString(message),level);
				}
			}
		}
		
		return response;
	}
}
