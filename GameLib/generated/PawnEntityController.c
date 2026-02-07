/*
===========================================
Do not modify, this script is generated
===========================================
*/

class PawnEntityController: ScriptAndConfig
{
	proto external PawnEntity GetPawnOwner();
	proto external void SetFlags(int flags);
	proto external void ClearFlags(int flags);
	proto external bool IsFlagSet(int flag);
	proto external void SetControlRotation(vector ypr);
	proto external vector GetControlRotation();
	proto external void SetAimRotation(vector ypr);
	proto external vector GetAimRotation();
	proto external void SetLookRotation(vector ypr);
	proto external vector GetLookRotation();

	// callbacks

	event protected void OnInit();
	event protected void OnUpdate(float timeSlice);
	event protected int OnPackControls();
	event protected void OnUnpackControls(int packedControls);
}
