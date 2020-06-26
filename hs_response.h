#ifndef HANDLERSOCKET_RESPONSE_H
#define HANDLERSOCKET_RESPONSE_H

#include "php_streams.h"

#define HS_CODE_NULL            0x00
#define HS_CODE_DELIMITER       0x09
#define HS_CODE_EOL             0x0a
#define HS_CODE_ESCAPE          0x10
#define HS_CODE_ESCAPE_PREFIX   0x01
#define HS_CODE_ESCAPE_ADD      0x40

int hs_response_value(php_stream *stream, double timeout, zval *return_value, zval *error, int modify TSRMLS_DC);
int hs_response_multi(php_stream *stream, double timeout, zval *return_value, zval *error, zval *mreq TSRMLS_DC);

#endif /* HANDLERSOCKET_RESPONSE_H */
