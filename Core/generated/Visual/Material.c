/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Visual
* @{
*/

sealed class Material: pointer
{
	/*!
	get material, must exist in internal cache
	\param matName	material resource
	\param flags		material creation flags, use 0
	*/
	static proto Material GetMaterial(ResourceName matName);
	/*!
	get material or load it of it doesn't exist in internal cache
	\param matName	material resource
	\param flags		material creation flags, use 0
	*/
	static proto Material GetOrLoadMaterial(ResourceName matName, int flags);
	static proto Material Create(string name, string matClassName, map<string, string> params = null);
	/*!
	set parametr of material by string name
	\param paramName	name of parameter
	\param value		value
	*/
	proto external bool SetParam(string paramName, void value);
	/*!
	reset parametr of material to default value
	\param paramName	name of parameter
	*/
	proto external void ResetParam(string paramName);
	/*!
	set parametr index for faster access to material properties
	\param paramName	name of parameter
	\return parameter index
	*/
	proto external int GetParamIndex(string paramName);
	/*!
	set parametr of material by index
	\param paramName	name of parameter
	\param value		value
	*/
	proto external void SetParamByIndex(int paramIndex, void value);
	/*!
	release material (created be CreateMaterial)
	*/
	proto external void Release();
	/*!
	material name
	*/
	proto external void GetName(out string name);
};

/** @}*/
