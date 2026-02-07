/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online
\{
*/

//! Ban Service API
class BanServiceApi
{
	/*!
	\brief Create ban for player in current session on the server
	\note For detailed explanation please consult documentation (wiki)
	\param pCallback Is script callback where you will receive result/error when request finishes
	\param iPlayerId Is Player Id of player in current session on the server
	\param sReason Is Reason of ban
	\param iBanDuration Is Ban Duration value - how long will ban exist in seconds (0 for permanent)
	*/
	proto external bool CreateBanPlayerId(BackendCallback pCallback, int iPlayerId, string sReason, int iBanDuration);
	/*!
	\brief Create ban for player based on his Identity Id. Can be created for players not connected in session
	\note For detailed explanation please consult documentation (wiki)
	\param pCallback Is script callback where you will receive result/error when request finishes
	\param sPlayerIdentityId Is Game Identity Id of player
	\param sReason Is Reason of ban
	\param iBanDuration Is Ban Duration value - how long will ban exist in seconds (0 for permanent)
	*/
	proto external bool CreateBanIdentityId(BackendCallback pCallback, string sPlayerIdentityId, string sReason, int iBanDuration);
	/*!
	\brief Remove bans for list of player by their Identity Ids
	\note For detailed explanation please consult documentation (wiki)
	\param pCallback Is script callback where you will receive result/error when request finishes
	\param identityIds Is list of identityIds of players to remove ban - unban
	*/
	proto external bool RemoveBans(BackendCallback pCallback, notnull array<string> identityIds);
	/*!
	\brief Request List of active bans for players banned on local server
	\note For detailed explanation please consult documentation (wiki)
	\param pCallback Is script callback where you will receive result/error when request finishes
	\param pParams Is PageParams object used to specify size and offset for pages of ban list
	*/
	proto external bool RequestServerBanList(BackendCallback pCallback, notnull PageParams params);
	//! \brief Get total item count on all pages
	proto external int GetTotalItemCount();
	//! \brief Get page count
	proto external int GetPageCount();
	//! \brief Set number of items per page
	proto external void SetPageItems(int iCount);
	//! \brief Get current page number
	proto external int GetPage();
	//! \brief Get item count on current page
	proto external int GetPageItemCount();
	//! \brief Get list of items on current page
	proto external int GetPageItems(notnull array<BanRecordData> bans);
}

/*!
\}
*/
