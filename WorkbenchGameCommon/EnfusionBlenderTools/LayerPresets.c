class LayerPresetsRequest: JsonApiStruct
{
	string Input;
	
	void LayerPresetsRequest()
	{
		RegV("Input");
	}
}

class LayerPresetsResponse: JsonApiStruct
{
	string Layers;
	
	void LayerPresetsResponse()
	{
		RegV("Layers");
	}
}

class LayerPresetsUtils
{
	void Getlayers(LayerPresetsResponse response)
	{
		BaseContainer cont = Workbench.GetGameProjectSettings();
		BaseContainerList config = cont.GetObjectArray("Configurations");
		
		string output;
		cont = config.Get(0);
		cont = cont.GetObject("PhysicsSettings");
		cont = cont.GetObject("Interactions");
		config = cont.GetObjectArray("LayerPresets");
		
		BaseContainer contLayerPresets;
		for(int i = 0; i < config.Count(); i++;)
		{
			contLayerPresets = config.Get(i);
			response.Layers += contLayerPresets.GetName() + " ";
		}
		return;
	}
}


class LayerPresets: NetApiHandler
{
	override JsonApiStruct GetRequest()
	{
		return new LayerPresetsRequest();
	}
	
	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		LayerPresetsRequest req = LayerPresetsRequest.Cast(request);
		LayerPresetsResponse response = new LayerPresetsResponse();
		LayerPresetsUtils utils = new LayerPresetsUtils();
		
		utils.Getlayers(response);
		
		
		return response;
	}
}