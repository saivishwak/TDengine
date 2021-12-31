/*
 * Copyright (c) 2019 TAOS Data, Inc. <jhtao@taosdata.com>
 *
 * This program is free software: you can use, redistribute, and/or modify
 * it under the terms of the GNU Affero General Public License, version 3
 * or later ("AGPL"), as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _TD_COMMON_TAOS_MSG_H_
#define _TD_COMMON_TAOS_MSG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "encode.h"
#include "taosdef.h"
#include "taoserror.h"
#include "tarray.h"
#include "tcoding.h"
#include "tdataformat.h"
#include "tlist.h"

/* ------------------------ MESSAGE DEFINITIONS ------------------------ */
#define TD_MSG_NUMBER_
#undef TD_MSG_DICT_
#undef TD_MSG_INFO_
#undef TD_MSG_SEG_CODE_
#include "tmsgdef.h"

#undef TD_MSG_NUMBER_
#undef TD_MSG_DICT_
#undef TD_MSG_INFO_
#define TD_MSG_SEG_CODE_
#include "tmsgdef.h"

#undef TD_MSG_NUMBER_
#undef TD_MSG_DICT_
#undef TD_MSG_INFO_
#undef TD_MSG_SEG_CODE_
#include "tmsgdef.h"

extern char* tMsgInfo[];
extern int   tMsgDict[];

#define TMSG_SEG_CODE(TYPE) (((TYPE)&0xff00) >> 8)
#define TMSG_SEG_SEQ(TYPE) ((TYPE)&0xff)
#define TMSG_INFO(TYPE) tMsgInfo[tMsgDict[TMSG_SEG_CODE(TYPE)] + TMSG_SEG_SEQ(TYPE)]
#define TMSG_INDEX(TYPE) (tMsgDict[TMSG_SEG_CODE(TYPE)] + TMSG_SEG_SEQ(TYPE))

typedef uint16_t tmsg_t;

/* ------------------------ ENCODE/DECODE FUNCTIONS AND MACROS ------------------------ */
struct SMEListNode {
  TD_SLIST_NODE(SMEListNode);
  SEncoder coder;
};

typedef struct SMsgEncoder {
  SEncoder coder;
  TD_SLIST(SMEListNode) eStack;  // encode stack
} SMsgEncoder;

struct SMDFreeListNode {
  TD_SLIST_NODE(SMDFreeListNode);
  char payload[];
};

struct SMDListNode {
  TD_SLIST_NODE(SMDListNode);
  SDecoder coder;
};

typedef struct SMsgDecoder {
  SDecoder coder;
  TD_SLIST(SMDListNode) dStack;
  TD_SLIST(SMDFreeListNode) freeList;
} SMsgDecoder;

#define TMSG_MALLOC(SIZE, DECODER)                                         \
  ({                                                                       \
    void* ptr = malloc((SIZE) + sizeof(struct SMDFreeListNode));           \
    if (ptr) {                                                             \
      TD_SLIST_PUSH(&((DECODER)->freeList), (struct SMDFreeListNode*)ptr); \
      ptr = POINTER_SHIFT(ptr, sizeof(struct SMDFreeListNode*));           \
    }                                                                      \
    ptr;                                                                   \
  })

void tmsgInitMsgDecoder(SMsgDecoder* pMD, td_endian_t endian, uint8_t* data, int64_t size);
void tmsgClearMsgDecoder(SMsgDecoder* pMD);

/* ------------------------ OTHER DEFINITIONS ------------------------ */
// IE type
#define TSDB_IE_TYPE_SEC 1
#define TSDB_IE_TYPE_META 2
#define TSDB_IE_TYPE_MGMT_IP 3
#define TSDB_IE_TYPE_DNODE_CFG 4
#define TSDB_IE_TYPE_NEW_VERSION 5
#define TSDB_IE_TYPE_DNODE_EXT 6
#define TSDB_IE_TYPE_DNODE_STATE 7

typedef enum _mgmt_table {
  TSDB_MGMT_TABLE_START,
  TSDB_MGMT_TABLE_ACCT,
  TSDB_MGMT_TABLE_USER,
  TSDB_MGMT_TABLE_DB,
  TSDB_MGMT_TABLE_TABLE,
  TSDB_MGMT_TABLE_DNODE,
  TSDB_MGMT_TABLE_MNODE,
  TSDB_MGMT_TABLE_QNODE,
  TSDB_MGMT_TABLE_SNODE,
  TSDB_MGMT_TABLE_BNODE,
  TSDB_MGMT_TABLE_VGROUP,
  TSDB_MGMT_TABLE_STB,
  TSDB_MGMT_TABLE_MODULE,
  TSDB_MGMT_TABLE_QUERIES,
  TSDB_MGMT_TABLE_STREAMS,
  TSDB_MGMT_TABLE_VARIABLES,
  TSDB_MGMT_TABLE_CONNS,
  TSDB_MGMT_TABLE_SCORES,
  TSDB_MGMT_TABLE_GRANTS,
  TSDB_MGMT_TABLE_VNODES,
  TSDB_MGMT_TABLE_CLUSTER,
  TSDB_MGMT_TABLE_STREAMTABLES,
  TSDB_MGMT_TABLE_TP,
  TSDB_MGMT_TABLE_FUNCTION,
  TSDB_MGMT_TABLE_MAX,
} EShowType;

