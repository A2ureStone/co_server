#pragma once
#include <string>

namespace redis
{
    /* error code */
    enum class op_status
    {
        redis_ok,
        redis_err
    };

    struct sharedObjectsStruct
    {
        std::string crlf = "\r\n";
        std::string ok = "+OK\r\n";
        std::string err = "-ERR\r\n";
        std::string emptybulk = "$0\r\n\r\n";
        std::string czero = ":0\r\n";
        std::string cone = ":1\r\n";
        std::string nullbulk = "$-1\r\n";
        std::string nullmultibulk = "*-1\r\n";
        std::string emptymultibulk = "*0\r\n";
        /* no such key */
        std::string pong = "+PONG\r\n";
        std::string wrongtypeerr =
            "-ERR Operation against a key holding the wrong kind of value\r\n";
        std::string nokeyerr =
            "-ERR no such key\r\n";
        std::string syntaxerr =
            "-ERR syntax error\r\n";
        std::string sameobjecterr =
            "-ERR source and destination objects are the same\r\n";
        std::string outofrangeerr =
            "-ERR index out of range\r\n";
        std::string space = " ";
        std::string colon = ":";
        std::string plus = "+";
        std::string select0 = "select 0\r\n";
        std::string select1 = "select 1\r\n";
        std::string select2 = "select 2\r\n";
        std::string select3 = "select 3\r\n";
        std::string select4 = "select 4\r\n";
        std::string select5 = "select 5\r\n";
        std::string select6 = "select 6\r\n";
        std::string select7 = "select 7\r\n";
        std::string select8 = "select 8\r\n";
        std::string select9 = "select 9\r\n";
    };

    extern sharedObjectsStruct shared_obj;
}

/* Static server configuration */
#define REDIS_DEFAULT_HZ 10 /* Time interrupt calls/sec. */
#define REDIS_MIN_HZ 1
#define REDIS_MAX_HZ 500
#define REDIS_SERVERPORT 6379 /* TCP port */
#define REDIS_MAXIDLETIME 0   /* default client timeout: infinite */
#define REDIS_DEFAULT_DBNUM 16
#define REDIS_CONFIGLINE_MAX 1024
#define REDIS_DBCRON_DBS_PER_CALL 16
#define REDIS_MAX_WRITE_PER_EVENT (1024 * 64)
#define REDIS_SHARED_SELECT_CMDS 10
#define REDIS_SHARED_INTEGERS 10000
#define REDIS_SHARED_BULKHDR_LEN 32
#define REDIS_MAX_LOGMSG_LEN 1024 /* Default maximum length of syslog messages */
#define REDIS_AOF_REWRITE_PERC 100
#define REDIS_AOF_REWRITE_MIN_SIZE (1024 * 1024)
#define REDIS_AOF_REWRITE_ITEMS_PER_CMD 64
#define REDIS_SLOWLOG_LOG_SLOWER_THAN 10000
#define REDIS_SLOWLOG_MAX_LEN 128
#define REDIS_MAX_CLIENTS 10000
#define REDIS_AUTHPASS_MAX_LEN 512
#define REDIS_DEFAULT_SLAVE_PRIORITY 100
#define REDIS_REPL_TIMEOUT 60
#define REDIS_REPL_PING_SLAVE_PERIOD 10
#define REDIS_RUN_ID_SIZE 40
#define REDIS_OPS_SEC_SAMPLES 16
#define REDIS_BGSAVE_RETRY_DELAY 5 /* Wait a few secs before trying again. */

#define ACTIVE_EXPIRE_CYCLE_LOOKUPS_PER_LOOP 20 /* Loopkups per loop. */
#define ACTIVE_EXPIRE_CYCLE_FAST_DURATION 1000  /* Microseconds */
#define ACTIVE_EXPIRE_CYCLE_SLOW_TIME_PERC 25   /* CPU max % for keys collection */
#define ACTIVE_EXPIRE_CYCLE_SLOW 0
#define ACTIVE_EXPIRE_CYCLE_FAST 1

/* Protocol and I/O related defines */
#define REDIS_MAX_QUERYBUF_LEN (1024 * 1024 * 1024) /* 1GB max query buffer. */
#define REDIS_IOBUF_LEN (1024 * 16)                 /* Generic I/O buffer size */
#define REDIS_REPLY_CHUNK_BYTES (16 * 1024)         /* 16k output buffer */
#define REDIS_INLINE_MAX_SIZE (1024 * 64)           /* Max size of inline reads */
#define REDIS_MBULK_BIG_ARG (1024 * 32)
#define REDIS_AOF_AUTOSYNC_BYTES (1024 * 1024 * 32) /* fdatasync every 32MB */

/* Client flags */
#define REDIS_SLAVE (1 << 0)             /* This client is a slave server */
#define REDIS_MASTER (1 << 1)            /* This client is a master server */
#define REDIS_MONITOR (1 << 2)           /* This client is a slave monitor, see MONITOR */
#define REDIS_MULTI (1 << 3)             /* This client is in a MULTI context */
#define REDIS_BLOCKED (1 << 4)           /* The client is waiting in a blocking operation */
#define REDIS_DIRTY_CAS (1 << 5)         /* Watched keys modified. EXEC will fail. */
#define REDIS_CLOSE_AFTER_REPLY (1 << 6) /* Close after writing entire reply. */
#define REDIS_UNBLOCKED (1 << 7)         /* This client was unblocked and is stored in \
                                            server.unblocked_clients */
#define REDIS_LUA_CLIENT (1 << 8)        /* This is a non connected client used by Lua */
#define REDIS_ASKING (1 << 9)            /* Client issued the ASKING command */
#define REDIS_CLOSE_ASAP (1 << 10)       /* Close this client ASAP */
#define REDIS_UNIX_SOCKET (1 << 11)      /* Client connected via Unix domain socket */
#define REDIS_DIRTY_EXEC (1 << 12)       /* EXEC will fail for errors while queueing */

/* Command flags */
#define REDIS_CMD_BULK 1   /* Bulk write command */
#define REDIS_CMD_INLINE 2 /* Inline command */
/* REDIS_CMD_DENYOOM reserves a longer comment: all the commands marked with
   this flags will return an error when the 'maxmemory' option is set in the
   config file and the server is using more than maxmemory bytes of memory.
   In short this commands are denied on low memory conditions. */
#define REDIS_CMD_DENYOOM 4