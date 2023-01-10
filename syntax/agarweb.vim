" Vim syntax file
" Language:     LibAgar - Agar-Web C API
" URL:
" https://github.com/JulNadeauCA/libagar/blob/master/syntax/agarweb.vim
" Maintainer:   Julien Nadeau Carriere <vedge@csoft.net>
" Last Change:  2023 January 09

if !exists("c_no_agar_web") || exists("c_agar_web_typedefs")
  " net/web.h
  syn keyword cType VAR WEB_Query WEB_Session WEB_SessionOps
  syn keyword cType WEB_Argument WEB_Cookie WEB_CommandPreAuth
  syn keyword cType WEB_Module WEB_Command WEB_Filter WEB_DestroyFn
  syn keyword cType WEB_LogFn WEB_LanguageFn WEB_CommandFn
  syn keyword cType WEB_SessionVar WEB_MenuSection WEB_MenuFn
  syn keyword cType WEB_SockLen_t MIME_Entity WEB_Method
  syn keyword cType WEB_FlagDescr WEB_Variable WEB_ModuleClass
  syn keyword cType WEB_MethodOps WEB_RangeReq WEB_SessionSocket
  syn keyword cType WEB_EventFilterFn WEB_ControlCmd
  syn keyword cConstant WEB_METHOD_GET WEB_METHOD_HEAD WEB_METHOD_POST
  syn keyword cConstant WEB_METHOD_OPTIONS WEB_METHOD_PUT WEB_METHOD_DELETE
  syn keyword cConstant WEB_METHOD_TRACE WEB_METHOD_CONNECT WEB_METHOD_LAST
  syn keyword cConstant WEB_GET_ARGUMENT WEB_POST_ARGUMENT WEB_ARGUMENT_LAST
  syn keyword cConstant WEB_FRONTEND_RDBUFSIZE WEB_DATA_BUFSIZE
  syn keyword cConstant WEB_DATA_COMPRESS_MIN WEB_DATA_COMPRESS_LVL
  syn keyword cConstant WEB_FORMDATA_MAX WEB_HTTP_REQ_TIMEOUT
  syn keyword cConstant WEB_WORKER_RESP_TIMEOUT WEB_EVENT_READ_TIMEOUT
  syn keyword cConstant WEB_EVENT_INACT_TIMEOUT WEB_EVENT_MAXRETRY
  syn keyword cConstant WEB_EVENT_PING_IVAL WEB_HTTP_HEADER_MIN WEB_HTTP_HEADER_MAX
  syn keyword cConstant WEB_HTTP_PER_HEADER_MAX WEB_HTTP_MAXHEADERS
  syn keyword cConstant WEB_QUERY_MAX WEB_MAXHTTPSOCKETS WEB_MAXWORKERSOCKETS
  syn keyword cConstant WEB_MAX_ARGS WEB_MAX_COOKIES WEB_ARG_KEY_MAX
  syn keyword cConstant WEB_ARG_TYPE_MAX WEB_ARG_LENGTH_MAX WEB_LANGS_MAX
  syn keyword cConstant WEB_LANG_CODE_MAX WEB_URL_MAX WEB_ERROR_MAX
  syn keyword cConstant WEB_USERAGENT_MAX WEB_EVENT_MAX WEB_RANGE_STRING_MAX
  syn keyword cConstant WEB_RANGE_MAXRANGES WEB_OPNAME_MAX WEB_USERNAME_MAX
  syn keyword cConstant WEB_PASSWORD_MAX WEB_EMAIL_MAX WEB_INT_RANGE_MAX
  syn keyword cConstant WEB_HELP_TOPIC_MAX WEB_SESSID_MAX
  syn keyword cConstant WEB_SESSION_VAR_KEY_MAX WEB_SESSION_VAR_VALUE_MAX
  syn keyword cConstant WEB_SESSION_VARIABLES_MAX WEB_SESSION_DATA_MAX
  syn keyword cConstant WEB_SESSION_DATA_MAGIC WEB_COOKIE_NAME_MAX
  syn keyword cConstant WEB_COOKIE_VALUE_MAX WEB_COOKIE_EXPIRE_MAX
  syn keyword cConstant WEB_COOKIE_DOMAIN_MAX WEB_COOKIE_PATH_MAX
  syn keyword cConstant WEB_COOKIE_SET_MAX WEB_COOKIE_SECURE WEB_COOKIE_HTTPONLY
  syn keyword cConstant WEB_VAR_NAME_MAX WEB_VAR_BUF_INIT
  syn keyword cConstant WEB_VAR_BUF_GROW WEB_GLYPHICON WEB_PATH_HTML
  syn keyword cConstant WEB_PATH_SESSIONS WEB_PATH_SOCKETS WEB_PATH_EVENTS
  syn keyword cConstant WEB_QUERY_CONTENT_READ WEB_QUERY_KEEPALIVE
  syn keyword cConstant WEB_QUERY_DEFLATE WEB_QUERY_NOCOMPRESSION WEB_QUERY_RANGE
  syn keyword cConstant WEB_QUERY_PROXIED WEB_CONTROL_CMD_SYNC WEB_CONTROL_NOOP
  syn keyword cConstant WEB_CONTROL_SHUTDOWN WEB_CONTROL_WORKER_CHLD
  syn keyword cConstant WEB_LOG_EMERG WEB_LOG_ALERT WEB_LOG_CRIT WEB_LOG_ERR
  syn keyword cConstant WEB_LOG_WARNING WEB_LOG_NOTICE WEB_LOG_INFO WEB_LOG_DEBUG
  syn keyword cConstant WEB_LOG_QUERY WEB_LOG_WORKER WEB_LOG_EVENT
  syn keyword cConstant WEB_SESSION_PREFORK_AUTH
endif