#define TSDB_ALTER_TABLE_ADD_TAG_COLUMN 1
#define TSDB_ALTER_TABLE_DROP_TAG_COLUMN 2
#define TSDB_ALTER_TABLE_CHANGE_TAG_COLUMN 3
#define TSDB_ALTER_TABLE_UPDATE_TAG_VAL 4

#define TSDB_ALTER_TABLE_ADD_COLUMN 5
#define TSDB_ALTER_TABLE_DROP_COLUMN 6
#define TSDB_ALTER_TABLE_CHANGE_COLUMN 7
#define TSDB_ALTER_TABLE_MODIFY_TAG_COLUMN 8

#define TSDB_FILL_NONE 0
#define TSDB_FILL_NULL 1
#define TSDB_FILL_SET_VALUE 2
#define TSDB_FILL_LINEAR 3
#define TSDB_FILL_PREV 4
#define TSDB_FILL_NEXT 5

#define TSDB_ALTER_USER_PASSWD 0x1
#define TSDB_ALTER_USER_PRIVILEGES 0x2

#define TSDB_KILL_MSG_LEN 30

#define TSDB_VN_READ_ACCCESS ((char)0x1)
#define TSDB_VN_WRITE_ACCCESS ((char)0x2)
#define TSDB_VN_ALL_ACCCESS (TSDB_VN_READ_ACCCESS | TSDB_VN_WRITE_ACCCESS)

#define TSDB_COL_NORMAL 0x0u  // the normal column of the table
#define TSDB_COL_TAG 0x1u     // the tag column type
#define TSDB_COL_UDC 0x2u     // the user specified normal string column, it is a dummy column
#define TSDB_COL_TMP 0x4u     // internal column generated by the previous operators
#define TSDB_COL_NULL 0x8u    // the column filter NULL or not

#define TSDB_COL_IS_TAG(f) (((f & (~(TSDB_COL_NULL))) & TSDB_COL_TAG) != 0)
#define TSDB_COL_IS_NORMAL_COL(f) ((f & (~(TSDB_COL_NULL))) == TSDB_COL_NORMAL)
#define TSDB_COL_IS_UD_COL(f) ((f & (~(TSDB_COL_NULL))) == TSDB_COL_UDC)
#define TSDB_COL_REQ_NULL(f) (((f)&TSDB_COL_NULL) != 0)

typedef struct SBuildTableMetaInput {
  int32_t vgId;
  char*   tableFullName;
} SBuildTableMetaInput;

typedef struct SBuildUseDBInput {
  char    db[TSDB_TABLE_FNAME_LEN];
  int32_t vgVersion;
} SBuildUseDBInput;

#pragma pack(push, 1)

// null-terminated string instead of char array to avoid too many memory consumption in case of more than 1M tableMeta
typedef struct {
  char     fqdn[TSDB_FQDN_LEN];
  uint16_t port;
} SEpAddrMsg;

typedef struct {
  char     fqdn[TSDB_FQDN_LEN];
  uint16_t port;
} SEpAddr;

typedef struct {
  int32_t numOfVnodes;
} SMsgDesc;

typedef struct SMsgHead {
  int32_t contLen;
  int32_t vgId;
} SMsgHead;

// Submit message for one table
typedef struct SSubmitBlk {
  uint64_t uid;        // table unique id
  int32_t  tid;        // table id
  int32_t  padding;    // TODO just for padding here
  int32_t  sversion;   // data schema version
  int32_t  dataLen;    // data part length, not including the SSubmitBlk head
  int32_t  schemaLen;  // schema length, if length is 0, no schema exists
  int16_t  numOfRows;  // total number of rows in current submit block
  char     data[];
} SSubmitBlk;

// Submit message for this TSDB
typedef struct SSubmitMsg {
  SMsgHead header;
  int32_t  length;
  int32_t  numOfBlocks;
  char     blocks[];
} SSubmitMsg;

typedef struct {
  int32_t index;  // index of failed block in submit blocks
  int32_t vnode;  // vnode index of failed block
  int32_t sid;    // table index of failed block
  int32_t code;   // errorcode while write data to vnode, such as not created, dropped, no space, invalid table
} SShellSubmitRspBlock;

typedef struct {
  int32_t              code;          // 0-success, > 0 error code
  int32_t              numOfRows;     // number of records the client is trying to write
  int32_t              affectedRows;  // number of records actually written
  int32_t              failedRows;    // number of failed records (exclude duplicate records)
  int32_t              numOfFailedBlocks;
  SShellSubmitRspBlock failedBlocks[];
} SShellSubmitRspMsg;

typedef struct SSchema {
  int8_t  type;
  int32_t colId;
  int32_t bytes;
  char    name[TSDB_COL_NAME_LEN];
} SSchema;

typedef struct {
  int32_t  contLen;
  int32_t  vgId;
  int8_t   tableType;
  int16_t  numOfColumns;
  int16_t  numOfTags;
  int32_t  tid;
  int32_t  sversion;
  int32_t  tversion;
  int32_t  tagDataLen;
  int32_t  sqlDataLen;
  uint64_t uid;
  uint64_t superTableUid;
  uint64_t createdTime;
  char     tableFname[TSDB_TABLE_FNAME_LEN];
  char     stbFname[TSDB_TABLE_FNAME_LEN];
  char     data[];
} SMDCreateTableMsg;

