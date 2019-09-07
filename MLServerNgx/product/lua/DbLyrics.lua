
local sqlite3 = require("lsqlite3")

local ml_lyrics_dir = "/mlserver/lyrics"

local db_file = "/mlserver/database/lyrics.db"
local db = sqlite3.open(db_file)

local SELECT_LYRICS_FIELDS = "SELECT artist, title, album, related_link, edited_by, dl_count, media_length, rate_count, rate_total "

local stmtSearchLyricsByTitle = assert( db:prepare(SELECT_LYRICS_FIELDS .. "FROM lyrics WHERE titlecmp=? LIMIT 50") )
local stmtSearchLyricsByArtist = assert( db:prepare(SELECT_LYRICS_FIELDS .. "FROM lyrics WHERE artistcmp=? LIMIT 50") )
local stmtSearchLyricsByArtistAndTitle = assert( db:prepare(SELECT_LYRICS_FIELDS .. "FROM lyrics WHERE titlecmp=? and artistcmp=? LIMIT 50") )
local stmtSelectLyricsByUserId = assert( db:prepare("SELECT related_link, artist, title, album, dl_count, media_length, rate_count, rate_total, upload_time, id FROM lyrics WHERE uploader_id=? ORDER BY upload_time DESC LIMIT 50 OFFSET ?") )
local stmtCountLyricsByUserId = assert( db:prepare("SELECT COUNT(*) FROM lyrics WHERE uploader_id=?") )
local stmtSelectLyricsById = assert( db:prepare("SELECT * FROM lyrics WHERE id = ?") )
local stmtDeleteLyricsByIdAndUserId = assert( db:prepare("DELETE FROM lyrics WHERE id = ? AND uploader_id=?") )

local _M = {}

function _M.searchByArtistAndTitle(artistcmp, titlecmp)
    stmtSearchLyricsByArtistAndTitle:bind_values(artistcmp, titlecmp)
    return stmtSearchLyricsByArtistAndTitle:nrows()
end

function _M.searchByTitle(titlecmp)
    stmtSearchLyricsByTitle:bind_values(titlecmp)
    return stmtSearchLyricsByTitle:nrows()
end

function _M.searchByArtist(artistcmp)
    stmtSearchLyricsByArtist:bind_values(artistcmp)
    return stmtSearchLyricsByArtist:nrows()
end

function _M.getLyricsByUserId(userId, offset)
    stmtSelectLyricsByUserId:reset()
    stmtSelectLyricsByUserId:bind_values(userId, offset)

    local rows = {}
    local i = 1
    for row in stmtSelectLyricsByUserId:rows() do
        rows[i] = row
        i = i + 1
    end

    return rows
end

function _M.getLyricsByUserIdColumns()
    return stmtSelectLyricsByUserId:get_names()
end

function _M.getLyricsById(id)
    stmtSelectLyricsById:reset()
    stmtSelectLyricsById:bind_values(id)

    local n = stmtSelectLyricsById:step()
    if sqlite3.ROW == n then
        return stmtSelectLyricsById:get_named_values()
    end

    return nil
end

function _M.deleteLyricsByIdAndUserId(id, userId)
    local info = _M.getLyricsById(id)
    if info == nil then
        return false;
    end

    stmtDeleteLyricsByIdAndUserId:reset()
    stmtDeleteLyricsByIdAndUserId:bind_values(id, userId)

    local n = stmtDeleteLyricsByIdAndUserId:step()
    if sqlite3.DONE == n then
        fn = ml_lyrics_dir .. '/' .. info.related_link
        fnDel = fn .. '.del'
        os.rename(fn, fnDel)

        fn = fn .. '.hst'
        fnDel = fn .. '.del'
        os.rename(fn, fnDel)

        return true
    end

    ngx.log(ngx.ERR, 'Database error: ', db:errmsg())

    return false
end

function _M.getCountLyricsByUserId(userId)
    stmtCountLyricsByUserId:reset()
    stmtCountLyricsByUserId:bind_values(userId)

    local n = stmtCountLyricsByUserId:step()
    if sqlite3.ROW == n then
        return stmtCountLyricsByUserId:get_value(0)
    end

    return 0
end

return _M