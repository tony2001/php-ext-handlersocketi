--TEST--
find() with '=' operator
--FILE--
<?php
include(__DIR__."/common.inc");

if (!class_exists('handlersocketi')) dl('handlersocketi.so');

$options = ['timeout' => 1, 'persistent' => true];
$hs = new \HandlerSocketi($config['server'], $config['read_port'], $options);
$open_options = ['id' => 1, 'index' => 'PRIMARY', 'filter' => "user_id,ts,data"];
$index = $hs->openIndex('test', 'HandlerSocketTestTbl', array('user_id', 'ts', 'data'),$open_options);
var_dump(is_a($index, 'HandlerSocketi_Index'));

$user_id = 1;
$operation = '=';
$res = $index->find([$operation => $user_id], ['safe' => 0]); 
var_dump(empty($res));
var_dump(is_array($res));
var_dump($res[0][0] == $user_id);
?>
--EXPECT--
bool(true)
bool(false)
bool(true)
bool(true)