// typedef struct {
//  int32_t len;  // one create table message
//  char    tableName[TSDB_TABLE_FNAME_LEN];
//  int16_t numOfColumns;
//  int16_t sqlLen;  // the length of SQL, it starts after schema , sql is a null-terminated string
//  int8_t  igExists;
//  int8_t  rspMeta;
//  int8_t  reserved[16];
//  char    schema[];
//} SCreateTableMsg;

typedef struct {
  char    tableName[TSDB_TABLE_FNAME_LEN];
  int16_t numOfColumns;
  int16_t numOfTags;
  int8_t  igExists;
  int8_t  rspMeta;
  char    schema[];
} SCreateCTableMsg;

typedef struct {
  char    name[TSDB_TABLE_FNAME_LEN];
  int8_t  igExists;
  int32_t numOfTags;
  int32_t numOfColumns;
  SSchema pSchema[];
} SCreateStbMsg, SCreateTableMsg;

typedef struct {
  char   name[TSDB_TABLE_FNAME_LEN];
  int8_t igNotExists;
} SDropStbMsg;

typedef struct {
  char    name[TSDB_TABLE_FNAME_LEN];
  int8_t  alterType;
  SSchema schema;
} SAlterStbMsg;

typedef struct {
  SMsgHead head;
  char     name[TSDB_TABLE_FNAME_LEN];
  uint64_t suid;
} SVDropStbReq;

typedef struct {
  SMsgHead head;
  char     name[TSDB_TABLE_FNAME_LEN];
  int8_t   type;      /* operation type   */
  int32_t  numOfCols; /* number of schema */
  int32_t  numOfTags;
  char     data[];
} SAlterTableMsg;

typedef struct {
  SMsgHead head;
  char     name[TSDB_TABLE_FNAME_LEN];
  int8_t   ignoreNotExists;
} SDropTableMsg;

typedef struct {
  SMsgHead head;
  int64_t  uid;
  int32_t  tid;
  int16_t  tversion;
  int16_t  colId;
  int8_t   type;
  int16_t  bytes;
  int32_t  tagValLen;
  int16_t  numOfTags;
  int32_t  schemaLen;
  char     data[];
} SUpdateTableTagValMsg;

typedef struct {
  int32_t pid;
  char    app[TSDB_APP_NAME_LEN];
  char    db[TSDB_DB_NAME_LEN];
  int64_t startTime;
} SConnectMsg;

typedef struct SEpSet {
  int8_t   inUse;
  int8_t   numOfEps;
  uint16_t port[TSDB_MAX_REPLICA];
  char     fqdn[TSDB_MAX_REPLICA][TSDB_FQDN_LEN];
} SEpSet;

static FORCE_INLINE int taosEncodeSEpSet(void** buf, const SEpSet* pEp) {
  if (buf == NULL) return sizeof(SEpSet);
  memcpy(buf, pEp, sizeof(SEpSet));
  // TODO: endian conversion
  return sizeof(SEpSet);
}

static FORCE_INLINE void* taosDecodeSEpSet(void* buf, SEpSet* pEpSet) {
  memcpy(pEpSet, buf, sizeof(SEpSet));
  return buf;
}

typedef struct {
  int32_t acctId;
  int64_t clusterId;
  int32_t connId;
  int8_t  superUser;
  int8_t  reserved[5];
  SEpSet  epSet;
} SConnectRsp;

typedef struct {
  char    user[TSDB_USER_LEN];
  char    pass[TSDB_PASSWORD_LEN];
  int32_t maxUsers;
  int32_t maxDbs;
  int32_t maxTimeSeries;
  int32_t maxStreams;
  int32_t accessState;  // Configured only by command
  int64_t maxStorage;   // In unit of GB
  int32_t reserve[8];
} SCreateAcctMsg, SAlterAcctMsg;

typedef struct {
  char    user[TSDB_USER_LEN];
  int32_t reserve[8];
} SDropUserMsg, SDropAcctMsg;

typedef struct {
  int8_t  type;
  char    user[TSDB_USER_LEN];
  char    pass[TSDB_PASSWORD_LEN];
  int8_t  superUser;  // denote if it is a super user or not
  int32_t reserve[8];
} SCreateUserMsg, SAlterUserMsg;

typedef struct {
  int32_t  contLen;
  int32_t  vgId;
  int32_t  tid;
  uint64_t uid;
  char     tableFname[TSDB_TABLE_FNAME_LEN];
} SMDDropTableMsg;

typedef struct {
  int32_t  contLen;
  int32_t  vgId;
  uint64_t uid;
  char     tableFname[TSDB_TABLE_FNAME_LEN];
} SDropSTableMsg;

typedef struct SColIndex {
  int16_t colId;     // column id
  int16_t colIndex;  // column index in colList if it is a normal column or index in tagColList if a tag
  int16_t flag;      // denote if it is a tag or a normal column
  char    name[TSDB_DB_FNAME_LEN];
} SColIndex;

