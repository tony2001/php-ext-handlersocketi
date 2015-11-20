
#include "php.h"
#include "php_ini.h"
#include "ext/standard/php_smart_string.h"
#include "ext/standard/php_string.h"
#include "zend_exceptions.h"

#include "php_verdep.h"
#include "php_handlersocketi.h"
#include "handlersocketi_exception.h"
#include "handlersocketi_class.h"
#include "handlersocketi_index.h"
#include "hs_common.h"
#include "hs_request.h"
#include "hs_response.h"

ZEND_EXTERN_MODULE_GLOBALS(handlersocketi);

#define HS_DEFAULT_LIMIT  1
#define HS_DEFAULT_OFFSET 0

static zend_class_entry *hs_ce_index;
static zend_object_handlers hs_index_object_handlers;

typedef struct hs_index_obj {
	long id;
	zval name;
	zval db;
	zval table;
	zval field;
	long field_num;
	zval filter;
	zval link;
	zval error;
	zend_object std;
	char *hash;
} hs_index_obj_t;

	ZEND_BEGIN_ARG_INFO_EX(arginfo_hs_index_method__construct, 0, 0, 4)
	ZEND_ARG_INFO(0, hs)
	ZEND_ARG_INFO(0, db)
	ZEND_ARG_INFO(0, table)
	ZEND_ARG_INFO(0, fields)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

	ZEND_BEGIN_ARG_INFO_EX(arginfo_hs_index_method_find, 0, 0, 1)
	ZEND_ARG_INFO(0, query)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

	ZEND_BEGIN_ARG_INFO_EX(arginfo_hs_index_method_insert, 0, 0, 0)
ZEND_END_ARG_INFO()

	ZEND_BEGIN_ARG_INFO_EX(arginfo_hs_index_method_update, 0, 0, 2)
	ZEND_ARG_INFO(0, query)
	ZEND_ARG_INFO(0, update)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

	ZEND_BEGIN_ARG_INFO_EX(arginfo_hs_index_method_remove, 0, 0, 1)
	ZEND_ARG_INFO(0, query)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

	ZEND_BEGIN_ARG_INFO_EX(arginfo_hs_index_method_multi, 0, 0, 1)
	ZEND_ARG_INFO(0, args)
ZEND_END_ARG_INFO()

	ZEND_BEGIN_ARG_INFO_EX(arginfo_hs_index_method_get_error, 0, 0, 0)
ZEND_END_ARG_INFO()

	ZEND_BEGIN_ARG_INFO_EX(arginfo_hs_index_method_get_id, 0, 0, 0)
ZEND_END_ARG_INFO()

	ZEND_BEGIN_ARG_INFO_EX(arginfo_hs_index_method_get_name, 0, 0, 0)
ZEND_END_ARG_INFO()

	ZEND_BEGIN_ARG_INFO_EX(arginfo_hs_index_method_get_db, 0, 0, 0)
ZEND_END_ARG_INFO()

	ZEND_BEGIN_ARG_INFO_EX(arginfo_hs_index_method_get_table, 0, 0, 0)
ZEND_END_ARG_INFO()

	ZEND_BEGIN_ARG_INFO_EX(arginfo_hs_index_method_get_field, 0, 0, 0)
ZEND_END_ARG_INFO()

	ZEND_BEGIN_ARG_INFO_EX(arginfo_hs_index_method_get_filter, 0, 0, 0)
ZEND_END_ARG_INFO()

	ZEND_BEGIN_ARG_INFO_EX(arginfo_hs_index_method_get_operator, 0, 0, 0)
