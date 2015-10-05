--TEST--
find() with limit
--FILE--
<?php
include(__DIR__."/common.inc");

if (!class_exists('handlersocketi')) dl('handlersocketi.so');

$options = ['timeout' => 1, 'persistent' => true];
$hs = new \HandlerSocketi($config['server'], $config['read_port'], $options);
$open_options = ['id' => 1, 'index' => 'PRIMARY', 'filter' => "user_id,ts,data"];
$index = $hs->openIndex('test', 'HandlerSocketTestTbl', array('user_id', 'ts', 'data'),$open_options);
$user_id = 1;
$find_options = ['limit' => 5];
$res = $index->find(['>' => $user_id], $find_options); 
var_dump(empty($res));
var_dump(is_array($res));
var_dump(count($res) == 5);
?>
--EXPECT--
bool(false)
bool(true)
bool(true)