typedef struct SColumnFilterInfo {
  int16_t lowerRelOptr;
  int16_t upperRelOptr;
  int16_t filterstr;  // denote if current column is char(binary/nchar)

  union {
    struct {
      int64_t lowerBndi;
      int64_t upperBndi;
    };
    struct {
      double lowerBndd;
      double upperBndd;
    };
    struct {
      int64_t pz;
      int64_t len;
    };
  };
} SColumnFilterInfo;

typedef struct SColumnFilterList {
  int16_t numOfFilters;
  union {
    int64_t            placeholder;
    SColumnFilterInfo* filterInfo;
  };
} SColumnFilterList;
/*
 * for client side struct, we only need the column id, type, bytes are not necessary
 * But for data in vnode side, we need all the following information.
 */
typedef struct SColumnInfo {
  int16_t           colId;
  int16_t           type;
  int16_t           bytes;
  SColumnFilterList flist;
} SColumnInfo;

typedef struct STableIdInfo {
  uint64_t uid;
  TSKEY    key;  // last accessed ts, for subscription
} STableIdInfo;

typedef struct STimeWindow {
  TSKEY skey;
  TSKEY ekey;
} STimeWindow;

typedef struct {
  int32_t tsOffset;       // offset value in current msg body, NOTE: ts list is compressed
  int32_t tsLen;          // total length of ts comp block
  int32_t tsNumOfBlocks;  // ts comp block numbers
  int32_t tsOrder;        // ts comp block order
} STsBufInfo;

typedef struct SInterval {
  int32_t tz;  // query client timezone
  char    intervalUnit;
  char    slidingUnit;
  char    offsetUnit;
  int64_t interval;
  int64_t sliding;
  int64_t offset;
} SInterval;

typedef struct {
  SMsgHead head;
  char     version[TSDB_VERSION_LEN];

  bool stableQuery;        // super table query or not
  bool topBotQuery;        // TODO used bitwise flag
  bool interpQuery;        // interp query or not
  bool groupbyColumn;      // denote if this is a groupby normal column query
  bool hasTagResults;      // if there are tag values in final result or not
  bool timeWindowInterpo;  // if the time window start/end required interpolation
  bool queryBlockDist;     // if query data block distribution
  bool stabledev;          // super table stddev query
  bool tsCompQuery;        // is tscomp query
  bool simpleAgg;
  bool pointInterpQuery;  // point interpolation query
  bool needReverseScan;   // need reverse scan
  bool stateWindow;       // state window flag

  STimeWindow window;
  int32_t     numOfTables;
  int16_t     order;
  int16_t     orderColId;
  int16_t     numOfCols;  // the number of columns will be load from vnode
  SInterval   interval;
  //  SSessionWindow sw;            // session window
  int16_t tagCondLen;      // tag length in current query
  int16_t colCondLen;      // column length in current query
  int16_t numOfGroupCols;  // num of group by columns
  int16_t orderByIdx;
  int16_t orderType;    // used in group by xx order by xxx
  int64_t vgroupLimit;  // limit the number of rows for each table, used in order by + limit in stable projection query.
  int16_t prjOrder;     // global order in super table projection query.
  int64_t limit;
  int64_t offset;
  int32_t queryType;    // denote another query process
  int16_t numOfOutput;  // final output columns numbers
  int16_t fillType;     // interpolate type
  int64_t fillVal;      // default value array list
  int32_t secondStageOutput;
  STsBufInfo  tsBuf;          // tsBuf info
  int32_t     numOfTags;      // number of tags columns involved
  int32_t     sqlstrLen;      // sql query string
  int32_t     prevResultLen;  // previous result length
  int32_t     numOfOperator;
  int32_t     tableScanOperator;  // table scan operator. -1 means no scan operator
  int32_t     udfNum;             // number of udf function
  int32_t     udfContentOffset;
  int32_t     udfContentLen;
  SColumnInfo tableCols[];
} SQueryTableMsg;

typedef struct {
  int32_t code;
} SQueryTableRsp;

// todo: the show handle should be replaced with id
typedef struct {
  SMsgHead header;
  union {
    int64_t showId;
    int64_t qhandle;
    int64_t qId;
  };  // query handle
  int8_t free;
} SRetrieveTableMsg;

typedef struct SRetrieveTableRsp {
  int64_t useconds;
  int8_t  completed;  // all results are returned to client
  int8_t  precision;
  int8_t  compressed;
  int32_t compLen;

  int32_t numOfRows;
  char    data[];
} SRetrieveTableRsp;

typedef struct {
  char    db[TSDB_DB_FNAME_LEN];
  int32_t numOfVgroups;
  int32_t cacheBlockSize;  // MB
  int32_t totalBlocks;
  int32_t daysPerFile;
  int32_t daysToKeep0;
  int32_t daysToKeep1;
  int32_t daysToKeep2;
  int32_t minRows;
  int32_t maxRows;
  int32_t commitTime;
  int32_t fsyncPeriod;
  int8_t  walLevel;
  int8_t  precision;  // time resolution
  int8_t  compression;
  int8_t  replications;
  int8_t  quorum;
  int8_t  update;
  int8_t  cacheLastRow;
  int8_t  ignoreExist;
  int32_t reserve[8];
} SCreateDbMsg;

