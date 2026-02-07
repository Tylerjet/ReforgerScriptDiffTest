[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class CP_SlotDeliveryClass : CP_SlotTaskClass
{
	// prefab properties here
}

//------------------------------------------------------------------------------------------------
/*!
	Class generated via ScriptWizard.
*/
class CP_SlotDelivery : CP_SlotTask
{
	
	[Attribute(defvalue: "", desc: "Name of the task layer associated with this deliver point, Doesn't need to be set if nested under task layer Deliver.", category: "Task")];
	protected ref array<string>						m_aAssociatedTaskLayers;
	
	/*
	[Attribute(defvalue: "", desc: "Name of the entity to be delivered")];
	protected string					m_sIDToDeliver;
	*/	
	
	
	//------------------------------------------------------------------------------------------------
	override void StoreTaskSubjectToParentTaskLayer()
	{
		if (m_aAssociatedTaskLayers.IsEmpty())
		{
			//the Associated layer task attribute is empty, lets check if the parent layer of this delivery slot is Task Deliver
			//if yes, the user didn't need to fill the mentioned attribute since it's nested the layer
			//Not every time is the delivery slot nested under the Layer task Deliver since it can be created later.
			m_pTaskLayer = GetParentTaskLayer();
			if (m_pTaskLayer && CP_LayerTaskDeliver.Cast(m_pTaskLayer))
				CP_LayerTaskDeliver.Cast(m_pTaskLayer).SetDeliveryPointEntity(m_pEntity);
			else
				PrintFormat("CP: ->Task->Delivery point %1 doesn't have associated layer attribute set (and is nested outside of its layer task delivery)", GetOwner().GetName());
		}
		else
		{
			IEntity pEnt;
			foreach (string sLayerName : m_aAssociatedTaskLayers)
			{
				pEnt = GetGame().GetWorld().FindEntityByName(sLayerName);
				if (pEnt)
					m_pTaskLayer = CP_LayerTaskDeliver.Cast(pEnt.FindComponent(CP_LayerTaskDeliver));
						
				if (m_pTaskLayer)
				{
					if (m_pEntity)
						CP_LayerTaskDeliver.Cast(m_pTaskLayer).SetDeliveryPointEntity(m_pEntity);
				}
				else
				{
					PrintFormat("CP: ->Task->Delivery point: Task Layer %1 doesn't exist.", sLayerName);
				}
			}
		}
	}
	
	
	//------------------------------------------------------------------------------------------------
	override void Init(CP_Area pArea = null, CP_EActivationType EActivation = CP_EActivationType.SAME_AS_PARENT, bool bInit = true)
	{
		if (m_EActivationType != EActivation)
			return;
		super.Init(pArea, EActivation);
		SCR_BaseTriggerEntity pTrigger = SCR_BaseTriggerEntity.Cast(m_pEntity);
		if (pTrigger)
		{
 			if (m_pTaskLayer)
			{
				SCR_TaskDeliver pTask = SCR_TaskDeliver.Cast(m_pTaskLayer.GetTask());
				if (pTask)
				{
					pTask.SetDeliveryTrigger(pTrigger);
					pTask.UpdateTaskTitleAndDescription();
				}
				else
				{
					if (m_aAssociatedTaskLayers.IsEmpty())
					{
						PrintFormat("CP: ->Task->Delivery point: Associated Task Layers are empty");
						return;
					}
					
					IEntity pEnt;
					foreach (string sLayerName : m_aAssociatedTaskLayers)
					{
						pEnt = GetGame().GetWorld().FindEntityByName(sLayerName);
						if (pEnt)
							m_pTaskLayer = CP_LayerTaskDeliver.Cast(pEnt.FindComponent(CP_LayerTaskDeliver));
								
						if (m_pTaskLayer)
						{
							pTask = SCR_TaskDeliver.Cast(m_pTaskLayer.GetTask());
							if (pTask)
							{
								pTask.SetDeliveryTrigger(pTrigger);
								pTask.UpdateTaskTitleAndDescription();
							}
						}
						else
						{
							PrintFormat("CP: ->Task->Delivery point: Task Delivery does not exist for Layer %1", sLayerName);
						}
					}
				}
			}
		}
	}
}
