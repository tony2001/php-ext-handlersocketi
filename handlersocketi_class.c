
#include "php.h"
#include "php_ini.h"
#include "php_streams.h"
#include "ext/standard/php_smart_str.h"

#include "php_verdep.h"
#include "php_handlersocketi.h"
#include "handlersocketi_class.h"
#include "handlersocketi_index.h"
#include "hs_common.h"
#include "hs_request.h"
#include "hs_response.h"

ZEND_EXTERN_MODULE_GLOBALS(handlersocketi);

#define HS_STREAM_DEFAULT_TIMEOUT 5

static zend_class_entry *hs_ce;
static zend_object_handlers hs_object_handlers;

ZEND_BEGIN_ARG_INFO_EX(arginfo_hs_method__construct, 0, 0, 2)
    ZEND_ARG_INFO(0, host)
    ZEND_ARG_INFO(0, port)
    ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_hs_method_auth, 0, 0, 1)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(0, type)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_hs_method_open_index, 0, 0, 3)
    ZEND_ARG_INFO(0, db)
    ZEND_ARG_INFO(0, table)
    ZEND_ARG_INFO(0, fields)
    ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

ZEND_METHOD(HandlerSocketi, __construct);
ZEND_METHOD(HandlerSocketi, auth);
ZEND_METHOD(HandlerSocketi, open_index);

