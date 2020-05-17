/*	Public domain	*/

#ifndef _AGAR_NET_WEB_H_
#define _AGAR_NET_WEB_H_

#include <agar/config/ag_web.h>
#ifdef AG_WEB

#include <agar/net/begin.h>

#include <sys/socket.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#define WEB_COMPAT_APACHE		/* Behind Apache 2.4 mod_proxy */
/* #define WEB_COMPAT_NGINX */		/* Behind nginx */
/* #define WEB_CHUNKED_EVENTS */	/* Chunked event streams */
#define HAVE_SETPROCTITLE

#define WEB_FRONTEND_RDBUFSIZE	16384	/* Frontend I/O buffer (must fit header) */
#define WEB_DATA_BUFSIZE	65536	/* Data buffer size */
#define WEB_DATA_COMPRESS_MIN	8192	/* Compression threshold */
#define WEB_DATA_COMPRESS_LVL	6	/* Default compression level */

#define WEB_FORMDATA_MAX (8*1024*1024)	/* Accepted multipart/form-data size */

#define WEB_HTTP_REQ_TIMEOUT	 30	/* HTTP request (and keepalive) timeout) */
#define WEB_WORKER_RESP_TIMEOUT  15	/* Worker response timeout */
#define WEB_EVENT_READ_TIMEOUT	 10	/* Event source read timeout */
#define WEB_EVENT_INACT_TIMEOUT	 3600	/* Event source inactivity timeout */
#define WEB_EVENT_MAXRETRY	 10	/* Max Redirect/Retry attempts */
#define WEB_EVENT_PING_IVAL	 15	/* Event source ping interval */

#define WEB_HTTP_HEADER_MIN	14	/* Min HTTP header size */
#define WEB_HTTP_PER_HEADER_MAX	256	/* Max HTTP header size (per header) */
#define WEB_HTTP_HEADER_MAX	1024	/* HTTP header size (total) */
#define WEB_HTTP_MAXHEADERS	32	/* Max HTTP response headers */
#define WEB_QUERY_MAX		4096	/* Max serialized WEB_Query size */

#define WEB_MAXHTTPSOCKETS	5	/* Max listening sockets */
#define WEB_MAXWORKERSOCKETS	30	/* Max Worker->Frontend sockets */

#define WEB_MAX_ARGS		256	/* URL-encoded argument count */
#define WEB_MAX_COOKIES		32	/* Number of cookies */

#define WEB_ARG_KEY_MAX		60	   /* Argument key */
#if AG_MODEL == AG_MEDIUM
#define WEB_ARG_TYPE_MAX	28
#else
#define WEB_ARG_TYPE_MAX	32
#endif
#define WEB_ARG_LENGTH_MAX	100000000  /* Argument data (100MB) */

#define WEB_LANGS_MAX		64	/* Accept-Language entries */
#define WEB_LANG_CODE_MAX	6	/* Language code */
#define WEB_URL_MAX		768	/* URL length */
#define WEB_ERROR_MAX		1024	/* Error message length */
#define WEB_USERAGENT_MAX	512	/* HTTP User-Agent length */
#define WEB_EVENT_MAX		4096	/* size of event packets */
#define WEB_RANGE_STRING_MAX	32	/* Byte range specifier */
#define WEB_RANGE_MAXRANGES	8	/* Maximum ranges in a Range request */

#define WEB_OPNAME_MAX		32	/* Operation name length */
#define WEB_USERNAME_MAX	64	/* Username length */
#define WEB_PASSWORD_MAX	128	/* Password length */
#define WEB_EMAIL_MAX		128	/* E-mail address length */
#define WEB_INT_RANGE_MAX	21	/* Maximum integer range length */
#define WEB_HELP_TOPIC_MAX	80

#define WEB_SESSID_MAX		  11	/* Session ID length */
#define WEB_SESSION_VAR_KEY_MAX	  32	/* Session variable key */
#define WEB_SESSION_VAR_VALUE_MAX 1024	/* Session variable value */
#define WEB_SESSION_VARIABLES_MAX 100	/* Session variables total */
#define WEB_SESSION_DATA_MAX	  4096	/* Session data total (bytes) */
#define WEB_SESSION_DATA_MAGIC	  0x50657243

#define WEB_COOKIE_NAME_MAX	48	/* Cookie name */
#define WEB_COOKIE_VALUE_MAX	3807	/* Cookie value */
#define WEB_COOKIE_EXPIRE_MAX	64	/* Cookie expiration field */
#define WEB_COOKIE_DOMAIN_MAX	48	/* Cookie domain field */
#define WEB_COOKIE_PATH_MAX	101	/* Cookie path argument */
#define WEB_COOKIE_SET_MAX	128	/* Max generated Set-Cookie length */

#define WEB_VAR_NAME_MAX	32	/* Web variable name */
#define WEB_VAR_BUF_INIT	128	/* Variable buffer size */
#define WEB_VAR_BUF_GROW	1024

#ifndef WEB_GLYPHICON
#define WEB_GLYPHICON(x) "<span class='glyphicons glyphicons-" #x "'></span>"
#endif

#ifndef WEB_PATH_HTML
#define WEB_PATH_HTML "html"
#endif
#ifndef WEB_PATH_SESSIONS
#define WEB_PATH_SESSIONS "sessions/"
#endif
#ifndef WEB_PATH_SOCKETS
#define WEB_PATH_SOCKETS "sockets/"
#endif
#ifndef WEB_PATH_EVENTS
#define WEB_PATH_EVENTS "events/"
#endif

typedef enum web_method {
	WEB_METHOD_GET,
	WEB_METHOD_HEAD,
	WEB_METHOD_POST,
	WEB_METHOD_OPTIONS,
	WEB_METHOD_PUT,
	WEB_METHOD_DELETE,
	WEB_METHOD_TRACE,
	WEB_METHOD_CONNECT,
	WEB_METHOD_LAST
} WEB_Method;

/* Web variable */
typedef struct web_variable {
	char key[WEB_VAR_NAME_MAX];		/* Name */
	char *_Nullable value;			/* Value */
	AG_Size len;				/* Content length (characters) */
	AG_Size bufSize;			/* Buffer size */
	int global;				/* 1 = Persist across queries */
	Uint32 _pad;
	AG_TAILQ_ENTRY(web_variable) vars;
} WEB_Variable;

