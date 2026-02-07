/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components
\{
*/

class VehicleBaseSimulationClass: GameComponentClass
{
}

class VehicleBaseSimulation: GameComponent
{
	//! Returns true if this component was properly initialized.
	proto external bool IsValid();

	// callbacks

	/*!
	Called during EOnInit.
	\param owner Entity this component is attached to.
	*/
	event protected void OnInit(IEntity owner);
}

/*!
\}
*/
