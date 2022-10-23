#include <kore/kore.h>
#include <kore/http.h>
#include <kore/curl.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include "md5.h"
#include "sha1.h"

#include "s_curl.h"

struct memory_token {
    char    token[32+1];
    time_t  token_time;  
} ;

char raise_base_url[512];
char raise_passwd[16];

typedef struct memory_token  s_memory_token;
typedef struct memory_token* ps_memory_token;

s_memory_token    token;

struct kore_curl*
state_setup(struct http_request *req) ;

void    kore_parent_configure(int argc, char **argv);
int		init(int);
int		left_nozzle(struct http_request *);
int		right_nozzle(struct http_request *);
int		printer_basics(struct http_request *);
int		job_status(struct http_request *);
int     refresh_token(struct http_request *, struct kore_curl*);
int     get_with_token(const char*, const char* , struct http_request *, struct kore_curl*);

void
kore_parent_configure(int argc, char **argv) {
    FILE* hConfFile = NULL ;
    char  bufConfFile[1024];

    struct kore_json         json ;
    struct kore_json_item*   itemRaiseBaseURL;
    struct kore_json_item*   itemRaisePasswd;

    kore_default_getopt(argc, argv);
    printf("*********************************** kore_parent_configure\n");

    memset(&token,0,sizeof(token));

    memset(&raise_base_url,0,sizeof(raise_base_url));
    memset(&raise_passwd,0,sizeof(raise_passwd));

    hConfFile = fopen("raise-conf.json","rt");
    if ( hConfFile != NULL ) {
        memset(&bufConfFile,0,sizeof(bufConfFile));
        size_t nbRead = fread(bufConfFile, sizeof(char), sizeof(bufConfFile), hConfFile) ;
        if ( nbRead > 0 ) {            
            kore_json_init(&json, bufConfFile, nbRead);
            if ( kore_json_parse(&json) ) {
                itemRaiseBaseURL = kore_json_find_string(json.root, "raise_base_url");
                itemRaisePasswd = kore_json_find_string(json.root, "raise_passwd");
                if ( itemRaiseBaseURL != NULL && itemRaisePasswd != NULL ) {
                    kore_strlcpy(raise_base_url,itemRaiseBaseURL->data.string, sizeof(raise_base_url));
                    if ( raise_base_url[strlen(raise_base_url)-1] != '/' ) {
                        strcat(raise_base_url,"/");
                    }
                    kore_strlcpy(raise_passwd,itemRaisePasswd->data.string, sizeof(raise_passwd));
                    printf("Ok use base URL %s and passwd %s for Raise API\n",raise_base_url, raise_passwd);
                } else {
                    printf("ERROR missing key raise_base_url or raise_passwd in raise-conf.json file\n");
                }
            } else {
                printf("ERROR json parsing in raise-conf.json file\n");
            }
            kore_json_cleanup(&json);           
        } else {
            printf("ERROR empty file raise-conf.json\n");
        }
        fclose(hConfFile);
    } else {
        printf("ERROR missing file raise-conf.json\n");
    }
}

int
init(int state) {
    if ( state == KORE_MODULE_UNLOAD ) {
        return KORE_RESULT_OK ;
    }
    
	return KORE_RESULT_OK;
}

int
left_nozzle(struct http_request *req) {
    int                 ret;
    struct kore_buf     buf;
    struct kore_curl*   client;
    
    kore_buf_init(&buf,512);
    client = state_setup(req);
    if ( client != NULL ) {
        if ( KORE_RESULT_OK == refresh_token(req, client) ) {
            ret = get_with_token(raise_base_url, "printer/nozzle1",req, client);
        }
        kore_curl_cleanup(client);
        return ret;
    }
    req->status = HTTP_STATUS_INTERNAL_ERROR;   
    kore_buf_appendf(&buf, "can not get token\n");
    http_response(req, req->status, buf.data, buf.offset);
    kore_buf_cleanup(&buf);
	return KORE_RESULT_OK;        
}

