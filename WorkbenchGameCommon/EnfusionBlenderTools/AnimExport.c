class AnimExportRequest: JsonApiStruct
{
	ref array<string> exportInfo = new array<string>;
	ref array<string> animationInfo = new array<string>;
	ref array<string> boneList = new array<string>;
	ref array<string> boneParents = new array<string>;
	ref array<string> emptyObjs = new array<string>;
	ref array<float> boneMatrix = new array<float>;
	ref array<string> BMatrixAss = new array<string>;
	ref array<string> EMatrixAss = new array<string>;
	ref array<float> emptyObjectMatrix = new array<float>;
	string path;
	string profiles;
	ref array<string> accessorBones = new array<string>;
	string blendFile;
	ref array<int> frameCounts = new array<int>;
	ref array<int> parents = new array<int>;
	
	void AnimExportRequest()
	{
		RegV("exportInfo");
		RegV("animationInfo");
		RegV("boneList");
		RegV("boneParents");
		RegV("emptyObjs");
		RegV("boneMatrix");
		RegV("BMatrixAss");
		RegV("EMatrixAss");
		RegV("emptyObjectMatrix");
		RegV("path");
		RegV("profiles");
		RegV("accessorBones");
		RegV("blendFile");
		RegV("frameCounts");
		RegV("parents");
	}
};

class AnimExportResponse: JsonApiStruct
{
	string Output;
	string Export;
	void AnimExportResponse()
	{
		RegV("Output");
		RegV("Export");
	}
};

class AnimExportUtils : AnimExporterAcc{
	
	protected map<string,ref array<string>> exportInfoToDict(string info)
	{
		array<string> temp = {};
		array<string> settings = {"export_enabled","export_file","export_profile","export_diff_pose_name","export_diff_pose_frame"};
		map<string,ref array<string>> output = new map<string,ref array<string>>;
		info.Split(",",temp,false);
		
		for(int i = 0; i < settings.Count(); i++)
		{
			array<string> value = new array<string>;
			for(int j = 0; j < temp.Count(); j+= 5)
			{
				value.Insert(temp[i+j]);
			}
			output.Insert(settings[i] ,value);
		}
		return output;
	}
	
	int addTakeToAccessor(map<string,int> takesDictionary,string animationName)
	{
		if(!takesDictionary.Contains(animationName))
		{
			int takeIdx = FillTakeInfo(animationName);
			takesDictionary[animationName] = takeIdx;
			return takeIdx;
		}
		else
		{
			return takesDictionary[animationName];
		}
	}
	void fillExportsForAnimation(string exportInfo, map<string,string> animation, map<string,int> takesDictionary, AnimExportProfileCtx ctx)
	{
		map<string,ref array<string>> exportInfoDict = exportInfoToDict(exportInfo);
		int frameRange = animation["frame_range"].ToInt();
		string fileName = animation["name"];
		
		int takeIdx = addTakeToAccessor(takesDictionary, fileName);
		Print("Adding take: " + animation["name"]);
		for(int i = 0; i < exportInfoDict["export_enabled"].Count(); i++)
		{
			if(!exportInfoDict["export_enabled"])
			{
				continue;
			}
			if(exportInfoDict["export_file"][i] != "")
			{
				fileName = exportInfoDict["export_file"][i];
			}
			AddExportForTake(takeIdx, exportInfoDict["export_profile"][i], fileName, 0, frameRange+1,string.Format("%1,%2",exportInfoDict["export_diff_pose_name"][i],exportInfoDict["export_diff_pose_frame"][i]));
			
			if(ctx.HasProfileDiffBones(exportInfoDict["export_profile"][i]))
			{
				takeIdx = addTakeToAccessor(takesDictionary, exportInfoDict["export_diff_pose_name"][i]);
			}
		}
	}
	int setArmatureAccessor(map<string,int> bonesDictionary, array<string> boneList, array<int> parents)
	{
		int selfID = -1;
		int rootID = -1;
		for(int index = 0; index < boneList.Count(); index++)
		{
			selfID = AddBone(parents[index], boneList[index]);
			bonesDictionary[boneList[index]] = selfID;
			rootID = selfID - index;
		}
		return rootID;
	}
	void setEmptyObjects(map<string,int> bonesDictionary, int rootID, array<string> emptyObjs)
	{
		foreach(string obj : emptyObjs)
		{
			int selfID = AddBone(rootID, obj);
			bonesDictionary[obj] = selfID;
		}
	}
	void setBoneAccessor(map<string,int> bonesDictionary, array<float> matrices, int takeID, array<string> bones)
	{
		float matrix[16];
		foreach(string child : bones)
		{
			for(int v = 0; v < 16; v++)
			{
				matrix[v] = matrices[0];
				matrices.RemoveOrdered(0);
			}
			int selfID = bonesDictionary[child];
			AddTrackKey(selfID,matrix,takeID);
		}
	}
	void setEmptyObjectsAccessor(map<string,int> bonesDictionary ,array<string> objs, array<float> matrices ,int takeID, int framesCount)
	{
		float matrix[16];
		
		for(int i = 0; i < framesCount + 1; i++)
		{
			foreach (string obj : objs)
			{	
				for(int v = 0; v < 16; v++)
				{
					matrix[v] = matrices[0];
					matrices.RemoveOrdered(0);
				}
				int selfID = bonesDictionary[obj];
				AddTrackKey(selfID,matrix,takeID);
			}
		}
	}
	
};


