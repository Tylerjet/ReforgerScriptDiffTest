class SCR_AIWaitWhileUsingItem : AITaskScripted
{
	protected CharacterControllerComponent m_CharacterController;
	
	protected bool m_bStartedUsingItem = false;
	protected float m_fTimeout_ms;
	
	//------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		IEntity myEntity = owner.GetControlledEntity();
		if (!myEntity)
			return;
		m_CharacterController = CharacterControllerComponent.Cast(myEntity.FindComponent(CharacterControllerComponent));
	}
	
	//------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_CharacterController)
			return ENodeResult.FAIL;
		
		if (!m_bStartedUsingItem)
		{
			// Initially wait until we start using item, it might take a fraction of second
			float worldTime = GetGame().GetWorld().GetWorldTime();
			
			if (m_CharacterController.IsUsingItem())
				m_bStartedUsingItem = true;
			else if (m_fTimeout_ms == 0)
				m_fTimeout_ms = worldTime + 1500.0;
			else if (worldTime > m_fTimeout_ms)
				return ENodeResult.SUCCESS;
			
			return ENodeResult.RUNNING;
		}
		else
		{
			// We have started using item, now wait till we end using it
			if (m_CharacterController.IsUsingItem())
				return ENodeResult.RUNNING;
			else
			{
				m_fTimeout_ms = 0;
				m_bStartedUsingItem = false;
				return ENodeResult.SUCCESS;
			}
		}
		
		return ENodeResult.FAIL;
	}
	
	//------------------------------------------------------------------------
	override void OnAbort(AIAgent owner, Node nodeCausingAbort)
	{
		m_fTimeout_ms = 0;
		m_bStartedUsingItem = false;
	}
	
	//------------------------------------------------------------------------
	override string GetOnHoverDescription()
	{
		return "Returns Running while character is using item  (while bandaging for instance), returns Success when character has finished using item.";
	}
	
	//------------------------------------------------------------------------
	override bool VisibleInPalette() { return true; }
	
	//------------------------------------------------------------------------
	override bool CanReturnRunning() { return true; }
}