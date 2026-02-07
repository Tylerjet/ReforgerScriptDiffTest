/*************************************************************************************************
*	BASE USER ACTION FOR FUEL SYSTEM 
**************************************************************************************************/
class SCR_FuelUserAction : ScriptedUserAction
{
	[Attribute( defvalue: "1", uiwidget: UIWidgets.EditBox, desc: "Tank ID (FuelNode pair)" )]
	private int m_iFuelTankID;	//for pairing with the user action
	protected bool m_bIsProvider = true;
	
	
};