int
right_nozzle(struct http_request *req) {
    int                 ret;
    struct kore_buf     buf;
    struct kore_curl*   client;
    
    kore_buf_init(&buf,512);
    client = state_setup(req);
    if ( client != NULL ) {
        if ( KORE_RESULT_OK == refresh_token(req, client) ) {
            ret = get_with_token(raise_base_url, "printer/nozzle2",req, client);
        }
        kore_curl_cleanup(client);
        return ret;
    }
    req->status = HTTP_STATUS_INTERNAL_ERROR;
    kore_buf_appendf(&buf, "can not get token\n");
    http_response(req, req->status, buf.data, buf.offset);
    kore_buf_cleanup(&buf);
	return KORE_RESULT_OK;    
}

int
printer_basics(struct http_request *req) {
    int                 ret;
    struct kore_buf     buf;
    struct kore_curl*   client;
    
    kore_buf_init(&buf,512);
    client = state_setup(req);
    if ( client != NULL ) {
        if ( KORE_RESULT_OK == refresh_token(req, client) ) {
            ret = get_with_token(raise_base_url, "printer/basic",req, client);
        }
        kore_curl_cleanup(client);
        return ret;
    }
    req->status = HTTP_STATUS_INTERNAL_ERROR;   
    kore_buf_appendf(&buf, "can not get token\n");
    http_response(req, req->status, buf.data, buf.offset);
    kore_buf_cleanup(&buf);
	return KORE_RESULT_OK;
}

int
job_status(struct http_request *req) {
    int                 ret;
    struct kore_buf     buf;
    struct kore_curl*   client;
    
    kore_buf_init(&buf,512);
    client = state_setup(req);
    if ( client != NULL ) {
        if ( KORE_RESULT_OK == refresh_token(req, client) ) {
            ret = get_with_token(raise_base_url, "job/currentjob",req, client);
        }
        kore_curl_cleanup(client);
        return ret;
    }
    req->status = HTTP_STATUS_INTERNAL_ERROR;   
    kore_buf_appendf(&buf, "can not get token\n");
    http_response(req, req->status, buf.data, buf.offset);
    kore_buf_cleanup(&buf);
	return KORE_RESULT_OK;
}

struct kore_curl*
state_setup(struct http_request *req) {
    struct kore_curl*        client;
	client = http_state_create(req, sizeof(*client));
    return client;
}

int
get_with_token(const char* raiseURL, const char* baseUrl, struct http_request *req, struct kore_curl* client) {
    int ret = KORE_RESULT_ERROR;

    struct kore_json         json ;
    struct kore_buf          url;
    struct kore_buf          buf;
    struct kore_buf          errors;
    
    kore_buf_init(&url, 512);
    kore_buf_init(&buf, 512);
    kore_buf_init(&errors, 512);
    if ( client != NULL ) {
        kore_buf_appendf(&url, "%s%s?token=%s",raiseURL, baseUrl, token.token);
        printf("call GET %s?token=%s",baseUrl, token.token);
        if ( curl_execute(client,
                          &buf,
                          &errors,
                          kore_buf_stringify(&url, NULL),
                          HTTP_METHOD_GET,
                          NULL,
                          0) ) {
            printf(" Response buf length : %ld\n",buf.offset);
            kore_json_init(&json, buf.data, buf.offset);
            if ( kore_json_parse(&json) ) {
                req->status = HTTP_STATUS_OK;
                http_response_header(req,"Content-Type","application/json");
                http_response(req, req->status, buf.data, buf.offset);
                ret = KORE_RESULT_OK;
            } else {
                printf("error kore_json_parse\n");
                ret = KORE_RESULT_ERROR;
            }
        } else {
            printf(" error curl execute\n");
            kore_buf_append(&buf, errors.data, errors.offset);           
            ret = KORE_RESULT_ERROR;
        }
    } else {
        printf("error client is null\n");
        kore_buf_appendf(&buf, "Error : can not create client");
        ret = KORE_RESULT_ERROR;
    }
    
    kore_buf_cleanup(&errors);
    kore_buf_cleanup(&buf);
    kore_buf_cleanup(&url);
    kore_json_cleanup(&json);
    
    return ret;
}

