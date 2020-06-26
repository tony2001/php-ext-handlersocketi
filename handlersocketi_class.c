
#include "php.h"
#include "php_ini.h"
#include "php_streams.h"
#include "ext/standard/php_smart_string.h"
#include "Zend/zend_exceptions.h"

#include "php_verdep.h"
#include "php_handlersocketi.h"
#include "handlersocketi_class.h"
#include "handlersocketi_index.h"
#include "handlersocketi_exception.h"
#include "hs_common.h"
#include "hs_request.h"
#include "hs_response.h"

ZEND_EXTERN_MODULE_GLOBALS(handlersocketi);

#define HS_STREAM_DEFAULT_TIMEOUT 5.0

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
ZEND_METHOD(HandlerSocketi, has_open_index);

const zend_function_entry hs_methods[] = {
    ZEND_ME(HandlerSocketi, __construct, arginfo_hs_method__construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
    ZEND_ME(HandlerSocketi, auth, arginfo_hs_method_auth, ZEND_ACC_PUBLIC)
    ZEND_ME(HandlerSocketi, open_index, arginfo_hs_method_open_index, ZEND_ACC_PUBLIC)
    ZEND_ME(HandlerSocketi, has_open_index, arginfo_hs_method_open_index, ZEND_ACC_PUBLIC)
    ZEND_MALIAS(HandlerSocketi, openIndex, open_index, arginfo_hs_method_open_index, ZEND_ACC_PUBLIC)
    ZEND_MALIAS(HandlerSocketi, hasOpenIndex, has_open_index, arginfo_hs_method_open_index, ZEND_ACC_PUBLIC)
    ZEND_FE_END
};

static inline hs_obj_t *php_hs(zend_object *obj) {
	return (hs_obj_t *)((char*)(obj) - XtOffsetOf(hs_obj_t, std));
}

#define HS_OBJ(zv, obj)													\
	obj = php_hs(Z_OBJ_P(zv));											\
	if (!(obj)) {														\
		HS_EXCEPTION("The HandlerSocketi object has not been correctly initialized by its constructor"); \
		RETURN_FALSE;													\
	}

static void
hs_object_free_storage(zend_object *object)
{
    hs_obj_t *intern = php_hs(object);
    zend_object_std_dtor(&intern->std);

    if (intern->conn && !intern->conn->is_persistent) {
		zend_list_delete(intern->conn->rsrc);
    }

	zval_ptr_dtor(&intern->server);
	zval_ptr_dtor(&intern->auth);
	zval_ptr_dtor(&intern->error);

	efree(intern->hashkey);
}

#define HS_EXCEPTION(...)                                                                                  \
    zend_throw_exception_ex(handlersocketi_get_ce_exception(), 0, "HandlerSocketi::" __VA_ARGS__)

#define HS_EXCEPTION_EX(code, ...)                                                                                  \
    zend_throw_exception_ex(handlersocketi_get_ce_exception(), code, "HandlerSocketi::" __VA_ARGS__)

#define HS_CHECK_OBJECT(object, classname)                        \
	if (!(object)) {                                              \
		HS_EXCEPTION("The " #classname " object has not been correctly initialized by its constructor"); \
		RETURN_FALSE;                                             \
	}

#define HS_CHECK_CONNECTION(object, classname)                    \
	if (!(object)->conn || !(object)->conn->stream) {             \
		HS_EXCEPTION("Trying to uset a " #classname " object with already closed connection"); \
		RETURN_FALSE;                                             \
	}

static inline zend_object *hs_object_new_ex(zend_class_entry *ce, hs_obj_t **ptr)
{
	hs_obj_t *intern;

	intern = ecalloc(1, sizeof(hs_obj_t) + zend_object_properties_size(ce));
	if (ptr) {
		*ptr = intern;
	}

	zend_object_std_init(&intern->std, ce);
	object_properties_init(&intern->std, ce);

	intern->std.handlers = &hs_object_handlers;

	intern->timeout = 0;
	intern->rw_timeout = 0;
	ZVAL_NULL(&intern->server);
	ZVAL_NULL(&intern->auth);
	ZVAL_NULL(&intern->error);
	return &intern->std;
}

static inline int hs_object_connection(hs_obj_t *obj, zend_string **errstr, int *errc)
{
	char *hashkey = NULL;
	struct timeval tv;

	if (obj->timeout > 0) {
		double fraction, integral;
		fraction = modf(obj->timeout, &integral);
		tv.tv_sec = (int)integral;
		tv.tv_usec = (int)(fraction*1000000);
	}

	if (obj->conn->is_persistent) {
		php_stream *stream;

		spprintf(&hashkey, 0, "stream:%s", obj->hashkey);

		/* this code is copied from main/streams/transports.c with small modifications */
		switch(php_stream_from_persistent_id(hashkey, &stream)) {
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

	obj->conn->stream = php_stream_xport_create(Z_STRVAL_P(&obj->server), Z_STRLEN_P(&obj->server), REPORT_ERRORS, STREAM_XPORT_CLIENT|STREAM_XPORT_CONNECT, hashkey, &tv, NULL, errstr, errc);

	if (hashkey) {
		efree(hashkey);
	}

	if (!obj->conn->stream) {
		return FAILURE;
	}

	/* non-blocking */
	if (php_stream_set_option(obj->conn->stream, PHP_STREAM_OPTION_BLOCKING, 0, NULL) == -1) {
		php_error_docref(NULL, E_WARNING, "HandlerSocketi: failed to turn on non-blocking mode");
	}

	php_stream_auto_cleanup(obj->conn->stream);
	return SUCCESS;
}

static inline zend_object *hs_object_new(zend_class_entry *ce)
{
	return hs_object_new_ex(ce, NULL);
}

static inline zend_object *hs_object_clone(zval *this_ptr)
{
	hs_obj_t *new_obj = NULL;
	hs_obj_t *old_obj = php_hs(Z_OBJ_P(this_ptr));
	zend_object *new_ov = hs_object_new_ex(old_obj->std.ce, &new_obj);

	zend_objects_clone_members(new_ov, Z_OBJ_P(this_ptr));

	new_obj->timeout = old_obj->timeout;
	new_obj->rw_timeout = old_obj->rw_timeout;

	ZVAL_COPY_VALUE(&new_obj->server, &old_obj->server);
	zval_copy_ctor(&new_obj->server);

	ZVAL_COPY_VALUE(&new_obj->auth, &old_obj->auth);
	zval_copy_ctor(&new_obj->auth);

	ZVAL_NULL(&new_obj->error);
	new_obj->hashkey_len = old_obj->hashkey_len;
	new_obj->hashkey = estrdup(old_obj->hashkey);

	if (old_obj->conn->is_persistent) {
		new_obj->conn = old_obj->conn;
	} else {
		new_obj->conn = ecalloc(1, sizeof(hs_conn_t));
		zend_hash_init(&new_obj->conn->open_indices, 16, NULL, ZVAL_PTR_DTOR, 0);

		hs_object_connection(new_obj, NULL, NULL);
	}
	return new_ov;
}

PHP_HANDLERSOCKETI_API int handlersocketi_register_class()
{
	zend_class_entry ce;

	INIT_CLASS_ENTRY(ce, "HandlerSocketi", hs_methods);
	ce.create_object = hs_object_new;

	hs_ce = zend_register_internal_class(&ce);
	if (hs_ce == NULL) {
		return FAILURE;
	}

	memcpy(&hs_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	hs_object_handlers.dtor_obj = zend_objects_destroy_object;
	hs_object_handlers.free_obj = hs_object_free_storage;
	hs_object_handlers.clone_obj = hs_object_clone;
	hs_object_handlers.offset = XtOffsetOf(hs_obj_t, std);
	return SUCCESS;
}

PHP_HANDLERSOCKETI_API zend_class_entry *handlersocketi_get_ce(void)
{
	return hs_ce;
}

PHP_HANDLERSOCKETI_API void handlersocketi_object_store_close_conn(zval *link)
{
	hs_obj_t *hs;

	hs = php_hs(Z_OBJ_P(link));
	if (hs && hs->conn && hs->conn->stream) {
		if (hs->conn->is_persistent) {
			php_stream_pclose(hs->conn->stream);
		} else {
			php_stream_close(hs->conn->stream);
		}
		hs->conn->stream = NULL;
	}
}

PHP_HANDLERSOCKETI_API php_stream *handlersocketi_object_store_get_stream(zval *link)
{
	hs_obj_t *hs;

	hs = php_hs(Z_OBJ_P(link));
	if (hs && hs->conn) {
		return hs->conn->stream;
	} else {
		return NULL;
	}
}

PHP_HANDLERSOCKETI_API double handlersocketi_object_store_get_timeout(zval *link)
{
	hs_obj_t *hs;

	hs = php_hs(Z_OBJ_P(link));
	if (hs) {
		return hs->timeout;
	} else {
		return 0;
	}
}

PHP_HANDLERSOCKETI_API double handlersocketi_object_store_get_rw_timeout(zval *link)
{
	hs_obj_t *hs;

	hs = php_hs(Z_OBJ_P(link));
	if (hs) {
		return hs->rw_timeout;
	} else {
		return 0;
	}
}

PHP_HANDLERSOCKETI_API int handlersocketi_object_store_get_index_id(zval *link, const char *hash_index, size_t hash_index_len)
{
	hs_obj_t *hs;
	zval *old_id;

	hs = php_hs(Z_OBJ_P(link));
	if (hs && hs->conn) {
		old_id = zend_hash_str_find(&hs->conn->open_indices, hash_index, hash_index_len);
		if (old_id) {
			return Z_LVAL_P(old_id);
		}
	}
	return -1;
}

PHP_HANDLERSOCKETI_API int handlersocketi_object_store_get_index_hash(const char *db, size_t db_len, const char *table, size_t table_len, zval *fields, zval *options, char **hash_index_str, size_t *hash_index_len)
{
	smart_string hash_index = {0};
	char *index = NULL;
	zval filter, fields_str;
	size_t index_len;

	ZVAL_NULL(&filter);

	hs_zval_to_comma_string(fields, &fields_str);

	smart_string_appendl(&hash_index, db, db_len);
	smart_string_appendc(&hash_index, '|');

	smart_string_appendl(&hash_index, table, table_len);
	smart_string_appendc(&hash_index, '|');

	if (options) {
		zval *tmp;

		tmp = zend_hash_str_find(Z_ARRVAL_P(options), "index", sizeof("index") - 1);
		if (tmp) {
			index = Z_STRVAL_P(tmp);
			index_len = Z_STRLEN_P(tmp);
			if (!index_len) {
				zval_ptr_dtor(&fields_str);
				smart_string_free(&hash_index);
				return FAILURE;
			}
		}

		tmp = zend_hash_str_find(Z_ARRVAL_P(options), "filter", sizeof("filter") - 1);
		if (tmp) {
			hs_zval_to_comma_array(tmp, &filter);
		}
	}

	if (!index) {
		smart_string_appendl(&hash_index, "PRIMARY", sizeof("PRIMARY")-1);
	} else {
		smart_string_appendl(&hash_index, index, index_len);
	}
	smart_string_appendc(&hash_index, '|');

	/* make sure we're case insensitive */
	zend_str_tolower(Z_STRVAL_P(&fields_str), Z_STRLEN_P(&fields_str));
	smart_string_appendl(&hash_index, Z_STRVAL_P(&fields_str), Z_STRLEN_P(&fields_str));
	smart_string_appendc(&hash_index, '|');

	if (Z_TYPE_P(&filter) == IS_ARRAY) {
		zval *tmp;
		long n, i = 0;

		n = zend_hash_num_elements(HASH_OF(&filter));
		if (n >= 0) {
			smart_string_appendc(&hash_index, '|');

			zend_hash_internal_pointer_reset(HASH_OF(&filter));
			while ((tmp = zend_hash_get_current_data(HASH_OF(&filter))) != NULL) {
				switch (Z_TYPE_P(tmp)) {
					case IS_STRING:
						smart_string_appendl(&hash_index, Z_STRVAL_P(tmp), Z_STRLEN_P(tmp));
						break;
					case IS_LONG:
						smart_string_append_long(&hash_index, Z_LVAL_P(tmp));
						break;
					default:
						{
							zend_string *tmp_str = zval_get_string(tmp);
							smart_string_appendl(&hash_index, tmp_str->val, tmp_str->len);
							zend_string_release(tmp_str);
						}
						break;
				}

				if (++i != n) {
					smart_string_appendc(&hash_index, '|');
				}
				zend_hash_move_forward(HASH_OF(&filter));
			}
		}
	}

	smart_string_0(&hash_index);
	zval_ptr_dtor(&fields_str);
	zval_ptr_dtor(&filter);

	*hash_index_str = hash_index.c;
	*hash_index_len = hash_index.len;

	return SUCCESS;
}

PHP_HANDLERSOCKETI_API int handlersocketi_object_store_store_index_id(zval *link, const char *hash_index, size_t hash_index_len, size_t id)
{
	hs_obj_t *hs;

	hs = php_hs(Z_OBJ_P(link));
	if (hs && hs->conn) {
		zval tmp;
		ZVAL_LONG(&tmp, id);
		zend_hash_str_update(&hs->conn->open_indices, hash_index, hash_index_len, &tmp);
		return SUCCESS;
	}
	return FAILURE;
}

PHP_HANDLERSOCKETI_API int handlersocketi_object_store_remove_index(zval *link, const char *hash_index, size_t hash_index_len)
{
	hs_obj_t *hs;

	hs = php_hs(Z_OBJ_P(link));
	if (hs && hs->conn && zend_hash_num_elements(&hs->conn->open_indices) > 0) {
		zend_hash_str_del(&hs->conn->open_indices, hash_index, hash_index_len);
		return SUCCESS;
	}
	return FAILURE;
}

ZEND_METHOD(HandlerSocketi, __construct)
{
	char *server, *host = NULL;
	size_t is_persistent = 0, server_len, host_len = 0;
	long port = -1;
	zval *options = NULL;
	hs_obj_t *hs;
	hs_conn_t *conn = NULL;
	zend_resource *le, new_le;
	zend_string *errstr;
	int errcode = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "sl|z", &host, &host_len, &port, &options) == FAILURE) {
		return;
	}

	if (!host || host_len <= 0) {
		HS_EXCEPTION("__construct(): no host name");
		return;
	}

	if (port > 0) {
		server_len = spprintf(&server, 0, "%s:%ld", host, port);
	} else {
		server_len = spprintf(&server, 0, "%s", host);
	}

	HS_OBJ(getThis(), hs);
	HS_CHECK_OBJECT(hs, HandlerSocketi);

	ZVAL_STRINGL(&hs->server, server, server_len);

	hs->hashkey_len = spprintf(&hs->hashkey, 0, "hsi:%s", server);
	hs->timeout = HS_STREAM_DEFAULT_TIMEOUT;
	hs->rw_timeout = HS_STREAM_DEFAULT_TIMEOUT;
	efree(server);

	if (options && Z_TYPE_P(options) == IS_ARRAY) {
		zval *tmp;
		tmp = zend_hash_str_find(Z_ARRVAL_P(options), "timeout", sizeof("timeout") - 1);
		if (tmp) {
			hs->timeout = zval_get_double(tmp);
		}

		tmp = zend_hash_str_find(Z_ARRVAL_P(options), "rw_timeout", sizeof("rw_timeout") - 1);
		if (tmp) {
			hs->rw_timeout = zval_get_double(tmp);
		}

		tmp = zend_hash_str_find(Z_ARRVAL_P(options), "persistent", sizeof("persistent") - 1);
		if (tmp) {
			is_persistent = zval_get_long(tmp);
		}
	}

	if (is_persistent) {
		zval *zv;
		/* look for existing hash first */
		zv = zend_hash_str_find(&EG(persistent_list), hs->hashkey, hs->hashkey_len);
		if (zv && Z_RES_P(zv)) {
			le = Z_RES_P(zv);
			if (le->type == le_hs_pconn && le->ptr) {
				conn = (hs_conn_t *)le->ptr;
			} else {
				zend_hash_str_del(&EG(persistent_list), hs->hashkey, hs->hashkey_len);
			}
		}

		if (conn) {
			/* check liveness */
			if (php_stream_set_option(conn->stream, PHP_STREAM_OPTION_CHECK_LIVENESS, 0, NULL) == PHP_STREAM_OPTION_RETURN_ERR) {
				hs_conn_dtor(le TSRMLS_CC);
				conn = NULL;
			}
		}

		/* invalid or not found -> create new one */
		if (conn == NULL) {
			conn = pemalloc(sizeof(hs_conn_t), 1);
			conn->is_persistent = 1;
			zend_hash_init(&conn->open_indices, 16, NULL, ZVAL_PTR_DTOR, 1);

			new_le.ptr = conn;
			new_le.type = le_hs_pconn;

			zend_hash_str_update_mem(&EG(persistent_list), hs->hashkey, hs->hashkey_len, (void *) &new_le, sizeof(zend_resource));
		}
	} else {
		/* open indices hash is private for every non-persistent connection */
		conn = pemalloc(sizeof(hs_conn_t), 0);
		conn->is_persistent = 0;
		zend_hash_init(&conn->open_indices, 16, NULL, ZVAL_PTR_DTOR, 0);
		conn->rsrc = zend_register_resource(conn, le_hs_conn);
	}
	hs->conn = conn;

	if (hs_object_connection(hs, &errstr, &errcode) != SUCCESS) {
		HS_EXCEPTION_EX(errcode, "__construct(): unable to connect to %s: %s (%d)", Z_STRVAL_P(&hs->server), ZSTR_VAL(errstr), errcode);
		return;
	}
}

ZEND_METHOD(HandlerSocketi, auth)
{
	char *key, *type = NULL;
	size_t key_len, type_len = 0;
	hs_obj_t *hs;
	smart_string request = {0};

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "s|s", &key, &key_len, &type, &type_len) == FAILURE) {
		RETURN_FALSE;
	}

	if (key_len <= 0) {
		RETURN_FALSE;
	}

	/* handerlsocket: object */
	HS_OBJ(getThis(), hs);
	HS_CHECK_OBJECT(hs, HandlerSocketi);
	HS_CHECK_CONNECTION(hs, HandlerSocketi);

	ZVAL_STRINGL(&hs->auth, key, key_len);

	/* auth */
	hs_request_string(&request, HS_PROTOCOL_AUTH, 1);
	hs_request_delim(&request);
	hs_request_string(&request, "1", 1);
	hs_request_delim(&request);
	hs_request_string(&request, Z_STRVAL_P(&hs->auth), Z_STRLEN_P(&hs->auth));
	hs_request_next(&request);

	/* request: send */
	if (hs_request_send(hs->conn->stream, &request) < 0) {
		ZVAL_BOOL(return_value, 0);
	} else {
		zval retval;

		/* response */
		hs_response_value(hs->conn->stream, hs->rw_timeout, &retval, NULL, 0);
		if (Z_TYPE_P(&retval) == IS_TRUE) {
			ZVAL_BOOL(return_value, 1);
		} else {
			ZVAL_BOOL(return_value, 0);
		}

		zval_ptr_dtor(&retval);
	}

	smart_string_free(&request);
}

ZEND_METHOD(HandlerSocketi, open_index)
{
	char *db, *table;
	size_t db_len, table_len;
	zval *fields, *options = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "ssz|z", &db, &db_len, &table, &table_len, &fields, &options) == FAILURE) {
		return;
	}

	handlersocketi_create_index(return_value, getThis(), db, db_len, table, table_len, fields, options);
}

ZEND_METHOD(HandlerSocketi, has_open_index)
{
	char *db, *table;
	size_t db_len, table_len, hash_index_len;
	int ret, id;
	zval *fields, *options = NULL;
	char *hash_index;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "ssz|z", &db, &db_len, &table, &table_len, &fields, &options) == FAILURE) {
		return;
	}

	ret = handlersocketi_object_store_get_index_hash(db, db_len, table, table_len, fields, options, &hash_index, &hash_index_len);
	if (ret != SUCCESS) {
		HS_EXCEPTION("has_open_index(): invalid index parameters passed");
		RETURN_FALSE;
	}

	id = handlersocketi_object_store_get_index_id(getThis(), hash_index, hash_index_len);
	efree(hash_index);
	if (id < 0) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
