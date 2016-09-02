#ifndef __URLDATA_H
#define __URLDATA_H
/*****************************************************************************
 *                                  _   _ ____  _     
 *  Project                     ___| | | |  _ \| |    
 *                             / __| | | | |_) | |    
 *                            | (__| |_| |  _ <| |___ 
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) 2000, Daniel Stenberg, <daniel@haxx.se>, et al.
 *
 * In order to be useful for every potential user, curl and libcurl are
 * dual-licensed under the MPL and the MIT/X-derivate licenses.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the MPL or the MIT/X-derivate
 * licenses. You may pick one of these licenses.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 * $Id: urldata.h,v 1.1.1.2 2001/04/24 18:49:13 wsanchez Exp $
 *****************************************************************************/

/* This file is for lib internal stuff */

#include "setup.h"

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 256
#endif

#define PORT_FTP 21
#define PORT_TELNET 23
#define PORT_GOPHER 70
#define PORT_HTTP 80
#define PORT_HTTPS 443
#define PORT_DICT 2628
#define PORT_LDAP 389

#define DICT_MATCH "/MATCH:"
#define DICT_MATCH2 "/M:"
#define DICT_MATCH3 "/FIND:"
#define DICT_DEFINE "/DEFINE:"
#define DICT_DEFINE2 "/D:"
#define DICT_DEFINE3 "/LOOKUP:"

#define CURL_DEFAULT_USER "anonymous"
#define CURL_DEFAULT_PASSWORD "curl_by_daniel@haxx.se"

#include "cookie.h"
#include "formdata.h"
    
#ifdef USE_SSLEAY
/* SSLeay stuff usually in /usr/local/ssl/include */
#ifdef USE_OPENSSL
#include "openssl/rsa.h"
#include "openssl/crypto.h"
#include "openssl/x509.h"
#include "openssl/pem.h"
#include "openssl/ssl.h"
#include "openssl/err.h"
#else
#include "rsa.h"
#include "crypto.h"
#include "x509.h"
#include "pem.h"
#include "ssl.h"
#include "err.h"
#endif
#endif

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#include "timeval.h"

#include <curl/curl.h>

#include "http_chunks.h" /* for the structs and enum stuff */

/* Download buffer size, keep it fairly big for speed reasons */
#define BUFSIZE (1024*50)

/* Defaul upload buffer size, keep it smallish to get faster progress meter
   updates. This is just default, it is dynamic and adjusts to the upload
   speed. */
#define UPLOAD_BUFSIZE (1024*2)

/* Initial size of the buffer to store headers in, it'll be enlarged in case
   of need. */
#define HEADERSIZE 256

/* Just a convenience macro to get the larger value out of two given */
#ifndef MAX
#define MAX(x,y) ((x)>(y)?(x):(y))
#endif

#ifdef KRB4
/* Types needed for krb4-ftp connections */
struct krb4buffer {
  void *data;
  size_t size;
  size_t index;
  int eof_flag;
};
enum protection_level { 
    prot_clear, 
    prot_safe, 
    prot_confidential, 
    prot_private 
};
#endif

/* struct for data related to SSL and SSL connections */
struct ssl_connect_data {
  bool use;              /* use ssl encrypted communications TRUE/FALSE */
#ifdef USE_SSLEAY
  /* these ones requires specific SSL-types */
  SSL_CTX* ctx;
  SSL*     handle;
  X509*    server_cert;
#endif /* USE_SSLEAY */
};

struct ssl_config_data {
  long version;          /* what version the client wants to use */
  long certverifyresult; /* result from the certificate verification */
  long verifypeer;       /* set TRUE if this is desired */
  char *CApath;          /* DOES NOT WORK ON WINDOWS */
  char *CAfile;          /* cerficate to verify peer against */
  char *random_file;     /* path to file containing "random" data */
  char *egdsocket;       /* path to file containing the EGD daemon socket */
};

