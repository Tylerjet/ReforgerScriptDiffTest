/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Dialogue
\{
*/

class DialogueNode
{
	private void DialogueNode();
	private void ~DialogueNode();

	proto external DialogueData FindDataOfType(typename type);
	proto external IEntity GetSpeaker();
	//When AutoContinue is enabled and the node completes it's functionality, it will automatically continue to a pre-designated next node.
	proto external bool AutoContinueEnabled();

	// callbacks

	event protected void OnStart(Dialogue dial);
	event protected void OnStartAt(Dialogue dial, float startTime);
	event protected void OnEnd(Dialogue dial);
	event protected void OnInterrupt();
	event protected void OnSkip(Dialogue dial);
}

/*!
\}
*/
