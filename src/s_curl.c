
#include "s_curl.h"

int    curl_execute(struct kore_curl*      p_client,
                    struct kore_buf*       p_buf_out,
                    struct kore_buf*       p_buf_err,
                    const char*            psz_url,
                    int                    http_method,
                    struct kore_buf*       p_buf_body,
                    int                    count,
                    ...) {
    int ret = 0 ;
    va_list args;

    va_start(args, count);
    ret = curl_execute_v(p_client, p_buf_out, p_buf_err, psz_url, http_method, p_buf_body, count, args);
    va_end(args);

    return ret;
}

int    curl_execute_v(struct kore_curl*      p_client,
                      struct kore_buf*       p_buf_out,
                      struct kore_buf*       p_buf_err,
                      const char*            psz_url,
                      int                    http_method,
                      struct kore_buf*       p_buf_body,
                      int                    count,
                      va_list                args) {
    int      ret = 0 ;
    char*    psz_param_name ;
    char*    psz_param_value;
    char*    p_response_as_string;

    if ( kore_curl_init(p_client, psz_url, KORE_CURL_SYNC)) {
        // setup curl client
        if ( p_buf_body != NULL ) {
            kore_curl_http_setup(p_client, http_method, p_buf_body->data, p_buf_body->offset);
        } else {
            kore_curl_http_setup(p_client, http_method, NULL, 0);
        }

        // add headers if present
        if ( count > 0 ) {
            for(int i=0; i<count; i++) {
                psz_param_name = va_arg(args, char*) ;
                psz_param_value = va_arg(args, char*) ;
                if ( psz_param_name != NULL && psz_param_value != NULL ) {
                    kore_curl_http_set_header(p_client, psz_param_name, psz_param_value);
                }
            }
        }

        // accept auto-signed certificat
        curl_easy_setopt(p_client->handle, CURLOPT_SSL_VERIFYHOST, 0);
        curl_easy_setopt(p_client->handle, CURLOPT_SSL_VERIFYPEER, 0);

        // execute curl command
        kore_curl_run(p_client);

        if ( kore_curl_success(p_client) ) {
            p_response_as_string = kore_curl_response_as_string(p_client) ;
            if ( p_response_as_string != NULL ) {
                kore_buf_appendf(p_buf_out, p_response_as_string) ;
                ret = 1 ;
            } else {
                kore_buf_appendf(p_buf_err, "Error : kore_curl_response_as_string return null pointer");
            }
        } else {
            kore_curl_logerror(p_client);
            kore_buf_appendf(p_buf_err, "Error : kore_curl_init %s",kore_curl_strerror(p_client));      
        }       
    } else {
        kore_curl_logerror(p_client);
        kore_buf_appendf(p_buf_err, "Error : kore_curl_init %s",kore_curl_strerror(p_client));
    }

    return ret;
}
