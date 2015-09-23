
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
#include "handlersocketi_exception.h"

int le_hs_conn;
int le_hs_pconn;

ZEND_DECLARE_MODULE_GLOBALS(handlersocketi)

static void hs_init_globals(zend_handlersocketi_globals *hs_globals)
{
    hs_globals->id = 1;
}

static void hs_conn_dtor_ex(zend_resource *rsrc, int persistent TSRMLS_DC) /* {{{ */
{
	hs_conn_t *conn = (hs_conn_t *)rsrc->ptr;

	if (!conn->is_persistent && conn->stream) {
		php_stream_close(conn->stream);
	}

	zend_hash_clean(&conn->open_indices);
	zend_hash_destroy(&conn->open_indices);
	pefree(conn, persistent);
}
/* }}} */

static void hs_conn_dtor(zend_resource *rsrc TSRMLS_DC) /* {{{ */
{
	hs_conn_dtor_ex(rsrc, 0 TSRMLS_CC);
}
/* }}} */

static void hs_pconn_dtor(zend_resource *rsrc TSRMLS_DC) /* {{{ */
{
	hs_conn_dtor_ex(rsrc, 1 TSRMLS_CC);
}
/* }}} */

ZEND_MINIT_FUNCTION(handlersocketi)
{
    ZEND_INIT_MODULE_GLOBALS(handlersocketi, hs_init_globals, NULL);

	le_hs_pconn = zend_register_list_destructors_ex(NULL, hs_pconn_dtor, "HandlerSocketi persistent connection", module_number);
	le_hs_conn = zend_register_list_destructors_ex(hs_conn_dtor, NULL, "HandlerSocketi connection", module_number);

    handlersocketi_register_class(TSRMLS_C);
    handlersocketi_register_index(TSRMLS_C);
    handlersocketi_register_exception(TSRMLS_C);

    return SUCCESS;
}

ZEND_MSHUTDOWN_FUNCTION(handlersocketi)
{
    return SUCCESS;
}

ZEND_MINFO_FUNCTION(handlersocketi)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "handlersocketi support", "enabled");
    php_info_print_table_header(2, "extension version", HANDLERSOCKETI_EXT_VERSION);
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