/****************************************************************************
 * HTTP unique setup
 ***************************************************************************/
struct HTTP {
  struct FormData *sendit;
  int postsize;
  char *p_pragma;      /* Pragma: string */
  char *p_accept;      /* Accept: string */
  long readbytecount; 
  long writebytecount;

  /* For FORM posting */
  struct Form form;
  size_t (*storefread)(char *, size_t , size_t , FILE *);
  FILE *in;

  struct Curl_chunker chunk;
};

/****************************************************************************
 * FTP unique setup
 ***************************************************************************/
struct FTP {
  long *bytecountp;
  char *user;    /* user name string */
  char *passwd;  /* password string */
  char *urlpath; /* the originally given path part of the URL */
  char *dir;     /* decoded directory */
  char *file;    /* decoded file */

  char *entrypath; /* the PWD reply when we logged on */
};

/****************************************************************************
 * FILE unique setup
 ***************************************************************************/
struct FILE {
  int fd; /* open file descriptor to read from! */
};

/*
 * Boolean values that concerns this connection.
 */
struct ConnectBits {
  bool close; /* if set, we close the connection after this request */
  bool reuse; /* if set, this is a re-used connection */
  bool chunk; /* if set, this is a chunked transfer-encoding */
  bool httpproxy; /* if set, this transfer is done through a http proxy */
  bool user_passwd;       /* do we use user+password for this connection? */
  bool proxy_user_passwd; /* user+password for the proxy? */

  bool use_range;
  bool rangestringalloc; /* the range string is malloc()'ed */
};

/*
 * The connectdata struct contains all fields and variables that should be
 * unique for an entire connection.
 */
struct connectdata {
  /**** Fields set when inited and not modified again */
  struct UrlData *data; /* link to the root CURL struct */
  int connectindex; /* what index in the connects index this particular
                       struct has */

  long protocol; /* PROT_* flags concerning the protocol set */
#define PROT_MISSING (1<<0)
#define PROT_GOPHER  (1<<1)
#define PROT_HTTP    (1<<2)
#define PROT_HTTPS   (1<<3)
#define PROT_FTP     (1<<4)
#define PROT_TELNET  (1<<5)
#define PROT_DICT    (1<<6)
#define PROT_LDAP    (1<<7)
#define PROT_FILE    (1<<8)
#define PROT_FTPS    (1<<9)

#ifdef ENABLE_IPV6
  struct addrinfo *hp; /* host info pointer list */
  struct addrinfo *ai; /* the particular host we use */
#else
  char *hostent_buf; /* pointer to allocated memory for name info */
  struct hostent *hp;
  struct sockaddr_in serv_addr;
#endif
  char protostr[64];  /* store the protocol string in this buffer */
  char gname[257]; /* store the hostname in this buffer */
  char *name;      /* host name pointer to fool around with */
  char *path;      /* allocated buffer to store the URL's path part in */
  char *hostname;  /* hostname to connect, as parsed from url */
  long port;       /* which port to use locally */
  unsigned short remote_port; /* what remote port to connect to,
                                 not the proxy port! */
  char *ppath;
  long bytecount;

  char *range; /* range, if used. See README for detailed specification on
                  this syntax. */
  int resume_from; /* continue [ftp] transfer from here */

  char *proxyhost; /* name of the http proxy host */

  struct timeval now;     /* "current" time */
  struct timeval created; /* creation time */
  int firstsocket;     /* the main socket to use */
  int secondarysocket; /* for i.e ftp transfers */

  long upload_bufsize; /* adjust as you see fit, never bigger than BUFSIZE
                          never smaller than UPLOAD_BUFSIZE */

  long maxdownload; /* in bytes, the maximum amount of data to fetch, 0
                       means unlimited */
  
  struct ssl_connect_data ssl; /* this is for ssl-stuff */

  struct ConnectBits bits;    /* various state-flags for this connection */

