//=====================================================================
//
// inetcode.c - core interface of socket operation
//
// NOTE:
// for more information, please see the readme file
// 
//=====================================================================

#ifndef __INETCODE_H__
#define __INETCODE_H__

#include "inetbase.h"
#include "imemdata.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif


//=====================================================================
// Network Information
//=====================================================================
#define IMAX_HOSTNAME	256
#define IMAX_ADDRESS	64

/* host name */
extern char ihostname[];

/* host address list */
extern struct in_addr ihost_addr[];

/* host ip address string list */
extern char *ihost_ipstr[];

/* host names */
extern char *ihost_names[];

/* host address count */
extern int ihost_addr_num;	

/* refresh address list */
int inet_updateaddr(int resolvname);

/* create socketpair */
int inet_socketpair(int fds[2]);


//=====================================================================
// CAsyncSock 
//=====================================================================
struct CAsyncSock
{
	IUINT32 time;					// timeout
	int fd;							// socket fd
	int state;						// CLOSED/CONNECTING/ESTABLISHED
	long hid;						// hid
	long tag;						// tag
	int error;						// errno value
	int header;						// header mode (0-13)
	int mask;						// poll event mask
	int mode;						// socket mode
	int ipv6;						// 0:ipv4, 1:ipv6
	char *buffer;					// internal working buffer
	char *external;					// external working buffer
	long bufsize;					// working buffer size
	long maxsize;					// max packet size
	long limited;					// buffer limited
	int rc4_send_x;					// rc4 encryption variable 
	int rc4_send_y;					// rc4 encryption variable 
	int rc4_recv_x;					// rc4 encryption variable 
	int rc4_recv_y;					// rc4 encryption variable 
	struct IQUEUEHEAD node;			// list node
	struct IMSTREAM sendmsg;		// send buffer
	struct IMSTREAM recvmsg;		// recv buffer
	unsigned char rc4_send_box[256];	
	unsigned char rc4_recv_box[256];
};


#ifndef ITMH_WORDLSB
#define ITMH_WORDLSB		0		// header: 2 bytes LSB
#define ITMH_WORDMSB		1		// header: 2 bytes MSB
#define ITMH_DWORDLSB		2		// header: 4 bytes LSB
#define ITMH_DWORDMSB		3		// header: 4 bytes MSB
#define ITMH_BYTELSB		4		// header: 1 byte LSB
#define ITMH_BYTEMSB		5		// header: 1 byte MSB
#define ITMH_EWORDLSB		6		// header: 2 bytes LSB (exclude self)
#define ITMH_EWORDMSB		7		// header: 2 bytes MSB (exclude self)
#define ITMH_EDWORDLSB		8		// header: 4 bytes LSB (exclude self)
#define ITMH_EDWORDMSB		9		// header: 4 bytes MSB (exclude self)
#define ITMH_EBYTELSB		10		// header: 1 byte LSB (exclude self)
#define ITMH_EBYTEMSB		11		// header: 1 byte MSB (exclude self)
#define ITMH_DWORDMASK		12		// header: 4 bytes LSB (self and mask)
#endif

#define ASYNC_SOCK_STATE_CLOSED			0
#define ASYNC_SOCK_STATE_CONNECTING		1
#define ASYNC_SOCK_STATE_ESTAB			2

typedef struct CAsyncSock CAsyncSock;


// create a new asyncsock
void async_sock_init(CAsyncSock *asyncsock, struct IMEMNODE *nodes);

// delete asyncsock
void async_sock_destroy(CAsyncSock *asyncsock);


// connect to remote address
int async_sock_connect(CAsyncSock *asyncsock, const struct sockaddr *remote,
	int addrlen, int header);

// assign a new socket
int async_sock_assign(CAsyncSock *asyncsock, int sock, int header);

// close socket
void async_sock_close(CAsyncSock *asyncsock);


// get state
int async_sock_state(const CAsyncSock *asyncsock);

