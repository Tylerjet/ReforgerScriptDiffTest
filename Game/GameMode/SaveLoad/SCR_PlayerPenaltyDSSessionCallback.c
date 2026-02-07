/*!
Callback for easy handling of player bans.
Controlled from SCR_PlayerPenaltyComponent.
*/
class SCR_PlayerPenaltyDSSessionCallback: DSSessionCallback
{
	SCR_PlayerPenaltyComponent m_PlayerPenaltyComponent;
	
	//------------------------------------------------------------------------------------------------
	override void OnBanResult( int iPlayerId, bool triggered, int currentValue, int triggerValue )
	{
		super.OnBanResult(iPlayerId, triggered, currentValue, triggerValue);
		m_PlayerPenaltyComponent.OnBanResult(iPlayerId, triggered, currentValue, triggerValue);
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_PlayerPenaltyDSSessionCallback(SCR_PlayerPenaltyComponent component)
	{
		m_PlayerPenaltyComponent = component;
	}
};