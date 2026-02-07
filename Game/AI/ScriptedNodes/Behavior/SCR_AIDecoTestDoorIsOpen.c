 class SCR_AIDecoTestDoorIsOpen : DecoratorTestScripted
{
	private DoorComponent m_doorComponent;
	
	//------------------------------------------------------------------------------------------------
	protected override bool TestFunction(AIAgent agent, IEntity controlled)
	{	
		if (controlled)
		{	
			if (!m_doorComponent)
				m_doorComponent = DoorComponent.Cast(controlled.FindComponent(DoorComponent));
			if (!m_doorComponent)
				return false;
			return m_doorComponent.IsOpen();
		}
		return false;
	}
};

class SCR_AIDecoTestDoorIsOpenWithSiblings : DecoratorTestScripted
{
	private ref array<DoorComponent> m_aDoorComponents;
	
	//------------------------------------------------------------------------------------------------
	protected override bool TestFunction(AIAgent agent, IEntity controlled)
	{	
		if (controlled)
		{	
			if (!m_aDoorComponents)
			{
				m_aDoorComponents = {};
				FindDoorComponents(controlled, m_aDoorComponents);
			}
			
			if (m_aDoorComponents.IsEmpty())
				return false;
			
			foreach (DoorComponent dc : m_aDoorComponents)
			{
				if (!dc)
					return false;
				
				bool isOpen = dc.IsOpen() && !dc.IsOpening();
				
				if (!isOpen)
					return false;
			}
			
			return true;
		}
		return false;
	}
	
	static void FindDoorComponents(notnull IEntity doorEntity, notnull array<DoorComponent> outDoorComponents)
	{
		outDoorComponents.Clear();
		
		IEntity doorParent = doorEntity.GetParent();
		if (!doorParent)
		{
			// Door is in world root, or we don't care for siblings
			DoorComponent doorComponent = DoorComponent.Cast(doorEntity.FindComponent(DoorComponent));
			if (doorComponent)
				outDoorComponents.Insert(doorComponent);
			return;
		}
		
		IEntity doorSibling = doorParent.GetChildren();
		
		// Door is in some hierarchy, check all children of parent of the door
		while (doorSibling)
		{
			DoorComponent doorComponent = DoorComponent.Cast(doorSibling.FindComponent(DoorComponent));
			if (doorComponent)
				outDoorComponents.Insert(doorComponent);
			
			doorSibling = doorSibling.GetSibling();
		}
	}
};