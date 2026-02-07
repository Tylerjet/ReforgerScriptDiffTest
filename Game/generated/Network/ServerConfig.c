/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Network
* @{
*/

/**
Dedicated server configuration file.
Holds data used for starting a server with user data, both for replication and server browser.
*/
class ServerConfig: ScriptAndConfig
{
	//! Returns the name of the server.
	proto external owned string GetServerName();
	/*!
	Returns the IP Address of the server.
	Tries to find valid listen address when left empty.
	*/
	proto external owned string GetServerAddress();
	//! Returns the client connect IP Address.
	proto external owned string GetClientConnectAddress();
	/*!
	Returns the port through which clients should connect.
	*/
	proto external int GetClientConnectPort();
	/*!
	Returns the resource name of the mission header this server should run.
	*/
	proto external ResourceName GetServerMissionHeader();
	/*!
	Returns the maximum view distance that should be set by the server.
	0	: unlimited
	>0	: maximum range in meters for all connected clients
	*/
	proto external float GetServerMaxViewDistance();
	/*!
	Returns the maximum network view distance that should be set by the server.
	>0	: maximum range in meters for all connected clients
	*/
	proto external int GetNetworkViewDistance();
	/*!
	Returns the minimum grass distance that should be set by the server.
	0	: unlimited
	>0	: minimum range in meters for all connected clients
	*/
	proto external int GetServerMinGrassDistance();
	/*!
	Returns `true` when client need to have first person only, `false` otherwise.
	*/
	proto external bool GetDisableThirdPerson();
	/*!
	Tries to open provided resource as server config.
	\return Returns server config or null if resource is invalid or not a server config.
	*/
	static proto ref ServerConfig Open(ResourceName resource);
};

/** @}*/