  /* These two functions MUST be set by the curl_connect() function to be
     be protocol dependent */
  CURLcode (*curl_do)(struct connectdata *connect);
  CURLcode (*curl_done)(struct connectdata *connect);

  /* This function *MAY* be set to a protocol-dependent function that is run
   * after the connect() and everything is done, as a step in the connection.
   */ 
  CURLcode (*curl_connect)(struct connectdata *connect);

  /* This function *MAY* be set to a protocol-dependent function that is run
   * by the curl_disconnect(), as a step in the disconnection.
   */ 
  CURLcode (*curl_disconnect)(struct connectdata *connect);

  /* This function *MAY* be set to a protocol-dependent function that is run
   * in the curl_close() function if protocol-specific cleanups are required.
   */ 
  CURLcode (*curl_close)(struct connectdata *connect);

  /**** curl_get() phase fields */

  /* READ stuff */
  int sockfd;		 /* socket to read from or -1 */
  int size;		 /* -1 if unknown at this point */
  bool getheader;	 /* TRUE if header parsing is wanted */
  long *bytecountp;	 /* return number of bytes read or NULL */
          
  /* WRITE stuff */
  int writesockfd;       /* socket to write to, it may very well be
                            the same we read from. -1 disables */
  long *writebytecountp; /* return number of bytes written or NULL */

  /** Dynamicly allocated strings, may need to be freed before this **/
  /** struct is killed.                                             **/
  struct dynamically_allocated_data {
    char *proxyuserpwd; /* free later if not NULL! */
    char *uagent; /* free later if not NULL! */
    char *userpwd; /* free later if not NULL! */
    char *rangeline; /* free later if not NULL! */
    char *ref; /* free later if not NULL! */
    char *cookie; /* free later if not NULL! */
    char *host; /* free later if not NULL */
  } allocptr;

  char *newurl; /* This can only be set if a Location: was in the
		   document headers */

#ifdef KRB4
  enum protection_level command_prot;
  enum protection_level data_prot;
  enum protection_level request_data_prot;

  size_t buffer_size;

  struct krb4buffer in_buffer, out_buffer;
  int sec_complete;
  void *app_data;

#endif

  /*************** Request - specific items ************/
  /* previously this was in the urldata struct */
  union {
    struct HTTP *http;
    struct HTTP *gopher; /* alias, just for the sake of being more readable */
    struct HTTP *https;  /* alias, just for the sake of being more readable */
    struct FTP *ftp;
    struct FILE *file;
    void *telnet;        /* private for telnet.c-eyes only */
#if 0 /* no need for special ones for these: */
    struct LDAP *ldap;
    struct DICT *dict;
#endif
    void *generic;
  } proto;


};

struct Progress {
  long lastshow; /* time() of the last displayed progress meter or NULL to
                    force redraw at next call */
  double size_dl;
  double size_ul;
  double downloaded;
  double uploaded;

  double current_speed; /* uses the currently fastest transfer */

  bool callback;  /* set when progress callback is used */
  int width; /* screen width at download start */
  int flags; /* see progress.h */
  double timespent;
  double dlspeed;
  double ulspeed;

  struct timeval start;
  struct timeval t_startsingle;
  /* various data stored for possible later report */
  double t_nslookup;
  double t_connect;
  double t_pretransfer;
  int httpcode;
  int httpversion;
  time_t filetime; /* If requested, this is might get set. It may be 0 if
                      the time was unretrievable */

#define CURR_TIME 5

  double speeder[ CURR_TIME ];
  int speeder_c;
};

typedef enum {
  HTTPREQ_NONE, /* first in list */
  HTTPREQ_GET,
  HTTPREQ_POST,
  HTTPREQ_POST_FORM, /* we make a difference internally */
  HTTPREQ_PUT,
  HTTPREQ_CUSTOM,
  HTTPREQ_LAST /* last in list */
} Curl_HttpReq;