// get fd
int async_sock_fd(const CAsyncSock *asyncsock);

// get how many bytes remain in the send buffer
long async_sock_remain(const CAsyncSock *asyncsock);


// send data
long async_sock_send(CAsyncSock *asyncsock, const void *ptr, 
	long size, int mask);

// recv vector: returns packet size, -1 for not enough data, -2 for 
// buffer size too small, -3 for packet size error, -4 for size over limit,
// returns packet size if ptr equals NULL.
long async_sock_recv(CAsyncSock *asyncsock, void *ptr, int size);


// send vector
long async_sock_send_vector(CAsyncSock *asyncsock, const void *vecptr[],
	const long veclen[], int count, int mask);

// recv vector: returns packet size, -1 for not enough data, -2 for 
// buffer size too small, -3 for packet size error, -4 for size over limit,
// returns packet size if vecptr equals NULL.
long async_sock_recv_vector(CAsyncSock *asyncsock, void *vecptr[], 
	const long veclen[], int count);


// update
int async_sock_update(CAsyncSock *asyncsock, int what);

// process
void async_sock_process(CAsyncSock *asyncsock);


// set send cryption key
void async_sock_rc4_set_skey(CAsyncSock *asyncsock, 
	const unsigned char *key, int keylen);

// set recv cryption key
void async_sock_rc4_set_rkey(CAsyncSock *asyncsock, 
	const unsigned char *key, int keylen);

// set nodelay
int async_sock_nodelay(CAsyncSock *asyncsock, int nodelay);

// set buf size
int async_sock_sys_buffer(CAsyncSock *asyncsock, long rcvbuf, long sndbuf);

// set keepalive
int async_sock_keepalive(CAsyncSock *asyncsock, int keepcnt, int keepidle,
	int keepintvl);


//=====================================================================
// CAsyncCore
//=====================================================================
struct CAsyncCore;
typedef struct CAsyncCore CAsyncCore;

#define ASYNC_CORE_EVT_NEW		0	// new: (hid, tag)
#define ASYNC_CORE_EVT_LEAVE	1	// leave: (hid, tag)
#define ASYNC_CORE_EVT_ESTAB	2	// estab: (hid, tag)
#define ASYNC_CORE_EVT_DATA		3	// data: (hid, tag)

#define ASYNC_CORE_NODE_IN			1		// accepted node
#define ASYNC_CORE_NODE_OUT			2		// connected out node
#define ASYNC_CORE_NODE_LISTEN4		3		// ipv4 listener
#define ASYNC_CORE_NODE_LISTEN6		4		// ipv6 listener

// Remote IP Validator: returns 1 to accept it, 0 to reject
typedef int (*CAsyncValidator)(const struct sockaddr *remote, int len,
	CAsyncCore *core, long listenhid, void *user);


// new CAsyncCore object
CAsyncCore* async_core_new(void);

// delete async core
void async_core_delete(CAsyncCore *core);


// wait for events for millisec ms. and process events, 
// if millisec equals zero, no wait.
void async_core_process(CAsyncCore *core, IUINT32 millisec);

// read events, returns data length of the message, 
// and returns -1 for no event, -2 for buffer size too small,
long async_core_read(CAsyncCore *core, int *event, long *wparam,
	long *lparam, void *data, long size);


// send data to given hid
long async_core_send(CAsyncCore *core, long hid, const void *ptr, long len);

// close given hid
int async_core_close(CAsyncCore *core, long hid, int code);

// send vector
long async_core_send_vector(CAsyncCore *core, long hid, const void *vecptr[],
	const long veclen[], int count, int mask);


// new connection to the target address, returns hid
long async_core_new_connect(CAsyncCore *core, const struct sockaddr *addr,
	int addrlen, int header);

// new listener, returns hid
long async_core_new_listen(CAsyncCore *core, const struct sockaddr *addr, 
	int addrlen, int header);



