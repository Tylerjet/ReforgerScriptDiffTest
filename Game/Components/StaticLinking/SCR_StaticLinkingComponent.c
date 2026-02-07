[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "THIS IS THE SCRIPT DESCRIPTION.", color: "0 0 255 255")]
class SCR_StaticLinkingComponentClass: ScriptComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_StaticLinkingComponent : ScriptComponent
{
	//****************//
	//STATIC VARIABLES//
	//****************//
	protected static ref map<int, IEntity> s_mEntries = new ref map<int, IEntity>();
	static bool s_bGameInitialized = false;
	static int s_iHighestID = 0;
	#ifdef WORKBENCH
	static WorldEditorAPI s_API = null;
	#endif
	
	//************************//
	//RUNTIME MEMBER VARIABLES//
	//************************//
	protected IEntityComponentSource m_Source = null;
	protected int m_iTemporaryID = -1;
	protected IEntity m_Owner = null;
	
	//*****************//
	//MEMBER ATTRIBUTES//
	//*****************//
	[Attribute(defvalue: "-1")]
	protected int m_iStaticID;
	
	
	//**************//
	//STATIC METHODS//
	//**************//
	
	//------------------------------------------------------------------------------------------------
	static void Initialize()
	{
		s_bGameInitialized = true;
	}
	
	//------------------------------------------------------------------------------------------------
	static void Deinitialize()
	{
		s_bGameInitialized = false;
	}
	
	//------------------------------------------------------------------------------------------------
	static IEntity FindEntry(int id)
	{
		return s_mEntries.Get(id);
	}
	
	//------------------------------------------------------------------------------------------------
	static int FindID(IEntity entity)
	{
		int id = s_mEntries.GetKeyByValue(entity);
		return id;
	}
	
	//------------------------------------------------------------------------------------------------
	static void AddEntry(int id, IEntity entity)
	{
		if (!entity)
		{
			Print("Entity is null (SCR_StaticLinkingComponent.AddEntry(), returning.", LogLevel.WARNING);
			return;
		}
		
		if (id > s_iHighestID)
			s_iHighestID = id;
		else
		{
			IEntity foundEntity = FindEntry(id);
			if (foundEntity)
			{
				if (foundEntity != entity)
				{
					Print("ID: " + id + " is already used, please change ID of " + entity.GetName(), LogLevel.WARNING);
					return;
				}
				return;
			}
		}
		
		if (s_mEntries)
			s_mEntries.Insert(id, entity);
	}
	
	//------------------------------------------------------------------------------------------------
	static int AddEntry(IEntity entity, int idOffset = -1)
	{
		if (!entity)
		{
			Print("Entity is null (SCR_StaticLinkingComponent.AddEntry()), returning -1.", LogLevel.WARNING);
			return -1;
		}
		
		if (idOffset != -1)
			s_iHighestID++;
		
		int id = s_iHighestID;
		
		if (idOffset != -1)
			id += idOffset;
		
		if (s_mEntries)
			s_mEntries.Insert(id, entity);
		
		return id;
	}
	
	//------------------------------------------------------------------------------------------------
	static void RemoveEntry(int id)
	{
		s_mEntries.Remove(id);
	}
	
	//*********************************//
	//OVERRIDE MEMBER WORKBENCH METHODS//
	//*********************************//
	
	#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	override void _WB_OnCreate(IEntity owner, IEntitySource src)
	{
		
	}
	#endif
	
	//**************//
	//MEMBER METHODS//
	//**************//
	
	//------------------------------------------------------------------------------------------------
	protected void StoreTemporaryID()
	{
		#ifdef WORKBENCH
		if (!s_API || s_API.UndoOrRedoIsRestoring() || !m_Source)
			return;
		
		auto source = s_API.EntityToSource(m_Owner);
		auto containerPath = new array<ref ContainerIdPathEntry>();
		
		if (s_API.IsDoingEditAction())
			s_API.SetVariableValue(m_Source, containerPath, "m_iStaticID", m_iTemporaryID.ToString());
		else
		{
			s_API.BeginEntityAction();
			s_API.SetVariableValue(m_Source, containerPath, "m_iStaticID", m_iTemporaryID.ToString());
			s_API.EndEntityAction();
		}
		return;
		#endif
	}
	
	//------------------------------------------------------------------------------------------------
	protected void RegisterSelf(IEntity entity)
	{
		if (m_iStaticID != -1)
		{
			if (!s_bGameInitialized)
				AddEntry(m_iStaticID, entity);
			else
				m_iTemporaryID = AddEntry(entity, m_iStaticID);
		}
		else
			m_iTemporaryID = AddEntry(entity);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UnregisterSelf()
	{
		RemoveEntry(m_iStaticID);
	}
	
	//***********************//
	//OVERRIDE ON/EON METHODS//
	//***********************//
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		if (m_iTemporaryID != -1)
			StoreTemporaryID();
	}

	//************************//
	//CONSTRUCTOR / DESTRUCTOR//
	//************************//
	
	//------------------------------------------------------------------------------------------------
	void SCR_StaticLinkingComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		#ifdef WORKBENCH
		auto castedEntity = GenericEntity.Cast(ent);
		if (castedEntity)
			s_API = castedEntity._WB_GetEditorAPI();
		#endif
		m_Owner = ent;
		m_Source = src;
		RegisterSelf(m_Owner);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_StaticLinkingComponent()
	{
		UnregisterSelf();
	}

};
