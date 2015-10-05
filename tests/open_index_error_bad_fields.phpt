--TEST--
openIndex() with bad fields
--FILE--
<?php
include(__DIR__."/common.inc");

if (!class_exists('handlersocketi')) dl('handlersocketi.so');

$options = ['timeout' => 1, 'persistent' => true];
$hs = new \HandlerSocketi($config['server'], $config['read_port'], $options);
$open_options = ['id' => 1, 'index' => 'PRIMARY', 'filter' => "user_id,ts,data"];
$index = $hs->openIndex('test', 'HandlerSocketTestTbl', ['field1', 'ts', 'field2', 'field3'],$open_options);
?>
--EXPECTF--
Fatal error: Uncaught HandlerSocketi_IO_Exception: failed to open index 'field1,ts,field2,field3', server responded with: 'fld' in %s:%d
Stack trace:
#0 %s(%d): HandlerSocketi->openIndex('test', 'HandlerSocketTe...', Array, Array)
#1 {main}
  thrown in %s on line %d
