/*
===========================================
Do not modify, this script is generated
===========================================
*/

class WeatherVariant
{
	proto external string GetVariantName();
	proto external float GetDurationMin();
	proto external float GetDurationMax();
	proto external void GetTransitionsList(out notnull array<ref WeatherTransition> transitions);
	proto external void GetWindPattern(out notnull WeatherWindPattern windPattern);
	proto external void GetRainPattern(out notnull WeatherRainPattern rainPattern);
};
