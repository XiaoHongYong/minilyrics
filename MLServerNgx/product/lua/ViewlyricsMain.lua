local template = require "resty.template"
local viewUser = require "ViewUser"
local viewLyrics = require "ViewLyrics"
local session = require "Session"

local function home(ctx, userId)
    template.render("home.html", ctx)
end

local uriHandlers = {
    ['/'] = home,
}

local function starts_with(str, start)
    return str:sub(1, #start) == start
end

local function handleAll()
    local userId = nil
    local sessionInfo = ngx.var.cookie_session
    if sessionInfo ~= nil then
        userId = session.decryptSessionInfo(sessionInfo)
    end

    local handler = uriHandlers[ngx.var.uri]
    if handler then
        local ctx = {
            error = '',
        }
        if (userId ~= nil) then
            ctx.isSignedIn = 'true'
        else
            ctx.isSignedIn = 'false'
        end

        if starts_with(ngx.var.uri, '/user/') and ngx.var.uri ~= '/user/signin' and
            ngx.var.uri ~= '/user/signup' and ngx.var.uri ~= '/user/password_reset' and userId == nil then
            ngx.redirect('/user/signin?redirect=' .. ngx.var.uri)
        end

        handler(ctx, userId)
    else
        ngx.say('404 not found: ' .. ngx.var.uri)
    end
end

local function mergeUriHandlers(handlers, additionHandlers)
    for k, v in pairs(additionHandlers) do
        handlers[k] = v
    end
end

mergeUriHandlers(uriHandlers, viewUser)
mergeUriHandlers(uriHandlers, viewLyrics)
handleAll()
