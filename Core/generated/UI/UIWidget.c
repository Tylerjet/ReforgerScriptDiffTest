/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup UI
\{
*/

sealed class UIWidget: Widget
{
	proto external void SetTextColor(int color);
	/*!
	Sets outline style.
	The underlying font implementation may not support it, if this is the case, it will do nothing.
	*/
	proto external void SetTextOutline(int outlineSize, int argb = 0xFF000000);
	proto external int GetTextOutlineSize();
	proto external int GetTextOutlineColor();
	/*!
	Sets shadow style.
	The underlying font implementation may not support it, if this is the case, it will do nothing.
	*/
	proto external void SetTextShadow(int shadowSize, int shadowARGB = 0xFF000000, float shadowOpacity = 1.0, float shadowOffsetX = 0.0, float shadowOffsetY = 0.0);
	proto external int GetTextShadowSize();
	/*!
	Returns shadow color encoded as ARGB
	*/
	proto external int GetTextShadowColor();
	proto external float GetTextShadowOpacity();
	proto external float GetTextShadowOffsetX();
	proto external float GetTextShadowOffsetY();
	/*!
	Sets italic style.
	The underlying font implementation may not support it, if this is the case, it will do nothing.
	*/
	proto external void SetTextItalic(bool italic);
	/*!
	Gets current italic style.
	*/
	proto external bool GetTextItalic();
	/*!
	Sets bold style.
	The underlying font implementation may not support it, if this is the case, it will do nothing.
	*/
	proto external void SetTextBold(bool bold);
	/*!
	Gets current bold style.
	*/
	proto external bool GetTextBold();
}

/*!
\}
*/
