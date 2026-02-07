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
Container for loading data in json format
*/
class JsonLoadContainer: BaseLoadContainer
{
	void JsonLoadContainer() {}
	void ~JsonLoadContainer() {}
	
	proto external bool ImportFromString(string jsonData);
	proto external bool LoadFromFile(string fileName);
};

/** @}*/
