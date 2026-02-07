/*!
@ingroup Editor_Containers_Backend

Saved data for editor attribute.
*/
class SCR_EditorAttributeStruct: JsonApiStruct
{
	protected int m_iId;
	protected int m_iType;
	protected float m_fVar0;
	protected float m_fVar1;
	protected float m_fVar2;
	
	/*!
	Save all attributes for given item.
	\param[out] outEntries Array to be filled with save entries
	\param attributeList List of attributes to be evaluated
	\param item Target whose attributes will be saved
	*/
	static void SerializeAttributes(out notnull array<ref SCR_EditorAttributeStruct> outEntries, SCR_EditorAttributeList attributeList = null, Managed item = null)
	{
		outEntries.Clear();
		
		SCR_BaseEditorAttribute attribute;
		SCR_BaseEditorAttributeVar var;
		for (int i = 0, count = attributeList.GetAttributesCount(); i < count; i++)
		{
			attribute = attributeList.GetAttribute(i);
			if (!attribute.IsServer())
				continue;
			
			var = attribute.ReadVariable(item, null);
			if (!var)
				continue;
			
			SCR_EditorAttributeStruct entry = new SCR_EditorAttributeStruct();
			outEntries.Insert(entry);
			entry.m_iId = i;
			
			entry.m_fVar0 = var.GetVector()[0];
			entry.m_fVar1 = var.GetVector()[1];
			entry.m_fVar2 = var.GetVector()[2];
		}
	}
	/*!
	Load all attributes for given item.
	\param entries List of saved entries
	\param attributeList List of attributes to be evaluated
	\param item Target whose attributes will be loaded
	*/
	static void DeserializeAttributes(notnull array<ref SCR_EditorAttributeStruct> entries, SCR_EditorAttributeList attributeList = null, Managed item = null)
	{
		SCR_BaseEditorAttribute attribute;
		SCR_BaseEditorAttributeVar var;
		
		foreach (SCR_EditorAttributeStruct entry: entries)
		{
			attribute = attributeList.GetAttribute(entry.m_iId);
			var = SCR_BaseEditorAttributeVar.CreateVector(Vector(entry.m_fVar0, entry.m_fVar1, entry.m_fVar2));
			attribute.WriteVariable(item, var, null, -1);
		}
	}
	/*!
	Print out all attributes.
	\param entries List of saved entries
	\param attributeList List of attributes to be evaluated
	\param prefix String added at the beginning of each print-out
	*/
	static void LogAttributes(out notnull array<ref SCR_EditorAttributeStruct> entries, SCR_EditorAttributeList attributeList = null, string prefix = "")
	{
		Print(prefix + "  SCR_EditorAttributeStruct: " + entries.Count());
		foreach (SCR_EditorAttributeStruct entry: entries)
		{
			PrintFormat(prefix + "    %1: %2, %3, %4", attributeList.GetAttribute(entry.m_iId).ClassName(), entry.m_fVar0, entry.m_fVar1, entry.m_fVar2);
		}
	}
	void SCR_EditorAttributeStruct()
	{
		RegV("m_iId");
		RegV("m_fVar0");
		RegV("m_fVar1");
		RegV("m_fVar2");
	}
};