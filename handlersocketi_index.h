#ifndef HANDLERSOCKETI_INDEX_H
#define HANDLERSOCKETI_INDEX_H

PHP_HANDLERSOCKETI_API int handlersocketi_register_index();
PHP_HANDLERSOCKETI_API zend_class_entry *handlersocketi_get_ce_index(void);
PHP_HANDLERSOCKETI_API void handlersocketi_create_index(zval *return_value, zval *link, char *db, size_t db_len, char *table, size_t table_len, zval *fields, zval *options);
void hs_zval_to_comma_string(zval *val, zval *retval);
void hs_zval_to_comma_array(zval *val, zval *retval);

#endif /* HANDLERSOCKETI_INDEX_H */
