/**
Respawn attribute to enable/disable respawn and respawn time, for getting and setting varriables in Editor Attribute window
*/
// Script File 
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_WindSpeedEditorAttribute : SCR_BaseValueListEditorAttribute
{
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		//If opened in global attributes
		if (!IsGameMode(item)) return null;
		
		TimeAndWeatherManagerEntity weatherManager = GetGame().GetTimeAndWeatherManager();
		if (!weatherManager) return null;
		
		return SCR_BaseEditorAttributeVar.CreateFloat(weatherManager.GetWindSpeed());
	}
	
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{		
		if (!var) 
			return;
		
		TimeAndWeatherManagerEntity weatherManager = GetGame().GetTimeAndWeatherManager();
		if (!weatherManager) 
			return;
		
		weatherManager.DelayedOverrideWindSpeed(var.GetFloat(), playerID);
	}
	
	override int GetEntries(notnull array<ref SCR_BaseEditorAttributeEntry> outEntries)
	{
		//Make sure max wind speed is set to heighest wind speed in varients
		TimeAndWeatherManagerEntity weatherManager = GetGame().GetTimeAndWeatherManager();
		if (weatherManager)
		{
			array<ref WeatherState> weatherStates = new array<ref WeatherState>;
			weatherManager.GetWeatherStatesList(weatherStates);
			
			float maxValue = m_baseValues.GetMaxValue();
			array<ref WeatherVariant> variants = {};
			WeatherWindPattern windPattern = new WeatherWindPattern();
			float newMax;
			
			foreach(WeatherState state: weatherStates)
			{
				state.GetVariantsList(variants);
				
				foreach(WeatherVariant variant: variants)
				{
					variant.GetWindPattern(windPattern);
					
					if (!windPattern)
					 continue;
					
					newMax = windPattern.GetMaxSpeed();
					
					if (newMax > maxValue)
						maxValue = newMax;
				}
			}
			
			m_baseValues.SetMaxValue(Math.Ceil(maxValue));
		}

		return super.GetEntries(outEntries);
	}
	
	//Call PreviewVariable of SCR_WindOverrideEditorAttribute
	override void PreviewVariable(bool setPreview, SCR_AttributesManagerEditorComponent manager)
	{
		if (!setPreview)
			return;
		
		SCR_BaseEditorAttribute overrideAttribute = manager.GetAttributeRef(SCR_WindOverrideEditorAttribute);
		if (!overrideAttribute)
			return;
		
		overrideAttribute.PreviewVariable(setPreview, manager);
	}
};