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
	[DEPRECATED]
	Sets Widget of type RTTextureWidgetTypeID, to which it is possible to reference in shader as $rendertarget.
	It is possible to use only after object selection. When selecting another object, it is necessary to set GUI widget again.

	Currently index of render target is not used and zero index is always assigned in the case the index is below 4 (this is the
	maximum internal render targets amount). Otherwise the rt is deleted from entity object component - usually should be called
	in this manner (e.g. rtIndex = -1) when the widget RT is destroyed.
	*/
	[Obsolete("Use SetRenderTarget(IEntity ent) instead")]
	proto external void SetGUIWidget(IEntity ent, int rtIndex);
	/*!
	Sets this widget instance (of type RTTextureWidgetTypeID) as render resource which can be referenced in material as $rendertarget.
	It is possible to use only after object selection. When selecting another object, it is necessary to set the render target again.
	When destroying the widget, use RemoveRenderTarget(IEntity ent) to remove the render target from the entity's mesh object.
	*/
	proto external void SetRenderTarget(IEntity ent);
	/*!
	Removes this widget instance (of type RTTextureWidgetTypeID) as render resource from the entity's mesh object. Must be called
	when destroying the widget.
	*/
	proto external void RemoveRenderTarget(IEntity ent);
	/*!
	Sets Widget of type RTTextureWidgetTypeID, to which it is possible to reference in shader as $renderview
	It is possible to use only after object selection. When selecting another object, it is necessary to set widget again.

	Camera index should be set to the camera that is used for rendering in this render target, set it to -1 if you are destroying
	the widget.
	*/
	proto external void SetRenderView(IEntity ent, int cameraIndex);
}

/*!
\}
*/