ZEND_END_ARG_INFO()

	ZEND_METHOD(HandlerSocketi_Index, __construct);
	ZEND_METHOD(HandlerSocketi_Index, find);
	ZEND_METHOD(HandlerSocketi_Index, insert);
	ZEND_METHOD(HandlerSocketi_Index, update);
	ZEND_METHOD(HandlerSocketi_Index, remove);
	ZEND_METHOD(HandlerSocketi_Index, multi);
	ZEND_METHOD(HandlerSocketi_Index, get_error);
	ZEND_METHOD(HandlerSocketi_Index, get_id);
	ZEND_METHOD(HandlerSocketi_Index, get_name);
	ZEND_METHOD(HandlerSocketi_Index, get_db);
	ZEND_METHOD(HandlerSocketi_Index, get_table);
	ZEND_METHOD(HandlerSocketi_Index, get_field);
	ZEND_METHOD(HandlerSocketi_Index, get_filter);
	ZEND_METHOD(HandlerSocketi_Index, get_operator);

	const zend_function_entry hs_index_methods[] = {
		ZEND_ME(HandlerSocketi_Index, __construct,
				arginfo_hs_index_method__construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
			ZEND_ME(HandlerSocketi_Index, find,
					arginfo_hs_index_method_find, ZEND_ACC_PUBLIC)
			ZEND_ME(HandlerSocketi_Index, insert,
					arginfo_hs_index_method_insert, ZEND_ACC_PUBLIC)
			ZEND_ME(HandlerSocketi_Index, update,
					arginfo_hs_index_method_update, ZEND_ACC_PUBLIC)
			ZEND_ME(HandlerSocketi_Index, remove,
					arginfo_hs_index_method_remove, ZEND_ACC_PUBLIC)
			ZEND_ME(HandlerSocketi_Index, multi,
					arginfo_hs_index_method_multi, ZEND_ACC_PUBLIC)
			ZEND_ME(HandlerSocketi_Index, get_error,
					arginfo_hs_index_method_get_error, ZEND_ACC_PUBLIC)
			ZEND_ME(HandlerSocketi_Index, get_id,
					arginfo_hs_index_method_get_id, ZEND_ACC_PUBLIC)
			ZEND_ME(HandlerSocketi_Index, get_name,
					arginfo_hs_index_method_get_name, ZEND_ACC_PUBLIC)
			ZEND_ME(HandlerSocketi_Index, get_db,
					arginfo_hs_index_method_get_db, ZEND_ACC_PUBLIC)
			ZEND_ME(HandlerSocketi_Index, get_table,
					arginfo_hs_index_method_get_table, ZEND_ACC_PUBLIC)
			ZEND_ME(HandlerSocketi_Index, get_field,
					arginfo_hs_index_method_get_field, ZEND_ACC_PUBLIC)
			ZEND_ME(HandlerSocketi_Index, get_filter,
					arginfo_hs_index_method_get_filter, ZEND_ACC_PUBLIC)
			ZEND_ME(HandlerSocketi_Index, get_operator,
					arginfo_hs_index_method_get_operator, ZEND_ACC_PUBLIC)
			ZEND_MALIAS(HandlerSocketi_Index, getError, get_error,
					arginfo_hs_index_method_get_error, ZEND_ACC_PUBLIC)
			ZEND_MALIAS(HandlerSocketi_Index, getId, get_id,
					arginfo_hs_index_method_get_id, ZEND_ACC_PUBLIC)
			ZEND_MALIAS(HandlerSocketi_Index, getName, get_name,
					arginfo_hs_index_method_get_name, ZEND_ACC_PUBLIC)
			ZEND_MALIAS(HandlerSocketi_Index, getDb, get_db,
					arginfo_hs_index_method_get_db, ZEND_ACC_PUBLIC)
			ZEND_MALIAS(HandlerSocketi_Index, getTable, get_table,
					arginfo_hs_index_method_get_table, ZEND_ACC_PUBLIC)
			ZEND_MALIAS(HandlerSocketi_Index, getField, get_field,
					arginfo_hs_index_method_get_field, ZEND_ACC_PUBLIC)
			ZEND_MALIAS(HandlerSocketi_Index, getFilter, get_filter,
					arginfo_hs_index_method_get_filter, ZEND_ACC_PUBLIC)
			ZEND_MALIAS(HandlerSocketi_Index, getOperator, get_operator,
					arginfo_hs_index_method_get_operator, ZEND_ACC_PUBLIC)
			ZEND_FE_END
	};


static inline hs_index_obj_t *php_hs_index(zend_object *obj) {
	return (hs_index_obj_t *)((char*)(obj) - XtOffsetOf(hs_index_obj_t, std));
}

#define HS_INDEX_OBJ(zv, obj)											\
	obj = php_hs_index(Z_OBJ_P(zv));									\
if (!(obj)) {														\
	HS_EXCEPTION("The HandlerSocketi_Index object has not been correctly initialized by its constructor"); \
	RETURN_FALSE;													\
}

#define HS_EXCEPTION(...)                                       \
	zend_throw_exception_ex(handlersocketi_get_ce_exception(), 0, "HandlerSocketi_Index::" __VA_ARGS__); \

#define HS_CHECK_OBJECT(object, classname)                        \
	if (!(object)) {                                              \
		HS_EXCEPTION("The " #classname " object has not been correctly initialized by its constructor"); \
		RETURN_FALSE;                                             \
	}

#define HS_ERROR_RESET(error)  \
	zval_ptr_dtor(&error); \
ZVAL_NULL(&error)

static inline void hs_index_object_free_storage(zend_object *object)
{
	hs_index_obj_t *intern = php_hs_index(object);
	zend_object_std_dtor(&intern->std);

	if (intern->hash) {
		handlersocketi_object_store_remove_index(&intern->link, intern->hash, strlen(intern->hash));
		efree(intern->hash);
	}
	zval_ptr_dtor(&intern->link);
	zval_ptr_dtor(&intern->name);
	zval_ptr_dtor(&intern->db);
	zval_ptr_dtor(&intern->table);
	zval_ptr_dtor(&intern->field);
	zval_ptr_dtor(&intern->filter);
	zval_ptr_dtor(&intern->error);
}

static inline zend_object *hs_index_object_new_ex(zend_class_entry *ce, hs_index_obj_t **ptr)
{
	hs_index_obj_t *intern;

	intern = ecalloc(1, sizeof(hs_index_obj_t) + zend_object_properties_size(ce));
	if (ptr) {
		*ptr = intern;
	}

	zend_object_std_init(&intern->std, ce);
	object_properties_init(&intern->std, ce);

	intern->std.handlers = &hs_index_object_handlers;

	intern->id = 0;
	ZVAL_NULL(&intern->name);
	ZVAL_NULL(&intern->db);
	ZVAL_NULL(&intern->table);
	ZVAL_NULL(&intern->field);
	intern->field_num = 0;
	ZVAL_NULL(&intern->filter);
	ZVAL_NULL(&intern->link);
	ZVAL_NULL(&intern->error);

	return &intern->std;
}


static inline zend_object *hs_index_object_new(zend_class_entry *ce)
{
	return hs_index_object_new_ex(ce, NULL);
}

static inline zend_object *hs_index_object_clone(zval *this_ptr)
{
	hs_index_obj_t *new_obj = NULL;
	hs_index_obj_t *old_obj = php_hs_index(Z_OBJ_P(this_ptr));
	zend_object *new_ov = hs_index_object_new_ex(old_obj->std.ce, &new_obj);

	zend_objects_clone_members(new_ov, Z_OBJ_P(this_ptr));

	new_obj->id = old_obj->id;

	ZVAL_COPY_VALUE(&new_obj->name, &old_obj->name);
	zval_copy_ctor(&new_obj->name);

	ZVAL_COPY_VALUE(&new_obj->db, &old_obj->db);
	zval_copy_ctor(&new_obj->db);

	ZVAL_COPY_VALUE(&new_obj->table, &old_obj->table);
	zval_copy_ctor(&new_obj->table);

	ZVAL_COPY_VALUE(&new_obj->field, &old_obj->field);
	zval_copy_ctor(&new_obj->field);

	new_obj->field_num = old_obj->field_num;

	ZVAL_COPY_VALUE(&new_obj->filter, &old_obj->filter);
	zval_copy_ctor(&new_obj->filter);

	ZVAL_COPY_VALUE(&new_obj->link, &old_obj->link);
	zval_copy_ctor(&new_obj->link);

	ZVAL_NULL(&new_obj->error);

	return (zend_object *)new_ov;
}

void hs_zval_to_comma_string(zval *val, zval *retval)
{
	smart_string comma = {0};

	if (Z_TYPE_P(val) == IS_ARRAY) {
		zval *tmp;

		zend_hash_internal_pointer_reset(HASH_OF(val));
		while ((tmp = zend_hash_get_current_data(HASH_OF(val))) != NULL) {
			zend_string *str = zval_get_string(tmp);
			if (str->len > 0) {
				smart_string_appendl(&comma, str->val, str->len);
				smart_string_appendl(&comma, ",", strlen(","));
			}
			zend_hash_move_forward(HASH_OF(val));
			zend_string_release(str);
		}
		if (comma.len > 0) {
			comma.len--;
		}
	} else {
		zend_string *str = zval_get_string(val);
		if (str->len) {
			smart_string_appendl(&comma, str->val, str->len);
		}
		zend_string_release(str);
	}
	smart_string_0(&comma);
	ZVAL_STRINGL(retval, comma.c, comma.len);
	smart_string_free(&comma);
}

void hs_zval_to_comma_array(zval *val, zval *retval)
{
	if (Z_TYPE_P(val) == IS_ARRAY) {
		ZVAL_COPY_VALUE(retval, val);
		zval_copy_ctor(retval);
	} else if (Z_TYPE_P(val) == IS_STRING) {
		zval delim;
		ZVAL_STRINGL(&delim, ",", strlen(","));

		array_init(retval);
		php_explode(Z_STR_P(&delim), Z_STR_P(val), retval, LONG_MAX);
		zval_ptr_dtor(&delim);
	} else {
		ZVAL_NULL(retval);
	}
}

static inline int hs_get_options_long(HashTable *options, char *item, long def)
{
	zval *tmp;

	tmp = zend_hash_str_find(options, item, strlen(item));
	if (tmp && Z_TYPE_P(tmp) == IS_LONG) {
		return Z_LVAL_P(tmp);
	}
	return def;
}

static inline int hs_is_options_safe(HashTable *options)
{
	zval *tmp;

	tmp = zend_hash_str_find(options, "safe", sizeof("safe") - 1);
	if (!tmp) {
		return 0;
	}

	switch (Z_TYPE_P(tmp)) {
		case IS_STRING:
			return 1;
		case IS_LONG:
			if (Z_LVAL_P(tmp) >= 1) {
				return 1;
			}
		case IS_TRUE:
			return 1;
	}
	return 0;
}

static inline int hs_zval_to_operate_criteria(zval *query, zval *operate, zval *criteria, char *defaults)
{
	if (query == NULL) {
		return FAILURE;
	}

	if (Z_TYPE_P(query) == IS_ARRAY) {
		zend_string *key;
		zend_ulong index;
		zval *tmp;

		tmp = zend_hash_get_current_data(HASH_OF(query));
		if (!tmp) {
			return FAILURE;
		}

		if (zend_hash_get_current_key(HASH_OF(query), &key, &index) == HASH_KEY_IS_STRING) {
			ZVAL_STRINGL(operate, key->val, key->len);
			ZVAL_COPY_VALUE(criteria, tmp);
			return SUCCESS;
		}
	}

	ZVAL_STRINGL(operate, defaults, strlen(defaults));
	ZVAL_COPY_VALUE(criteria, query);
	return SUCCESS;
}

static inline void hs_zval_search_key(zval *value, zval *array, zval *return_value)
{
	zval *entry, res;
	zend_ulong index;
	zend_string *key;
	int (*is_equal_func)(zval *, zval *, zval *) = is_equal_function;

	zend_hash_internal_pointer_reset(HASH_OF(array));
	while ((entry = zend_hash_get_current_data(HASH_OF(array))) != NULL) {
		is_equal_func(&res, value, entry);
		if (Z_LVAL(res)) {
			switch (zend_hash_get_current_key(HASH_OF(array), &key, &index)) {
				case HASH_KEY_IS_STRING:
					ZVAL_STR(return_value, zend_string_copy(key));
					break;
				case HASH_KEY_IS_LONG:
					ZVAL_LONG(return_value, index);
					break;
				default:
					ZVAL_NULL(return_value);
					break;
			}
			return;
		}
		zend_hash_move_forward(HASH_OF(array));
	}
	ZVAL_NULL(return_value);
}

static inline void hs_zval_to_filter(zval *return_value, zval *filter, zval *value, char *type)
{
	zval *tmp, *ftmp, *vtmp, index, item;
	zend_string *tmp_str;
	long n;

	ZVAL_NULL(return_value);
	if (value == NULL || Z_TYPE_P(value) != IS_ARRAY) {
		return;
	}

	n = zend_hash_num_elements(HASH_OF(value));

	if (!n) {
		return;
	}

	ftmp = zend_hash_index_find(HASH_OF(value), 0);
	if (!ftmp) {
		return;
	}

	zend_hash_internal_pointer_reset(HASH_OF(value));

	if (Z_TYPE_P(ftmp) == IS_ARRAY) {
		do {
			zend_hash_move_forward(HASH_OF(value));
			hs_zval_to_filter(return_value, filter, ftmp, type);
		} while ((ftmp = zend_hash_get_current_data(HASH_OF(value))) != NULL);
		return;
	} else if (n < 3) {
		return;
	}

	tmp  = zend_hash_index_find(HASH_OF(value), 1);
	if (!tmp) {
		return;
	}

	hs_zval_search_key(ftmp, filter, &index);
	if (Z_TYPE_P(&index) != IS_LONG) {
		zval_ptr_dtor(&index);
		return;
	}

	vtmp = zend_hash_index_find(HASH_OF(value), 2);
	if (!vtmp) {
		zval_ptr_dtor(&index);
		return;
	}

	array_init(&item);

	add_next_index_stringl(&item, type, strlen(type));

	tmp_str = zval_get_string(tmp);
	add_next_index_str(&item, tmp_str);
	zend_string_release(tmp_str);

	add_next_index_long(&item, Z_LVAL_P(&index));

	if (Z_TYPE_P(vtmp) == IS_NULL) {
		add_next_index_null(&item);
	} else if (Z_TYPE_P(vtmp) == IS_LONG) {
		add_next_index_long(&item, Z_LVAL_P(vtmp));
	} else if (Z_TYPE_P(vtmp) == IS_DOUBLE) {
		add_next_index_double(&item, Z_DVAL_P(vtmp));
	} else {
		zend_string *vtmp_str = zval_get_string(vtmp);
		add_next_index_str(&item, vtmp_str);
		zend_string_release(vtmp_str);
	}


	if (Z_TYPE_P(return_value) != IS_ARRAY) {
		array_init(return_value);
	}

	add_next_index_zval(return_value, &item);
	zval_ptr_dtor(&item);
}

static inline void hs_array_to_in_filter(HashTable *ht, zval *filter, zval *filters, long *in_key, zval *in_values)
{
	zval *val;
	zend_string *key;
	zend_ulong key_index;

	ZVAL_NULL(filters);
	ZVAL_NULL(in_values);

	if (!filter) {
		return;
	}

	zend_hash_internal_pointer_reset(ht);
	while ((val = zend_hash_get_current_data(ht)) != NULL) {
		if (zend_hash_get_current_key(ht, &key, &key_index) != HASH_KEY_IS_STRING) {
			zend_hash_move_forward(ht);
			continue;
		}

		if (strcmp(key->val, "in") == 0) {
			/* in */
			if (Z_TYPE_P(val) == IS_ARRAY) {
				zval *tmp;
				zend_string *in_key_name;
				zend_ulong in_key_index;

				zend_hash_internal_pointer_reset(HASH_OF(val));
				tmp = zend_hash_get_current_data(HASH_OF(val));
				if (tmp != NULL) {
					if (Z_TYPE_P(tmp) == IS_ARRAY) {
						switch (zend_hash_get_current_key(HASH_OF(val), &in_key_name, &in_key_index)) {
							case HASH_KEY_NON_EXISTENT:
								*in_key = 0;
								break;
							case HASH_KEY_IS_LONG:
								*in_key = in_key_index;
								break;
							default:
								{
									zval key;
									ZVAL_STR(&key, in_key_name);
									*in_key = zval_get_long(&key);
									zval_ptr_dtor(&key);
									break;
								}
						}
						ZVAL_COPY_VALUE(in_values, tmp);
					} else {
						*in_key = 0;
						ZVAL_COPY_VALUE(in_values, val);
					}
				}
			} else {
				*in_key = 0;
				ZVAL_COPY_VALUE(in_values, val);
			}
		} else if (strcmp(key->val, "filter") == 0 && filter != NULL) {
			/* filter */
			hs_zval_to_filter(filters, filter, val, HS_PROTOCOL_FILTER);
		} else if (strcmp(key->val, "while") == 0 && filter != NULL) {
			/* while */
			hs_zval_to_filter(filters, filter, val, HS_PROTOCOL_WHILE);
		}

		zend_hash_move_forward(ht);
	}
}

static inline void hs_index_object_init(hs_index_obj_t *hsi, zval *this_ptr, zval *link, char *db, int db_len, char *table, int table_len, zval *fields, zval *options)
{
	long id = 0, old_id;
	zval index, fields_str, filter, retval;
	php_stream *stream;
	long timeout;
	smart_string request = {0};
	smart_string hash_index = {0};

	if (db_len == 0) {
		HS_EXCEPTION("invalid database %s", db);
		return;
	}

	if (table_len == 0) {
		HS_EXCEPTION("invalid table %s", table);
		return;
	}

	hs_zval_to_comma_string(fields, &fields_str);

	ZVAL_NULL(&index);
	ZVAL_NULL(&filter);
	if (options && Z_TYPE_P(options) == IS_ARRAY) {
		zval *tmp;
		tmp = zend_hash_str_find(Z_ARRVAL_P(options), "id", sizeof("id") - 1);

		if (tmp) {
			id = zval_get_long(tmp);
		}

		tmp = zend_hash_str_find(Z_ARRVAL_P(options), "index", sizeof("index") - 1);
		if (tmp) {
			zend_string *index_str = zval_get_string(tmp);
			ZVAL_STR(&index, index_str);
		}

		tmp = zend_hash_str_find(Z_ARRVAL_P(options), "filter", sizeof("filter") - 1);
		if (tmp) {
			hs_zval_to_comma_array(tmp, &filter);
		}
	}

	if (Z_TYPE_P(&index) == IS_NULL) {
		ZVAL_STRINGL(&index, "PRIMARY", sizeof("PRIMARY")-1);
	} else if (Z_STRLEN_P(&index) <= 0) {
		zval_ptr_dtor(&index);
		zval_ptr_dtor(&fields_str);
		zval_ptr_dtor(&filter);
		HS_EXCEPTION("invalid index");
		return;
	}

	if (id < 0) {
		id = HANDLERSOCKETI_G(id)++;
	}

	/* handerlsocket: object */
	if (!hsi) {
		HS_EXCEPTION("The HandlerSocketi_Index object has not been correctly initialized by its constructor");
		return;
	}

	ZVAL_COPY_VALUE(&hsi->link, link);
	zval_add_ref(&hsi->link);

	/* id */
	hsi->id = id;

	/* name */
	ZVAL_COPY_VALUE(&hsi->name, &index);
	zval_copy_ctor(&hsi->name);

	/* db */
	ZVAL_STRINGL(&hsi->db, db, db_len);

	/* table */
	ZVAL_STRINGL(&hsi->table, table, db_len);

	/* field */
	if (Z_TYPE_P(fields) == IS_STRING) {
		zval delim;

		array_init(&hsi->field);

		ZVAL_STRINGL(&delim, ",", strlen(","));
		php_explode(Z_STR_P(&delim), Z_STR_P(fields), &hsi->field, LONG_MAX);

		hsi->field_num = zend_hash_num_elements(HASH_OF(&hsi->field));

		zval_ptr_dtor(&delim);
	} else {
		hsi->field_num = zend_hash_num_elements(HASH_OF(fields));
		ZVAL_COPY_VALUE(&hsi->field, fields);
		zval_copy_ctor(&hsi->field);
	}

	if (Z_TYPE_P(&filter) != IS_NULL) {
		ZVAL_COPY_VALUE(&hsi->filter, &filter);
		zval_copy_ctor(&hsi->filter);
	} else {
		array_init(&hsi->filter);
	}

	/* stream */
	stream = handlersocketi_object_store_get_stream(&hsi->link);
	if (!stream) {
		ZVAL_NULL(this_ptr);
		return;
	}
	timeout = handlersocketi_object_store_get_timeout(&hsi->link);

	hs_request_string(&request, HS_PROTOCOL_OPEN, 1);
	hs_request_delim(&request);

	/* id */
	hs_request_long(&request, hsi->id);
	hs_request_delim(&request);

	/* db */
	hs_request_string(&request, db, db_len);
	hs_request_delim(&request);
	smart_string_appendl(&hash_index, db, db_len);
	smart_string_appendc(&hash_index, '|');

	/* table */
	hs_request_string(&request, table, table_len);
	hs_request_delim(&request);
	smart_string_appendl(&hash_index, table, table_len);
	smart_string_appendc(&hash_index, '|');

	/* index */
	hs_request_string(&request, Z_STRVAL_P(&index), Z_STRLEN_P(&index));
	hs_request_delim(&request);
	smart_string_appendl(&hash_index, Z_STRVAL_P(&index), Z_STRLEN_P(&index));
	smart_string_appendc(&hash_index, '|');

	/* fields */
	hs_request_string(&request, Z_STRVAL_P(&fields_str), Z_STRLEN_P(&fields_str));
	{
		char *lfields_str;

		/* make sure we're case insensitive */
		lfields_str = zend_str_tolower_dup(Z_STRVAL_P(&fields_str), Z_STRLEN_P(&fields_str));
		smart_string_appendl(&hash_index, lfields_str, Z_STRLEN_P(&fields_str));
		efree(lfields_str);
	}
	smart_string_appendc(&hash_index, '|');

	/* filters */
	hs_request_filter(&request, &hash_index, HASH_OF(&hsi->filter));

	/* eol */
	hs_request_next(&request);
	smart_string_0(&hash_index);

	/* first we look into the hash, then try to open the index for real */
	old_id = handlersocketi_object_store_get_index_id(link, hash_index.c, hash_index.len);
	if (old_id >= 0) {
		/* ok, we have this index in the hash, no need to open it again */
		hsi->id = old_id;
	} else {
		int res;

		/* request: send */
		if (hs_request_send(stream, &request) < 0) {
			zend_throw_exception_ex(handlersocketi_get_ce_io_exception(), 0, "failed to send request");
			goto cleanup;
		}

		/* read response */
		res = hs_response_value(stream, timeout, &retval, &hsi->error, 0);

		if (res == -1) {
			zend_throw_exception_ex(handlersocketi_get_ce_io_exception(), 0, "failed to open index '%s', server responded with: '%s'", Z_STRVAL_P(&fields_str), Z_STRVAL_P(&hsi->error));
			goto cleanup;
		}
		if (res == -2) {
			zend_throw_exception_ex(handlersocketi_get_ce_io_exception(), 0, "timeout while reading server response");
			goto cleanup;
		}

		if (Z_TYPE_P(&retval) == IS_FALSE) {
			zval_ptr_dtor(&retval);
			HS_EXCEPTION("unable to open index: %d: %s", id, Z_TYPE_P(&hsi->error) != IS_STRING ? "Unknown error" : Z_STRVAL_P(&hsi->error));
			goto cleanup;
		}
		zval_ptr_dtor(&retval);

		/* success, add the id to the hash */
		handlersocketi_object_store_store_index_id(link, hash_index.c, hash_index.len, id);
	}

	hsi->hash = estrndup(hash_index.c, hash_index.len);

	/* cleanup */
cleanup:

	smart_string_free(&request);
	smart_string_free(&hash_index);

	zval_ptr_dtor(&index);
	zval_ptr_dtor(&filter);
	zval_ptr_dtor(&fields_str);
}

PHP_HANDLERSOCKETI_API int handlersocketi_register_index()
{
	zend_class_entry ce;

	INIT_CLASS_ENTRY(ce, "HandlerSocketi_Index", hs_index_methods);

	ce.create_object = hs_index_object_new;

	hs_ce_index = zend_register_internal_class(&ce);
	if (hs_ce_index == NULL) {
		return FAILURE;
	}

	memcpy(&hs_index_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	hs_index_object_handlers.clone_obj = hs_index_object_clone;
	hs_index_object_handlers.dtor_obj = zend_objects_destroy_object;
	hs_index_object_handlers.free_obj = hs_index_object_free_storage;
	hs_index_object_handlers.offset = XtOffsetOf(hs_index_obj_t, std);
	return SUCCESS;
}

PHP_HANDLERSOCKETI_API zend_class_entry *handlersocketi_get_ce_index(void)
{
	return hs_ce_index;
}

PHP_HANDLERSOCKETI_API void handlersocketi_create_index(zval *return_value, zval *link, char *db, size_t db_len, char *table, size_t table_len, zval *fields, zval *options)
{
	object_init_ex(return_value, hs_ce_index);

	hs_index_object_init(php_hs_index(Z_OBJ_P(return_value)), return_value, link, db, db_len, table, table_len, fields, options);
}

ZEND_METHOD(HandlerSocketi_Index, __construct)
{
	zval *link;
	char *db, *table;
	size_t db_len, table_len;
	zval *fields, *options = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "Ossz|z", &link, handlersocketi_get_ce(), &db, &db_len, &table, &table_len, &fields, &options) == FAILURE) {
		return;
	}

	handlersocketi_create_index(return_value, link, db, db_len, table, table_len, fields, options);
}

ZEND_METHOD(HandlerSocketi_Index, find)
{
	zval *query, operate, criteria;
	zval *options = NULL;
	long safe = -1, limit = HS_DEFAULT_LIMIT, offset = HS_DEFAULT_OFFSET;
	zval filters, in_values;
	long in_key = -1;
	hs_index_obj_t *hsi;
	php_stream *stream;
	long timeout;
	smart_string request = {0};
	int res = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "z|z", &query, &options) == FAILURE) {
		RETURN_FALSE;
	}

	/* handerlsocket: object */
	HS_INDEX_OBJ(getThis(), hsi);
	HS_CHECK_OBJECT(hsi, HandlerSocketi_Index);
	HS_ERROR_RESET(hsi->error);

	/* options */
	if (options != NULL && Z_TYPE_P(options) == IS_ARRAY) {
		/* limit */
		limit = hs_get_options_long(HASH_OF(options), "limit", limit);
		/* offset */
		offset = hs_get_options_long(HASH_OF(options), "offset", offset);
		/* safe */
		safe = hs_is_options_safe(HASH_OF(options));
		/* in, fiter, while */
		hs_array_to_in_filter(HASH_OF(options), &hsi->filter, &filters, &in_key, &in_values);
	} else {
		ZVAL_NULL(&filters);
		ZVAL_NULL(&in_values);
	}

	/* stream */
	stream = handlersocketi_object_store_get_stream(&hsi->link);
	if (!stream) {
		RETURN_FALSE;
	}
	timeout = handlersocketi_object_store_get_timeout(&hsi->link);

	/* operate : criteria */
	ZVAL_NULL(&operate);
	if (hs_zval_to_operate_criteria(query, &operate, &criteria, HS_FIND_EQUAL) != SUCCESS) {
		zval_ptr_dtor(&operate);
		zval_ptr_dtor(&filters);
		zval_ptr_dtor(&in_values);
		RETURN_FALSE;
	}

	/* command */
	hs_request_command(&request, hsi->id, &operate, &criteria, limit, offset, &filters, in_key, &in_values);

	/* eol */
	hs_request_next(&request);

	/* request: send */
	if (hs_request_send(stream, &request) < 0) {
		zval_ptr_dtor(&operate);
		zval_ptr_dtor(&filters);
		zval_ptr_dtor(&in_values);
		smart_string_free(&request);
		zend_throw_exception_ex(handlersocketi_get_ce_io_exception(), 0, "failed to send request");
		RETURN_FALSE;
	} else {
		/* response */
		res = hs_response_value(stream, timeout, return_value, &hsi->error, 0);
	}

	zval_ptr_dtor(&operate);
	zval_ptr_dtor(&filters);
	zval_ptr_dtor(&in_values);
	smart_string_free(&request);

	if (res == -1) {
		zend_throw_exception_ex(handlersocketi_get_ce_io_exception(), 0, "failed to read server response");
		return;
	}
	if (res == -2) {
		zend_throw_exception_ex(handlersocketi_get_ce_io_exception(), 0, "timeout while reading server response");
		return;
	}

	/* exception */
	if (safe > 0 && Z_TYPE_P(return_value) == IS_FALSE) {
		HS_EXCEPTION("response error: %s", Z_TYPE_P(&hsi->error) != IS_STRING ? "Unknown error" : Z_STRVAL_P(&hsi->error));
		RETURN_FALSE;
	}
}

