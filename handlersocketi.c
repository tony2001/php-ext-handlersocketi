
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"

#include "php_verdep.h"
#include "php_handlersocketi.h"
#include "handlersocketi_class.h"
#include "handlersocketi_index.h"

int le_hs_conn;
int le_hs_pconn;

ZEND_DECLARE_MODULE_GLOBALS(handlersocketi)

static void hs_init_globals(zend_handlersocketi_globals *hs_globals)
{
    hs_globals->id = 1;
}

static void hs_conn_dtor_ex(zend_rsrc_list_entry *rsrc, int persistent TSRMLS_DC) /* {{{ */
{
	hs_conn_t *conn = (hs_conn_t *)rsrc->ptr;

	if (!conn->is_persistent) {
		php_stream_close(conn->stream);
	}

	zend_hash_destroy(&conn->open_indices);
	pefree(conn, persistent);
}
/* }}} */

static void hs_conn_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC) /* {{{ */
{
	hs_conn_dtor_ex(rsrc, 0 TSRMLS_CC);
}
/* }}} */

static void hs_pconn_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC) /* {{{ */
{
	hs_conn_dtor_ex(rsrc, 1 TSRMLS_CC);
}
/* }}} */

ZEND_MINIT_FUNCTION(handlersocketi)
{
    ZEND_INIT_MODULE_GLOBALS(handlersocketi, hs_init_globals, NULL);
    /*
    REGISTER_INI_ENTRIES();
    */

	le_hs_pconn = zend_register_list_destructors_ex(NULL, hs_pconn_dtor, "HandlerSocketi persistent connection", module_number);
	le_hs_conn = zend_register_list_destructors_ex(hs_conn_dtor, NULL, "HandlerSocketi connection", module_number);

    handlersocketi_register_class(TSRMLS_C);
    handlersocketi_register_index(TSRMLS_C);

    /*
    REGISTER_LONG_CONSTANT("MSGPACKI_MODE_ORIGIN", PHP_MSGPACKI_MODE_ORIGIN,
                           CONST_CS | CONST_PERSISTENT);

    REGISTER_STRING_CONSTANT("MSGPACKI_FILTER_REGISTER",
                             PHP_MSGPACKI_FILTER_REGISTER,
                             CONST_CS | CONST_PERSISTENT);
    */


/* constant
#if ZEND_MODULE_API_NO < 20050922
    REGISTER_STRING_CONSTANT(
        "HANDLERSOCKETI_PRIMARY", HS_PRIMARY, CONST_CS | CONST_PERSISTENT);
    REGISTER_STRING_CONSTANT(
        "HANDLERSOCKETI_UPDATE", HS_MODIFY_UPDATE, CONST_CS | CONST_PERSISTENT);
    REGISTER_STRING_CONSTANT(
        "HANDLERSOCKETI_DELETE", HS_MODIFY_REMOVE, CONST_CS | CONST_PERSISTENT);
#else
    zend_declare_class_constant_string(
        hs_ce, "PRIMARY", strlen("PRIMARY"), HS_PRIMARY TSRMLS_CC);
    zend_declare_class_constant_string(
        hs_ce, "UPDATE", strlen("UPDATE"), HS_MODIFY_UPDATE TSRMLS_CC);
    zend_declare_class_constant_string(
        hs_ce, "DELETE", strlen("DELETE"), HS_MODIFY_REMOVE TSRMLS_CC);
#endif
*/

    return SUCCESS;
}

ZEND_MSHUTDOWN_FUNCTION(handlersocketi)
{
    /*
    UNREGISTER_INI_ENTRIES();
    */

    return SUCCESS;
}

/*
ZEND_RINIT_FUNCTION(handlersocketi)
{
    return SUCCESS;
}

ZEND_RSHUTDOWN_FUNCTION(handlersocketi)
{
    return SUCCESS;
}
*/

ZEND_MINFO_FUNCTION(handlersocketi)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "handlersocketi support", "enabled");
    php_info_print_table_header(2, "extension version",
                                HANDLERSOCKETI_EXT_VERSION);
    php_info_print_table_end();

    DISPLAY_INI_ENTRIES();
}

const zend_function_entry handlersocketi_functions[] = {
    ZEND_FE_END
};

zend_module_entry handlersocketi_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
    STANDARD_MODULE_HEADER,
#endif
    "handlersocketi",
    NULL,
    ZEND_MINIT(handlersocketi),
    ZEND_MSHUTDOWN(handlersocketi),
    NULL, /* ZEND_RINIT(handlersocketi), */
    NULL, /* ZEND_RSHUTDOWN(handlersocketi), */
    ZEND_MINFO(handlersocketi),
#if ZEND_MODULE_API_NO >= 20010901
    HANDLERSOCKETI_EXT_VERSION,
#endif
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_HANDLERSOCKETI
ZEND_GET_MODULE(handlersocketi)
#endif
