local resty_md5 = require "resty.md5"

local function hashUserPassword(password)
    local md5 = resty_md5:new()
    md5:update(password)
    md5:update('@crintsoft.com')
    return ngx.encode_base64(md5:final())
end

local function hash_minilyrics_client_password(password)
    local md5 = resty_md5:new()
    md5:update(password)
    local digest = md5:final()
    digest = string.sub(digest, 5, 12)

    local str = require "resty.string"
    return string.upper(str.to_hex(digest))
end

local sqlite3 = require("lsqlite3")

local db_file = "/mlserver/database/users.db"
local db = sqlite3.open(db_file)

local stmtGetUserById = assert(db:prepare("SELECT * FROM users WHERE id=?"))
local stmtGetUserByEmail = assert(db:prepare("SELECT * FROM users WHERE EmailLower=?"))
local stmtGetUserByNameOrEmail = assert(db:prepare("SELECT id, UserNameLower, EmailLower, PasswordHash FROM users WHERE UserNameLower=? or EmailLower=?"))
local stmtChangeEmail = assert(db:prepare("UPDATE users set Email = ?, EmailLower = ? WHERE id=?"))
local stmtChangePassword = assert(db:prepare("UPDATE users set PasswordHash = ?, MLPasswordHash = ? WHERE id=?"))
local stmtCreateAccount = assert(db:prepare("INSERT INTO users (MLPasswordHash, PasswordHash, Email, EmailLower, CreateDate, LastLoginDate, LastPasswordChangedDate, UserName, UserNameLower) VALUES (?,?,?,?,?,?,?,?,?)"))

local _M = {}

_M.hashUserPassword = hashUserPassword
_M.hash_minilyrics_client_password = hash_minilyrics_client_password

function _M.getUserById(userId)
    stmtGetUserById:reset()
    stmtGetUserById:bind_values(userId)
    for row in stmtGetUserById:nrows() do
        return row
    end

    return nil
end

function _M.getUserByEmail(email)
    stmtGetUserByEmail:reset()
    stmtGetUserByEmail:bind_values(string.lower(email))
    for row in stmtGetUserByEmail:nrows() do
        return row
    end

    return nil
end

function _M.signinUser(user, password)
    local passwordHash = hashUserPassword(password)
    user = string.lower(user)
    stmtGetUserByNameOrEmail:reset()
    stmtGetUserByNameOrEmail:bind_values(user, user)
    for row in stmtGetUserByNameOrEmail:nrows() do
        if row.PasswordHash == passwordHash then
            return row.id
        end
    end

    return nil
end

local function string_trim(s)
    return s:match("^%s*(.-)%s*$")
end

function _M.createAccount(ctx)
    ctx.username = string_trim(ctx.username)
    ctx.email = string_trim(ctx.email)
    if string.len(ctx.username) < 3 then
        ctx.error = 'Username must be at least 3 chars.'
        return false;
    end

    if #ctx.email < 6 or string.find(ctx.email, '@') == nil then
        ctx.error = 'Invalid email address format.'
        return false;
    end

    username = string.lower(ctx.username)
    email = string.lower(ctx.email)

    stmtGetUserByNameOrEmail:reset()
    stmtGetUserByNameOrEmail:bind_values(username, email)
    for row in stmtGetUserByNameOrEmail:nrows() do
        ctx.error = 'User with the same name or email exists already.'
        return false;
    end

    stmtCreateAccount:reset()
    -- MLPasswordHash, PasswordHash, Email, EmailLower, CreateDate, LastLoginDate, LastPasswordChangedDate, UserName, UserNameLower
    stmtCreateAccount:bind_values(
        hash_minilyrics_client_password(ctx.password),
        hashUserPassword(ctx.password),
        ctx.email, email,
        ngx.utctime(),
        ngx.utctime(),
        ngx.utctime(),
        ctx.username, username
    )

    if sqlite3.DONE ~= stmtCreateAccount:step() then
        ctx.error = 'Failed to create account. Please contact us to solve the problem.'
        return false
    end

    ctx.userId = stmtCreateAccount:last_insert_rowid()
    ngx.log(ngx.ERR, 'Last user id:', ctx.userId)

    return true
end

function _M.changeEmail(userId, newEmail)
    stmtChangeEmail:reset()
    stmtChangeEmail:bind_values(newEmail, string.lower(newEmail), userId)
    if sqlite3.DONE == stmtChangeEmail:step() then
        return ''
    end

    ngx.log(ngx.ERR, 'Failed to change email address', userId, ', ', newEmail)

    return 'Failed to change email address.'
end

function _M.changePassword(userId, password)
    local PasswordHash = hashUserPassword(password)
    local MLPasswordHash = hash_minilyrics_client_password(password)

    stmtChangePassword:reset()
    stmtChangePassword:bind_values(PasswordHash, MLPasswordHash, userId)
    if sqlite3.DONE == stmtChangePassword:step() then
        return ''
    end

    ngx.log(ngx.ERR, 'Failed to change password, Database error: ', db:errmsg())

    return 'Failed to change password.'
end

return _M