/* This struct is for boolean settings that define how to behave during
   this session. */
struct Configbits {
  /* these four request types mirror the httpreq field */
  bool http_formpost;
  bool http_post;
  bool http_put;
  bool http_get;

  bool get_filetime;
  bool tunnel_thru_httpproxy;
  bool ftp_append;
  bool ftp_ascii;
  bool ftp_list_only;
  bool ftp_use_port;
  bool hide_progress;
  bool http_fail_on_error;
  bool http_follow_location;
  bool http_include_header;
  bool http_set_referer;
  bool http_auto_referer; /* set "correct" referer when following location: */
  bool httpproxy;
  bool mute;
  bool no_body;
  bool set_port;
  bool set_range;
  bool upload;
  bool use_netrc;
  bool verbose;
  bool this_is_a_follow; /* this is a followed Location: request */
  bool krb4; /* kerberos4 connection requested */
  bool proxystringalloc; /* the http proxy string is malloc()'ed */
  bool urlstringalloc;   /* the URL string is malloc()'ed */
  bool reuse_forbid;     /* if this is forbidden to be reused, close 
                            after use */
  bool reuse_fresh;      /* do not re-use an existing connection for this
                            transfer */
};

/*
 * As of April 11, 2000 we're now trying to split up the urldata struct in
 * three different parts:
 *
 * (Global)
 * 1 - No matter how many hosts and requests that are being performed, this
 *     goes for all of them.
 *
 * (Session)
 * 2 - Host and protocol-specific. No matter if we do several transfers to and
 *     from this host, these variables stay the same.
 *
 * (Request)
 * 3 - Request-specific. Variables that are of interest for this particular
 *     transfer being made right now. THIS IS WRONG STRUCT FOR THOSE.
 *
 * In Febrary 2001, this is being done stricter. The 'connectdata' struct
 * MUST have all the connection oriented stuff as we may now have several
 * simultaneous connections and connection structs in memory.
 *
 * From now on, the 'UrlData' must only contain data that is set once to go
 * for many (perhaps) independent connections. Values that are generated or
 * calculated internally MUST NOT be a part of this struct.
 */

struct UrlData {
  /*************** Global - specific items  ************/
  FILE *err;    /* the stderr writes goes here */
  char *errorbuffer; /* store failure messages in here */

  /*************** Session - specific items ************/
  char *proxy; /* if proxy, set it here */
  char *proxyuserpwd;  /* Proxy <user:password>, if used */
  long proxyport; /* If non-zero, use this port number by default. If the
                     proxy string features a ":[port]" that one will override
                     this. */

  
  long header_size;  /* size of read header(s) in bytes */
  long request_size; /* the amount of bytes sent in the request(s) */

  FILE *out;    /* the fetched file goes here */
  FILE *in;     /* the uploaded file is read from here */
  FILE *writeheader; /* write the header to this is non-NULL */
  char *url;   /* what to get */
  char *freethis; /* if non-NULL, an allocated string for the URL */
  long use_port;  /* which port to use (when not using default) */
  struct Configbits bits; /* new-style (v7) flag data */
  struct ssl_config_data ssl; /* this is for ssl-stuff */

  char *userpwd;  /* <user:password>, if used */
  char *set_range; /* range, if used. See README for detailed specification on
                      this syntax. */

  /* stuff related to HTTP */

  long followlocation;
  long maxredirs; /* maximum no. of http(s) redirects to follow */
  char *referer;
  bool free_referer; /* set TRUE if 'referer' points to a string we
                        allocated */
  char *useragent;   /* User-Agent string */
  char *postfields; /* if POST, set the fields' values here */
  long postfieldsize; /* if POST, this might have a size to use instead of
                         strlen(), and then the data *may* be binary (contain
                         zero bytes) */

  /* stuff related to FTP */
  char *ftpport; /* port to send with the PORT command */

  /* general things */
  char *device;  /* Interface to use */