int
refresh_token(struct http_request *req, struct kore_curl* client) {
    int ret = KORE_RESULT_ERROR;
    struct timeval tv;

    char* pwd = raise_passwd;
    char  tosha[256];
    char  timestamp[14];
    
    char  sha1_result[21];
    char  sha1_hexresult[42];
    
    uint8_t* md5result ;
    char     md5_hexresult[42];

    struct kore_json         json ;
    struct kore_json_item*   itemData;
    struct kore_json_item*   itemToken;
    
    gettimeofday(&tv,NULL);
    unsigned long long millisecondsSinceEpoch = (unsigned long long)(tv.tv_sec) * 1000 + (unsigned long long)(tv.tv_usec) / 1000;

    if ( tv.tv_sec - token.token_time < 3600*23 ) {
        return KORE_RESULT_OK;
    }
    
    
    sprintf(timestamp, "%llu", millisecondsSinceEpoch);
    sprintf(tosha,"password=%s&timestamp=%s",pwd,timestamp);
    sha1( sha1_result, tosha, strlen(tosha) ) ;
    memset(sha1_hexresult, 0, sizeof(sha1_hexresult));
    for( size_t offset = 0; offset < 20; offset++) {
        sprintf( ( sha1_hexresult + (2*offset)), "%02x", sha1_result[offset]&0xff);
    }

    md5result = md5String(sha1_hexresult);
    memset(md5_hexresult, 0, sizeof(md5_hexresult));
    char digit[3];
	for(unsigned int i = 0; i < 16; ++i){
        digit[0]=digit[1]=digit[2]=0;
		sprintf(digit, "%02x", md5result[i]);
        strcat(md5_hexresult, digit);
	}
    
    struct kore_buf          url;
    struct kore_buf          buf;
    struct kore_buf          errors;
    
    req->status = HTTP_STATUS_INTERNAL_ERROR;
    kore_buf_init(&url, 512);
    kore_buf_init(&buf, 512);
    kore_buf_init(&errors, 512);
    if ( client != NULL ) {
        kore_buf_appendf(&url, "%slogin?sign=%s&timestamp=%s",raise_base_url, md5_hexresult, timestamp);
        printf("call GET %slogin?sign=%s&timestamp=%s\n",raise_base_url, md5_hexresult, timestamp);
        if ( curl_execute(client,
                          &buf,
                          &errors,
                          kore_buf_stringify(&url, NULL),
                          HTTP_METHOD_GET,
                          NULL,
                          0) ) {
            kore_json_init(&json, buf.data, buf.offset);
            if ( kore_json_parse(&json) ) {
                itemData = kore_json_find_object(json.root, "data");
                if ( NULL != itemData ) {
                    itemToken = kore_json_find_string(itemData, "token");
                    if ( itemToken != NULL ) {
                        kore_strlcpy(token.token,itemToken->data.string, sizeof(token.token));
                        token.token_time = tv.tv_sec ;
                        ret = KORE_RESULT_OK;
                    } else {
                        printf("kore_json_find string token\n");
                        ret = KORE_RESULT_ERROR;
                    }
                } else {
                    printf("kore_json_find object data\n");
                    ret = KORE_RESULT_ERROR;
                }
            } else {
                printf("kore_json_parse\n");
                ret = KORE_RESULT_ERROR;
            }
        } else {
            printf("error curl execute\n");
            kore_buf_append(&buf, errors.data, errors.offset);           
            ret = KORE_RESULT_ERROR;
        }
    } else {
        printf("error client is null\n");
        kore_buf_appendf(&buf, "Error : can not create client");
        ret = KORE_RESULT_ERROR;
    }
    
    kore_buf_cleanup(&errors);
    kore_buf_cleanup(&buf);
    kore_buf_cleanup(&url);
    kore_json_cleanup(&json);
    return ret;    
}
