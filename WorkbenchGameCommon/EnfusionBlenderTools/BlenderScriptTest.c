class BlenderScriptTestRequest: JsonApiStruct
{
	string Input;
	
	void BlenderScriptTestRequest()
	{
		RegV("Input");
	}
}

class BlenderScriptTestResponse: JsonApiStruct
{
	string Output;
	
	void BlenderScriptTestResponse()
	{
		RegV("Output");
	}
}

class BlenderScriptTest: NetApiHandler
{
	override JsonApiStruct GetRequest()
	{
		return new BlenderScriptTestRequest();
	}
	
	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		BlenderScriptTestRequest req = BlenderScriptTestRequest.Cast(request);
		Print("BlenderScriptTest request = " + req.Input);
		
		BlenderScriptTestResponse response = new BlenderScriptTestResponse();
		response.Output = "Hello from Workbench!";
		
		return response;
	}
}