AG_TAILQ_HEAD(web_variableq, web_variable);

/* Argument to script (key=value pair). */
typedef struct web_argument {
	enum web_argument_type {
		WEB_GET_ARGUMENT,		/* HTTP GET method */
		WEB_POST_ARGUMENT,		/* HTTP POST method */
		WEB_ARGUMENT_LAST
	} type;
	char key[WEB_ARG_KEY_MAX];		/* Key */
	char *_Nullable value;			/* Value data (allocated) */
	AG_Size len;				/* Value length in bytes */
	char contentType[WEB_ARG_TYPE_MAX];	/* Content-Type or "" */
	AG_TAILQ_ENTRY(web_argument) args;
} WEB_Argument;

/* HTTP cookie */
typedef struct web_cookie {
	/* Read from client */
	char name[WEB_COOKIE_NAME_MAX];		/* Name of cookie */
	char value[WEB_COOKIE_VALUE_MAX];	/* Value of cookie */
	/* For Set-Cookie output */
	char expires[WEB_COOKIE_EXPIRE_MAX];	/* Expiration date or \0 */
	char domain[WEB_COOKIE_DOMAIN_MAX];	/* Domain match or \0 */
	char path[WEB_COOKIE_PATH_MAX];		/* Path attribute or \0 */
	Uint flags;
#define WEB_COOKIE_SECURE	0x01		/* Secure attribute */
	AG_TAILQ_ENTRY(web_cookie) cookies;
} WEB_Cookie;

/* Computed, satisfiable Range request */
typedef struct web_range_req {
	AG_Size first[WEB_RANGE_MAXRANGES];	/* First byte pos */
	AG_Size last[WEB_RANGE_MAXRANGES];	/* Last byte pos */
	Uint nRanges;
} WEB_RangeReq;

/* Query from web server */
typedef struct web_query {
	WEB_Method method;			/* HTTP method */
	Uint flags;
#define WEB_QUERY_CONTENT_READ	0x01		/* Content has been read */
#define WEB_QUERY_KEEPALIVE	0x02		/* Keepalive requested */
#define WEB_QUERY_DEFLATE	0x04		/* Accepts deflate encoding */
#define WEB_QUERY_NOCOMPRESSION	0x08		/* Disable compression */
#define WEB_QUERY_RANGE		0x10		/* Range request */
#define WEB_QUERY_PROXIED	0x20		/* Behind proxy */

	int  compressLvl;			/* Compression level */
	char  acceptLangs[WEB_LANGS_MAX]	/* Accept-Language list */
	                 [WEB_LANG_CODE_MAX];
	Uint  nAcceptLangs;

	AG_TAILQ_HEAD_(web_argument) args;	/* Call arguments */
	AG_TAILQ_HEAD_(web_cookie) cookies;	/* HTTP cookies */
	Uint                        nArgs;
	Uint                      nCookies;

	char contentType[128];			/* Client Content-Type (+attrs) */
	AG_Size contentLength;			/* Client Content-Length */
	int rangeFrom, rangeTo;			/* Range request */

	char userIP[64];			/* Client IP address */
	char userHost[256];			/* Client hostname */
	char userAgent[WEB_USERAGENT_MAX];	/* Client User-Agent */

	char code[64];			  	/* HTTP/1.0 code */

	char    head[WEB_HTTP_HEADER_MAX];	/* HTTP response headers */
	Uint    headLine[WEB_HTTP_MAXHEADERS];	/* Line start offsets */
	Uint    headLineCount;			/* Total lines in header */
	Uint    headLen;			/* Length of header in bytes */
#if AG_MODEL == AG_MEDIUM
	Uint32 _pad;
#endif
	Uchar *_Nullable data;			/* Raw response entity-body */
	AG_Size dataSize, dataLen;

	char lang[8];				/* Negotiated language code */
	void *_Nullable sess;			/* Session object */
	void *_Nullable mod;			/* WEB_Module executing op */
	int sock;				/* Client socket (or -1) */
	char date[36];				/* HTTP time */
} WEB_Query;

/* Control command sent to running Frontend process. */
typedef struct web_control_cmd {
	Uint flags;
#define WEB_CONTROL_CMD_SYNC	0x01		/* Block and return status */
	enum {
		WEB_CONTROL_NOOP,
		WEB_CONTROL_SHUTDOWN,		/* Perform graceful exit */
		WEB_CONTROL_WORKER_CHLD,	/* Report Worker process exit */
	} type;
	union {
		struct {
			char reason[16];	/* Reason for shutdown */
		} shutdown;
		struct {
			pid_t pid;		/* Worker PID */
			int status;		/* Reported exit status */
		} workerQuit;
	} data;
} WEB_ControlCmd;

/* Description of user-provided option flags */
typedef struct web_flag_descr {
	Uint bitmask;
	const char *_Nonnull name;
	const char *_Nonnull permsRD;
	const char *_Nonnull permsWR;
	const char *_Nonnull descr;
} WEB_FlagDescr;

/* Log levels; sync with webLogLvlNames[] */
enum web_loglvl {
	WEB_LOG_EMERG,
	WEB_LOG_ALERT,
	WEB_LOG_CRIT,
	WEB_LOG_ERR,
	WEB_LOG_WARNING,
	WEB_LOG_NOTICE,
	WEB_LOG_INFO,
	WEB_LOG_DEBUG,
	WEB_LOG_QUERY,
	WEB_LOG_WORKER,
	WEB_LOG_EVENT
};

typedef int (*WEB_CommandFn)(WEB_Query *_Nonnull q);

/* Map an URL-specified op to a module method */
typedef struct web_command {
	char *_Nullable name;		/* Name (NULL = list terminator) */
	_Nullable WEB_CommandFn fn;	/* Callback function */
	const char *_Nullable type;	/* Content-type (NULL = fn-defined) */
	const char *_Nullable flags;	/* Flags ("P"=public, "i"=index) */
} WEB_Command;

