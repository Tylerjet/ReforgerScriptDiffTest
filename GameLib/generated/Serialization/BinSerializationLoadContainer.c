/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Serialization
\{
*/

/*!
Implementation of BaseLoadContainer to read data from json format.
*/
//! Container for loading data in binary format
class BinSerializationLoadContainer: BaseSerializationLoadContainer
{
	proto external bool LoadFromFile(string fileName);
	proto external bool LoadFromContainer(notnull BinSerializationContainer container);
}

/*!
\}
*/
