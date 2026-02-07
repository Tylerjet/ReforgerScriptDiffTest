class SCR_ResourceSystem : GameSystem
{
	static const int DYNAMIC_COMPONENTS_MAX_FRAME_BUDGET		= 10;
	static const int SUBSCRIBED_INTERACTORS_MAX_FRAME_BUDGET	= 10;
	static const int CONTAINERS_MAX_FRAME_BUDGET				= 10;
	
    protected ref array<SCR_ResourceInteractor> m_aSubscribedInteractors	= {};
    protected ref array<SCR_ResourceComponent> m_aDynamicComponents			= {};
    protected ref array<SCR_ResourceContainer> m_aContainers				= {};
	protected SCR_ResourceSystemSubscriptionManager m_ResourceSystemSubscriptionManager;
 	protected ref SCR_ResourceGrid m_ResourceGrid;
	protected ref SCR_ContainerBudgetManager<array<SCR_ResourceInteractor>,	SCR_ResourceInteractor>	m_SubscribedInteractorsBudgetManager;
	protected ref SCR_ContainerBudgetManager<array<SCR_ResourceComponent>,	SCR_ResourceComponent>	m_DynamicComponentsBudgetManager;
	protected ref SCR_ContainerBudgetManager<array<SCR_ResourceContainer>,	SCR_ResourceContainer>	m_ContainersBudgetManager;
 	
	//------------------------------------------------------------------------------------------------
	bool IsRegistered(notnull SCR_ResourceComponent component)
	{
		return m_aDynamicComponents.Contains(component);
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsRegistered(notnull SCR_ResourceContainer container)
	{
		return m_aContainers.Contains(container);
	}
	
	//------------------------------------------------------------------------------------------------
    override protected void OnUpdate(ESystemPoint point)
    {
		float timeSlice = GetWorld().GetFixedTimeSlice();
		
		//------------------------------------------------------------ Process new items in the grid.
		m_ResourceGrid.ProcessFlaggedItems();
		
		//----------------------------------------------------------------- Process graceful handles.
		m_ResourceSystemSubscriptionManager.ProcessGracefulHandles();
		
		//------------------------------------------------------------------- Container self updates.
		SCR_ResourceContainer container;
		
		for (int i = m_aContainers.Count() - 1; i >= 0; --i)
		{
			container = m_aContainers[i];
			
			if (container)
				container.Update(timeSlice);
			else
				/*
				It could not always remove the specific null container, but eventually it should
					clear them out.
				*/
				m_aContainers.Remove(i);
		}
		
		//-------------------------------------------------------- Process dynamic items in the grid.
		bool wasGridUpdateIdIncreased;
		
		foreach (SCR_ResourceComponent component : m_DynamicComponentsBudgetManager.ProcessNextBatch())
		{
			/*
			It could not always remove the specific null component, but eventually it should clear
				them out.
			*/
			if (!component && m_aDynamicComponents.RemoveItem(component))
				continue;
			
			if (vector.DistanceSq(component.GetOwner().GetOrigin(), component.GetLastPosition()) <= SCR_ResourceComponent.UPDATE_DISTANCE_TRESHOLD_SQUARE)
				continue;
			
			if (!wasGridUpdateIdIncreased)
			{
				m_ResourceGrid.IncreaseGridUpdateId();
				
				wasGridUpdateIdIncreased = true;
			}
			
			m_ResourceGrid.UpdateResourceDynamicItem(component);
			component.UpdateLastPosition();
		}
		
		//------------------------------------------------ Process and update subscribed interactors.
		m_ResourceGrid.ResetFrameBudget();
		
		foreach (SCR_ResourceInteractor interactor : m_SubscribedInteractorsBudgetManager.ProcessNextBatch())
		{
			/*
			It could not always remove the specific null interactor, but eventually it should clear
				them out.
			*/
			if (!interactor && m_aSubscribedInteractors.RemoveItem(interactor))
				continue;
			
			m_ResourceGrid.UpdateInteractor(interactor, true);
			
			if (m_ResourceGrid.GetFrameBudget() <= 0)
				break;
		}
		
		//------------------------------------------------------------- Replicate resource listeners.
		m_ResourceSystemSubscriptionManager.ReplicateListeners();
    }
 	
	//------------------------------------------------------------------------------------------------
    override protected void OnDiag(float timeSlice)
    {
        DbgUI.Begin("SCR_ResourceSystem");
        DbgUI.Text("Containers: " + m_aContainers.Count());
        DbgUI.Text("Dynamic components: " + m_aDynamicComponents.Count());
        DbgUI.Text("Subscribed interactors: " + m_aSubscribedInteractors.Count());
 
        if (DbgUI.Button("Dump active components"))
        {
			foreach (SCR_ResourceContainer container : m_aContainers)
            {
                Print(container.GetOwner(), LogLevel.ERROR);
            }
			
            foreach (SCR_ResourceComponent component : m_aDynamicComponents)
            {
                Print(component.GetOwner(), LogLevel.ERROR);
            }
			
			foreach (SCR_ResourceInteractor interactor : m_aSubscribedInteractors)
            {
                Print(interactor.GetOwner(), LogLevel.ERROR);
            }
        }
 
        DbgUI.End();
    }
	
	//------------------------------------------------------------------------------------------------
    void RegisterDynamicComponent(notnull SCR_ResourceComponent component)
    {
		const RplComponent rplComponent = component.GetReplicationComponent();
		
        if (rplComponent.Role() == RplRole.Authority && !m_aDynamicComponents.Contains(component))
            m_aDynamicComponents.Insert(component);
    }
	
	//------------------------------------------------------------------------------------------------
    void RegisterContainer(notnull SCR_ResourceContainer container)
    {
		const RplComponent rplComponent = container.GetComponent().GetReplicationComponent();
		
        if (rplComponent.Role() == RplRole.Authority && !m_aContainers.Contains(container))
            m_aContainers.Insert(container);
    }
	
	//------------------------------------------------------------------------------------------------
    void RegisterSubscribedInteractor(notnull SCR_ResourceInteractor interactor)
    {
        if (!m_aSubscribedInteractors.Contains(interactor))
            m_aSubscribedInteractors.Insert(interactor);
    }
	
	//------------------------------------------------------------------------------------------------
    void UnregisterDynamicComponent(notnull SCR_ResourceComponent component)
    {
        m_aDynamicComponents.RemoveItem(component);
    }
	
	//------------------------------------------------------------------------------------------------
    void UnregisterContainer(notnull SCR_ResourceContainer container)
    {
        m_aContainers.RemoveItem(container);
    }
	
	//------------------------------------------------------------------------------------------------
    void UnregisterSubscribedInteractor(notnull SCR_ResourceInteractor interactor)
    {
        m_aSubscribedInteractors.RemoveItem(interactor);
    }
	
	//------------------------------------------------------------------------------------------------
	override event protected void OnStarted()
	{
		m_ResourceGrid = GetGame().GetResourceGrid();
		m_ResourceSystemSubscriptionManager = GetGame().GetResourceSystemSubscriptionManager();
		
		m_SubscribedInteractorsBudgetManager	= new SCR_ContainerBudgetManager<array<SCR_ResourceInteractor>,	SCR_ResourceInteractor>(m_aSubscribedInteractors,	SUBSCRIBED_INTERACTORS_MAX_FRAME_BUDGET);
		m_DynamicComponentsBudgetManager		= new SCR_ContainerBudgetManager<array<SCR_ResourceComponent>,	SCR_ResourceComponent>(m_aDynamicComponents,		DYNAMIC_COMPONENTS_MAX_FRAME_BUDGET);
		m_ContainersBudgetManager				= new SCR_ContainerBudgetManager<array<SCR_ResourceContainer>,	SCR_ResourceContainer>(m_aContainers,				CONTAINERS_MAX_FRAME_BUDGET);
		
	}
}