/* Map URL to operation (pre-auth) */
typedef struct web_command_pre_auth {
	const char *_Nullable name;      /* Op name (NULL = list terminator) */
	void (*_Nullable fn)(WEB_Query *_Nonnull q);    /* Callback function */
	const char *_Nullable type;      /* Content-Type (NULL = fn-defined) */
} WEB_CommandPreAuth;

/* Entry in per-module menu section table. */
typedef struct web_menu_section {
	const char *_Nullable name;  /* Display name (NULL = list terminator) */
	const char *_Nullable cmd;   /* Target op */
} WEB_MenuSection;

/* Application server module */
typedef struct web_module_class {
	struct ag_object_class _inherit;

	char *_Nonnull name;	 	/* Module class name */
	char *_Nonnull icon;	 	/* Icon HTML */
	char *_Nonnull lname;		/* Long name (HTML ok) */
	char *_Nonnull desc;	 	/* Description (HTML ok) */

	int  (*_Nullable workerInit)(void *_Nonnull, void *_Nonnull);
	void (*_Nullable workerDestroy)(void *_Nonnull);
	int  (*_Nullable sessOpen)(void *_Nonnull, void *_Nonnull);
	void (*_Nullable sessClose)(void *_Nonnull, void *_Nonnull);
	void (*_Nullable menu)(void *_Nonnull, WEB_Query *_Nonnull,
	                       WEB_Variable *_Nonnull);

	WEB_MenuSection *_Nullable menuSections;
	WEB_Command     *_Nonnull commands;
} WEB_ModuleClass;

typedef struct web_module {
	struct ag_object _inherit;	/* AG_Object -> WEB_Module */
} WEB_Module;

/* Session manager interface */
typedef struct web_session_ops {
	const char *_Nonnull name;	/* Session class name */
	AG_Size size;			/* Structure size */
#if AG_MODEL == AG_MEDIUM
	Uint32 _pad;
#endif
#ifdef AG_HAVE_64BIT
	Uint64 flags;
#else
	Uint flags;
#endif
#define WEB_SESSION_PREFORK_AUTH 0x01	/* Call auth() before fork() */

	time_t sessTimeout;		/* Session inactivity timeout (s) */
	time_t workerTimeout;		/* Worker inactivity timeout (s) */

	void (*_Nullable init)(void *_Nonnull);
	void (*_Nullable destroy)(void *_Nonnull);
	int  (*_Nullable load)(void *_Nonnull, AG_DataSource *_Nonnull);
	void (*_Nullable save)(void *_Nonnull, AG_DataSource *_Nonnull);
	int  (*_Nullable auth)(void *_Nullable, const char *_Nonnull,
	                       const char *_Nonnull);

	WEB_CommandPreAuth preAuthCmds[10];

	int  (*_Nullable sessOpen)(void *_Nonnull, const char *_Nonnull);
	void (*_Nullable sessRestored)(void *_Nonnull, const char *_Nonnull);
	void (*_Nullable sessClose)(void *_Nonnull);
	void (*_Nullable sessExpired)(void *_Nonnull);
	void (*_Nullable beginFrontQuery)(WEB_Query *_Nonnull,
	                                  const char *_Nonnull);
	void (*_Nonnull  loginPage)(WEB_Query *_Nonnull);
	void (*_Nonnull  logout)(WEB_Query *_Nonnull);

	void (*_Nullable addSelectFDs)(void *_Nonnull, fd_set *_Nullable,
	                               fd_set *_Nullable, int *_Nonnull);
	void (*_Nullable procSelectFDs)(void *_Nonnull, fd_set *_Nullable,
	                                fd_set *_Nullable);
} WEB_SessionOps;

/* HTTP method */
typedef struct web_method_ops {
	const char *_Nonnull name;
	int (*_Nonnull fn)(int, const char *_Nonnull, char *_Nonnull,
	                   void *_Nonnull, AG_Size rdBufLen,
	                   const WEB_SessionOps *_Nonnull Sops);
} WEB_MethodOps;

/* Session variable */
typedef struct web_session_var {
	char key[WEB_SESSION_VAR_KEY_MAX];	/* Variable name */
	char value[WEB_SESSION_VAR_VALUE_MAX];	/* Variable value */
	AG_TAILQ_ENTRY(web_session_var) vars;
} WEB_SessionVar;

/* Open socket between a Frontend and a running Worker. */
typedef struct web_session_socket {
	char  sessID[WEB_SESSID_MAX];		/* Open session ID */
	Uint8 _pad;
	int   fd;				/* Socket */
	pid_t workerPID;			/* Worker process PID */
	int   workerIsMyChld;			/* Worker is my child */
	AG_TAILQ_ENTRY(web_session_socket) sockets;
} WEB_SessionSocket;

AG_TAILQ_HEAD(web_session_socketq, web_session_socket);

/* Session Instance */
typedef struct web_session {
	const WEB_SessionOps *_Nonnull ops;	/* Operations */
	char id[WEB_SESSID_MAX];		/* Session identifier */
	Uint8 _pad1;
	Uint                           nVars;
	AG_TAILQ_HEAD_(web_session_var) vars;	/* Session variables */
	AG_TAILQ_ENTRY(web_session) sessions;
	int pp[2];				/* Status pipe */
	Uint nEvents;				/* Event counter */
	Uint32 _pad2;
} WEB_Session;

typedef int  (*WEB_LanguageFn)(const char *_Nonnull, void *_Nullable);
typedef void (*WEB_MenuFn)(WEB_Query *_Nonnull, WEB_Variable *_Nonnull,
                           void *_Nullable);
typedef void (*WEB_DestroyFn)(void);
typedef void (*WEB_LogFn)(enum web_loglvl, const char *_Nonnull);

#define WEB_TRIM_WHITESPACE(s,end)			\
	while (isspace(*s)) { s++; }			\
	if (*s != '\0') {				\
		end = &s[strlen(s) - 1];		\
		while (end > s && isspace(*end)) {	\
			*end = '\0';			\
			end--;				\
		}					\
	}

typedef int (*WEB_EventFilterFn)(char *_Nonnull, char *_Nonnull, char *_Nonnull,
                                 const void *_Nullable);

__BEGIN_DECLS
extern WEB_ModuleClass                 webModuleClass;   /* Web(Module) class */
extern WEB_Module *_Nullable *_Nonnull webModules;       /* Registered modules */
extern Uint                            webModuleCount;

