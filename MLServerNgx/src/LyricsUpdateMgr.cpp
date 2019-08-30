#include "StdAfx.h"
#include "LyricsDB.h"
#include "LyricsUpdateMgr.h"

/*

DROP TABLE LyricsUpdate;
DROP PROC LYRICS_UPDATE_INSERT;
DROP PROC LYRICS_UPDATE_QUERY_BY_DATE;
DROP PROC LYRICS_UPDATE_QUERY_BY_ID;
DROP PROC LYRICS_UPDATE_SUBMIT_REVIEW;

GO

CREATE TABLE LyricsUpdate
(
  id integer Primary Key Identity,
  lyr_id integer,
  review_status integer,
  update_date integer,
  uploader varchar(48)
);

GO

CREATE INDEX LyricsUpdate_date on LyricsUpdate (update_date);

GO

CREATE PROC LYRICS_UPDATE_INSERT
	@id integer Out,
	@lyr_id integer,
	@review_status integer,
	@update_date integer,
	@uploader varchar(48)
AS
	INSERT INTO LyricsUpdate (
		lyr_id, review_status, update_date, uploader
	) VALUES (
		@lyr_id, @review_status, @update_date, @uploader
	);
	set @id=SCOPE_IDENTITY();
GO

CREATE PROC LYRICS_UPDATE_QUERY_BY_DATE
	@review_status_filter integer,
	@update_date integer
AS
	SELECT TOP 100 * FROM LyricsUpdate
	WHERE update_date > @update_date and review_status = @review_status_filter
	ORDER BY id
GO


CREATE PROC LYRICS_UPDATE_QUERY_BY_ID
	@review_status_filter integer,
	@idStart integer
AS
	SELECT TOP 100 * FROM LyricsUpdate
	WHERE id > @idStart and review_status = @review_status_filter
	ORDER BY id
GO

CREATE PROC LYRICS_UPDATE_SUBMIT_REVIEW
	@lyr_id integer,
	@id_start integer,
	@id_accepted integer,
	@id_rejected integer
AS

	-- 16=RS_REVIEWED
	update LyricsUpdate
	set review_status = 16
	where id >= @id_start and lyr_id = @lyr_id and id <> @id_accepted and id <> @id_rejected

	-- 8= RS_REJECTED
	update LyricsUpdate
	set review_status = 8
	where id = @id_rejected

	-- 4= RS_ACCPTED
	update LyricsUpdate
	set review_status = 4
	where id = @id_accepted

GO


DROP TABLE LyricsDeleted;
DROP PROC LYRICS_DELETED_INSERT;

GO

CREATE TABLE LyricsDeleted
(
	id integer Primary Key Identity,
	related_url varchar(48),
	review_status integer,
	deleted_date integer,
	deleted_by varchar(48)
);

GO

CREATE INDEX LyricsDeleted_date on LyricsDeleted (deleted_date);

GO

CREATE PROC LYRICS_DELETED_INSERT
	@id integer Out,
	@related_url varchar(48),
	@review_status integer,
	@deleted_date integer,
	@deleted_by varchar(48)
AS
	INSERT INTO LyricsDeleted (
		related_url, review_status, deleted_date, deleted_by
	) VALUES (
		@related_url, @review_status, @deleted_date, @deleted_by
	);
	set @id=SCOPE_IDENTITY();
GO

*/


CLyricsUpdateMgr		g_lyrUpdateMgr;

CLyricsUpdateMgr::CLyricsUpdateMgr(void)
{
}

CLyricsUpdateMgr::~CLyricsUpdateMgr(void)
{
}

int CLyricsUpdateMgr::AddUpdate(long nLyricsId, LPCTSTR szLyrFile, LyrUpdateItem &lyrUpdateItem, CCharBuffer &bufLyrData, long nDerivedFromId)
{
	CLyrUpdateHistory	histFile;

	int nRet = histFile.Open(szLyrFile);
	if (nRet != ERR_OK)
		return nRet;

	if (lyrUpdateItem.reviewStatus == RS_OPTIONAL
		|| lyrUpdateItem.reviewStatus == RS_REQUIRED)
	{
	}
	else
		lyrUpdateItem.nUpdateId = LYR_UPDATE_ID_ORG;

	return histFile.Add(lyrUpdateItem, bufLyrData, nDerivedFromId);
}
