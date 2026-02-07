//------------------------------------------------------------------------------------------------
//! Contains various global constants
//------------------------------------------------------------------------------------------------

const float METERS_PER_SEC_TO_KILOMETERS_PER_HOUR = 3.6;
const float KILOMETERS_PER_HOUR_TO_METERS_PER_SEC = 0.277778;

const float METERS_TO_KNOTS = 1.943844;
const float KNOTS_TO_METERS = 0.514444;

const float COLOR_255_TO_1 = 0.003921568;
const float COLOR_1_TO_255 = 255;

const int TRACE_LAYER_CAMERA = EPhysicsLayerDefs.Camera;

class UIColors
{
	static const ref Color DARK_SAGE        		= Color.FromSRGBA(86, 92, 84, 255);
	static const ref Color DARK_GREY        		= Color.FromSRGBA(85, 85, 85, 255);
	static const ref Color CONTRAST_COLOR	 		= Color.FromSRGBA(226, 167, 79, 255);
	static const ref Color TRANSPARENT      		= Color.FromSRGBA(0, 0, 0, 0);

	// Refined UI colors
	static const ref Color CONTRAST_DISABLED		= Color.FromSRGBA(0, 0, 0, 38);
	static const ref Color CONTRAST_DEFAULT   		= Color.FromSRGBA(226, 167, 79, 76);
	static const ref Color CONTRAST_HOVERED			= Color.FromSRGBA(239, 199, 139, 102);
	static const ref Color CONTRAST_CLICKED  		= Color.FromSRGBA(226, 167, 79, 255);
	static const ref Color CONTRAST_CLICKED_HOVERED	= Color.FromSRGBA(226, 167, 79, 255);
	
	static const ref Color WHITE_DISABLED			= Color.FromSRGBA(255, 255, 255, 38);
	static const ref Color WHITE_DEFAULT			= Color.FromSRGBA(255, 255, 255, 25);
	static const ref Color WHITE_HOVERED			= Color.FromSRGBA(255, 255, 255, 102);
	
	static const ref Color BACKGROUND_DISABLED		= Color.FromSRGBA(0, 0, 0, 38);
	static const ref Color BACKGROUND_DEFAULT		= Color.FromSRGBA(0, 0, 0, 102);
	static const ref Color BACKGROUND_HOVERED		= Color.FromSRGBA(0, 0, 0, 153);
	
	static const ref Color WARNING_FOCUSED			= Color.FromSRGBA(249, 67, 67, 107);
	static const ref Color WARNING					= Color.FromSRGBA(249, 67, 67, 255);
	static const ref Color CONFIRM					= Color.FromSRGBA(67, 194, 93, 255);
	static const ref Color INFO						= Color.FromSRGBA(0, 128, 255, 255);
};

class UIConstants
{
    const float DISABLED_WIDGET_OPACITY = 0.3;
    const float ENABLED_WIDGET_OPACITY = 1;
	
	const int LOADING_SCREEN_Z_ORDER = 1000;
	const int SPLASH_SCREEN_Z_ORDER = 1001;
};