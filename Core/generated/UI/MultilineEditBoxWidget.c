/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup UI
* @{
*/

sealed class MultilineEditBoxWidget: UIWidget
{
	proto external int GetLinesCount();
	proto external int GetCarriageLine();
	proto external int GetCarriagePos();
	proto external void SetText(string text);
	proto external void SetLine(int line, string text);
	proto string GetText();
	proto string GetLine(int line);
};

/** @}*/
