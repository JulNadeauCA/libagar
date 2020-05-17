/*
 * Copyright (c) 2012-2018 Julien Nadeau Carriere <vedge@csoft.net>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * MySQL database access.
 */

#include <agar/core/core.h>

#include <ctype.h>
#include <mysql.h>

typedef struct ag_db_mysql {
	struct ag_db _inherit;
	MYSQL *my;
} AG_DbMySQL;

static void
Init(void *_Nonnull obj)
{
	AG_DbMySQL *db = obj;

	AG_SetString(db, "host",		NULL);
	AG_SetInt(db,    "port",		0);
	AG_SetString(db, "database",		NULL);
	AG_SetString(db, "user",		NULL);
	AG_SetString(db, "password",		NULL);
	AG_SetString(db, "unix-socket",		NULL);
	AG_SetInt(db,    "compress",		0);
	AG_SetInt(db,    "local-files",		-1);
	AG_SetInt(db,    "ssl",			-1);
	AG_SetInt(db,    "ssl-verify-cert",	-1);
	AG_SetInt(db,    "secure-auth",		-1);
	AG_SetString(db, "cnf-file",		NULL);
	AG_SetString(db, "cnf-group",		NULL);
	AG_SetString(db, "protocol",		"default");
	AG_SetUint(db,   "read-timeout",	0);
	AG_SetInt(db,    "write-timeout",	-1);
	AG_SetInt(db,    "reconnect",		1);
	AG_SetString(db, "charset",		NULL);
	AG_SetString(db, "charset-dir",		NULL);
	
	AG_SetString(db, "init-cmd", NULL);
	AG_SetString(db, "get-cmd", "SELECT my_field FROM my_table "
                                    "WHERE my_field = '%s'");
	AG_SetString(db, "put-cmd", "INSERT INTO my_table VALUES('%s')");
}

static int
Open(void *_Nonnull obj, const char *_Nonnull path, Uint flags)
{
	char dbName[128];
	AG_DbMySQL *db = obj;
	MYSQL *my;
	unsigned long myFlags = CLIENT_REMEMBER_OPTIONS;
	char *s;
	Uint i;
	my_bool b = 1;
	
	if ((my = mysql_init(NULL)) == NULL) {
		AG_SetError("mysql_init failed");
		return (-1);
	}
	if (path != NULL) {
		Strlcpy(dbName, path, sizeof(dbName));
	} else {
		AG_GetString(db, "database", dbName, sizeof(dbName));
	}
	if (AG_GetInt(db,"compress") == 1)	{ myFlags |= CLIENT_COMPRESS; }
	if (AG_GetInt(db,"local-files") == 1)	{ myFlags |= CLIENT_LOCAL_FILES; }
	if (AG_GetInt(db,"ssl") == 1)		{ myFlags |= CLIENT_SSL; }

	if (AG_GetUint(db,"ssl-verify-cert") == 1)	 { mysql_options(my, MYSQL_OPT_SSL_VERIFY_SERVER_CERT, &b); }
	if (AG_GetUint(db,"secure-auth") == 1)		 { mysql_options(my, MYSQL_SECURE_AUTH, &b); }
	if ((s = AG_GetStringP(db,"init-cmd")) != NULL)	 { mysql_options(my, MYSQL_INIT_COMMAND, s); }
	if ((s = AG_GetStringP(db,"cnf-file")) != NULL)	 { mysql_options(my, MYSQL_READ_DEFAULT_FILE, s); }
	if ((s = AG_GetStringP(db,"cnf-group")) != NULL) { mysql_options(my, MYSQL_READ_DEFAULT_GROUP, s); }

	if ((s = AG_GetStringP(db,"protocol")) != NULL) {
		switch (tolower(s[0])) {
		case 't':	i = MYSQL_PROTOCOL_TCP;		break;
		case 's':	i = MYSQL_PROTOCOL_SOCKET;	break;
		case 'p':	i = MYSQL_PROTOCOL_PIPE;	break;
		case 'm':	i = MYSQL_PROTOCOL_MEMORY;	break;
		default:	i = MYSQL_PROTOCOL_DEFAULT;	break;
		}
		mysql_options(my, MYSQL_OPT_PROTOCOL, (const char *)&i);
	}
	if ((i = AG_GetUint(db,"read-timeout")) != 0)	 { mysql_options(my, MYSQL_OPT_READ_TIMEOUT, (const char *)&i); }
	if ((i = AG_GetUint(db,"write-timeout")) != 0)	 { mysql_options(my, MYSQL_OPT_WRITE_TIMEOUT, (const char *)&i); }
	if (AG_GetUint(db,"reconnect") == 1)		 { mysql_options(my, MYSQL_OPT_RECONNECT, (const char *)&b); }

	if ((s = AG_GetStringP(db,"charset")) != NULL)	 { mysql_options(my, MYSQL_SET_CHARSET_NAME, s); }
	if ((s = AG_GetStringP(db,"charset-dir")) != NULL) { mysql_options(my, MYSQL_SET_CHARSET_DIR, s); }

	db->my = mysql_real_connect(my,
	    AG_GetStringP(my,"host"),
	    AG_GetStringP(my,"user"),
	    AG_GetStringP(my,"pass"),
	    dbName,
	    AG_GetInt(my,"port"),
	    AG_GetStringP(my,"unix-socket"),
	    myFlags);
	if (db->my == NULL) {
		AG_SetError("MySQL: %s", mysql_error(my));
		mysql_close(my);
		return (-1);
	}
	return (0);
}

static void
Close(void *_Nonnull obj)
{
	AG_DbMySQL *db = obj;

	mysql_close(db->my);
	db->my = NULL;
}

static __inline__ char *_Nonnull
EncodeKey(const AG_Dbt *_Nonnull key)
{
	/* XXX: TODO */
	return Strdup(key->data);
}

static int
Exists(void *_Nonnull obj, const AG_Dbt *_Nonnull key)
{
	char q[64];
	AG_DbMySQL *db = obj;
	char *ks, *q;

	ks = EncodeKey(key);
	Snprintf(q, sizeof(q), AG_GetStringP(db,"get-cmd"), ks);
	free(ks);

	if (mysql_query(db->my, q) != 0) {
		AG_SetError("Get: %s", mysql_error(db->my));
		return (-1);
	}
	free(q);
	return (mysql_field_count(db->my) > 0) ? 1 : 0;
}

static int
Get(void *_Nonnull obj, const AG_Dbt *_Nonnull key, AG_Dbt *_Nonnull val)
{
	return (-1);
}
		
static int
Put(void *_Nonnull obj, const AG_Dbt *_Nonnull key, const AG_Dbt *_Nonnull val)
{
	return (-1);
}

static int
Del(void *_Nonnull obj, const AG_Dbt *_Nonnull key)
{
	return (-1);
}

static int
Iterate(void *_Nonnull obj, AG_DbIterateFn fn, void *_Nullable arg)
{
	return (-1);
}

AG_DbClass agDbMySQLClass = {
	{
		"AG_Db:AG_DbMySQL",
		sizeof(AG_DbMySQL),
		{ 0,0 },
		Init,
		NULL,		/* free */
		NULL,		/* destroy */
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	"mysql",
	N_("MySQL database interface"),
	AG_DB_KEY_DATA,		/* Key is variable data */
	AG_DB_REC_VARIABLE,	/* Variable-sized records */
	Open,
	Close,
	NULL,			/* sync */
	Exists,
	Get,
	Put,
	Del,
	Iterate
};
