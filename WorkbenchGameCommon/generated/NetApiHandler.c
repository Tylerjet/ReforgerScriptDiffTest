/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
\brief Base class for NetApi handlers.
Inherit to create custom NetApi handler. Inherited class name is than used as NetApi function name.
1) NetApi call is received
2) NetApiHandler descendants with call function name is searched
3) GetRequest() is called
4) Request JsonApiStruct is filled
5) GetResponse() is called with filled request
6) Response JsonApiStruct is returned to caller
*/
class NetApiHandler
{
	//! override to create custom request data
	JsonApiStruct GetRequest();
	//! override to create custom respond data
	JsonApiStruct GetResponse(JsonApiStruct request);
	
};