const zend_function_entry hs_methods[] = {
    ZEND_ME(HandlerSocketi, __construct,
            arginfo_hs_method__construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
    ZEND_ME(HandlerSocketi, auth,
            arginfo_hs_method_open_index, ZEND_ACC_PUBLIC)
    ZEND_ME(HandlerSocketi, open_index,
            arginfo_hs_method_open_index, ZEND_ACC_PUBLIC)
    ZEND_MALIAS(HandlerSocketi, openIndex, open_index,
                arginfo_hs_method_open_index, ZEND_ACC_PUBLIC)
    ZEND_FE_END
};

static void
hs_object_free_storage(void *object TSRMLS_DC)
{
    hs_obj_t *intern = (hs_obj_t *)object;
    zend_object_std_dtor(&intern->std TSRMLS_CC);

    if (intern->conn && !intern->conn->is_persistent) {
		zend_list_delete(intern->conn->rsrc_id);
    }

    if (intern->server) {
        zval_ptr_dtor(&intern->server);
    }

    if (intern->auth) {
        zval_ptr_dtor(&intern->auth);
    }

    if (intern->error) {
        zval_ptr_dtor(&intern->error);
    }

	efree(intern->hashkey);
    efree(object);
}

#define HS_CHECK_OBJECT(object, classname)                        \
	if (!(object)) {                                              \
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "The " #classname " object has not been correctly initialized by its constructor"); \
		RETURN_FALSE;                                             \
	}

#define HS_CHECK_CONNECTION(object, classname)                    \
	if (!(object)->conn || !(object)->conn->stream) {             \
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Using dead connection object of class " #classname); \
		RETURN_FALSE;                                             \
	}

static inline zend_object_value
hs_object_new_ex(zend_class_entry *ce, hs_obj_t **ptr TSRMLS_DC)
{
    hs_obj_t *intern;
    zend_object_value retval;
#if ZEND_MODULE_API_NO < 20100525
    zval *tmp;
#endif

    intern = emalloc(sizeof(hs_obj_t));
    memset(intern, 0, sizeof(hs_obj_t));
    if (ptr) {
        *ptr = intern;
    }

    zend_object_std_init(&intern->std, ce TSRMLS_CC);

#if ZEND_MODULE_API_NO >= 20100525
    object_properties_init(&intern->std, ce);
#else
    zend_hash_copy(intern->std.properties, &ce->default_properties,
                   (copy_ctor_func_t)zval_add_ref, (void *)&tmp, sizeof(zval *));
#endif

    retval.handle = zend_objects_store_put(
        intern, (zend_objects_store_dtor_t)zend_objects_destroy_object,
        (zend_objects_free_object_storage_t)hs_object_free_storage,
        NULL TSRMLS_CC);
    retval.handlers = &hs_object_handlers;

    intern->timeout = 0;
    intern->server = NULL;
    intern->auth = NULL;
    intern->error = NULL;
    return retval;
}

static inline int
hs_object_connection(hs_obj_t *obj)
{
    char *hashkey = NULL;
    char *errstr = NULL;
    int err;
    struct timeval tv;

    if (obj->timeout > 0) {
        tv.tv_sec = obj->timeout;
        tv.tv_usec = 0;
    }

	if (obj->conn->is_persistent) {
		php_stream *stream;

		spprintf(&hashkey, 0, "stream:%s", obj->hashkey);

		/* this code is copied from main/streams/transports.c with small modifications */
		switch(php_stream_from_persistent_id(hashkey, &stream TSRMLS_CC)) {
			case PHP_STREAM_PERSISTENT_SUCCESS:
				/* use a 0 second timeout when checking if the socket
				 * has already died */
				if (PHP_STREAM_OPTION_RETURN_OK == php_stream_set_option(stream, PHP_STREAM_OPTION_CHECK_LIVENESS, 0, NULL)) {
					obj->conn->stream = stream;
					efree(hashkey);
					return SUCCESS;
				}
				/* dead - kill it */
				php_stream_pclose(stream);

				/* fall through */

			case PHP_STREAM_PERSISTENT_FAILURE:
			default:
				/* failed; get a new one */
				;
		}
	}

	if (zend_hash_num_elements(&obj->conn->open_indices)) {
		zend_hash_clean(&obj->conn->open_indices);
	}

    obj->conn->stream = php_stream_xport_create(Z_STRVAL_P(obj->server),
                                          Z_STRLEN_P(obj->server),
                                          ENFORCE_SAFE_MODE|REPORT_ERRORS,
                                          STREAM_XPORT_CLIENT|
                                          STREAM_XPORT_CONNECT,
                                          hashkey, &tv, NULL, &errstr, &err);

	if (hashkey) {
		efree(hashkey);
	}

    if (!obj->conn->stream) {
        if (errstr) {
            efree(errstr);
        }
        return FAILURE;
    }

    if (errstr) {
        efree(errstr);
    }

    /* non-blocking */
    if (php_stream_set_option(obj->conn->stream, PHP_STREAM_OPTION_BLOCKING,
                              0, NULL) == -1) {
        zend_error(E_WARNING,
                   "HandlerSocketi: Un set non-blocking mode on a stream");
    }

	php_stream_auto_cleanup(obj->conn->stream);
    return SUCCESS;
}

static inline zend_object_value
hs_object_new(zend_class_entry *ce TSRMLS_DC)
{
    return hs_object_new_ex(ce, NULL TSRMLS_CC);
}

static inline zend_object_value
hs_object_clone(zval *this_ptr TSRMLS_DC)
{
    hs_obj_t *new_obj = NULL;
    hs_obj_t *old_obj =
        (hs_obj_t *)zend_object_store_get_object(this_ptr TSRMLS_CC);
    zend_object_value new_ov = hs_object_new_ex(old_obj->std.ce,
                                                &new_obj TSRMLS_CC);

    zend_objects_clone_members(&new_obj->std, new_ov, &old_obj->std,
                               Z_OBJ_HANDLE_P(this_ptr) TSRMLS_CC);

    new_obj->timeout = old_obj->timeout;

    MAKE_STD_ZVAL(new_obj->server);
    *new_obj->server = *old_obj->server;
    zval_copy_ctor(new_obj->server);

    if (old_obj->auth) {
        MAKE_STD_ZVAL(new_obj->auth);
        *new_obj->auth = *old_obj->auth;
        zval_copy_ctor(new_obj->auth);
    } else {
        new_obj->auth = NULL;
    }

    new_obj->error = NULL;
	new_obj->hashkey_len = old_obj->hashkey_len;
	new_obj->hashkey = estrdup(old_obj->hashkey);

	if (old_obj->conn->is_persistent) {
		new_obj->conn = old_obj->conn;
	} else {
		new_obj->conn = ecalloc(1, sizeof(hs_conn_t));
		zend_hash_init(&new_obj->conn->open_indices, 16, NULL, NULL, 0);

		hs_object_connection(new_obj);
	}
    return new_ov;
}

PHP_HANDLERSOCKETI_API int
handlersocketi_register_class(TSRMLS_D)
{
    zend_class_entry ce;

    INIT_CLASS_ENTRY(ce, "HandlerSocketi", hs_methods);

    ce.create_object = hs_object_new;

    hs_ce = zend_register_internal_class(&ce TSRMLS_CC);
    if (hs_ce == NULL) {
        return FAILURE;
    }

    memcpy(&hs_object_handlers, zend_get_std_object_handlers(),
           sizeof(zend_object_handlers));

    hs_object_handlers.clone_obj = hs_object_clone;

    return SUCCESS;
}

PHP_HANDLERSOCKETI_API zend_class_entry
*handlersocketi_get_ce(void)
{
    return hs_ce;
}

PHP_HANDLERSOCKETI_API php_stream
*handlersocketi_object_store_get_stream(zval *link TSRMLS_DC)
{
    hs_obj_t *hs;

    hs = (hs_obj_t *)zend_object_store_get_object(link TSRMLS_CC);
    if (hs && hs->conn) {
        return hs->conn->stream;
    } else {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Trying to use an object with already closed connection");
        return NULL;
    }
}

PHP_HANDLERSOCKETI_API long
handlersocketi_object_store_get_timeout(zval *link TSRMLS_DC)
{
    hs_obj_t *hs;

    hs = (hs_obj_t *)zend_object_store_get_object(link TSRMLS_CC);
    if (hs) {
        return hs->timeout;
    } else {
        return 0;
    }
}

PHP_HANDLERSOCKETI_API int
handlersocketi_object_store_get_index_id(zval *link, const char *hash_index, int hash_index_len TSRMLS_DC)
{
    hs_obj_t *hs;
	int *old_id;

    hs = (hs_obj_t *)zend_object_store_get_object(link TSRMLS_CC);
    if (hs && hs->conn) {
		if (zend_hash_find(&hs->conn->open_indices, hash_index, hash_index_len+1, (void **)&old_id) == SUCCESS) {
			return *old_id;
		}
    }
	return -1;
}

PHP_HANDLERSOCKETI_API int
handlersocketi_object_store_store_index_id(zval *link, const char *hash_index, int hash_index_len, int id TSRMLS_DC)
{
    hs_obj_t *hs;

    hs = (hs_obj_t *)zend_object_store_get_object(link TSRMLS_CC);
    if (hs && hs->conn) {
		return zend_hash_update(&hs->conn->open_indices, hash_index, hash_index_len + 1, (void *)&id, sizeof(int), NULL);
    }
	return FAILURE;
}

ZEND_METHOD(HandlerSocketi, __construct)
{
    char *server, *host = NULL;
    int is_persistent = 0, server_len, host_len = 0;
    long port = -1;
    zval *options = NULL;
    hs_obj_t *hs;
	hs_conn_t *conn = NULL;
	zend_rsrc_list_entry *le, new_le;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sl|z", &host, &host_len, &port, &options) == FAILURE) {
        zval *object = getThis();
        ZVAL_NULL(object);
        return;
    }

    if (!host || host_len <= 0) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "__construct(): no host name");
        zval *object = getThis();
        ZVAL_NULL(object);
        return;
    }

    if (port > 0) {
        server_len = spprintf(&server, 0, "%s:%ld", host, port);
    } else {
        server_len = spprintf(&server, 0, "%s", host);
    }

    hs = (hs_obj_t *)zend_object_store_get_object(getThis() TSRMLS_CC);
    HS_CHECK_OBJECT(hs, HandlerSocketi);

    MAKE_STD_ZVAL(hs->server);
    ZVAL_STRINGL(hs->server, server, server_len, 0);

	hs->hashkey_len = spprintf(&hs->hashkey, 0, "hsi:%s", server);
    hs->timeout = HS_STREAM_DEFAULT_TIMEOUT;

    if (options && Z_TYPE_P(options) == IS_ARRAY) {
        zval **tmp;
        if (zend_hash_find(Z_ARRVAL_P(options), "timeout", sizeof("timeout"), (void **)&tmp) == SUCCESS) {
            convert_to_long_ex(tmp);
            hs->timeout = Z_LVAL_PP(tmp);
        }

        if (zend_hash_find(Z_ARRVAL_P(options), "persistent", sizeof("persistent"), (void **)&tmp) == SUCCESS) {
            convert_to_boolean_ex(tmp);
			is_persistent = Z_BVAL_PP(tmp);
        }
    }

	if (is_persistent) {
		/* look for existing hash first */
		if (zend_hash_find(&EG(persistent_list), hs->hashkey, hs->hashkey_len + 1, (void **) &le) == SUCCESS) {
			if (le->type == le_hs_pconn && le->ptr) {
				conn = (hs_conn_t *)le->ptr;
			} else {
				zend_hash_del(&EG(persistent_list), hs->hashkey, hs->hashkey_len + 1);
			}
		}

		/* invalid or not found -> create new one */
		if (conn == NULL) {
			conn = pemalloc(sizeof(hs_conn_t), 1);
			conn->is_persistent = 1;
			zend_hash_init(&conn->open_indices, 16, NULL, NULL, 1);

			new_le.ptr = conn;
			new_le.type = le_hs_pconn;

			zend_hash_update(&EG(persistent_list), hs->hashkey, hs->hashkey_len + 1, (void *) &new_le, sizeof(zend_rsrc_list_entry), NULL);
		}
	} else {
		/* open indices hash is private for every non-persistent connection */
		conn = pemalloc(sizeof(hs_conn_t), 0);
		conn->is_persistent = 0;
		zend_hash_init(&conn->open_indices, 16, NULL, NULL, 0);
		conn->rsrc_id = zend_list_insert(conn, le_hs_conn);
	}
	hs->conn = conn;

    if (hs_object_connection(hs) != SUCCESS) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "unable to connect %s", Z_STRVAL_P(hs->server));
        zval *object = getThis();
        ZVAL_NULL(object);
		return;
    }
}