extern char *_Nullable *_Nullable webLangs;              /* Handled languages */
extern Uint                       webLangCount;

extern char webWorkerUser[WEB_USERNAME_MAX];  /* Username (in Worker) */
extern char webHomeOp[WEB_OPNAME_MAX];        /* Default (home) operation */

extern const WEB_MethodOps  webMethods[];  /* HTTP methods handled */
extern struct web_variableq webVars;       /* Variables for template substitution */
extern Uint                 webQueryCount; /* Query counter (per process) */

extern struct web_session_socketq webWorkSockets; /* Frontend->Worker sockets */

/*
 * Initialization
 */
void  WEB_Init(Uint, int);
void  WEB_Destroy(void);
void  WEB_CheckSignals(void);
void  WEB_RegisterModule(WEB_ModuleClass *_Nonnull);
void  WEB_SetLogFile(const char *_Nonnull);
void  WEB_SetLogFn(_Nullable WEB_LogFn);
void  WEB_Exit(int, const char *_Nullable, ...);
void  WEB_SetLanguageFn(_Nullable WEB_LanguageFn, void *_Nullable);
void  WEB_SetMenuFn(_Nullable WEB_MenuFn, void *_Nullable);
void  WEB_SetDestroyFn(_Nullable WEB_DestroyFn);
void  WEB_AddLanguage(const char *_Nonnull);

/*
 * Query processing
 */
int   WEB_WorkerMain(const WEB_SessionOps *_Nonnull, WEB_Query *_Nonnull,
                     const char *_Nonnull, const char *_Nonnull,
		     const char *_Nonnull, int [_Nonnull 2], int);
void  WEB_QueryLoop(const char *_Nonnull,
                    const char *_Nonnull,
                    const WEB_SessionOps *_Nonnull);
int   WEB_ControlCommand(Uint, const WEB_ControlCmd *_Nonnull);
int   WEB_ControlCommandS(Uint, const char *_Nonnull);
void  WEB_QueryInit(WEB_Query *_Nonnull, const char *_Nonnull);
void  WEB_QueryDestroy(WEB_Query *_Nonnull);
int   WEB_QueryLoad(WEB_Query *_Nonnull, const void *_Nonnull, AG_Size);
int   WEB_QuerySave(int, const WEB_Query *_Nonnull);
void  WEB_BeginFrontQuery(WEB_Query *_Nonnull, const char *_Nonnull,
                          const WEB_SessionOps *_Nonnull);
void  WEB_BeginWorkerQuery(WEB_Query *_Nonnull);
int   WEB_ExecWorkerQuery(WEB_Query *_Nonnull);
void  WEB_FlushQuery(WEB_Query *_Nonnull);
int   WEB_ProcessQuery(WEB_Query *_Nonnull, const WEB_SessionOps *_Nonnull,
                       void *_Nonnull, AG_Size);
/*
 * HTTP Headers
 */
void WEB_EditHeader(WEB_Query *_Nonnull, char *_Nonnull, const char *_Nonnull);

WEB_Cookie *_Nonnull WEB_SetCookie(WEB_Query *_Nonnull, const char *_Nonnull,
                                   const char *_Nonnull, ...)
				  FORMAT_ATTRIBUTE(printf,3,4);

WEB_Cookie *_Nonnull WEB_SetCookieS(WEB_Query *_Nonnull, const char *_Nonnull,
                                    const char *_Nonnull);

void WEB_DelCookie(WEB_Query *_Nonnull, const char *_Nonnull);

/*
 * Form and argument parsing
 */
int WEB_ParseFormUrlEncoded(WEB_Query *_Nonnull, char *_Nonnull,
                            enum web_argument_type);
int WEB_ReadFormData(WEB_Query *_Nonnull, int);

const char *_Nullable WEB_Get(WEB_Query *_Nonnull, const char *_Nonnull, AG_Size);
const char *_Nullable WEB_GetTrim(WEB_Query *_Nonnull, const char *_Nonnull, AG_Size);

int   WEB_GetInt(WEB_Query *_Nonnull, const char *_Nonnull, int *_Nonnull);
int   WEB_GetUint(WEB_Query *_Nonnull, const char *_Nonnull, Uint *_Nonnull);
int   WEB_GetIntR(WEB_Query *_Nonnull, const char *_Nonnull, int *_Nonnull,
                  int, int);
int   WEB_GetUintR(WEB_Query *_Nonnull, const char *_Nonnull, Uint *_Nonnull,
                   Uint, Uint);
int   WEB_GetIntRange(WEB_Query *_Nonnull, const char *_Nonnull, int *_Nonnull,
                      const char *_Nonnull, int *_Nonnull);
int   WEB_GetUint64(WEB_Query *_Nonnull, const char *_Nonnull, Uint64 *_Nonnull);
int   WEB_GetSint64(WEB_Query *_Nonnull, const char *_Nonnull, Sint64 *_Nonnull);

int   WEB_GetEnum(WEB_Query *_Nonnull, const char *_Nonnull, Uint *_Nonnull,
                  Uint);

int   WEB_GetFloat(WEB_Query *_Nonnull, const char *_Nonnull, float *_Nonnull);
int   WEB_GetDouble(WEB_Query *_Nonnull, const char *_Nonnull, double *_Nonnull);
int   WEB_GetBool(WEB_Query *_Nonnull, const char *_Nonnull);

void  WEB_SetS(WEB_Query *_Nonnull, const char *_Nonnull, const char *_Nullable);
void  WEB_Set(WEB_Query *_Nonnull, const char *_Nonnull, const char *_Nullable,
              ...);
int   WEB_Unset(WEB_Query *_Nonnull, const char *_Nonnull);

char *_Nonnull WEB_EscapeURL(WEB_Query *_Nonnull, const char *_Nonnull);
char *_Nonnull WEB_UnescapeURL(WEB_Query *_Nonnull, const char *_Nonnull);

/*
 * Logging
 */
void WEB_LogS(enum web_loglvl, const char *_Nonnull);
void WEB_Log(enum web_loglvl, const char *_Nonnull, ...)
             FORMAT_ATTRIBUTE(__printf__,2,3);