ZEND_METHOD(HandlerSocketi_Index, insert)
{
	zval operate, fields, *args;
	long i;
	hs_index_obj_t *hsi;
	php_stream *stream;
	long timeout;
	smart_string request = {0};
	long fnum = 0;
	int argc;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "+", &args, &argc) == FAILURE) {
		return;
	}

	if (Z_TYPE_P(&args[0]) == IS_ARRAY) {
		ZVAL_COPY_VALUE(&fields, &args[0]);
		zval_copy_ctor(&fields);
	} else {
		array_init(&fields);

		for (i = 0; i < argc; i++) {
			switch (Z_TYPE_P(&args[i])) {
				case IS_NULL:
					add_next_index_null(&fields);
					break;
				case IS_LONG:
					add_next_index_long(&fields, Z_LVAL_P(&args[i]));
					break;
				case IS_DOUBLE:
					add_next_index_double(&fields, Z_DVAL_P(&args[i]));
					break;
				default:
					{
						zend_string *arg_str = zval_get_string(&args[i]);
						add_next_index_str(&fields, arg_str);
					}
					break;
			}
		}
	}

	/* handerlsocket: object */
	HS_INDEX_OBJ(getThis(), hsi);
	HS_CHECK_OBJECT(hsi, HandlerSocketi_Index);
	HS_ERROR_RESET(hsi->error);

	/* fiedls check */
	fnum = zend_hash_num_elements(HASH_OF(&fields));
	if (fnum < hsi->field_num) {
		for (i = 0; i < fnum; i++) {
			add_next_index_null(&fields);
		}
	} else if (fnum > hsi->field_num) {
		php_error_docref(NULL, E_WARNING, "invalid field count");
	}

	stream = handlersocketi_object_store_get_stream(&hsi->link);
	if (!stream) {
		RETURN_FALSE;
	}
	timeout = handlersocketi_object_store_get_timeout(&hsi->link);

	/* operate */
	ZVAL_STRINGL(&operate, HS_PROTOCOL_INSERT, strlen(HS_PROTOCOL_INSERT));

	/* command */
	hs_request_command(&request, hsi->id, &operate, &fields, 0, 0, NULL, -1, NULL);

	/* eol */
	hs_request_next(&request);

	/* request: send */
	if (hs_request_send(stream, &request) < 0) {
		zval_ptr_dtor(&operate);
		zend_throw_exception_ex(handlersocketi_get_ce_io_exception(), 0, "failed to send request");
		RETURN_FALSE;
	} else {
		/* response */
		int res;

		res = hs_response_value(stream, timeout, return_value, &hsi->error, 1);

		if (res == -1) {
			zend_throw_exception_ex(handlersocketi_get_ce_io_exception(), 0, "failed to insert values, server responded with: '%s'", Z_STRVAL_P(&hsi->error));
			goto cleanup;
		}
		if (res == -2) {
			zend_throw_exception_ex(handlersocketi_get_ce_io_exception(), 0, "timeout while when reading server response");
			goto cleanup;
		}

	}

