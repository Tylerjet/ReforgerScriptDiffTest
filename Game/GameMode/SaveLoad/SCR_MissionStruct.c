/*!
Base struct for use in SCR_MissionStruct.
Register new structs to be saved here.
The must be declared specifically, we can't have an array of structs, as JSON does not recognize class inheritance.
*/
[BaseContainerProps()]
class SCR_MissionStruct: SCR_JsonApiStruct
{
	[Attribute()]
	protected ref SCR_JsonApiStruct m_Struct0;
	
	[Attribute()]
	protected ref SCR_JsonApiStruct m_Struct1;
	
	[Attribute()]
	protected ref SCR_JsonApiStruct m_Struct2;
	
	[Attribute()]
	protected ref SCR_JsonApiStruct m_Struct3;
	
	[Attribute()]
	protected ref SCR_JsonApiStruct m_Struct4;
	
	[Attribute()]
	protected ref SCR_JsonApiStruct m_Struct5;
	
	[Attribute()]
	protected ref SCR_JsonApiStruct m_Struct6;
	
	[Attribute()]
	protected ref SCR_JsonApiStruct m_Struct7;
	
	[Attribute()]
	protected ref SCR_JsonApiStruct m_Struct8;
	
	[Attribute()]
	protected ref SCR_JsonApiStruct m_Struct9;
	
	//--- Array of structs can't be attribute directly, because it would be deserialzied using base class SCR_JsonApiStruct
	protected ref array<SCR_JsonApiStruct> m_aStructs = {m_Struct0, m_Struct1, m_Struct2, m_Struct3, m_Struct4, m_Struct5, m_Struct6, m_Struct7, m_Struct8, m_Struct9};
	
	override bool Serialize()
	{
		foreach (SCR_JsonApiStruct struct: m_aStructs)
		{
			if (struct && !struct.Serialize())
				return false;
		}
		return true;
	}
	override bool Deserialize()
	{
		foreach (SCR_JsonApiStruct struct: m_aStructs)
		{
			if (struct && !struct.Deserialize())
				return false;
		}
		return true;
	}
	override void Log()
	{
		foreach (SCR_JsonApiStruct struct: m_aStructs)
		{
			if (struct)
				struct.Log();
		}
	}
	void SCR_MissionStruct()
	{
		foreach (int i, SCR_JsonApiStruct struct: m_aStructs)
		{
			if (struct)
				RegV("m_Struct" + i); //--- Register only structs that are actually defined
		}
	}
};