void WEB_LogErr(const char *_Nonnull, ...)    FORMAT_ATTRIBUTE(__printf__,1,2);
void WEB_LogWarn(const char *_Nonnull, ...)   FORMAT_ATTRIBUTE(__printf__,1,2);
void WEB_LogInfo(const char *_Nonnull, ...)   FORMAT_ATTRIBUTE(__printf__,1,2);
void WEB_LogNotice(const char *_Nonnull, ...) FORMAT_ATTRIBUTE(__printf__,1,2);
void WEB_LogDebug(const char *_Nonnull, ...)  FORMAT_ATTRIBUTE(__printf__,1,2);
void WEB_LogWorker(const char *_Nonnull, ...) FORMAT_ATTRIBUTE(__printf__,1,2);
void WEB_LogEvent(const char *_Nonnull, ...)  FORMAT_ATTRIBUTE(__printf__,1,2);

/*
 * Formatted output
 */
void WEB_Printf(WEB_Query *_Nonnull, const char *_Nonnull, ...)
               FORMAT_ATTRIBUTE(__printf__,2,3);

void WEB_PutJSON(WEB_Query  *_Nonnull,
                 const char *_Nonnull,
                 const char *_Nonnull, ...)
                FORMAT_ATTRIBUTE(__printf__,3,4);

int  WEB_PutJSON_HTML(WEB_Query *_Nonnull, const char *_Nonnull,
                      const char *_Nonnull);

void WEB_VAR_FilterDocument(WEB_Query *_Nonnull, const char *_Nonnull, AG_Size);
void WEB_VAR_FilterFragment(WEB_Query *_Nonnull, const char *_Nonnull, AG_Size);

WEB_Variable *_Nonnull WEB_VAR_Set(const char *_Nullable,
                                   const char *_Nullable, ...)
                                  FORMAT_ATTRIBUTE(__printf__, 2,3);

WEB_Variable *_Nonnull WEB_VAR_SetS(const char *_Nullable,
                                    const char *_Nullable);

WEB_Variable *_Nonnull WEB_VAR_SetS_NODUP(const char *_Nullable,
                                          char       *_Nonnull);

WEB_Variable *_Nonnull WEB_VAR_SetGlobal(const char *_Nonnull,
                                         const char *_Nullable, ...)
                                        FORMAT_ATTRIBUTE(__printf__,2,3);

WEB_Variable *_Nonnull WEB_VAR_SetGlobalS(const char *_Nonnull,
                                          const char *_Nonnull);

void WEB_VAR_Cat(WEB_Variable *_Nonnull, const char *_Nonnull, ...)
                FORMAT_ATTRIBUTE(__printf__,2,3);

void WEB_VAR_Unset(const char *_Nonnull);
void WEB_VAR_Wipe(const char *_Nonnull);
int  WEB_VAR_Defined(const char *_Nonnull) _Pure_Attribute;
void WEB_VAR_Free(WEB_Variable *_Nonnull);

int  WEB_OutputHTML(WEB_Query *_Nonnull, const char *_Nonnull);
void WEB_OutputError(WEB_Query *_Nonnull, const char *_Nonnull);

void WEB_SetErrorS(const char *_Nonnull);
void WEB_SetError(const char *_Nonnull, ...) FORMAT_ATTRIBUTE(__printf__,1,2);
void WEB_SetSuccess(const char *_Nonnull, ...) FORMAT_ATTRIBUTE(__printf__,1,2);

/*
 * Authentication and Session Management
 */
void    WEB_SessionInit(WEB_Session *_Nonnull, const WEB_SessionOps *_Nonnull);
void    WEB_SessionDestroy(WEB_Session *_Nonnull);
void    WEB_CloseSession(WEB_Session *_Nonnull);
int     WEB_SessionLoad(void *_Nonnull, const char *_Nonnull);
int     WEB_SessionSaveToFD(void *_Nonnull, int);
#define WEB_SessionSave(sess) WEB_SessionSaveToFD((sess),-1)

void  WEB_SetSV_S(void *_Nonnull, const char *_Nonnull, const char *_Nonnull);
void  WEB_SetSV(void *_Nonnull, const char *_Nonnull, const char *_Nonnull, ...)
               FORMAT_ATTRIBUTE(__printf__,3,4);

int   WEB_SetSV_ALL(const WEB_SessionOps *_Nonnull, const char *_Nonnull,
                    const char *_Nonnull, const char *_Nonnull);

#ifdef HAVE_SETPROCTITLE
#define WEB_SetProcTitle setproctitle
#else
void    WEB_SetProcTitle(const char *_Nonnull, ...)
		         FORMAT_ATTRIBUTE(__printf__,1,2);
#endif

/*
 * Push Events
 */
int WEB_PostEventS(const char *_Nullable, _Nullable WEB_EventFilterFn,
                   const void *_Nullable, const char *_Nonnull,
                   const char *_Nonnull);

int WEB_PostEvent(const char *_Nullable, _Nullable WEB_EventFilterFn,
                  const void *_Nullable, const char *_Nonnull,
                  const char *_Nonnull, ...)
                 FORMAT_ATTRIBUTE(__printf__,5,6);

static __inline__ void
WEB_SetHomeOp(const char *_Nonnull op)
{
	AG_Strlcpy(webHomeOp, op, sizeof(webHomeOp));
}

static __inline__ void
WEB_SessionFree(WEB_Session *_Nonnull S)
{
	WEB_SessionDestroy(S);
	AG_Free(S);
}

static __inline__ WEB_Variable *_Nonnull
WEB_VAR_New(const char *_Nullable key)
{
	return WEB_VAR_SetS(key, NULL);
}

static __inline__ void
WEB_VAR_Grow(WEB_Variable *_Nonnull V, AG_Size len)
{
	if (V->len+len >= V->bufSize) {
		V->bufSize = V->len + len + WEB_VAR_BUF_GROW;
		V->value = AG_Realloc(V->value, V->bufSize);
	}
}

static __inline__ void
WEB_VAR_CatC(WEB_Variable *_Nonnull V, const char c)
{
	WEB_VAR_Grow(V, 1);
	V->value[V->len] = c;
	V->value[V->len+1] = '\0';
	V->len++;
}

