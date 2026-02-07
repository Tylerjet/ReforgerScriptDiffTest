/*!
@ingroup Editor_Containers_Backend

Class which carries saved data for the editor.
Managed by SCR_DSSessionCallback.
*/
[BaseContainerProps()]
class SCR_EditorStruct : SCR_JsonApiStruct
{
	[Attribute(desc: "Array of attributes evaluated during saving. Can include gloval as well as entity attributes.")]
	protected ref SCR_EditorAttributeList m_AttributeList;
	
	//protected string m_sBuildVersion;
	protected ref array<ref SCR_EditableFactionStruct> m_aFactions = {};
	protected ref array<ref SCR_EditableEntityStruct> m_aEntities = {};
	protected ref array<ref SCR_EditorAttributeStruct> m_aAttributes = {};
	
	/*!
	Print out contents of saved data.
	*/
	override void Log()
	{
		Print("--- SCR_EditorStruct ------------------------");
		SCR_EditableFactionStruct.LogFactions(m_aFactions, m_AttributeList);
		SCR_EditableEntityStruct.LogEntities(m_aEntities, m_AttributeList);
		SCR_EditorAttributeStruct.LogAttributes(m_aAttributes, m_AttributeList);
		Print("---------------------------------------------");
	}
	/*!
	Write world data into the struct.
	*/
	override bool Serialize()
	{
		//m_sBuildVersion = GetGame().GetBuildVersion();
		
		SCR_EditableFactionStruct.SerializeFactions(m_aFactions, m_AttributeList);
		SCR_EditableEntityStruct.SerializeEntities(m_aEntities, m_AttributeList);
		SCR_EditorAttributeStruct.SerializeAttributes(m_aAttributes, m_AttributeList, GetGame().GetGameMode());
		
		Print("SUCCESS: SCR_EditorStruct.SaveEditor()", LogLevel.VERBOSE);
		return true;
	}
	/*!
	Read data from the struct and apply them in the world.
	*/
	override bool Deserialize()
	{
		/*
		if (m_sBuildVersion != GetGame().GetBuildVersion())
		{
			Print(string.Format("Cannot load editor save due to incompatible versions! Saved is %1, but the current is %2", m_sBuildVersion, GetGame().GetBuildVersion()), LogLevel.ERROR);
			return false;
		}
		*/
		
		SCR_EditableFactionStruct.DeserializeFactions(m_aFactions, m_AttributeList);
		SCR_EditableEntityStruct.DeserializeEntities(m_aEntities, m_AttributeList);
		SCR_EditorAttributeStruct.DeserializeAttributes(m_aAttributes, m_AttributeList, GetGame().GetGameMode());
		
		Print("SUCCESS: SCR_EditorStruct.LoadEditor()", LogLevel.VERBOSE);
		return true;
	}
	/*!
	Delete all entities saved in the struct. Used only for development!
	*/
	void ResetEditor()
	{
		SCR_EditableEntityStruct.ClearEntities(m_aEntities);
	}
	void SCR_EditorStruct()
	{
		//RegV("m_sBuildVersion");
		RegV("m_aFactions");
		RegV("m_aEntities");
		RegV("m_aAttributes");
	}
};