cleanup:
	zval_ptr_dtor(&operate);
	zval_ptr_dtor(&fields);
	smart_string_free(&request);
}

ZEND_METHOD(HandlerSocketi_Index, update)
{
	zval *query, *update, *options = NULL;
	long safe = -1, modify = 1;
	long limit = HS_DEFAULT_LIMIT, offset = HS_DEFAULT_OFFSET;
	zval operate, criteria, modify_operate, modify_criteria;
	zval filters, in_values;
	long in_key = -1;
	hs_index_obj_t *hsi;
	php_stream *stream;
	long timeout;
	smart_string request = {0};
	long fnum = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "zz|z",
				&query, &update, &options) == FAILURE) {
		RETURN_FALSE;
	}

	/* handerlsocket: object */
	HS_INDEX_OBJ(getThis(), hsi);
	HS_CHECK_OBJECT(hsi, HandlerSocketi_Index);
	HS_ERROR_RESET(hsi->error);

	if (options != NULL && Z_TYPE_P(options) == IS_ARRAY) {
		/* limit */
		limit = hs_get_options_long(HASH_OF(options), "limit", limit);
		/* offset */
		offset = hs_get_options_long(HASH_OF(options), "offset", offset);
		/* safe */
		safe = hs_is_options_safe(HASH_OF(options));
		/* in, fiter, while */
		hs_array_to_in_filter(HASH_OF(options), &hsi->filter, &filters, &in_key, &in_values);
	} else {
		ZVAL_NULL(&filters);
		ZVAL_NULL(&in_values);
	}

	/* stream */
	stream = handlersocketi_object_store_get_stream(&hsi->link);
	if (!stream) {
		zval_ptr_dtor(&filters);
		zval_ptr_dtor(&in_values);
		RETURN_FALSE;
	}
	timeout = handlersocketi_object_store_get_timeout(&hsi->link);

	/* operate : criteria */
	if (hs_zval_to_operate_criteria(query, &operate, &criteria, HS_FIND_EQUAL) != SUCCESS) {
		zval_ptr_dtor(&operate);
		zval_ptr_dtor(&filters);
		zval_ptr_dtor(&in_values);
		RETURN_FALSE;
	}

	/* modify_operete : modify_criteria */
	if (hs_zval_to_operate_criteria(update, &modify_operate, &modify_criteria, HS_MODIFY_UPDATE)  != SUCCESS) {
		zval_ptr_dtor(&operate);
		zval_ptr_dtor(&modify_operate);
		zval_ptr_dtor(&filters);
		zval_ptr_dtor(&in_values);
		RETURN_FALSE;
	}

	/* fiedls check */
	if (Z_TYPE_P(&modify_criteria) == IS_ARRAY) {
		fnum = zend_hash_num_elements(HASH_OF(&modify_criteria));
	} else {
		fnum = 1;
	}

	if (fnum > hsi->field_num) {
		php_error_docref(NULL, E_WARNING, "invalid field count");
	}

	/* command */
	hs_request_command(&request, hsi->id, &operate, &criteria, limit, offset, &filters, in_key, &in_values);

	/* command: modify */
	modify = hs_request_command_modify(&request, &modify_operate, &modify_criteria, -1);
	if (modify >= 0) {
		/* eol */
		hs_request_next(&request);

		/* request: send */
		if (hs_request_send(stream, &request) < 0) {
			zend_throw_exception_ex(handlersocketi_get_ce_io_exception(), 0, "failed to send request");
			RETURN_FALSE;
		} else {
			int res;
			/* response */
			res = hs_response_value(stream, timeout, return_value, &hsi->error, modify);

			if (res == -1) {
				zend_throw_exception_ex(handlersocketi_get_ce_io_exception(), 0, "failed to read server response");
				return;
			}
			if (res == -2) {
				zend_throw_exception_ex(handlersocketi_get_ce_io_exception(), 0, "timeout while reading server response");
				return;
			}
		}
	} else {
		ZVAL_BOOL(return_value, 0);
		ZVAL_STRINGL(&hsi->error, "unable to update parameter", strlen("unable to update parameter"));
	}

	zval_ptr_dtor(&operate);
	zval_ptr_dtor(&modify_operate);
	zval_ptr_dtor(&filters);
	zval_ptr_dtor(&in_values);
	smart_string_free(&request);

	/* exception */
	if (safe > 0 && Z_TYPE_P(return_value) == IS_FALSE) {
		HS_EXCEPTION("response error: %s", Z_TYPE_P(&hsi->error) != IS_STRING ? "Unknown error" : Z_STRVAL_P(&hsi->error));
		RETURN_FALSE;
	}
}

