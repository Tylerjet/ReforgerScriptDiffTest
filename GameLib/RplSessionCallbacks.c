class RplSessionCallbacks
{
	void EOnBegan();
	void EOnEnded();
	void EOnFailed(string msg);
	void EOnConnected(RplIdentity identity);
	void EOnDisconnected(RplIdentity identity);
}
