[BaseContainerProps()]
class SCR_ScenarioFrameworkLogicInput
{
	[Attribute(desc: "Input connector", category: "Input")]
	protected ref SCR_ScenarioFrameworkActionInputBase m_InputAction;
	
	[Attribute(defvalue: "0", desc: "Input connector", category: "Input")]
	protected bool 			m_bLatch;
	
	protected bool			m_bSignal;
	protected IEntity		m_Entity;
	protected SCR_ScenarioFrameworkLogic		m_MasterLogic;
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] logic
	void Init(SCR_ScenarioFrameworkLogic logic)
	{
		m_MasterLogic = logic;
		if (m_InputAction)
			m_InputAction.Init(this);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] bSignal
	//! \param[in] entity
	void OnActivate(bool bSignal, IEntity entity)
	{
		if (m_MasterLogic)
			m_MasterLogic.OnInput(bSignal, entity);
	}
}

class SCR_ScenarioFrameworkLogicClass : GenericEntityClass
{
}

class SCR_ScenarioFrameworkLogic : GenericEntity
{
	
	[Attribute(desc: "What causes the increase", category: "Input")]
	protected ref array<ref SCR_ScenarioFrameworkLogicInput> m_aInputs;
		
	[Attribute(desc: "What to do once counter is reached", category: "OnActivate")]
	protected ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActions;
	
	protected bool												m_bIsTerminated; //Marks if this was terminated - either by death or deletion
	
	//------------------------------------------------------------------------------------------------
	//!
	void Init()
	{
		if (m_bIsTerminated)
			return;
		
		foreach (SCR_ScenarioFrameworkLogicInput input : m_aInputs)
		{
			input.Init(this);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] state
	void SetIsTerminated(bool state)
	{
		m_bIsTerminated = state;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	bool GetIsTerminated()
	{
		return m_bIsTerminated;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] pSignal
	//! \param[in] entity
	void OnInput(bool pSignal = true, IEntity entity = null);
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] entity
	void OnActivate(IEntity entity)
	{
		foreach (SCR_ScenarioFrameworkActionBase action : m_aActions)
			action.OnActivate(entity);
	}
	
#ifdef WORKBENCH	
	//------------------------------------------------------------------------------------------------
	override void _WB_OnCreate(IEntitySource src)
	{
		WorldEditorAPI api = _WB_GetEditorAPI();
		IEntitySource thisSrc = api.EntityToSource(this);
		api.RenameEntity(thisSrc, api.GenerateDefaultEntityName(thisSrc));
	}
#endif
}

class SCR_ScenarioFrameworkLogicCounterClass : SCR_ScenarioFrameworkLogicClass
{
}

class SCR_ScenarioFrameworkLogicCounter : SCR_ScenarioFrameworkLogic
{	
	[Attribute(defvalue: "1", desc: "Threshold", UIWidgets.Graph, category: "Counter")]
	protected int							m_iCountTo;
	
	[Attribute(desc: "What to do once value is increased", category: "OnIncrease")]
	protected ref array<ref SCR_ScenarioFrameworkActionBase>	m_aOnIncreaseActions;
	
	[Attribute(desc: "What to do once value is decreased", category: "OnDecrease")]
	protected ref array<ref SCR_ScenarioFrameworkActionBase>	m_aOnDecreaseActions;
			
	int 							m_iCnt = 0;
	
	//------------------------------------------------------------------------------------------------
	override void OnInput(bool pSignal = true, IEntity entity = null)
	{
		if (entity)
			Increase(entity);
		else
			Increase(this);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	int GetCounterValue()
	{
		return m_iCnt;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] value
	void SetCounterValue(int value)
	{
		m_iCnt = value;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Increases the counter value by 1
	//! \param[in] entity
	void Increase(IEntity entity)
	{
		m_iCnt++;
		if (m_iCnt == m_iCountTo)
		{
			OnActivate(entity);
		}
		
		foreach (SCR_ScenarioFrameworkActionBase increaseAction : m_aOnIncreaseActions)
		{
			increaseAction.OnActivate(entity);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Decreases the counter value by 1
	//! \param[in] entity
	void Decrease(IEntity entity)
	{
		m_iCnt--;
		if (m_iCnt == m_iCountTo)
		{
			OnActivate(entity);
		}
		
		foreach (SCR_ScenarioFrameworkActionBase decreaseAction : m_aOnDecreaseActions)
		{
			decreaseAction.OnActivate(entity);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void Reset()
	{
		m_iCnt = 0;
	}
}

class SCR_ScenarioFrameworkLogicORClass : SCR_ScenarioFrameworkLogicClass
{
}

class SCR_ScenarioFrameworkLogicOR : SCR_ScenarioFrameworkLogic
{
	protected int			m_iActivations = 0;
	
	//------------------------------------------------------------------------------------------------
	override void OnInput(bool pSignal = true, IEntity entity = null)
	{
		if (pSignal)
			m_iActivations = Math.Min(++m_iActivations, m_aInputs.Count());
		else
			m_iActivations--;
	}	
}

class SCR_ScenarioFrameworkLogicSwitchClass : SCR_ScenarioFrameworkLogicClass
{
}

class SCR_ScenarioFrameworkLogicSwitch : SCR_ScenarioFrameworkLogic
{
}