ZEND_METHOD(HandlerSocketi_Index, remove)
{
	zval *query, *options = NULL;
	zval operate, criteria;
	long safe = -1, limit = HS_DEFAULT_LIMIT, offset = HS_DEFAULT_OFFSET;
	zval filters, in_values;
	long in_key = -1;
	hs_index_obj_t *hsi;
	php_stream *stream;
	long timeout;
	smart_string request = {0};

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "z|z", &query, &options) == FAILURE) {
		RETURN_FALSE;
	}

	/* handelrsocket : object */
	HS_INDEX_OBJ(getThis(), hsi);
	HS_CHECK_OBJECT(hsi, HandlerSocketi_Index);
	HS_ERROR_RESET(hsi->error);

	if (options != NULL && Z_TYPE_P(options) == IS_ARRAY) {
		/* limit */
		limit = hs_get_options_long(HASH_OF(options), "limit", limit);
		/* offset */
		offset = hs_get_options_long(HASH_OF(options), "offset", offset);
		/* safe */
		safe = hs_is_options_safe(HASH_OF(options));
		/* in, fiter, while */
		hs_array_to_in_filter(HASH_OF(options), &hsi->filter, &filters, &in_key, &in_values);
	} else {
		ZVAL_NULL(&filters);
		ZVAL_NULL(&in_values);
	}


	/* stream */
	stream = handlersocketi_object_store_get_stream(&hsi->link);
	if (!stream) {
		RETURN_FALSE;
	}
	timeout = handlersocketi_object_store_get_timeout(&hsi->link);

	/* operate : criteria */
	if (hs_zval_to_operate_criteria(query, &operate, &criteria, HS_FIND_EQUAL) != SUCCESS) {
		zval_ptr_dtor(&operate);
		RETURN_FALSE;
	}

	/* command */
	hs_request_command(&request, hsi->id, &operate, &criteria, limit, offset, &filters, in_key, &in_values);

	/* find: modify: D */
	hs_request_delim(&request);
	hs_request_string(&request, HS_MODIFY_REMOVE, strlen(HS_MODIFY_REMOVE));

	/* eol */
	hs_request_next(&request);

	/* request: send */
	if (hs_request_send(stream, &request) < 0) {
		zval_ptr_dtor(&operate);
		zval_ptr_dtor(&filters);
		zval_ptr_dtor(&in_values);
		smart_string_free(&request);
		zend_throw_exception_ex(handlersocketi_get_ce_io_exception(), 0, "failed to send request");
		RETURN_FALSE;
	} else {
		int res;
		/* response */
		res = hs_response_value(stream, timeout, return_value, &hsi->error, 1);

		if (res == -1) {
			zend_throw_exception_ex(handlersocketi_get_ce_io_exception(), 0, "failed to read server response");
			return;
		}
		if (res == -2) {
			zend_throw_exception_ex(handlersocketi_get_ce_io_exception(), 0, "timeout while reading server response");
			return;
		}
	}

	zval_ptr_dtor(&operate);
	zval_ptr_dtor(&filters);
	zval_ptr_dtor(&in_values);
	smart_string_free(&request);

	/* exception */
	if (safe > 0 && Z_TYPE_P(return_value) == IS_FALSE) {
		HS_EXCEPTION("response error: %s", Z_TYPE_P(&hsi->error) != IS_STRING ? "Unknown error" : Z_STRVAL_P(&hsi->error));
	}
}