static __inline__ void
WEB_VAR_CatS(WEB_Variable *_Nonnull V, const char *_Nonnull s)
{
	AG_Size len;

	len = strlen(s);
	WEB_VAR_Grow(V, len+1);
	memcpy(&V->value[V->len], s, len+1);
	V->len += len;
}

static __inline__ void
WEB_VAR_CatS_NoHTML(WEB_Variable *_Nonnull V, const char *_Nonnull s)
{
	const char *c;

	WEB_VAR_Grow(V, strlen(s));
	for (c = &s[0]; *c != '\0'; c++) {
		if (*c == '"')		{ WEB_VAR_CatS(V, "&quot;"); }
		else if (*c == '&')	{ WEB_VAR_CatS(V, "&amp;"); }
		else if (*c == '<')	{ WEB_VAR_CatS(V, "&lt;"); }
		else if (*c == '>')	{ WEB_VAR_CatS(V, "&gt;"); }
		else			{ WEB_VAR_CatC(V, *c); }
	}
}

static __inline__ void
WEB_VAR_CatS_NODUP(WEB_Variable *_Nonnull V, char *_Nonnull s)
{
	AG_Size len = strlen(s);

	WEB_VAR_Grow(V, len+1);
	memcpy(&V->value[V->len], s, len+1);
	V->len += len;
	AG_Free(s);
}

static __inline__ void
WEB_VAR_CatJS(WEB_Variable *_Nonnull V, const char *_Nonnull s)
{
	const char *c;

	WEB_VAR_Grow(V, strlen(s)+3);
	WEB_VAR_CatC(V, '"');
	for (c = &s[0]; *c != '\0'; c++) {
		if (*c == '\\')		{ WEB_VAR_CatS(V, "\\\\"); }
		else if (*c == '"')	{ WEB_VAR_CatS(V, "\\\""); }
		else			{ WEB_VAR_CatC(V, *c); }
	}
	WEB_VAR_CatS(V, "\",");
}

static __inline__ void
WEB_VAR_CatJS_NODUP(WEB_Variable *_Nonnull V, char *_Nonnull s)
{
	WEB_VAR_CatJS(V, s);
	AG_Free(s);
}

static __inline__ void
WEB_VAR_CatJS_NoHTML(WEB_Variable *_Nonnull V, const char *_Nonnull s)
{
	const char *c;

	WEB_VAR_Grow(V, strlen(s)+3);
	WEB_VAR_CatC(V, '"');
	for (c = &s[0]; *c != '\0'; c++) {
		if (*c == '\\')		{ WEB_VAR_CatS(V, "\\\\"); }
		else if (*c == '"')	{ WEB_VAR_CatS(V, "\\\""); }
		else if (*c == '<')	{ WEB_VAR_CatS(V, "&lt;"); }
		else if (*c == '>')	{ WEB_VAR_CatS(V, "&gt;"); }
		else			{ WEB_VAR_CatC(V, *c); }
	}
	WEB_VAR_CatS(V, "\",");
}

static __inline__ void
WEB_VAR_CatJS_NoHTML_NODUP(WEB_Variable *_Nonnull V, char *_Nonnull s)
{
	WEB_VAR_CatJS_NoHTML(V, s);
	AG_Free(s);
}

static __inline__ void
WEB_VAR_CatN(WEB_Variable *_Nonnull V, const void *_Nonnull s, AG_Size len)
{
	WEB_VAR_Grow(V, len+1);
	memcpy(&V->value[V->len], s, len);
	V->value[V->len + len - 1] = '\0';
	V->len += len;
}

static __inline__ void
WEB_VAR_CatN_NoNUL(WEB_Variable *_Nonnull V, const void *_Nonnull s, AG_Size len)
{
	WEB_VAR_Grow(V, len+1);
	memcpy(&V->value[V->len], s, len);
	V->len += len;
}

static __inline__ char *_Nullable _Pure_Attribute
WEB_VAR_Get(const char *_Nonnull key)
{
	WEB_Variable *V;

	AG_TAILQ_FOREACH(V, &webVars, vars) {
		if (V->key[0] != '\0' &&
		    strcmp(V->key, key) == 0)
			break;
	}
	return (V != NULL) ? V->value : NULL;
}

/* Set an integer range (and collapse to a single integer if min=max). */
static __inline__ WEB_Variable *_Nonnull
WEB_VAR_SetIntRange(const char *_Nullable key, int min, int max)
{
	WEB_Variable *V;

	if (min == max) {
		V = WEB_VAR_Set(key, "%d", min);
	} else {
		V = WEB_VAR_Set(key, "%d-%d", min, max);
	}
	return (V);
}

/* Set a float range (and collapse to a single number if min=max). */
static __inline__ WEB_Variable *_Nonnull
WEB_VAR_SetFloatRange(const char *_Nullable key, float min, float max)
{
	WEB_Variable *V;

	if (min == max) {
		V = WEB_VAR_Set(key, "%.0f", min);
	} else {
		V = WEB_VAR_Set(key, "%.0f-%.0f", min, max);
	}
	return (V);
}

/* Set a double range (and collapse to a single number if min=max). */
static __inline__ WEB_Variable *_Nonnull
WEB_VAR_SetDoubleRange(const char *_Nullable key, double min, double max)
{
	WEB_Variable *V;

	if (min == max) {
		V = WEB_VAR_Set(key, "%.0f", min);
	} else {
		V = WEB_VAR_Set(key, "%.0f-%.0f", min, max);
	}
	return (V);
}

/* Get a pointer to the named argument. Return NULL if undefined. */
static __inline__ const WEB_Argument *_Nullable _Pure_Attribute
WEB_GetArgument(const WEB_Query *_Nonnull q, const char *_Nonnull key)
{
	const WEB_Argument *arg;

	AG_TAILQ_FOREACH(arg, &q->args, args) {
		if (strcmp(arg->key, key) == 0)
			break;
	}
	return (arg);
}

