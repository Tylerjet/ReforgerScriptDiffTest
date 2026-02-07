[ComponentEditorProps(category: "GameScripted/Character", description: "Stamina component for character.", icon: HYBRID_COMPONENT_ICON)]
class SCR_CharacterStaminaComponentClass: CharacterStaminaComponentClass
{
};

class SCR_CharacterStaminaComponent : CharacterStaminaComponent
{
	[Attribute( "0", UIWidgets.EditBox, "Threshold value for stamina drain. After the threshold is met, event is triggerd. Must be >0, otherwise the check will be skipped.")]
	protected float 					m_fStaminaDrainThreshold;
	
	[Attribute( "", uiwidget: UIWidgets.Callback, "This event will be called if stamina drain exceeds stamina threshold value." )]
	protected ref Event 				m_eStaminaDrainEvent;
	
	[Attribute( uiwidget: UIWidgets.Object )]
	protected ref StaminaEventParams 	m_EventParams;					//custom event params
	
	protected float 					m_fStaminaDrainAmount;
		
	protected IEntity 					m_Entity;						//reference to parent entity 
	
	//------------------------------------------------------------------------------------------------
	override event void OnStaminaDrain( float pDrain )
	{
		if ( m_fStaminaDrainThreshold > 0 )
		{
			m_fStaminaDrainAmount += pDrain;
			
			//Print( "m_fStaminaDrainAmount = " + m_fStaminaDrainAmount.ToString() + " | pDrain = " + pDrain.ToString() );
			
			if ( -m_fStaminaDrainAmount >= m_fStaminaDrainThreshold )
			{
				m_fStaminaDrainAmount += m_fStaminaDrainThreshold;		//reset
				
				Tuple2<IEntity, StaminaEventParams> params = new Tuple2<IEntity, StaminaEventParams>( m_Entity, m_EventParams );		//prepare event params
				m_eStaminaDrainEvent.Emit( params );																					//call event
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	float GetStaminaDrainThreshold()
	{
		return m_fStaminaDrainThreshold;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetStaminaDrainAmount()
	{
		return m_fStaminaDrainAmount;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_CharacterStaminaComponent( IEntityComponentSource src, IEntity ent, IEntity parent )
	{
		m_Entity = ent;
	}
};

[BaseContainerProps()]
class StaminaEventParams
{
};

[BaseContainerProps()]
class MetabolismEventParams : StaminaEventParams
{
	[Attribute( "0", UIWidgets.EditBox, "Defines how many calories will be added/subracted when triggering OnStaminaDrain event.")]
	float 		m_fCaloriesDrain;

	[Attribute( "0", UIWidgets.EditBox, "Defines how much hydration will be added/subracted when triggering OnStaminaDrain event.")]
	float 		m_fHydrationDrain;	

	[Attribute( "0", UIWidgets.EditBox, "Defines how much energy will be added/subracted when triggering OnStaminaDrain event.")]
	float 		m_fEnergyDrain;			
};