// get node mode: ASYNC_CORE_NODE_IN/OUT/LISTEN4/LISTEN6
int async_core_get_mode(const CAsyncCore *core, long hid);

// returns connection tag, -1 for hid not exist
long async_core_get_tag(const CAsyncCore *core, long hid);

// set connection tag
void async_core_set_tag(CAsyncCore *core, long hid, long tag);

// get send queue size
long async_core_remain(const CAsyncCore *core, long hid);

// set default buffer limit and max packet size
void async_core_limit(CAsyncCore *core, long limited, long maxsize);



// get first node
long async_core_node_head(const CAsyncCore *core);

// get next node
long async_core_node_next(const CAsyncCore *core, long hid);

// get prev node
long async_core_node_prev(const CAsyncCore *core, long hid);


#define ASYNC_CORE_OPTION_NODELAY		1
#define ASYNC_CORE_OPTION_REUSEADDR		2
#define ASYNC_CORE_OPTION_KEEPALIVE		3
#define ASYNC_CORE_OPTION_SYSSNDBUF		4
#define ASYNC_CORE_OPTION_SYSRCVBUF		5
#define ASYNC_CORE_OPTION_LIMITED		6
#define ASYNC_CORE_OPTION_MAXSIZE		7

// set connection socket option
int async_core_option(CAsyncCore *core, long hid, int opt, long value);

// set connection rc4 send key
int async_core_rc4_set_skey(CAsyncCore *core, long hid, 
	const unsigned char *key, int keylen);

// set connection rc4 recv key
int async_core_rc4_set_rkey(CAsyncCore *core, long hid,
	const unsigned char *key, int keylen);

// set remote ip validator
void async_core_firewall(CAsyncCore *core, CAsyncValidator v, void *user);


//=====================================================================
// System Utilities
//=====================================================================

#ifndef IDISABLE_SHARED_LIBRARY

// LoadLibraryA
void *iutils_shared_open(const char *dllname);

// GetProcAddress
void *iutils_shared_get(void *shared, const char *name);

// FreeLibrary
void iutils_shared_close(void *shared);

#endif


#ifndef IDISABLE_FILE_SYSTEM_ACCESS

// load file content
void *iutils_file_load_content(const char *filename, ilong *size);

// load file content
int iutils_file_load_to_str(const char *filename, ivalue_t *str);

// load line: returns -1 for end of file, 0 for success
int iutils_file_read_line(FILE *fp, ivalue_t *str);

// cross os GetModuleFileName, returns size for success, -1 for error
int iutils_get_proc_pathname(char *ptr, int size);

#endif


//---------------------------------------------------------------------
// PROXY
//---------------------------------------------------------------------
struct ISOCKPROXY
{
	int type;					// http? sock4? sock5?
	int next;					// state
	int socket;					// socket
	int offset;					// data pointer offset
	int totald;					// total send
	int authen;					// wheather auth
	int errorc;					// error code
	int block;					// is blocking
	struct sockaddr remote;		// remote address
	struct sockaddr proxyd;		// proxy address
	char data[1024];			// buffer
};


#define ISOCKPROXY_TYPE_NONE	0		// ����: �޴���������
#define ISOCKPROXY_TYPE_HTTP	1		// ����: HTTP����
#define ISOCKPROXY_TYPE_SOCKS4	2		// ����: SOCKS4����
#define ISOCKPROXY_TYPE_SOCKS5	3		// ����: SOCKS5����

///
/// ��ʼ���������� ISOCKPROXY
/// ѡ�� type��: ISOCKPROXY_TYPE_NONE, ISOCKPROXY_TYPE_HTTP, 
//  ISOCKPROXY_TYPE_SOCKS4, ISOCKPROXY_TYPE_SOCKS5
/// 
int iproxy_init(struct ISOCKPROXY *proxy, int sock, int type, 
	const struct sockaddr *remote, const struct sockaddr *proxyd, 
	const char *user, const char *pass, int mode);

