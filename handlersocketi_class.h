#ifndef HANDLERSOCKETI_CLASS_H
#define HANDLERSOCKETI_CLASS_H

typedef struct hs_hash {
	int rsrc_id;
    zend_bool is_persistent;
	php_stream *stream;
	HashTable open_indices;
} hs_conn_t;

typedef struct hs_obj {
    zend_object std;
    long timeout;
    zval *server;
	char *hashkey;
	int hashkey_len;
    zval *auth;
    zval *error;
	hs_conn_t *conn;
} hs_obj_t;

extern int le_hs_conn;
extern int le_hs_pconn;

PHP_HANDLERSOCKETI_API int handlersocketi_register_class(TSRMLS_D);
PHP_HANDLERSOCKETI_API zend_class_entry *handlersocketi_get_ce(void);

PHP_HANDLERSOCKETI_API php_stream *handlersocketi_object_store_get_stream(zval *link TSRMLS_DC);
PHP_HANDLERSOCKETI_API long handlersocketi_object_store_get_timeout(zval *link TSRMLS_DC);
PHP_HANDLERSOCKETI_API int handlersocketi_object_store_get_index_id(zval *link, const char *hash_index, int hash_index_len TSRMLS_DC);
PHP_HANDLERSOCKETI_API int handlersocketi_object_store_store_index_id(zval *link, const char *hash_index, int hash_index_len, int id TSRMLS_DC);
PHP_HANDLERSOCKETI_API int handlersocketi_object_store_get_index_hash(const char *db, int db_len, const char *table, int table_len, zval *fields, zval *filter, char **hash_index_str, int *hash_index_len TSRMLS_DC);

#endif /* HANDLERSOCKETI_CLASS_H */
