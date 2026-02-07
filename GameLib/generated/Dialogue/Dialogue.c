/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Dialogue
\{
*/

class Dialogue: ScriptAndConfig
{
	proto external void Start();
	proto external void End();
	proto external void Interrupt();
	proto external void Skip();
	proto external bool IsActive();
	proto external DialogueNode GetCurrentNode();

	// callbacks

	event protected void OnStart();
	event protected void OnStartAt(bool isStartNode, float time);
	event protected void OnEnd();
	event protected void OnInterrupt();
	event protected void OnSkip();
}

/*!
\}
*/
