--TEST--
openIndex() and nonexistent row
--FILE--
<?php
include(__DIR__."/common.inc");

if (!class_exists('handlersocketi')) dl('handlersocketi.so');

$options = ['timeout' => 1, 'persistent' => true];
$hs = new \HandlerSocketi($config['server'], $config['write_port'], $options);
$open_options = ['id' => 1, 'index' => 'PRIMARY', 'filter' => "user_id,ts,data"];
$index = $hs->openIndex('test', 'HandlerSocketTestTbl', array('user_id', 'ts', 'data'),$open_options);

$user_id = 100;
$update_values = [1,222222,3];
$index->update(['=' => $user_id], ['U' => $update_values]);
$updated_values = $index->find(['=' => $user_id], ['safe' => 0]);
var_dump(empty($updated_values));
?>
--EXPECT--
bool(true)