/* Standard read loop. Read up to len bytes while checking signals. */
static __inline__ int
WEB_SYS_Read(int fd, void *_Nonnull data, AG_Size len)
{
	AG_Size nread;
	AG_Size rv;
	
	for (nread=0; nread < len; ) {
		rv = read(fd, data+nread, len-nread);
		if (rv == -1) {
			if (errno == EINTR || errno == EAGAIN) {
				WEB_CheckSignals();
				continue;
			} else {
				AG_SetErrorS(strerror(errno));
				return (-1);
			}
		} else if (rv == 0) {
			AG_SetErrorS("EOF");
			return (-1);
		}
		nread += rv;
	}
	return (0);
}

/* Standard write loop. Write up to len bytes while checking signals. */
static __inline__ int
WEB_SYS_Write(int fd, const void *_Nonnull data, AG_Size len)
{
	AG_Size nwrote;
	AG_Size rv;

	for (nwrote = 0; nwrote < len; ) {
		rv = write(fd, data+nwrote, len-nwrote);
		if (rv == -1) {
			if (errno == EINTR || errno == EAGAIN) {
				WEB_CheckSignals();
				continue;
			} else {
				AG_SetErrorS(strerror(errno));
				return (-1);
			}
		} else if (rv == 0) {
			AG_SetErrorS("EOF");
			return (-1);
		}
		nwrote += rv;
	}
	return (0);
}

static __inline__ void
WEB_ClearHeaders(WEB_Query *_Nonnull q, const char *_Nonnull status)
{
	AG_Size len = strlen(status);

	memcpy(q->head, status, len+1);
	q->headLen = len;
	q->headLine[0] = len;
	q->headLineCount = 1;
}

static __inline__ int
WEB_WriteHeaders(int fd, WEB_Query *_Nonnull q)
{
	q->head[q->headLen  ] = '\r';
	q->head[q->headLen+1] = '\n';
	q->head[q->headLen+2] = '\0';

	return WEB_SYS_Write(fd, q->head, q->headLen+2);
}

#define WEB_Read(q,data,len)  WEB_SYS_Read((q)->sock,(data),(len))

/* Add data to the query response buffer. */
static __inline__ void
WEB_Write(WEB_Query *_Nonnull q, const void *_Nonnull data, AG_Size len)
{
	if (q->dataLen+len > q->dataSize) {
		q->dataSize += len+WEB_DATA_BUFSIZE;
		q->data = Realloc(q->data, q->dataSize);
	}
	memcpy(&q->data[q->dataLen], data, len);
	q->dataLen += len;
}

/* Recalculate header line offsets. */
static void
WEB_UpdateHeaderLines(WEB_Query *_Nonnull q)
{
	char *c;

	q->headLineCount = 0;
	for (c = q->head; *c != '\0'; c++) {
		if (*c == '\n' && c[1] != '\0') {
			if (q->headLineCount+1 >= WEB_HTTP_MAXHEADERS) {
				break;
			}
			q->headLine[q->headLineCount++] = (&c[1] - q->head);
		}
	}
}

/* Prepare to return the specified HTTP response code. */
static __inline__ void
WEB_SetCode(WEB_Query *_Nonnull q, const char *_Nonnull code)
{
	char httpCode[64];
	char *head = q->head, *cEnd;
	AG_Size oldLen, newLen;

	AG_Strlcpy(httpCode, "HTTP/1.0 ", sizeof(httpCode));
	AG_Strlcat(httpCode, code, sizeof(httpCode));
	AG_Strlcat(httpCode, "\r\n", sizeof(httpCode));
	cEnd = strchrnul(head, '\n');
	newLen = strlen(httpCode);
	oldLen = (cEnd-head)+1;
	if ((q->headLen - oldLen + newLen) >= sizeof(q->head)-2) {
		AG_FatalError("SetCode too big");
	}
	if (newLen != oldLen && (q->headLen - oldLen) > 0) {
		memmove(&head[newLen], &head[oldLen], (q->headLen - oldLen));
	}
	memcpy(head, httpCode, newLen);
	if (newLen != oldLen)
		WEB_UpdateHeaderLines(q);
}

/*
 * Append the specified HTTP response header.
 * Existing headers are ignored. Duplicate headers are allowed.
 */
static __inline__ void
WEB_AppendHeaderS(WEB_Query *_Nonnull q, const char *_Nonnull key,
    const char *_Nonnull value)
{
	char newLine[WEB_HTTP_PER_HEADER_MAX];
	char *cEnd;
	AG_Size newLen;

	if (q->headLineCount+1 >= WEB_HTTP_MAXHEADERS) {
		AG_FatalError("Too many headers");
	}
	AG_Strlcpy(newLine, key, sizeof(newLine));
	AG_Strlcat(newLine, ": ", sizeof(newLine));
	AG_Strlcat(newLine, value, sizeof(newLine));
	AG_Strlcat(newLine, "\r\n", sizeof(newLine));
	newLen = strlen(newLine);
	if ((q->headLen + newLen + 1) >= sizeof(q->head)-2) {
		AG_FatalError("Header too big");
	}
	cEnd = &q->head[q->headLen];
	memcpy(cEnd, newLine, newLen+1);
	q->headLine[q->headLineCount++] = (cEnd - q->head);
	q->headLen += newLen;
}

/*
 * Prepare to return the specified HTTP response header.
 * Existing headers are updated.
 */
static __inline__ void
WEB_SetHeaderS(WEB_Query *_Nonnull q, const char *_Nonnull key,
    const char *_Nonnull value)
{
	AG_Size keyLen = strlen(key);
	char *cLine;
	Uint i;

	for (i = 0; i < q->headLineCount; i++) {
		cLine = &q->head[q->headLine[i]];
		if (strncasecmp(cLine, key, keyLen) == 0 &&
		    cLine[keyLen  ] == ':' &&
		    cLine[keyLen+1] == ' ')
			break;
	}
	if (i < q->headLineCount) {			/* Update existing */
		WEB_EditHeader(q, cLine, value);
	} else {					/* Append new */
		WEB_AppendHeaderS(q, key, value);
	}
}

static __inline__ void
WEB_SetHeader(WEB_Query *_Nonnull q, const char *_Nonnull key,
    const char *_Nonnull fmt, ...)
{
	char value[WEB_HTTP_PER_HEADER_MAX];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(value, sizeof(value), fmt, ap);
	va_end(ap);
	WEB_SetHeaderS(q, key, value);
}

