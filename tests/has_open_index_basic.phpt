--TEST--
hasOpenIndex() basic test
--FILE--
<?php
include(__DIR__."/common.inc");

if (!class_exists('handlersocketi')) dl('handlersocketi.so');

$options = ['timeout' => 1, 'persistent' => true];
$hs = new \HandlerSocketi($config['server'], $config['read_port'], $options);
$open_options = ['id' => 1, 'index' => 'PRIMARY', 'filter' => "user_id,ts,data"];
$has_open_index = $hs->hasOpenIndex('test', 'HandlerSocketTestTbl', ['user_id', 'ts', 'data'], $open_options);
var_dump($has_open_index);
$index = $hs->openIndex('test', 'HandlerSocketTestTbl', array('user_id', 'ts', 'data'),$open_options);
$has_open_index = $hs->hasOpenIndex('test', 'HandlerSocketTestTbl', ['user_id', 'ts', 'data'], $open_options);
var_dump($has_open_index);
?>
--EXPECT--
bool(false)
bool(true)
