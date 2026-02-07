//-----------------------------------------------------------------------------
class ClientSessionParams : RplSessionClientParams
{
	string EndpointAddr;

	override string GetAddress() { return EndpointAddr; }

	void ClientSessionParams(string addr) { EndpointAddr = addr; }
};