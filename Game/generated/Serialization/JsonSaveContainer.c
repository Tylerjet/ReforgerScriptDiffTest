/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Serialization
* @{
*/

/**
Container for saving data in json format
*/
class JsonSaveContainer: BaseSaveContainer
{
	void JsonSaveContainer() {}
	void ~JsonSaveContainer() {}
	
	proto string ExportToString();
	proto external bool SaveToFile(string fileName);
};

/** @}*/
