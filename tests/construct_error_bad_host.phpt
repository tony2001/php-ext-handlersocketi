--TEST--
__construct() with bad host
--FILE--
<?php
include(__DIR__."/common.inc");

if (!class_exists('handlersocketi')) dl('handlersocketi.so');

$options = ['timeout' => 1, 'persistent' => true];
$hs = new \HandlerSocketi('nohost', $config['read_port'], $options);
?>
--EXPECTF--
Warning: HandlerSocketi::__construct(): php_network_getaddresses: getaddrinfo failed: Name or service not known in %s on line %d

Warning: HandlerSocketi::__construct(): connect() failed: php_network_getaddresses: getaddrinfo failed: Name or service not known in %s on line %d

Fatal error: Uncaught HandlerSocketi_Exception: HandlerSocketi::__construct(): unable to connect to nohost:%d in %s:%d
Stack trace:
#0 %s(%d): HandlerSocketi->__construct('nohost', %d, Array)
#1 {main}
  thrown in %s on line %d
