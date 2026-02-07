/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Dialogue
\{
*/

class DialogueInteractionComponentClass: GenericComponentClass
{
}

class DialogueInteractionComponent: GenericComponent
{
	proto external bool IsDialogueActive();
	//TODO: Consider if instead of providing only the nodes which satisfy their transitions conditions, the component should provide the transitions of the current node
	proto external int GetAvailableResponseNodes(notnull array<DialogueNode> responseNodes, notnull array<int> responseIndexes);
	//Interaction methods
	proto external bool RequestResponse(int responseId);
	proto external void RequestSkip();
	proto external void RequestInterrupt();

	// callbacks

	//Called after any interaction (Skip, Interrupt or Choice selection) is applied
	event void OnEventApplied();
	//Events
	event void OnDialogueSet(bool isNull);
	event void OnFrame(float timeSlice);
	event protected void OnResponsesSet();
}

/*!
\}
*/
