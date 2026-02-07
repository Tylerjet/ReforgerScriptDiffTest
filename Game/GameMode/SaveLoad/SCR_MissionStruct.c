/*!
Base struct for use in SCR_MissionStruct.
Register new structs to be saved here.
The must be declared specifically, we can't have an array of structs, as JSON does not recognize class inheritance.
*/
[BaseContainerProps()]
class SCR_MissionStruct: SCR_JsonApiStruct
{
	[Attribute()]
	protected ref SCR_CampaignStruct m_CampaignStruct;
	
	[Attribute()]
	protected ref SCR_EditorStruct m_EditorStruct;
	
	//[Attribute()]
	//protected ref SCR_ExampleStruct m_ExampleStruct;
	
	override bool Serialize()
	{
		if (m_CampaignStruct && !m_CampaignStruct.Serialize())
			return false;
		
		if (m_EditorStruct && !m_EditorStruct.Serialize())
			return false;
		
		//if (m_ExampleStruct && !m_ExampleStruct.Serialize())
		//	return false;
		
		return true;
	}
	override bool Deserialize()
	{
		if (m_CampaignStruct && !m_CampaignStruct.Deserialize())
			return false;
		
		if (m_EditorStruct && !m_EditorStruct.Deserialize())
			return false;
		
		//if (m_ExampleStruct && !m_ExampleStruct.Deserialize())
		//	return false;
		
		return true;
	}
	override void Log()
	{
		if (m_CampaignStruct)
			m_CampaignStruct.Log();
		
		if (m_EditorStruct)
			m_EditorStruct.Log();
		
		//if (m_ExampleStruct)
		//	m_ExampleStruct.Log();
	}
	void SCR_MissionStruct()
	{
		RegV("m_CampaignStruct");
		RegV("m_EditorStruct");
		//RegV("m_ExampleStruct");
	}
};