///
/// ��������
/// �ɹ�����1��ʧ�ܷ���<0����������0
///
int iproxy_process(struct ISOCKPROXY *proxy);


//---------------------------------------------------------------------
// CSV Reader/Writer
//---------------------------------------------------------------------
struct iCsvReader;
struct iCsvWriter;

typedef struct iCsvReader iCsvReader;
typedef struct iCsvWriter iCsvWriter;


// open csv reader from file 
iCsvReader *icsv_reader_open_file(const char *filename);

// open csv reader from memory 
iCsvReader *icsv_reader_open_memory(const char *text, ilong size);

// close csv reader
void icsv_reader_close(iCsvReader *reader);

// read csv row
int icsv_reader_read(iCsvReader *reader);

// get column count in current row
int icsv_reader_size(const iCsvReader *reader);

// returns 1 for end of file, 0 for not end.
int icsv_reader_eof(const iCsvReader *reader);

// get column string
ivalue_t *icsv_reader_get(iCsvReader *reader, int pos);

// get column string
const ivalue_t *icsv_reader_get_const(const iCsvReader *reader, int pos);

// return column string size, -1 for error
int icsv_reader_get_size(const iCsvReader *reader, int pos);

// return column string, returns string size for success, -1 for error
int icsv_reader_get_string(const iCsvReader *reader, int pos, ivalue_t *out);

// return column string, returns string size for success, -1 for error
int icsv_reader_get_cstr(const iCsvReader *reader, int pos, 
	char *out, int size);

// utils for reader
int icsv_reader_get_long(const iCsvReader *reader, int i, long *x);
int icsv_reader_get_ulong(const iCsvReader *reader, int i, unsigned long *x);
int icsv_reader_get_int(const iCsvReader *reader, int i, int *x);
int icsv_reader_get_uint(const iCsvReader *reader, int i, unsigned int *x);
int icsv_reader_get_int64(const iCsvReader *reader, int i, IINT64 *x);
int icsv_reader_get_uint64(const iCsvReader *reader, int i, IUINT64 *x);
int icsv_reader_get_float(const iCsvReader *reader, int i, float *x);
int icsv_reader_get_double(const iCsvReader *reader, int i, double *x);


// open csv writer from file: if filename is NULL, it will open in memory
iCsvWriter *icsv_writer_open(const char *filename, int append);

// close csv writer
void icsv_writer_close(iCsvWriter *writer);

// write row and reset
int icsv_writer_write(iCsvWriter *writer);

// return column count in current row
int icsv_writer_size(iCsvWriter *writer);

// clear columns in current row
void icsv_writer_clear(iCsvWriter *writer);

// dump output
void icsv_writer_dump(iCsvWriter *writer, ivalue_t *out);

// clear output
void icsv_writer_empty(iCsvWriter *writer);

// push string
int icsv_writer_push(iCsvWriter *writer, const ivalue_t *str);

// push c string
int icsv_writer_push_cstr(iCsvWriter *writer, const char *ptr, int size);


// utils for writer
int icsv_writer_push_long(iCsvWriter *writer, long x, int radix);
int icsv_writer_push_ulong(iCsvWriter *writer, unsigned long x, int radix);
int icsv_writer_push_int(iCsvWriter *writer, int x, int radix);
int icsv_writer_push_uint(iCsvWriter *writer, unsigned int x, int radix);
int icsv_writer_push_int64(iCsvWriter *writer, IINT64 x, int radix);
int icsv_writer_push_uint64(iCsvWriter *writer, IUINT64 x, int radix);
int icsv_writer_push_float(iCsvWriter *writer, float x);
int icsv_writer_push_double(iCsvWriter *writer, double x);

/*==================================================================*/
/* loop running per fix interval                                    */
/*===================================================================*/

void ifix_interval_start(IUINT32* time);

void ifix_interval_running(IUINT32 *time, long interval);

#ifdef __cplusplus
}
#endif



#endif


