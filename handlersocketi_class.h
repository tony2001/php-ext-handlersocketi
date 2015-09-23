#ifndef HANDLERSOCKETI_CLASS_H
#define HANDLERSOCKETI_CLASS_H

typedef struct hs_hash {
	zend_resource *rsrc;
    zend_bool is_persistent;
	php_stream *stream;
	HashTable open_indices;
} hs_conn_t;

typedef struct hs_obj {
    long timeout;
    zval server;
	char *hashkey;
	size_t hashkey_len;
    zval auth;
    zval error;
	hs_conn_t *conn;
    zend_object std;
} hs_obj_t;

extern int le_hs_conn;
extern int le_hs_pconn;

PHP_HANDLERSOCKETI_API int handlersocketi_register_class();
PHP_HANDLERSOCKETI_API zend_class_entry *handlersocketi_get_ce(void);

PHP_HANDLERSOCKETI_API php_stream *handlersocketi_object_store_get_stream(zval *link);
PHP_HANDLERSOCKETI_API long handlersocketi_object_store_get_timeout(zval *link);
PHP_HANDLERSOCKETI_API int handlersocketi_object_store_get_index_id(zval *link, const char *hash_index, size_t hash_index_len);
PHP_HANDLERSOCKETI_API int handlersocketi_object_store_store_index_id(zval *link, const char *hash_index, size_t hash_index_len, size_t id);
PHP_HANDLERSOCKETI_API int handlersocketi_object_store_get_index_hash(const char *db, size_t db_len, const char *table, size_t table_len, zval *fields, zval *filter, char **hash_index_str, size_t *hash_index_len);
PHP_HANDLERSOCKETI_API int handlersocketi_object_store_remove_index(zval *link, const char *hash_index, size_t hash_index_len);

#endif /* HANDLERSOCKETI_CLASS_H */
