// Script File 
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_AutomatedWeatherEditorAttribute : SCR_BaseEditorAttribute
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

		WeatherStateTransitionNode currentStateTransition = weatherTransitionManager.GetCurrentStateTransitionNode();
		
		return SCR_BaseEditorAttributeVar.CreateBool(!currentStateTransition.IsLooping());
	}
	
	override void UpdateInterlinkedVariables(SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, bool isInit = false)
	{
		if (!var)
			return;
		
		//Set sub labels
		if (isInit)
			manager.SetAttributeAsSubAttribute(SCR_WeatherInstantEditorAttribute);
		
		manager.SetAttributeEnabled(SCR_WeatherInstantEditorAttribute, !var.GetBool());
	}
	
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		//~ Ignore previews (!item)
		if (!var || !item) 
			return;
		
		TimeAndWeatherManagerEntity weatherManager = GetGame().GetTimeAndWeatherManager();
		if (!weatherManager) 
			return;
		
		WeatherStateTransitionManager weatherTransitionManager = weatherManager.GetTransitionManager();
		if (!weatherTransitionManager)
			return;
		
		if (!manager)
			return;
		
		WeatherStateTransitionNode currentStateTransition = weatherTransitionManager.GetCurrentStateTransitionNode();
		
		bool setAutomatic = var.GetBool();

		//Remove preview if any was set
		if (weatherTransitionManager.IsPreviewingState())
			weatherTransitionManager.SetStatePreview(false);
		
		//~ Set looping false
		if (setAutomatic)
		{
			weatherManager.ForceWeatherTo(!setAutomatic, string.Empty, 0, 0.001, playerID);
		}
		//~ Set looping true
		else 
		{
			float transitionTime = 0;
			
			SCR_BaseEditorAttributeVar transitionVar;
			if (manager.GetAttributeVariable(SCR_WeatherInstantTransitionTimeEditorAttribute, transitionVar))
				transitionTime = transitionVar.GetInt();
			
			weatherManager.ForceWeatherTo(!setAutomatic, string.Empty, transitionTime / 3600, 0.001, playerID);
		}
	}
	
	override void PreviewVariable(bool setPreview, SCR_AttributesManagerEditorComponent manager)
	{
		if (!GetVariable())
			return;
		
		bool SetAutomatic = GetVariable().GetBool();
		
		if (SetAutomatic || !setPreview)
		{
			TimeAndWeatherManagerEntity weatherManager = GetGame().GetTimeAndWeatherManager();
			if (!weatherManager) 
				return;
	
			WeatherStateTransitionManager weatherTransitionManager = weatherManager.GetTransitionManager();
			if (!weatherTransitionManager)
				return;
			
			//Remove preview if any was set
			if (weatherTransitionManager.IsPreviewingState())
				weatherTransitionManager.SetStatePreview(false);
		}
		else 
		{
			SCR_BaseEditorAttribute weatherInstantAttribute = manager.GetAttributeRef(SCR_WeatherInstantEditorAttribute);
			if (!weatherInstantAttribute)
				return;
		
			weatherInstantAttribute.PreviewVariable(setPreview, manager);
		}
	}
};