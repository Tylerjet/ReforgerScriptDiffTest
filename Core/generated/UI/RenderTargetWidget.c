/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup UI
* @{
*/

sealed class RenderTargetWidget: Widget
{
	//!when period > 1 then every n-th frame will be rendered. Offset is initial counter.
	proto external void SetRefresh(int period, int offset);
	proto external void SetResolutionScale(float xscale, float ycale);
	proto external void SetClearColor(bool useClearColor, int color);
	proto external void SetBlendMode(RenderTargetWidgetBlendMode blendMode);
	proto external void SetFormat(RenderTargetWidgetFormat format);
	proto external void SetWorld(BaseWorld world, int camera);
};

/** @}*/