  /* function that stores the output:*/
  curl_write_callback fwrite;

  /* optional function that stores the header output:*/
  curl_write_callback fwrite_header;

  /* function that reads the input:*/
  curl_read_callback fread;

  /* function that wants progress information */
  curl_progress_callback fprogress;
  void *progress_client; /* pointer to pass to the progress callback */

  /* function to call instead of the internal for password */
  curl_passwd_callback fpasswd;
  void *passwd_client; /* pointer to pass to the passwd callback */

  long timeout;        /* in seconds, 0 means no timeout */
  long connecttimeout; /* in seconds, 0 means no timeout */
  long infilesize;     /* size of file to upload, -1 means unknown */

  char buffer[BUFSIZE+1]; /* buffer with size BUFSIZE */

  double current_speed;  /* the ProgressShow() funcion sets this */

  long low_speed_limit; /* bytes/second */
  long low_speed_time;  /* number of seconds */

  int set_resume_from;    /* continue [ftp] transfer from here */

  char *cookie;       /* HTTP cookie string to send */

  struct curl_slist *headers; /* linked list of extra headers */
  struct HttpPost *httppost;  /* linked list of POST data */

  char *cert; /* PEM-formatted certificate */
  char *cert_passwd; /* plain text certificate password */

  struct CookieInfo *cookies;

  long crlf;
  struct curl_slist *quote;     /* before the transfer */
  struct curl_slist *postquote; /* after the transfer */

  /* Telnet negotiation options */
  struct curl_slist *telnet_options; /* linked list of telnet options */

  TimeCond timecondition; /* kind of comparison */
  time_t timevalue;       /* what time to compare with */

  Curl_HttpReq httpreq; /* what kind of HTTP request (if any) is this */

  char *customrequest; /* http/ftp request to use */

  char *headerbuff; /* allocated buffer to store headers in */
  int headersize;   /* size of the allocation */

  struct Progress progress; /* for all the progress meter data */

#define MAX_CURL_USER_LENGTH 128
#define MAX_CURL_PASSWORD_LENGTH 128

  char *auth_host; /* if set, this is the allocated string to the host name
                    * to which to send the authorization data to, and no other
                    * host (which location-following otherwise could lead to)
                    */

  /* buffers to store authentication data in */
  char user[MAX_CURL_USER_LENGTH];
  char passwd[MAX_CURL_PASSWORD_LENGTH];
  char proxyuser[MAX_CURL_USER_LENGTH];
  char proxypasswd[MAX_CURL_PASSWORD_LENGTH];

  char *krb4_level; /* what security level */
  struct timeval keeps_speed; /* this should be request-specific */

  /* 'connects' will be an allocated array with pointers. If the pointer is
     set, it holds an allocated connection. */
  struct connectdata **connects;
  size_t numconnects; /* size of the 'connects' array */
  curl_closepolicy closepolicy;

};

#define LIBCURL_NAME "libcurl"
#define LIBCURL_ID LIBCURL_NAME " " LIBCURL_VERSION " " SSL_ID

CURLcode Curl_getinfo(CURL *curl, CURLINFO info, ...);

/*
 * Here follows function prototypes from what we used to plan to call
 * the "low level" interface. It is no longer prioritized and it is not likely
 * to ever be supported to external users.
 *
 * I removed all the comments to them as well, as they were no longer accurate
 * and they're not meant for "public use" anymore.
 */

CURLcode Curl_open(CURL **curl, char *url);
CURLcode Curl_setopt(CURL *handle, CURLoption option, ...);
CURLcode Curl_close(CURL *curl); /* the opposite of curl_open() */
CURLcode Curl_connect(struct UrlData *,
                      struct connectdata **,
                      bool allow_port);
CURLcode Curl_do(struct connectdata *);
CURLcode Curl_done(struct connectdata *);
CURLcode Curl_disconnect(struct connectdata *);


#endif
