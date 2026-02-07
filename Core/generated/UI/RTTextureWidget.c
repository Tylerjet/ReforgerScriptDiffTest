/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup UI
\{
*/

sealed class RTTextureWidget: Widget
{
	/*!
	Sets Widget of type RTTextureWidgetTypeID, to which it is possible to reference in shader as $rendertarget
	it is possible to use only after object selection. When selecting another object, it is necessary to set GUI widget again

	Currently index of render target is not used and zero index is always assigned in the case the index is below 4 (this is the
	maximum internal render targets amount). Otherwise the rt is deleted from entity object component - usually should be called
	in this manner (e.g. index = -1) when the widget RT is destroyed.
	*/
	proto external void SetGUIWidget(IEntity ent, int index);
}

/*!
\}
*/
