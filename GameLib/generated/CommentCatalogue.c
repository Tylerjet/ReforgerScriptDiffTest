/*
===========================================
Do not modify, this script is generated
===========================================
*/

class CommentCatalogue
{
	//! Returns max possible size of page that is allowed to be set
	proto external int MaxPageSize();
	//! \brief Post comment
	proto external void CreateComment(string sContent, BackendCallback pCallback, int iParentId = 0);
	//! \brief Delete comment online
	proto external void DeleteComment(int iId, BackendCallback pCallback);
	//! \brief Update comment content
	proto external void UpdateComment(int iId, string sContent, BackendCallback pCallback);
	proto external void ReportComment(int iId, EWorkshopReportType eReport, string sDesc, BackendCallback pCallback);
	proto external void RequestPage(BackendCallback pCallback, notnull PageParams pParams, bool bClearCache);
	//! \brief Get total item count on all pages
	proto external int GetTotalItemCount();
	//! \brief Get page count
	proto external int GetPageCount();
	//! \brief Set number of items per page
	[Obsolete("Use SetPageSize() instead!")]
	proto external void SetPageItems(int iCount);
	//! \brief Set number of items per page
	proto external void SetPageSize(int iCount);
	//! \brief Get current page number
	proto external int GetPage();
	//! \brief Get item count on actual page
	proto external int GetPageItemCount();
	//! Get page content and set number of current items
	proto external int GetPageItems();
}