typedef struct {
  char    db[TSDB_DB_FNAME_LEN];
  int32_t totalBlocks;
  int32_t daysToKeep0;
  int32_t daysToKeep1;
  int32_t daysToKeep2;
  int32_t fsyncPeriod;
  int8_t  walLevel;
  int8_t  quorum;
  int8_t  cacheLastRow;
  int32_t reserve[8];
} SAlterDbMsg;

typedef struct {
  char    db[TSDB_TABLE_FNAME_LEN];
  int8_t  ignoreNotExists;
  int32_t reserve[8];
} SDropDbMsg;

typedef struct {
  char    db[TSDB_TABLE_FNAME_LEN];
  int32_t vgVersion;
  int32_t reserve[8];
} SUseDbMsg;

typedef struct {
  char    db[TSDB_TABLE_FNAME_LEN];
  int32_t reserve[8];
} SSyncDbMsg;

typedef struct {
  char    db[TSDB_TABLE_FNAME_LEN];
  int32_t reserve[8];
} SCompactDbMsg;

typedef struct {
  char    name[TSDB_FUNC_NAME_LEN];
  int8_t  funcType;
  int8_t  scriptType;
  int8_t  align;
  int8_t  outputType;
  int32_t outputLen;
  int32_t bufSize;
  int64_t sigature;
  int32_t commentSize;
  int32_t codeSize;
  char    pCont[];
} SCreateFuncMsg;

typedef struct {
  char name[TSDB_FUNC_NAME_LEN];
} SDropFuncMsg;

typedef struct {
  int32_t numOfFuncs;
  char    pFuncNames[];
} SRetrieveFuncMsg;

typedef struct {
  char    name[TSDB_FUNC_NAME_LEN];
  int8_t  funcType;
  int8_t  scriptType;
  int8_t  align;
  int8_t  outputType;
  int32_t outputLen;
  int32_t bufSize;
  int64_t sigature;
  int32_t commentSize;
  int32_t codeSize;
  char    pCont[];
} SFuncInfo;

typedef struct {
  int32_t numOfFuncs;
  char    pFuncInfos[];
} SRetrieveFuncRsp;

typedef struct {
  int32_t statusInterval;
  int64_t checkTime;                    // 1970-01-01 00:00:00.000
  char    timezone[TSDB_TIMEZONE_LEN];  // tsTimezone
  char    locale[TSDB_LOCALE_LEN];      // tsLocale
  char    charset[TSDB_LOCALE_LEN];     // tsCharset
} SClusterCfg;

typedef struct {
  int32_t vgId;
  int8_t  role;
  int8_t  reserved[3];
  int64_t totalStorage;
  int64_t compStorage;
  int64_t pointsWritten;
  int64_t tablesNum;
} SVnodeLoad;

typedef struct {
  int32_t    num;
  SVnodeLoad data[];
} SVnodeLoads;

typedef struct {
  int32_t     sver;
  int32_t     dnodeId;
  int64_t     clusterId;
  int64_t     rebootTime;
  int64_t     updateTime;
  int32_t     numOfCores;
  int32_t     numOfSupportVnodes;
  char        dnodeEp[TSDB_EP_LEN];
  SClusterCfg clusterCfg;
  SVnodeLoads vnodeLoads;
} SStatusMsg;

typedef struct {
  int32_t reserved;
} STransMsg;

typedef struct {
  int32_t dnodeId;
  int64_t clusterId;
} SDnodeCfg;

typedef struct {
  int32_t  id;
  int8_t   isMnode;
  int8_t   reserved;
  uint16_t port;
  char     fqdn[TSDB_FQDN_LEN];
} SDnodeEp;

typedef struct {
  int32_t  num;
  SDnodeEp eps[];
} SDnodeEps;

typedef struct {
  SDnodeCfg dnodeCfg;
  SDnodeEps dnodeEps;
} SStatusRsp;

typedef struct {
  int32_t  id;
  uint16_t port;                 // node sync Port
  char     fqdn[TSDB_FQDN_LEN];  // node FQDN
} SReplica;

typedef struct {
  int32_t  vgId;
  int32_t  dnodeId;
  char     db[TSDB_DB_FNAME_LEN];
  uint64_t dbUid;
  int32_t  vgVersion;
  int32_t  cacheBlockSize;
  int32_t  totalBlocks;
  int32_t  daysPerFile;
  int32_t  daysToKeep0;
  int32_t  daysToKeep1;
  int32_t  daysToKeep2;
  int32_t  minRows;
  int32_t  maxRows;
  int32_t  commitTime;
  int32_t  fsyncPeriod;
  int8_t   walLevel;
  int8_t   precision;
  int8_t   compression;
  int8_t   quorum;
  int8_t   update;
  int8_t   cacheLastRow;
  int8_t   replica;
  int8_t   selfIndex;
  SReplica replicas[TSDB_MAX_REPLICA];
} SCreateVnodeMsg, SAlterVnodeMsg;

typedef struct {
  int32_t  vgId;
  int32_t  dnodeId;
  char     db[TSDB_DB_FNAME_LEN];
  uint64_t dbUid;
} SDropVnodeMsg, SSyncVnodeMsg, SCompactVnodeMsg;

