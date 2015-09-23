
#include "php.h"
#include "php_ini.h"
#include "Zend/zend_exceptions.h"

#include "php_handlersocketi.h"
#include "handlersocketi_exception.h"

ZEND_EXTERN_MODULE_GLOBALS(handlersocketi);

static zend_class_entry *hs_ce_exception;
static zend_class_entry *hs_ce_io_exception;

PHP_HANDLERSOCKETI_API int handlersocketi_register_exception()
{
	zend_class_entry ce;

	INIT_CLASS_ENTRY(ce, "HandlerSocketi_Exception", NULL);
	hs_ce_exception = zend_register_internal_class_ex(&ce, zend_exception_get_default());

	INIT_CLASS_ENTRY(ce, "HandlerSocketi_IO_Exception", NULL);
	hs_ce_io_exception = zend_register_internal_class_ex(&ce, zend_exception_get_default());
	return SUCCESS;
}

PHP_HANDLERSOCKETI_API zend_class_entry *handlersocketi_get_ce_exception(void)
{
	return hs_ce_exception;
}

PHP_HANDLERSOCKETI_API zend_class_entry *handlersocketi_get_ce_io_exception(void)
{
	return hs_ce_io_exception;
}

