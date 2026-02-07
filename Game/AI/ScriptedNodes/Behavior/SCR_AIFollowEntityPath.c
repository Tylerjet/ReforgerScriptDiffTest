/**
* Task that takes group and uses it path to move
*/

class SCR_AIFollowEntityPath : SCR_AIActionTask
{
	static const string PORT_ENTITY_IN = "GroupIn";

	//------------------------------------------------------------------------------------------------
	ChimeraCharacter m_pCharacter;
	GenericEntity m_pFollowedEntity;
	AIBaseSteeringComponent m_pGroupSteeringComponent;
	AIBaseSteeringComponent m_pMySteeringComponent;
	
	override bool VisibleInPalette()
    {
        return true;
    }
	
	override void OnEnter(AIAgent owner)
	{
		m_pCharacter = ChimeraCharacter.Cast(owner.GetControlledEntity());
		GetVariableIn(PORT_ENTITY_IN, m_pFollowedEntity);
		if (!m_pFollowedEntity)
		{
			m_pFollowedEntity = owner.GetParentGroup();
		}
		
		m_pGroupSteeringComponent = AIBaseSteeringComponent.Cast(m_pFollowedEntity.FindComponent(AIBaseSteeringComponent));
		if (!m_pGroupSteeringComponent)
			return;
		
		if (m_pCharacter.GetCompartmentAccessComponent().IsInCompartment())
			m_pMySteeringComponent = AIBaseSteeringComponent.Cast(CompartmentAccessComponent.GetVehicleIn(m_pCharacter).FindComponent(AIBaseSteeringComponent));
		else 
			m_pMySteeringComponent = AIBaseSteeringComponent.Cast(m_pCharacter.FindComponent(AIBaseSteeringComponent));
		if (!m_pMySteeringComponent)
			return;
		m_pMySteeringComponent.SetPathFromSteeringComponent(m_pGroupSteeringComponent);
	}

	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_pGroupSteeringComponent || !m_pMySteeringComponent)
			return ENodeResult.FAIL;
		
		if (m_pMySteeringComponent.HasCompletedRequest())
			return ENodeResult.SUCCESS;
		return ENodeResult.RUNNING;
	}
	
	protected static ref TStringArray s_aVarsIn = {
		PORT_ENTITY_IN
	};
	override TStringArray GetVariablesIn()
    {
        return s_aVarsIn;
    }
};