typedef struct {
  int32_t vgId;
  int8_t  accessState;
} SAuthVnodeMsg;

typedef struct {
  int32_t vgId;
  char    tableFname[TSDB_TABLE_FNAME_LEN];
} STableInfoMsg;

typedef struct {
  int8_t  metaClone;  // create local clone of the cached table meta
  int32_t numOfVgroups;
  int32_t numOfTables;
  int32_t numOfUdfs;
  char    tableNames[];
} SMultiTableInfoMsg;

typedef struct SVgroupInfo {
  int32_t    vgId;
  uint32_t   hashBegin;
  uint32_t   hashEnd;
  int8_t     inUse;
  int8_t     numOfEps;
  SEpAddrMsg epAddr[TSDB_MAX_REPLICA];
} SVgroupInfo;

typedef struct {
  int32_t    vgId;
  int8_t     numOfEps;
  SEpAddrMsg epAddr[TSDB_MAX_REPLICA];
} SVgroupMsg;

typedef struct {
  int32_t    numOfVgroups;
  SVgroupMsg vgroups[];
} SVgroupsMsg, SVgroupsInfo;

typedef struct {
  char     tbFname[TSDB_TABLE_FNAME_LEN];  // table full name
  char     stbFname[TSDB_TABLE_FNAME_LEN];
  int32_t  numOfTags;
  int32_t  numOfColumns;
  int8_t   precision;
  int8_t   tableType;
  int8_t   update;
  int32_t  sversion;
  int32_t  tversion;
  uint64_t suid;
  uint64_t tuid;
  int32_t  vgId;
  SSchema  pSchema[];
} STableMetaMsg;

typedef struct SMultiTableMeta {
  int32_t numOfTables;
  int32_t numOfVgroup;
  int32_t numOfUdf;
  int32_t contLen;
  int8_t  compressed;  // denote if compressed or not
  int32_t rawLen;      // size before compress
  uint8_t metaClone;   // make meta clone after retrieve meta from mnode
  char    meta[];
} SMultiTableMeta;

typedef struct {
  int32_t dataLen;
  char    name[TSDB_TABLE_FNAME_LEN];
  char*   data;
} STagData;

typedef struct {
  char        db[TSDB_DB_FNAME_LEN];
  int32_t     vgVersion;
  int32_t     vgNum;
  int8_t      hashMethod;
  SVgroupInfo vgroupInfo[];
} SUseDbRsp;

/*
 * sql: show tables like '%a_%'
 * payload is the query condition, e.g., '%a_%'
 * payloadLen is the length of payload
 */
typedef struct {
  int8_t  type;
  char    db[TSDB_DB_FNAME_LEN];
  int16_t payloadLen;
  char    payload[];
} SShowMsg;

typedef struct {
  char    db[TSDB_DB_FNAME_LEN];
  int32_t numOfVgroup;
  int32_t vgid[];
} SCompactMsg;

typedef struct SShowRsp {
  int64_t       showId;
  STableMetaMsg tableMeta;
} SShowRsp;

typedef struct {
  char    fqdn[TSDB_FQDN_LEN];  // end point, hostname:port
  int32_t port;
} SCreateDnodeMsg;

typedef struct {
  int32_t dnodeId;
} SDropDnodeMsg;

typedef struct {
  int32_t dnodeId;
  char    config[TSDB_DNODE_CONFIG_LEN];
} SCfgDnodeMsg;

typedef struct {
  int32_t dnodeId;
} SMCreateMnodeMsg, SMDropMnodeMsg, SDDropMnodeMsg;

typedef struct {
  int32_t  dnodeId;
  int8_t   replica;
  SReplica replicas[TSDB_MAX_REPLICA];
} SDCreateMnodeMsg, SDAlterMnodeMsg;

typedef struct {
  int32_t dnodeId;
} SMCreateQnodeMsg, SMDropQnodeMsg, SDCreateQnodeMsg, SDDropQnodeMsg;

typedef struct {
  int32_t dnodeId;
} SMCreateSnodeMsg, SMDropSnodeMsg, SDCreateSnodeMsg, SDDropSnodeMsg;

typedef struct {
  int32_t dnodeId;
} SMCreateBnodeMsg, SMDropBnodeMsg, SDCreateBnodeMsg, SDDropBnodeMsg;

typedef struct {
  int32_t dnodeId;
  int32_t vgId;
  int32_t tid;
} SConfigTableMsg;

typedef struct {
  int32_t dnodeId;
  int32_t vgId;
} SConfigVnodeMsg;

typedef struct {
  char    sql[TSDB_SHOW_SQL_LEN];
  int32_t queryId;
  int64_t useconds;
  int64_t stime;
  int64_t qId;
  int64_t sqlObjId;
  int32_t pid;
  char    fqdn[TSDB_FQDN_LEN];
  int8_t  stableQuery;
  int32_t numOfSub;
  char    subSqlInfo[TSDB_SHOW_SUBQUERY_LEN];  // include subqueries' index, Obj IDs and states(C-complete/I-imcomplete)
} SQueryDesc;