static __inline__ void
WEB_AppendHeader(WEB_Query *_Nonnull q, const char *_Nonnull key,
    const char *_Nonnull fmt, ...)
{
	char value[WEB_HTTP_PER_HEADER_MAX];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(value, sizeof(value), fmt, ap);
	va_end(ap);
	WEB_AppendHeaderS(q, key, value);
}

/* Set the preferred compression level. */
static __inline__ void
WEB_SetCompression(WEB_Query *_Nonnull q, int enable, int level)
{
	if (enable) {
		q->flags &= ~(WEB_QUERY_NOCOMPRESSION);
		q->compressLvl = level;
	} else {
		q->flags |= WEB_QUERY_NOCOMPRESSION;
	}
}

static __inline__ void
WEB_PutC(WEB_Query *_Nonnull q, char c)
{
	WEB_Write(q, &c, 1);
}
static __inline__ void
WEB_PutS(WEB_Query *_Nonnull q, const char *_Nonnull s)
{
	WEB_Write(q, s, strlen(s));
}
static __inline__ void
WEB_PutJSON_S(WEB_Query *_Nonnull q, const char *_Nonnull key,
    const char *_Nonnull val)
{
	const char *c;

	WEB_PutC(q, '"');
	WEB_PutS(q, key);
	WEB_PutS(q, "\": \"");
	for (c = &val[0]; *c != '\0'; c++) {
		if     (*c == '\\') { WEB_PutS(q, "\\\\"); }
		else if (*c == '"') { WEB_PutS(q, "\\\""); }
		else if (*c == '\r') { WEB_PutS(q, "\\r"); }
		else if (*c == '\n') { WEB_PutS(q, "\\n"); }
		else if (*c == '\t') { WEB_PutS(q, "\\t"); }
		else { WEB_PutC(q, *c); }
	}
	WEB_PutS(q, "\",");
}
static __inline__ void
WEB_PutJSON_NoHTML_S(WEB_Query *_Nonnull q, const char *_Nonnull key,
    const char *_Nonnull val)
{
	const char *c;

	WEB_PutC(q, '"');
	WEB_PutS(q, key);
	WEB_PutS(q, "\": \"");
	for (c = &val[0]; *c != '\0'; c++) {
		if     (*c == '\\') { WEB_PutS(q, "\\\\"); }
		else if (*c == '"') { WEB_PutS(q, "\\\""); }
		else if (*c == '\n') { WEB_PutS(q, "\\n"); }
		else if (*c == '\t') { WEB_PutS(q, "\\t"); }
		else if (*c == '<') { WEB_PutS(q, "&lt;"); }
		else if (*c == '>') { WEB_PutS(q, "&gt;"); }
		else { WEB_PutC(q, *c); }
	}
	WEB_PutS(q, "\",");
}

/* Return the named cookie's value */
static __inline__ char *_Nullable _Pure_Attribute
WEB_GetCookie(WEB_Query *_Nonnull q, const char *_Nonnull name)
{
	WEB_Cookie *ck;

	AG_TAILQ_FOREACH(ck, &q->cookies, cookies) {
		if (strcmp(ck->name, name) == 0)
			return (ck->value);
	}
	return (NULL);
}

/* Return a pointer to the named cookie */
static __inline__ WEB_Cookie *_Nullable _Pure_Attribute
WEB_LookupCookie(WEB_Query *_Nonnull q, const char *_Nonnull name)
{
	WEB_Cookie *ck;

	AG_TAILQ_FOREACH(ck, &q->cookies, cookies) {
		if (strcmp(ck->name, name) == 0)
			return (ck);
	}
	return (NULL);
}

/* Return the value of a session variable (or "" if it doesn't exists) */
static __inline__ const char *_Nonnull _Pure_Attribute
WEB_GetSV(void *_Nonnull sess, const char *_Nonnull key)
{
	WEB_Session *S = sess;
	WEB_SessionVar *SV;

	AG_TAILQ_FOREACH(SV, &S->vars, vars) {
		if (strcmp(SV->key, key) == 0)
			break;
	}
	if (SV != NULL) {
		return (SV->value);
	} else {
		return ("");
	}
}
__END_DECLS

#if defined(_AGAR_INTERNAL) || defined(_USE_AGAR_WEB)
# define VAR		WEB_Variable
# define VAR_NAME_MAX	WEB_VAR_NAME_MAX

# define VAR_New	WEB_VAR_New
# define VAR_Unset	WEB_VAR_Unset
# define VAR_Wipe	WEB_VAR_Wipe
# define VAR_Defined	WEB_VAR_Defined
# define VAR_Free	WEB_VAR_Free

# define Get			WEB_VAR_Get
# define Set			WEB_VAR_Set
# define SetS			WEB_VAR_SetS
# define SetS_NODUP		WEB_VAR_SetS_NODUP
# define Cat			WEB_VAR_Cat
# define CatS			WEB_VAR_CatS
# define CatS_NODUP		WEB_VAR_CatS_NODUP
# define CatS_NoHTML		WEB_VAR_CatS_NoHTML
# define CatJS			WEB_VAR_CatJS
# define CatJS_NODUP		WEB_VAR_CatJS_NODUP
# define CatJS_NoHTML		WEB_VAR_CatJS_NoHTML
# define CatJS_NoHTML_NoDUP	WEB_VAR_CatJS_NoHTML
# define CatN			WEB_VAR_CatN
# define CatN_NoNUL		WEB_VAR_CatN_NoNUL
# define CatC			WEB_VAR_CatC
# define SetGlobal		WEB_VAR_SetGlobal
# define SetGlobalS		WEB_VAR_SetGlobalS

# define SetIntRange		WEB_VAR_SetIntRange
# define SetFloatRange		WEB_VAR_SetFloatRange
# define SetDoubleRange		WEB_VAR_SetDoubleRange
# define GetSV			WEB_GetSV
# define SetSV			WEB_SetSV
# define SetSV_S		WEB_SetSV_S

# define SESSION(p)   WEB_SESSION(p)
#endif /* _USE_AGAR_WEB */

#include <agar/net/close.h>

#endif /* AG_WEB */
#endif /* _AGAR_NET_WEB_H_ */
