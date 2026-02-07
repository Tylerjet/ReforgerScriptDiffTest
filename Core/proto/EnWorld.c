/**																																											 /**
 * \defgroup World World
 * @{
 */

/*! 
\brief Post-process effect type.
\attention Keep enum names in synch with post-process effect material class names. Postfix "Effect" is added automatically.
*/
enum PostProcessEffectType
{
	None,
	UnderWater,
	SSAO,
	DepthOfField,
	HBAO,
	RotBlur,
	GodRays,
	Rain,
	FilmGrain,
	RadialBlur,
	ChromAber,
	WetDistort,
	DynamicBlur,
	ColorGrading,
	Colors,
	HDR,
	SMAA,
	FXAA,
	Median,
	SunMask,
	GaussFilter,
	SSR,
	DepthOfFieldBokeh
};

class SharedItem: pointer
{

};

typedef func QueryEntitiesCallback;
bool QueryEntitiesCallback(IEntity e); 

typedef func TraceEntitiesCallback;
bool TraceEntitiesCallback(notnull IEntity e, vector start = "0 0 0", vector dir = "0 0 0"); 

//@}
 
