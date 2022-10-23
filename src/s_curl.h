#ifndef __s_curl_h__
#define __s_curl_h__

#include <kore/kore.h>
#include <kore/http.h>
#include <kore/curl.h>

int    curl_execute(struct kore_curl*      p_client,
                    struct kore_buf*       p_buf_out,
                    struct kore_buf*       p_buf_err,
                    const char*            psz_url,
                    int                    http_method,
                    struct kore_buf*       p_buf_body,
                    int                    count,
                    ...) ;


int    curl_execute_v(struct kore_curl*      p_client,
                      struct kore_buf*       p_buf_out,
                      struct kore_buf*       p_buf_err,
                      const char*            psz_url,
                      int                    http_method,
                      struct kore_buf*       p_buf_body,
                      int                    count,
                      va_list                args) ;

#endif /* defined __s_curl_h__ */
