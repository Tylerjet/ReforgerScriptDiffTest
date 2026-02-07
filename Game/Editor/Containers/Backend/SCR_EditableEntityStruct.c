/*!
@ingroup Editor_Containers_Backend

Saved data for editable entity.
*/
class SCR_EditableEntityStruct: JsonApiStruct
{
	//--- Serialized
	protected ResourceName m_sPrefab;
	protected int m_iParentID = -1;
	protected int m_iTargetID = -1;
	protected int m_iTargetValue = -1;
	protected float m_fPosX;
	protected float m_fPosY;
	protected float m_fPosZ;
	protected float m_fQuatX;
	protected float m_fQuatY;
	protected float m_fQuatZ;
	protected float m_fQuatW;
	protected float m_fScale;
	protected ref array<ref SCR_EditorAttributeStruct> m_aAttributes = {};
	
	//--- Non-serialized
	protected SCR_EditableEntityComponent m_Entity;
	protected SCR_EditableEntityComponent m_Target;
	
	/*!
	Save all editable entities.
	\param[out] outEntries Array to be filled with save entries
	\param attributeList List of attributes which will be evaluated for each entity
	*/
	static void SerializeEntities(out notnull array<ref SCR_EditableEntityStruct> outEntries, SCR_EditorAttributeList attributeList)
	{
		//--- Clear existing array
		outEntries.Clear();
		
		//--- Get root entities
		set<SCR_EditableEntityComponent> children;
		SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		if (core)
		{
			children = new set<SCR_EditableEntityComponent>();
			core.GetAllEntities(children, true);
		}
		
		//--- Process root entities
		array<int> entriesWithTarget = {};
		foreach (SCR_EditableEntityComponent child: children)
		{
			SerializeEntity(child, -1, outEntries, attributeList, entriesWithTarget);
		}
		
		//--- Link attached entities together (only after all of them were registered)
		int entriesCount = outEntries.Count();
		SCR_EditableEntityStruct entry;
		for (int i = 0, count = entriesWithTarget.Count(); i < count; i++)
		{
			entry = outEntries[entriesWithTarget[i]];
			for (int e = 0; e < entriesCount; e++)
			{
				if (entry.m_Target == outEntries[e].m_Entity)
				{
					entry.m_iTargetID = e;
					break;
				}
			}
			if (entry.m_iTargetID == -1)
				entry.m_Entity.Log("Error when serializing attach link!", true, LogLevel.WARNING);
		}
	}
	protected static void SerializeEntity(SCR_EditableEntityComponent entity, int parentID, out notnull array<ref SCR_EditableEntityStruct> outEntries, SCR_EditorAttributeList attributeList, out array<int> entriesWithTarget)
	{
		// Skip serialization for destroyed entities (i.e. dead AI characters would deserialize alive and without group [ARMA4-30815])
		if (entity.IsDestroyed())
			return;
		
		SCR_EditableEntityComponent target;
		int targetValue = -1;
		if (!entity.Serialize(target, targetValue))
			return;
		
		SCR_EditableEntityStruct entry = new SCR_EditableEntityStruct();
		entry.m_Entity = entity;
		entry.m_iParentID = parentID;
		parentID = outEntries.Insert(entry);
		
		entry.m_sPrefab = entity.GetPrefab();
		entry.m_fScale = entity.GetOwner().GetScale();
		
		vector transform[4];
		entity.GetOwner().GetWorldTransform(transform);
		entry.m_fPosX = transform[3][0];
		entry.m_fPosY = transform[3][1];
		entry.m_fPosZ = transform[3][2];
		
		float quat[4];
		Math3D.MatrixToQuat(transform, quat);
		entry.m_fQuatX = quat[0];
		entry.m_fQuatY = quat[1];
		entry.m_fQuatZ = quat[2];
		entry.m_fQuatW = quat[3];
		
		if (target)
		{
			entry.m_Target = target;
			entry.m_iTargetValue = targetValue;
			entriesWithTarget.Insert(parentID);
		}
		
		SCR_EditorAttributeStruct.SerializeAttributes(entry.m_aAttributes, attributeList, entity);
		
		for (int c = 0, count = entity.GetChildrenCount(true); c < count; c++)
		{
			SerializeEntity(entity.GetChild(c), parentID, outEntries, attributeList, entriesWithTarget);
		}
	}
	/*!
	Load all editable entities.
	\param entries Entries to be converted into editable entities.
	\param attributeList List of attributes which will be evaluated for each entity
	*/
	static void DeserializeEntities(notnull array<ref SCR_EditableEntityStruct> entries, SCR_EditorAttributeList attributeList = null)
	{
		SCR_EditableEntityComponent parent;
		map<int, SCR_EditableEntityComponent> entities = new map<int, SCR_EditableEntityComponent>();
		array<int> entriesWithTarget = {};
		foreach (int id, SCR_EditableEntityStruct entry: entries)
		{
			if (entry.m_iParentID != -1)
			{
				if (!entities.Find(entry.m_iParentID, parent))
				{
					Print(string.Format("SCR_EditableEntityStruct: Error when spawning entity @\"%1\", parent with ID %2 not found!", entry.m_sPrefab, entry.m_iParentID), LogLevel.WARNING);
					continue;
				}
			}
			else
			{
				parent = null;
			}
			
			EntitySpawnParams spawnParams = new EntitySpawnParams();
			
			float quat[4];
			quat[0] = entry.m_fQuatX;
			quat[1] = entry.m_fQuatY;
			quat[2] = entry.m_fQuatZ;
			quat[3] = entry.m_fQuatW;
			Math3D.QuatToMatrix(quat, spawnParams.Transform);
			spawnParams.Transform[3] = Vector(entry.m_fPosX, entry.m_fPosY, entry.m_fPosZ);// + Vector(10, 0, 0); //--- DEBUG OFFSET
			spawnParams.TransformMode = ETransformMode.WORLD;
			
			SCR_EditorLinkComponent.IgnoreSpawning(true);
			SCR_AIGroup.IgnoreSpawning(true);
			
			IEntity rawEntity = GetGame().SpawnEntityPrefab(Resource.Load(entry.m_sPrefab), GetGame().GetWorld(), spawnParams);
			entry.m_Entity = SCR_EditableEntityComponent.GetEditableEntity(rawEntity);
			if (entry.m_Entity)
			{
				entities.Insert(id, entry.m_Entity);
				
				rawEntity.SetScale(entry.m_fScale);
				entry.m_Entity.EOnEditorSessionLoad(parent);
				entry.m_Entity.SetParentEntity(parent);
				
				SCR_EditorAttributeStruct.DeserializeAttributes(entry.m_aAttributes, attributeList, entry.m_Entity);
				
				if (entry.m_iTargetID != -1)
					entriesWithTarget.Insert(id);
				
				Print(string.Format("SCR_EditableEntityStruct: Entity @\"%1\" spawned at %2 as a child of %3", entry.m_sPrefab, spawnParams.Transform, parent), LogLevel.VERBOSE);
			}
			else
			{
				SCR_Global.DeleteEntityAndChildren(rawEntity);
				Print(string.Format("SCR_EditableEntityStruct: Error when spawning entity @\"%1\" at %2, SCR_EditableEntityComponent not found!", entry.m_sPrefab, spawnParams.Transform, parent), LogLevel.WARNING);
			}
		}
		
		//--- Link attached entities together (only after all of them were spawned)
		SCR_EditableEntityStruct entry, targetEntry;
		for (int i = 0, count = entriesWithTarget.Count(); i < count; i++)
		{
			entry = entries[entriesWithTarget[i]];
			targetEntry = entries[entry.m_iTargetID];
			entry.m_Entity.Deserialize(targetEntry.m_Entity, entry.m_iTargetValue);
		}
	}
	/*!
	Delete all saved editable entities.
	\param entries All entities saved in these entries will be deleted
	*/
	static void ClearEntities(notnull array<ref SCR_EditableEntityStruct> entries)
	{
		for (int i = 0, count = entries.Count(); i < count; i++)
		{
			if (entries[i].m_Entity)
				entries[i].m_Entity.Delete();
		}
	}
	/*!
	Print out all saved editable entities.
	\param entries Entries to be logged
	\param attributeList List of attributes
	*/
	static void LogEntities(notnull array<ref SCR_EditableEntityStruct> entries, SCR_EditorAttributeList attributeList = null)
	{
		Print("  SCR_EditableEntityStruct: " + entries.Count());
		string textDefault = "    %1: %2 | file: %3 | pos: %4 | ang: %5 | scl: %6";
		string textTarget = "    %1: %2 | file: %3 | pos: %4 | ang: %5 | scl: %6 | tgt: %7 (#%8)";
		float quat[4];
		vector angles;
		foreach (int id, SCR_EditableEntityStruct entry: entries)
		{
			quat[0] = entry.m_fQuatX;
			quat[1] = entry.m_fQuatY;
			quat[2] = entry.m_fQuatZ;
			quat[3] = entry.m_fQuatW;
			angles = Math3D.QuatToAngles(quat);
			
			if (entry.m_iTargetID != -1)
				PrintFormat(textTarget, id, entry.m_iParentID, FilePath.StripPath(entry.m_sPrefab), Vector(entry.m_fPosX, entry.m_fPosY, entry.m_fPosZ), angles, entry.m_fScale, entry.m_iTargetID, entry.m_iTargetValue);
			else
				PrintFormat(textDefault, id, entry.m_iParentID, FilePath.StripPath(entry.m_sPrefab), Vector(entry.m_fPosX, entry.m_fPosY, entry.m_fPosZ), angles, entry.m_fScale, entry.m_iTargetID, entry.m_iTargetValue);
			
			SCR_EditorAttributeStruct.LogAttributes(entry.m_aAttributes, attributeList, "    ");
		}
	}
	/*
	override void OnExpand()
	{
		Print("OnExpand()");
	}
	override void OnPack()
	{
		Print("OnPack()");
	}
	override void OnBufferReady()
	{
		Print("OnBufferReady()");
		Print(AsString());
	}
	override void OnSuccess(int errorCode)
	{
		Print("OnSuccess() = " + errorCode);
	}
	override void OnError(int errorCode)
	{
		Print("OnError() = " + errorCode);
	}
	*/
	void SCR_EditableEntityStruct()
	{
		RegV("m_sPrefab");
		RegV("m_iParentID");
		RegV("m_iTargetID");
		RegV("m_iTargetValue");
		RegV("m_fPosX");
		RegV("m_fPosY");
		RegV("m_fPosZ");
		RegV("m_fQuatX");
		RegV("m_fQuatY");
		RegV("m_fQuatZ");
		RegV("m_fQuatW");
		RegV("m_fScale");
		RegV("m_aAttributes");
	}
};