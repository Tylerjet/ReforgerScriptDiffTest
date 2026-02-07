/**
Weather Attribute for getting and setting varriables in Editor Attribute window
*/
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_WeatherInstantEditorAttribute: SCR_BasePresetsEditorAttribute
{	
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{		
		//If opened in global attributes
		if (!IsGameMode(item)) 
			return null;
		
		TimeAndWeatherManagerEntity weatherManager = GetGame().GetTimeAndWeatherManager();
		if (!weatherManager) 
			return null;
		
		WeatherStateTransitionManager weatherTransitionManager = weatherManager.GetTransitionManager();
		if (!weatherTransitionManager)
			return null;
		
		array<ref WeatherState> weatherStates = new array<ref WeatherState>;
		
		//Get weather states
		weatherManager.GetWeatherStatesList(weatherStates);
		if (weatherStates.IsEmpty()) 
			return null;
		
		WeatherState currentState = weatherManager.GetCurrentWeatherState();
		return SCR_BaseEditorAttributeVar.CreateInt(currentState.GetStateID());
	}
	
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var) 
			return;
		
		TimeAndWeatherManagerEntity weatherManager = GetGame().GetTimeAndWeatherManager();
		if (!weatherManager) 
			return;
		
		int stateIndex = var.GetInt();
		
		array<ref WeatherState> weatherStates = new array<ref WeatherState>;
		weatherManager.GetWeatherStatesList(weatherStates);
		
		//Valid state
		if (weatherStates.IsEmpty() || stateIndex >= weatherStates.Count())
			return;
		
		WeatherState weatherToSet = weatherStates[stateIndex];
				
		//Preview Weather
		if (!item)
		{
			WeatherStateTransitionManager weatherTransitionManager = weatherManager.GetTransitionManager();
			if (!weatherTransitionManager)
				return;
			
			weatherTransitionManager.SetStatePreview(true, weatherToSet.GetStateName());
			return;
		}
		
		float transitionTime = 0;
		
		SCR_BaseEditorAttributeVar transitionVar;
		if (manager && manager.GetAttributeVariable(SCR_WeatherInstantTransitionTimeEditorAttribute, transitionVar))
			transitionTime = transitionVar.GetInt();
		
		weatherManager.ForceWeatherTo(true, weatherToSet.GetStateName(), transitionTime / 3600, 0.001, playerID);
	}
	
	override void PreviewVariable(bool setPreview, SCR_AttributesManagerEditorComponent manager)
	{
		//Preview weather. Note that if advanced is turned on preview should disapear
		if (setPreview)
		{
			WriteVariable(null, GetVariable(), manager, -1);
		}
		else 
		{
			TimeAndWeatherManagerEntity weatherManager = GetGame().GetTimeAndWeatherManager();
			if (!weatherManager) 
				return;
		
			WeatherStateTransitionManager weatherTransitionManager = weatherManager.GetTransitionManager();
			if (!weatherTransitionManager)
				return;
			
			//Remove preview
			if (weatherTransitionManager.IsPreviewingState())
				weatherTransitionManager.SetStatePreview(false);
		}
	}
	
	protected override void CreatePresets()
	{
		m_aValues.Clear();
		
		SCR_EditorAttributeFloatStringValueHolder value;
		
		array<ref WeatherState> weatherStates = new array<ref WeatherState>;
		TimeAndWeatherManagerEntity weatherManager = GetGame().GetTimeAndWeatherManager();
		if (!weatherManager)
			return;
		
		weatherManager.GetWeatherStatesList(weatherStates);
		
		int count = weatherStates.Count();
		
		//Create preset list
		for (int i = 0; i < count; i++)
		{
			value = new SCR_EditorAttributeFloatStringValueHolder();
			value.SetName(weatherStates[i].GetLocalizedName());
			value.SetIcon(weatherStates[i].GetIconPath());
			value.SetFloatValue(weatherStates[i].GetStateID());
			
			m_aValues.Insert(value);
		}
	}
};