ZEND_METHOD(HandlerSocketi_Index, multi)
{
	zval *args;
	zval mreq;
	zval *val;
	int err = -1;
	int res;
	hs_index_obj_t *hsi;
	php_stream *stream;
	long timeout;
	smart_string request = {0};

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "a", &args) == FAILURE) {
		RETURN_FALSE;
	}

	/* handlersocket: object */
	HS_INDEX_OBJ(getThis(), hsi);
	HS_CHECK_OBJECT(hsi, HandlerSocketi_Index);
	HS_ERROR_RESET(hsi->error);

	array_init(&mreq);

	zend_hash_internal_pointer_reset(HASH_OF(args));
	while ((val = zend_hash_get_current_data(HASH_OF(args))) != NULL) {
		zval *method;
		zend_string *method_str;

		if (Z_TYPE_P(val) != IS_ARRAY) {
			err = -1;
			break;
		}

		/* 0: method */
		method = zend_hash_index_find(HASH_OF(val), 0);
		if (!method) {
			err = -1;
			break;
		}

		method_str = zval_get_string(method);

		if (strncmp(method_str->val, "find", strlen("find")) == 0) {
			/* method: find */
			zval *query, *options, operate, criteria;
			zval filters, in_values;
			long in_key = -1;
			long limit = HS_DEFAULT_LIMIT, offset = HS_DEFAULT_OFFSET;

			if (zend_hash_num_elements(HASH_OF(val)) <= 1) {
				err = -1;
				break;
			}

			/* 1: query */
			query = zend_hash_index_find(HASH_OF(val), 1);
			if (!query) {
				err = -1;
				break;
			}

			/* 2: options */
			options = zend_hash_index_find(HASH_OF(val), 2);
			if (options && Z_TYPE_P(options) == IS_ARRAY) {
				limit = hs_get_options_long(HASH_OF(options), "limit", limit);
				offset = hs_get_options_long(HASH_OF(options), "offset", offset);
				hs_array_to_in_filter(HASH_OF(options), &hsi->filter, &filters, &in_key, &in_values);
			} else {
				ZVAL_NULL(&filters);
				ZVAL_NULL(&in_values);
			}

			/* operate : criteria */
			if (hs_zval_to_operate_criteria(query, &operate, &criteria, HS_FIND_EQUAL) != SUCCESS) {
				err = -1;
				zval_ptr_dtor(&operate);
				zval_ptr_dtor(&filters);
				zval_ptr_dtor(&in_values);
				break;
			}

			/* command */
			hs_request_command(&request, hsi->id, &operate, &criteria, limit, offset, &filters, in_key, &in_values);

			/* eol */
			hs_request_next(&request);

			add_next_index_long(&mreq, 0);
			err = 0;

			zval_ptr_dtor(&operate);
			zval_ptr_dtor(&filters);
			zval_ptr_dtor(&in_values);
		} else if (strncmp(method_str->val, "insert", strlen("insert")) == 0) {
			/* method: insert */
			zval operate, fields;
			zval *tmp;
			long i, n, fnum = 0;

			if (zend_hash_num_elements(HASH_OF(val)) <= 1) {
				err = -1;
				break;
			}

			tmp = zend_hash_index_find(HASH_OF(val), 1);
			if (tmp) {
				err = -1;
				break;
			}

			array_init(&fields);

			if (Z_TYPE_P(tmp) == IS_ARRAY) {
				n = zend_hash_num_elements(HASH_OF(tmp));
				for (i = 0; i < n; i++) {
					zval *val1;
					val1 = zend_hash_index_find(HASH_OF(tmp), i);
					if (val1 != NULL) {
						if (Z_TYPE_P(val1) == IS_NULL) {
							add_next_index_null(&fields);
						} else {
							zend_string *str = zval_get_string(val1);
							add_next_index_str(&fields, str);
						}
					}
				}
			} else {
				i = 1;
				do {
					if (Z_TYPE_P(tmp) == IS_NULL) {
						add_next_index_null(&fields);
					} else {
						zend_string *str = zval_get_string(tmp);
						add_next_index_str(&fields, str);
					}

					i++;

					tmp = zend_hash_index_find(HASH_OF(val), i);
					if (!tmp) {
						break;
					}
				} while (i < n);
			}

			/* fields check */
			fnum = zend_hash_num_elements(HASH_OF(&fields));
			if (fnum < hsi->field_num) {
				for (i = 0; i < fnum; i++) {
					add_next_index_null(&fields);
				}
			}

			ZVAL_STRINGL(&operate, HS_PROTOCOL_INSERT, strlen(HS_PROTOCOL_INSERT));

			/* command */
			hs_request_command(&request, hsi->id, &operate, &fields, 0, 0, NULL, -1, NULL);

			/* eol */
			hs_request_next(&request);

			add_next_index_long(&mreq, 1);
			err = 0;

			zval_ptr_dtor(&operate);
			zval_ptr_dtor(&fields);
		} else if (strncmp(method_str->val, "remove", strlen("remove")) == 0) {
			/* method: remove */
			zval *query, *options, operate, criteria;
			zval filters, in_values;
			long in_key = -1;
			long limit = HS_DEFAULT_LIMIT, offset = HS_DEFAULT_OFFSET;

			if (zend_hash_num_elements(HASH_OF(val)) <= 1) {
				err = -1;
				break;
			}

			/* 1: query */
			query = zend_hash_index_find(HASH_OF(val), 1);
			if (query) {
				err = -1;
				break;
			}

			/* 2: options */
			options = zend_hash_index_find(HASH_OF(val), 2);
			if (options && Z_TYPE_P(options) == IS_ARRAY) {
				limit = hs_get_options_long(HASH_OF(options), "limit", limit);
				offset = hs_get_options_long(HASH_OF(options), "offset", offset);
				hs_array_to_in_filter(HASH_OF(options), &hsi->filter, &filters, &in_key, &in_values);
			} else {
				ZVAL_NULL(&filters);
				ZVAL_NULL(&in_values);
			}

			/* operete : criteria */
			if (hs_zval_to_operate_criteria(query, &operate, &criteria, HS_FIND_EQUAL) != SUCCESS) {
				zval_ptr_dtor(&operate);
				zval_ptr_dtor(&filters);
				zval_ptr_dtor(&in_values);
				err = -1;
				break;
			}

			/* command */
			hs_request_command(&request, hsi->id, &operate, &criteria, limit, offset, &filters, in_key, &in_values);

			/* find: modify: D */
			hs_request_delim(&request);

			hs_request_string(&request, HS_MODIFY_REMOVE, 1);

			/* eol */
			hs_request_next(&request);

			add_next_index_long(&mreq, 1);
			err = 0;

			zval_ptr_dtor(&operate);
			zval_ptr_dtor(&filters);
			zval_ptr_dtor(&in_values);
		} else if (strncmp(method_str->val, "update", strlen("update")) == 0) {
			/* method: update */
			zval *query, *update, *options;
			zval operate, criteria, modify_operate, modify_criteria;
			zval filters, in_values;
			long in_key = -1;
			long limit = HS_DEFAULT_LIMIT, offset = HS_DEFAULT_OFFSET;
			int modify;

			if (zend_hash_num_elements(HASH_OF(val)) <= 2) {
				err = -1;
				break;
			}

			/* 1: query */
			query = zend_hash_index_find(HASH_OF(val), 1);
			if (!query) {
				err = -1;
				break;
			}

			/* 2: update */
			update = zend_hash_index_find(HASH_OF(val), 2);
			if (!update) {
				err = -1;
				break;
			}

			/* 3: options */
			options = zend_hash_index_find(HASH_OF(val), 3);
			if (options && Z_TYPE_P(options) == IS_ARRAY) {
				limit = hs_get_options_long(HASH_OF(options), "limit", limit);
				offset = hs_get_options_long(HASH_OF(options), "offset", offset);
				hs_array_to_in_filter(HASH_OF(options), &hsi->filter, &filters, &in_key, &in_values);
			} else {
				ZVAL_NULL(&filters);
				ZVAL_NULL(&in_values);
			}

			/* operete : criteria */
			if (hs_zval_to_operate_criteria(query, &operate, &criteria, HS_FIND_EQUAL) != SUCCESS) {
				zval_ptr_dtor(&operate);
				zval_ptr_dtor(&filters);
				zval_ptr_dtor(&in_values);
				err = -1;
				break;
			}

			/* modify_operete : modify_criteria */
			if (hs_zval_to_operate_criteria(update, &modify_operate, &modify_criteria, HS_MODIFY_UPDATE) != SUCCESS) {
				zval_ptr_dtor(&operate);
				zval_ptr_dtor(&modify_operate);
				zval_ptr_dtor(&filters);
				zval_ptr_dtor(&in_values);
				err = -1;
				break;
			}

			/* command */
			hs_request_command(&request, hsi->id, &operate, &criteria, limit, offset, &filters, in_key, &in_values);

			/* command: modify */
			modify = hs_request_command_modify(&request, &modify_operate, &modify_criteria, -1);
			if (modify < 0) {
				err = -1;
				break;
			}

			/* eol */
			hs_request_next(&request);

			add_next_index_long(&mreq, modify);
			err = 0;

			zval_ptr_dtor(&operate);
			zval_ptr_dtor(&modify_operate);
			zval_ptr_dtor(&filters);
			zval_ptr_dtor(&in_values);
		} else {
			err = -1;
			break;
		}

		zend_hash_move_forward(HASH_OF(args));
	}

	/* stream */
	stream = handlersocketi_object_store_get_stream(&hsi->link);
	if (!stream) {
		RETURN_FALSE;
	}
	timeout = handlersocketi_object_store_get_timeout(&hsi->link);

	/* request: send */
	if (err < 0  || hs_request_send(stream, &request) < 0) {
		zend_throw_exception_ex(handlersocketi_get_ce_io_exception(), 0, "failed to send request");
		smart_string_free(&request);
		zval_ptr_dtor(&mreq);
		RETURN_FALSE;
	}
	smart_string_free(&request);

	/* response */
	res = hs_response_multi(stream, timeout, return_value, &hsi->error, &mreq);

	zval_ptr_dtor(&mreq);

	if (res == -2) {
		;		zend_throw_exception_ex(handlersocketi_get_ce_io_exception(), 0, "timeout while reading server response");
		return;
	}
}