ZEND_METHOD(HandlerSocketi, auth)
{
    char *key, *type = NULL;
    int key_len, type_len = 0;
    hs_obj_t *hs;
    smart_str request = {0};

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|s", &key, &key_len, &type, &type_len) == FAILURE) {
        RETURN_FALSE;
    }

    if (key_len <= 0) {
        RETURN_FALSE;
    }

    /* handerlsocket: object */
    hs = (hs_obj_t *)zend_object_store_get_object(getThis() TSRMLS_CC);
    HS_CHECK_OBJECT(hs, HandlerSocketi);
    HS_CHECK_CONNECTION(hs, HandlerSocketi);

    MAKE_STD_ZVAL(hs->auth);
    ZVAL_STRINGL(hs->auth, key, key_len, 1);

    /* auth */
    hs_request_string(&request, HS_PROTOCOL_AUTH, 1);
    hs_request_delim(&request);
    hs_request_string(&request, "1", 1);
    hs_request_delim(&request);
    hs_request_string(&request, Z_STRVAL_P(hs->auth), Z_STRLEN_P(hs->auth));
    hs_request_next(&request);

    /* request: send */
    if (hs_request_send(hs->conn->stream, &request TSRMLS_CC) < 0) {
        ZVAL_BOOL(return_value, 0);
    } else {
        zval *retval;
        MAKE_STD_ZVAL(retval);

        /* response */
        hs_response_value(hs->conn->stream, hs->timeout, retval, NULL, 0 TSRMLS_CC);
        if (Z_TYPE_P(retval) == IS_BOOL && Z_LVAL_P(retval) == 1) {
            ZVAL_BOOL(return_value, 1);
        } else {
            ZVAL_BOOL(return_value, 0);
        }

        zval_ptr_dtor(&retval);
    }

    smart_str_free(&request);
}

ZEND_METHOD(HandlerSocketi, open_index)
{
    char *db, *table;
    int db_len, table_len;
    zval *fields, *options = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ssz|z",
                              &db, &db_len, &table, &table_len,
                              &fields, &options) == FAILURE) {
        return;
    }

    handlersocketi_create_index(return_value, getThis(), db, db_len,
                                table, table_len, fields, options TSRMLS_CC);
}
