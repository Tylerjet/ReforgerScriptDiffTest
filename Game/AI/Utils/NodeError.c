//------------------------------------------------------------------------------------------------
//! Error call to be used in scripted BT nodes

ENodeResult NodeError(Node node, AIAgent owner, string msg)
{
	string sOwner = owner.ToString();
	string nodeStack;
	node.GetCallstackStr(nodeStack);
	Debug.Error(sOwner + " : " + nodeStack + "\n" + msg);
	return ENodeResult.FAIL;
};