ZEND_METHOD(HandlerSocketi_Index, get_error)
{
	hs_index_obj_t *hsi;

	HS_INDEX_OBJ(getThis(), hsi);
	HS_CHECK_OBJECT(hsi, HandlerSocketi_Index);

	if (Z_TYPE_P(&hsi->error) == IS_NULL) {
		RETURN_NULL();
	} else {
		RETVAL_ZVAL(&hsi->error, 1, 0);
	}
}

ZEND_METHOD(HandlerSocketi_Index, get_id)
{
	hs_index_obj_t *hsi;

	HS_INDEX_OBJ(getThis(), hsi);
	HS_CHECK_OBJECT(hsi, HandlerSocketi_Index);

	RETVAL_LONG(hsi->id);
}

ZEND_METHOD(HandlerSocketi_Index, get_name)
{
	hs_index_obj_t *hsi;

	HS_INDEX_OBJ(getThis(), hsi);
	HS_CHECK_OBJECT(hsi, HandlerSocketi_Index);

	RETVAL_ZVAL(&hsi->name, 1, 0);
}

ZEND_METHOD(HandlerSocketi_Index, get_db)
{
	hs_index_obj_t *hsi;

	HS_INDEX_OBJ(getThis(), hsi);
	HS_CHECK_OBJECT(hsi, HandlerSocketi_Index);

	RETVAL_ZVAL(&hsi->db, 1, 0);
}

