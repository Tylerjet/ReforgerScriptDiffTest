/*!
@ingroup Editor_Containers_Backend

Saved data for editable entity.
*/
class SCR_EditableEntityStruct: JsonApiStruct
{
	//--- Serialized (names shortened to save memory)
	protected ResourceName pf; //--- Prefab
	protected bool hy; //--- Was hierarchy changed by user
	protected int pi = -1; //--- Parent ID
	protected int ti = -1; //--- Target ID
	protected int tv = -1; //--- Target value
	protected float px; //--- Pos X
	protected float py; //--- Pos Y
	protected float pz; //--- Pos Y
	protected float qx; //--- Quaternion X
	protected float qy; //--- Quaternion Y
	protected float qz; //--- Quaternion Z
	protected float qw; //--- Quaternion W
	protected float sc; //--- Scale
	protected ref array<ref SCR_EditorAttributeStruct> at = {};
	
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
			SerializeEntity(child, -1, outEntries, attributeList, entriesWithTarget, false);
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
					entry.ti = e;
					break;
				}
			}
			if (entry.ti == -1)
				entry.m_Entity.Log("Error when serializing attach link!", true, LogLevel.WARNING);
		}
	}
	protected static void SerializeEntity(SCR_EditableEntityComponent entity, int parentID, out notnull array<ref SCR_EditableEntityStruct> outEntries, SCR_EditorAttributeList attributeList, out array<int> entriesWithTarget, bool isParentDirty)
	{
		SCR_EditableEntityComponent target;
		int targetValue = -1;
		if (!entity.Serialize(target, targetValue))
			return;
		
		//--- Only placeable entities can have dirty hierarchy, artifically created ones (e.g., custom layer) are exempted
		bool canBeDirty = entity.HasEntityFlag(EEditableEntityFlag.PLACEABLE);
		
		SCR_EditableEntityStruct entry = new SCR_EditableEntityStruct();
		entry.m_Entity = entity;
		entry.pi = parentID;
		parentID = outEntries.Insert(entry);
		
		entry.pf = entity.GetPrefab(true);
		entry.sc = entity.GetOwner().GetScale();
		entry.hy = isParentDirty || entity.HasEntityFlag(EEditableEntityFlag.INDIVIDUAL_CHILDREN) || entity.HasEntityFlag(EEditableEntityFlag.DIRTY_HIERARCHY);
		
		vector transform[4];
		entity.GetOwner().GetWorldTransform(transform);
		entry.px = transform[3][0];
		entry.py = transform[3][1];
		entry.pz = transform[3][2];
		
		float quat[4];
		Math3D.MatrixToQuat(transform, quat);
		entry.qx = quat[0];
		entry.qy = quat[1];
		entry.qz = quat[2];
		entry.qw = quat[3];
		
		if (target)
		{
			entry.m_Target = target;
			entry.tv = targetValue;
			entriesWithTarget.Insert(parentID);
		}
		
		SCR_EditorAttributeStruct.SerializeAttributes(entry.at, attributeList, entity);
		
		//--- Process children if the composition is dirty or artificially created (i.e., non-placeable)
		if (entry.hy || !canBeDirty)
		{
			for (int c = 0, count = entity.GetChildrenCount(true); c < count; c++)
			{
				SerializeEntity(entity.GetChild(c), parentID, outEntries, attributeList, entriesWithTarget, entry.hy && canBeDirty);
			}
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
			if (entry.pi != -1)
			{
				if (!entities.Find(entry.pi, parent))
				{
					Print(string.Format("SCR_EditableEntityStruct: Error when spawning entity @\"%1\", parent with ID %2 not found!", entry.pf, entry.pi), LogLevel.WARNING);
					continue;
				}
			}
			else
			{
				parent = null;
			}
			
			EntitySpawnParams spawnParams = new EntitySpawnParams();
			
			float quat[4];
			quat[0] = entry.qx;
			quat[1] = entry.qy;
			quat[2] = entry.qz;
			quat[3] = entry.qw;
			Math3D.QuatToMatrix(quat, spawnParams.Transform);
			spawnParams.Transform[3] = Vector(entry.px, entry.py, entry.pz);// + Vector(10, 0, 0); //--- DEBUG OFFSET
			spawnParams.TransformMode = ETransformMode.WORLD;
			
			SCR_EditorLinkComponent.IgnoreSpawning(entry.hy);
			SCR_AIGroup.IgnoreSpawning(true);
			
			IEntity rawEntity = GetGame().SpawnEntityPrefab(Resource.Load(entry.pf), GetGame().GetWorld(), spawnParams);
			entry.m_Entity = SCR_EditableEntityComponent.GetEditableEntity(rawEntity);
			if (entry.m_Entity)
			{
				entities.Insert(id, entry.m_Entity);
				
				rawEntity.SetScale(entry.sc);
				entry.m_Entity.EOnEditorSessionLoad(parent);
				entry.m_Entity.SetParentEntity(parent);
				if (entry.hy)
					entry.m_Entity.SetHierarchyAsDirty();
				
				SCR_EditorAttributeStruct.DeserializeAttributes(entry.at, attributeList, entry.m_Entity);
				
				if (entry.ti != -1)
					entriesWithTarget.Insert(id);
				
				Print(string.Format("SCR_EditableEntityStruct: Entity @\"%1\" spawned at %2 as a child of %3", entry.pf, spawnParams.Transform, parent), LogLevel.VERBOSE);
			}
			else
			{
				SCR_EntityHelper.DeleteEntityAndChildren(rawEntity);
				Print(string.Format("SCR_EditableEntityStruct: Error when spawning entity @\"%1\" at %2, SCR_EditableEntityComponent not found!", entry.pf, spawnParams.Transform, parent), LogLevel.WARNING);
			}
		}
		
		//--- Link attached entities together (only after all of them were spawned)
		SCR_EditableEntityStruct entry, targetEntry;
		for (int i = 0, count = entriesWithTarget.Count(); i < count; i++)
		{
			entry = entries[entriesWithTarget[i]];
			targetEntry = entries[entry.ti];
			entry.m_Entity.Deserialize(targetEntry.m_Entity, entry.tv);
		}
		
		SCR_EditorLinkComponent.IgnoreSpawning(false);
		SCR_AIGroup.IgnoreSpawning(false);
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
		string textDefault = "    %1: %2 | file: %3 | pos: %4 | ang: %5 | scl: %6 | drt: %9";
		string textTarget = "    %1: %2 | file: %3 | pos: %4 | ang: %5 | scl: %6 | drt: %9 | tgt: %7 (#%8)";
		float quat[4];
		vector angles;
		Resource resource;
		string resourceName;
		foreach (int id, SCR_EditableEntityStruct entry: entries)
		{
			quat[0] = entry.qx;
			quat[1] = entry.qy;
			quat[2] = entry.qz;
			quat[3] = entry.qw;
			angles = Math3D.QuatToAngles(quat);
			
			resource = Resource.Load(entry.pf);
			resourceName = resource.GetResource().GetResourceName();
			
			if (entry.ti != -1)
				PrintFormat(textTarget, id, entry.pi, FilePath.StripPath(resourceName), Vector(entry.px, entry.py, entry.pz), angles, entry.sc, entry.ti, entry.tv, entry.hy);
			else
				PrintFormat(textDefault, id, entry.pi, FilePath.StripPath(resourceName), Vector(entry.px, entry.py, entry.pz), angles, entry.sc, entry.ti, entry.tv, entry.hy);
			
			SCR_EditorAttributeStruct.LogAttributes(entry.at, attributeList, "    ");
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
		RegV("pf");
		RegV("hy");
		RegV("pi");
		RegV("ti");
		RegV("tv");
		RegV("px");
		RegV("py");
		RegV("pz");
		RegV("qx");
		RegV("qy");
		RegV("qz");
		RegV("qw");
		RegV("sc");
		RegV("at");
	}
};