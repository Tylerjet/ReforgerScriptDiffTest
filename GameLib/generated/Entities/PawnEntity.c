/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Entities
\{
*/

class PawnEntityClass: GenericEntityClass
{
}

class PawnEntity: GenericEntity
{
	proto external void SetSimulationTickStep(float tickStep);
	proto external void Possess(RplIdentity identity);
	proto external void Dispossess();
	proto external bool IsPossessed();
	proto external RplIdentity GetOwnerRplIdentity();
	proto external PawnEntityController GetController();
	proto external PawnMovementComponent GetPawnMovementComponent();
	proto external bool IsLocal();
	proto external bool IsAuthority();
	proto external bool IsControlledAuthority();
	proto external bool IsControlledProxy();
	proto external bool IsControlledMaster();
	proto external bool IsSimulatedProxy();
	proto external void AddRelatedEntity(PawnRelatedEntity entity);
	proto external void RemoveRelatedEntity(PawnRelatedEntity entity);
	proto external void SetSimulationDisabled(bool state);

	// callbacks

	event protected void OnPossession(RplIdentity rplIdentity);
	event protected void OnDispossession();
	event protected void SimPhaseMainLogic(float timeSlice);
	event protected void SimPhasePreAnim(float timeSlice);
	event protected void SimPhasePostAnim(float timeSlice);
}

/*!
\}
*/