ZEND_METHOD(HandlerSocketi_Index, get_table)
{
	hs_index_obj_t *hsi;

	HS_INDEX_OBJ(getThis(), hsi);
	HS_CHECK_OBJECT(hsi, HandlerSocketi_Index);

	RETVAL_ZVAL(&hsi->table, 1, 0);
}

ZEND_METHOD(HandlerSocketi_Index, get_field)
{
	hs_index_obj_t *hsi;

	HS_INDEX_OBJ(getThis(), hsi);
	HS_CHECK_OBJECT(hsi, HandlerSocketi_Index);

	RETVAL_ZVAL(&hsi->field, 1, 0);
}

ZEND_METHOD(HandlerSocketi_Index, get_filter)
{
	hs_index_obj_t *hsi;

	HS_INDEX_OBJ(getThis(), hsi);
	HS_CHECK_OBJECT(hsi, HandlerSocketi_Index);

	if (Z_TYPE_P(&hsi->filter) != IS_NULL) {
		RETVAL_ZVAL(&hsi->filter, 1, 0);
	} else {
		RETVAL_NULL();
	}
}

ZEND_METHOD(HandlerSocketi_Index, get_operator)
{
	zval query, modify;

	array_init(&query);

	add_next_index_stringl(&query, HS_FIND_EQUAL, strlen(HS_FIND_EQUAL));
	add_next_index_stringl(&query, HS_FIND_LESS, strlen(HS_FIND_LESS));
	add_next_index_stringl(&query, HS_FIND_LESS_EQUAL, strlen(HS_FIND_LESS_EQUAL));
	add_next_index_stringl(&query, HS_FIND_GREATER, strlen(HS_FIND_GREATER));
	add_next_index_stringl(&query, HS_FIND_GREATER_EQUAL, strlen(HS_FIND_GREATER_EQUAL));

	array_init(&modify);

	add_next_index_stringl(&modify, HS_MODIFY_UPDATE, strlen(HS_MODIFY_UPDATE));
	add_next_index_stringl(&modify, HS_MODIFY_INCREMENT, strlen(HS_MODIFY_INCREMENT));
	add_next_index_stringl(&modify, HS_MODIFY_DECREMENT, strlen(HS_MODIFY_DECREMENT));
	add_next_index_stringl(&modify, HS_MODIFY_REMOVE, strlen(HS_MODIFY_REMOVE));
	add_next_index_stringl(&modify, HS_MODIFY_GET_UPDATE, strlen(HS_MODIFY_GET_UPDATE));
	add_next_index_stringl(&modify, HS_MODIFY_GET_INCREMENT, strlen(HS_MODIFY_GET_INCREMENT));
	add_next_index_stringl(&modify, HS_MODIFY_GET_DECREMENT, strlen(HS_MODIFY_GET_DECREMENT));
	add_next_index_stringl(&modify, HS_MODIFY_GET_REMOVE, strlen(HS_MODIFY_GET_REMOVE));

	array_init(return_value);
	add_assoc_zval(return_value, "query", &query);
	add_assoc_zval(return_value, "modify", &modify);
}
