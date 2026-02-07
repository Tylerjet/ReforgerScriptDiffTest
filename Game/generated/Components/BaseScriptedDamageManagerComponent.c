/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components
\{
*/

class BaseScriptedDamageManagerComponentClass: SCR_DamageManagerComponentClass
{
}

class BaseScriptedDamageManagerComponent: SCR_DamageManagerComponent
{
	/*!
	Called after all components are initialized.
	\param owner Entity this component is attached to.
	*/
	event protected void OnPostInit(IEntity owner);
	/*!
	Called during EOnInit.
	\param owner Entity this component is attached to.
	*/
	event protected void OnInit(IEntity owner);
	/*!
	Called during EOnFrame.
	\param owner Entity this component is attached to.
	\param timeSlice Delta time since last update.
	*/
	event protected void OnFrame(IEntity owner, float timeSlice);
	//! Must be first enabled with event mask
	event protected bool OnContact(IEntity owner, IEntity other, Contact contact);
	/*!
	Called during EOnDiag.
	\param owner Entity this component is attached to.
	\param timeSlice Delta time since last update.
	*/
	event protected void OnDiag(IEntity owner, float timeSlice);
	event protected void OnDamage(EDamageType type, float damage, HitZone pHitZone, IEntity instigator, inout vector hitTransform[3], float speed, int colliderID, int nodeID);
}

/*!
\}
*/