class AnimExport: NetApiHandler
{
	override JsonApiStruct GetRequest()
	{
		return new AnimExportRequest();
	}
	
	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		AnimExportRequest req = AnimExportRequest.Cast(request);
		AnimExportResponse response = new AnimExportResponse();		
		AnimExportUtils utils = new AnimExportUtils();
		AnimExportProfileCtx ctx = AnimExportProfileCtx.LoadProfile(req.profiles);
	 	map<string,int> bonesDictionary = new map<string,int>;
		array<string> temp = {};
		map<string,int> takesDictionary = new map<string,int>;
		map<string, string> animation = new map<string,string>;
		
		for(int i = 0; i < req.animationInfo.Count(); i++)
		{
			req.animationInfo[i].Split(",", temp, true);
			animation["name"] = temp[0];
			animation["frame_range"] = temp[1];
			utils.fillExportsForAnimation(req.exportInfo[i], animation, takesDictionary, ctx);
		}
		
		int rootID = -1;		
		foreach (string bone : req.accessorBones)
		{
			if(req.boneParents.Contains(bone))
			{
				int boneID = utils.setArmatureAccessor(bonesDictionary, req.accessorBones, req.parents);
				if (rootID == -1)
				{
					rootID = boneID;
				}
			}
		}	
		utils.setEmptyObjects(bonesDictionary, rootID, req.emptyObjs);

		for(int i = 0; i < takesDictionary.Count(); i++)
		{
			Print("Setting accessor for " + takesDictionary.GetKeyByValue(i));
			for(int frame = 0; frame < req.frameCounts[i] + 1; frame++)
			{
				foreach (string bone : req.accessorBones)
				{
					if(req.boneParents.Contains(bone))
					{
						utils.setBoneAccessor(bonesDictionary, req.boneMatrix, takesDictionary[takesDictionary.GetKeyByValue(i)], req.BMatrixAss);
					}
				}
			}
			utils.setEmptyObjectsAccessor(bonesDictionary, req.EMatrixAss, req.emptyObjectMatrix, takesDictionary[takesDictionary.GetKeyByValue(i)], req.frameCounts[i]);
		}
		utils.SetProjectName(req.blendFile);
		response.Output = ctx.GetErrorString();
		
		utils.ExportTake(ctx, req.path);
		response.Export = "Animations were succesfully exported to " + req.path;
		return response;
	}

}