typedef struct {
  int32_t connId;
  int32_t pid;
  int32_t numOfQueries;
  int32_t numOfStreams;
  char    app[TSDB_APP_NAME_LEN];
  char    pData[];
} SHeartBeatMsg;

typedef struct {
  int32_t connId;
  int32_t queryId;
  int32_t streamId;
  int32_t totalDnodes;
  int32_t onlineDnodes;
  int8_t  killConnection;
  int8_t  reserved[3];
  SEpSet  epSet;
} SHeartBeatRsp;

typedef struct {
  int32_t connId;
  int32_t streamId;
} SKillStreamMsg;

typedef struct {
  int32_t connId;
  int32_t queryId;
} SKillQueryMsg;

typedef struct {
  int32_t connId;
} SKillConnMsg;

typedef struct {
  char user[TSDB_USER_LEN];
  char spi;
  char encrypt;
  char secret[TSDB_PASSWORD_LEN];
  char ckey[TSDB_PASSWORD_LEN];
} SAuthMsg, SAuthRsp;

typedef struct {
  int8_t finished;
  int8_t reserved1[7];
  char   name[TSDB_STEP_NAME_LEN];
  char   desc[TSDB_STEP_DESC_LEN];
} SStartupMsg;

// mq related
typedef struct {
} SMqConnectReq;

typedef struct {
} SMqConnectRsp;

typedef struct {
} SMqDisconnectReq;

typedef struct {
} SMqDisconnectRsp;

typedef struct {
} SMqAckReq;

typedef struct {
} SMqAckRsp;

typedef struct {
} SMqResetReq;

typedef struct {
} SMqResetRsp;
// mq related end

typedef struct {
  /* data */
} SSubmitReq;

typedef struct {
  /* data */
} SSubmitRsp;

typedef struct {
  /* data */
} SSubmitReqReader;

typedef struct {
  /* data */
} SCreateTableReq;

typedef struct {
  /* data */
} SCreateTableRsp;

typedef struct {
  /* data */
} SDropTableReq;

typedef struct {
  /* data */
} SDropTableRsp;

typedef struct {
  /* data */
} SAlterTableReq;

typedef struct {
  /* data */
} SAlterTableRsp;

typedef struct {
  /* data */
} SDropStableReq;

typedef struct {
  /* data */
} SDropStableRsp;

typedef struct {
  /* data */
} SUpdateTagValReq;

typedef struct {
  /* data */
} SUpdateTagValRsp;

typedef struct SSubQueryMsg {
  uint64_t sId;
  uint64_t queryId;
  uint64_t taskId;
  uint32_t contentLen;
  char     msg[];
} SSubQueryMsg;

typedef struct SResReadyMsg {
  uint64_t sId;
  uint64_t queryId;
  uint64_t taskId;
} SResReadyMsg;

typedef struct SResReadyRsp {
  int32_t code;
} SResReadyRsp;

typedef struct SResFetchMsg {
  uint64_t sId;
  uint64_t queryId;
  uint64_t taskId;
} SResFetchMsg;

typedef struct SSchTasksStatusMsg {
  uint64_t sId;
} SSchTasksStatusMsg;

typedef struct STaskStatus {
  uint64_t queryId;
  uint64_t taskId;
  int8_t   status;
} STaskStatus;

typedef struct SSchedulerStatusRsp {
  uint32_t    num;
  STaskStatus status[];
} SSchedulerStatusRsp;

typedef struct STaskCancelMsg {
  uint64_t sId;
  uint64_t queryId;
  uint64_t taskId;
} STaskCancelMsg;

typedef struct STaskCancelRsp {
  int32_t code;
} STaskCancelRsp;

typedef struct STaskDropMsg {
  uint64_t sId;
  uint64_t queryId;
  uint64_t taskId;
} STaskDropMsg;

typedef struct STaskDropRsp {
  int32_t code;
} STaskDropRsp;

typedef struct {
  int8_t igExists;
  char*  name;
  char*  physicalPlan;
  char*  logicalPlan;
} SCMCreateTopicReq;

static FORCE_INLINE int tSerializeSCMCreateTopicReq(void** buf, const SCMCreateTopicReq* pReq) {
  int tlen = 0;
  tlen += taosEncodeString(buf, pReq->name);
  tlen += taosEncodeFixedI8(buf, pReq->igExists);
  tlen += taosEncodeString(buf, pReq->physicalPlan);
  tlen += taosEncodeString(buf, pReq->logicalPlan);
  return tlen;
}

static FORCE_INLINE void* tDeserializeSCMCreateTopicReq(void* buf, SCMCreateTopicReq* pReq) {
  buf = taosDecodeFixedI8(buf, &(pReq->igExists));
  buf = taosDecodeString(buf, &(pReq->name));
  buf = taosDecodeString(buf, &(pReq->physicalPlan));
  buf = taosDecodeString(buf, &(pReq->logicalPlan));
  return buf;
}

typedef struct {
  int64_t topicId;
} SCMCreateTopicRsp;

static FORCE_INLINE int tSerializeSCMCreateTopicRsp(void** buf, const SCMCreateTopicRsp* pRsp) {
  int tlen = 0;
  tlen += taosEncodeFixedI64(buf, pRsp->topicId);
  return tlen;
}

