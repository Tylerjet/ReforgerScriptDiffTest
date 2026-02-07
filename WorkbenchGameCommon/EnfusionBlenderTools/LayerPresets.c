class LayerPresetsRequest : JsonApiStruct
{
	string Input;

	void LayerPresetsRequest()
	{
		RegV("Input");
	}
}

class LayerPresetsResponse : JsonApiStruct
{
	ref array<string> layersPresets;

	void LayerPresetsResponse(array<string> layers)
	{
		this.layersPresets = layers;
	}
	
	override void OnPack()
	{
		StartArray("Layer Presets");
		foreach (string item : layersPresets)
		{
			ItemString(item);
		}
		EndArray();
	}
}


static array<string> GetLayerPresets()
{
	// get project setting conf
	BaseContainer cont = Workbench.GetGameProjectSettings();
	BaseContainerList config = cont.GetObjectArray(EBTContainerFields.conf);

	cont = config.Get(0);
	cont = cont.GetObject("PhysicsSettings");
	cont = cont.GetObject("Interactions");
	config = cont.GetObjectArray("LayerPresets");

	// getting all LayerPresets
	BaseContainer contLayerPresets;
	
	array<string> result = new array<string>();
	
	for (int i = 0; i < config.Count(); i++; )
	{
		contLayerPresets = config.Get(i);
		//response.Layers += contLayerPresets.GetName() + " ";
		result.Insert(contLayerPresets.GetName());
	}
	return result;
}

class LayerPresets : NetApiHandler
{
	override JsonApiStruct GetRequest()
	{
		return new LayerPresetsRequest();
	}

	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		LayerPresetsRequest req = LayerPresetsRequest.Cast(request);
		
		
		return new LayerPresetsResponse(GetLayerPresets());
	}
}
