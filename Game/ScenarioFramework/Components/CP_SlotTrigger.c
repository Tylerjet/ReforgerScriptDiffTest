[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class CP_SlotTriggerClass : CP_SlotBaseClass
{
	// prefab properties here
}

//------------------------------------------------------------------------------------------------
class CP_SlotTrigger : CP_SlotBase
{
	/*
	[Attribute(defvalue: "5.0", UIWidgets.Slider, params: "1.0 1000.0 0.5", desc: "Radius of the trigger if selected", category: "Trigger")];
	protected float							m_fAreaRadius;
	
	
	[Attribute("0", UIWidgets.ComboBox, "By whom the trigger is activated", "", ParamEnumArray.FromEnum(TA_EActivationPresence), category: "Trigger") ]
	protected TA_EActivationPresence		m_EActivationPresence;
	
	[Attribute(defvalue: "1", UIWidgets.CheckBox, desc: "Activate the trigger once or everytime the activation condition is true?", category: "Trigger")];
	protected bool 							m_bOnce;
	
	
	*/
	/*
	//TODO: prepared for dynamic invoker selection
	protected ref ScriptInvoker m_OnActivate = new ScriptInvoker();
	protected ref ScriptInvoker m_OnDeactivate = new ScriptInvoker();
	
	protected ref array<string> m_aInvokers = { "OnActivate", "OnDeactivate" };

	
	//------------------------------------------------------------------------------------------------
	array<string> GetInvokerArray() { return m_aInvokers; }
	*/
	
	[Attribute(UIWidgets.Auto, category: "OnActivation")];
	protected ref array<ref CP_ActionBase>	m_aTriggerActions;
	
	//------------------------------------------------------------------------------------------------
	override void Init(CP_Area pArea = null, CP_EActivationType EActivation = CP_EActivationType.SAME_AS_PARENT, bool bInit = true)
	{
		super.Init(pArea, EActivation, bInit);
		foreach(CP_ActionBase pTrigAction : m_aTriggerActions)
			pTrigAction.Init(m_pEntity);
		/*
		if (m_EActivationType != EActivation)
			return;
		super.Init(pArea, EActivation);
		if (!BaseGameTriggerEntity.Cast(m_pEntity))
		{
			Print("CP: SlotTrigger - The selected prefab is not trigger!");
			return;
		}
		BaseGameTriggerEntity.Cast(m_pEntity).SetSphereRadius(m_fAreaRadius);
		SCR_CharacterTriggerEntity pTrig = SCR_CharacterTriggerEntity.Cast(m_pEntity);
		if (pTrig)
		{
			pTrig.SetActivationPresence(m_EActivationPresence);
			pTrig.SetOwnerFaction(m_sFaction);
			ScriptInvoker pInvoker = pTrig.GetOnActivate();
			pInvoker.Insert(OnActivate);
		
			
		}
		*/
	}
	/*
	//------------------------------------------------------------------------------------------------
	void OnActivate()
	{
		if (!m_bOnce)
			return;
		
		SCR_CharacterTriggerEntity pTrig = SCR_CharacterTriggerEntity.Cast(m_pEntity);
		if (pTrig)
			pTrig.Deactivate();
	}
		
	#ifdef WORKBENCH	
	//------------------------------------------------------------------------------------------------
	override void _WB_AfterWorldUpdate(IEntity owner, float timeSlice)
	{
		if (m_rObjectToSpawn)
			super._WB_AfterWorldUpdate(owner, timeSlice);
	}
	#endif	
	
		
	//------------------------------------------------------------------------------------------------
	void CP_SlotTrigger(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		//m_rObjectToSpawn = "{698D02E7065F159B}Prefabs/Triggers/TriggerExtraction.et";
		#ifdef WORKBENCH
			m_fDebugShapeColor = ARGB(32, 0xFF, 0x00, 0x10);
			m_fDebugShapeRadius = m_fAreaRadius;
		#endif
	}
	*/
}