static FORCE_INLINE void* tDeserializeSCMCreateTopicRsp(void* buf, SCMCreateTopicRsp* pRsp) {
  buf = taosDecodeFixedI64(buf, &pRsp->topicId);
  return buf;
}

typedef struct {
  char*   topicName;
  char*   consumerGroup;
  int64_t consumerId;
} SCMSubscribeReq;

static FORCE_INLINE int tSerializeSCMSubscribeReq(void** buf, const SCMSubscribeReq* pReq) {
  int tlen = 0;
  tlen += taosEncodeString(buf, pReq->topicName);
  tlen += taosEncodeString(buf, pReq->consumerGroup);
  tlen += taosEncodeFixedI64(buf, pReq->consumerId);
  return tlen;
}

static FORCE_INLINE void* tDeserializeSCMSubscribeReq(void* buf, SCMSubscribeReq* pReq) {
  buf = taosDecodeString(buf, &pReq->topicName);
  buf = taosDecodeString(buf, &pReq->consumerGroup);
  buf = taosDecodeFixedI64(buf, &pReq->consumerId);
  return buf;
}

typedef struct {
  int32_t vgId;
  SEpSet  pEpSet;
} SCMSubscribeRsp;

static FORCE_INLINE int tSerializeSCMSubscribeRsp(void** buf, const SCMSubscribeRsp* pRsp) {
  int tlen = 0;
  tlen += taosEncodeFixedI32(buf, pRsp->vgId);
  tlen += taosEncodeSEpSet(buf, &pRsp->pEpSet);
  return tlen;
}

static FORCE_INLINE void* tDeserializeSCMSubscribeRsp(void* buf, SCMSubscribeRsp* pRsp) {
  buf = taosDecodeFixedI32(buf, &pRsp->vgId);
  buf = taosDecodeSEpSet(buf, &pRsp->pEpSet);
  return buf;
}

typedef struct {
  int64_t topicId;
  int64_t consumerId;
  int64_t consumerGroupId;
  int64_t offset;
} SMVSubscribeReq;

typedef struct {
  int64_t newOffset;
} SMVSubscribeRsp;

typedef struct {
  char    name[TSDB_TOPIC_FNAME_LEN];
  int8_t  igExists;
  int32_t execLen;
  void*   executor;
  int32_t sqlLen;
  char*   sql;
} SCreateTopicMsg;

typedef struct {
  char   name[TSDB_TABLE_FNAME_LEN];
  int8_t igNotExists;
} SDropTopicMsg;

typedef struct {
  char    name[TSDB_TABLE_FNAME_LEN];
  int8_t  alterType;
  SSchema schema;
} SAlterTopicMsg;

typedef struct {
  SMsgHead head;
  char     name[TSDB_TABLE_FNAME_LEN];
  uint64_t tuid;
  int32_t  sverson;
  int32_t  execLen;
  char*    executor;
  int32_t  sqlLen;
  char*    sql;
} SDCreateTopicMsg;

typedef struct {
  SMsgHead head;
  char     name[TSDB_TABLE_FNAME_LEN];
  uint64_t tuid;
} SDDropTopicMsg;

typedef struct SVCreateTbReq {
  uint64_t ver;  // use a general definition
  char*    name;
  uint32_t ttl;
  uint32_t keep;
#define TD_SUPER_TABLE 0
#define TD_CHILD_TABLE 1
#define TD_NORMAL_TABLE 2
  uint8_t type;
  union {
    struct {
      tb_uid_t suid;
      uint32_t nCols;
      SSchema* pSchema;
      uint32_t nTagCols;
      SSchema* pTagSchema;
    } stbCfg;
    struct {
      tb_uid_t suid;
      SKVRow   pTag;
    } ctbCfg;
    struct {
      uint32_t nCols;
      SSchema* pSchema;
    } ntbCfg;
  };
} SVCreateTbReq;

typedef struct {
  uint64_t ver;  // use a general definition
  SArray*  pArray;
} SVCreateTbBatchReq;

int   tmsgSVCreateTbReqEncode(SMsgEncoder* pCoder, SVCreateTbReq* pReq);
int   tmsgSVCreateTbReqDecode(SMsgDecoder* pCoder, SVCreateTbReq* pReq);
int   tSerializeSVCreateTbReq(void** buf, const SVCreateTbReq* pReq);
void* tDeserializeSVCreateTbReq(void* buf, SVCreateTbReq* pReq);

typedef struct SVCreateTbRsp {
} SVCreateTbRsp;

typedef struct SVShowTablesReq {
  SMsgHead head;
} SVShowTablesReq;

typedef struct SVShowTablesRsp {
  int64_t       id;
  STableMetaMsg metaInfo;
} SVShowTablesRsp;

typedef struct SVShowTablesFetchReq {
  SMsgHead head;
  int32_t  id;
} SVShowTablesFetchReq;

typedef struct SVShowTablesFetchRsp {
  int64_t useconds;
  int8_t  completed;  // all results are returned to client
  int8_t  precision;
  int8_t  compressed;
  int32_t compLen;

  int32_t numOfRows;
  char    data[];
} SVShowTablesFetchRsp;

#pragma pack(pop)

#ifdef __cplusplus
}
#endif

#endif /*_TD_COMMON_TAOS_MSG_H_*/
