--TEST--
find() with limit and offset
--FILE--
<?php
include(__DIR__."/common.inc");

if (!class_exists('handlersocketi')) dl('handlersocketi.so');

$options = ['timeout' => 1, 'persistent' => true];
$hs = new \HandlerSocketi($config['server'], $config['read_port'], $options);
$open_options = ['id' => 1, 'index' => 'PRIMARY', 'filter' => "user_id,ts,data"];
$index = $hs->openIndex('test', 'HandlerSocketTestTbl', array('user_id', 'ts', 'data'),$open_options);
$user_id = 1;
$offset = 100;
$res_without_offset = $index->find(['>' => $user_id], ['limit' => 10, 'safe' => 0, ]);
$res_with_offset = $index->find(['>' => $user_id], ['limit' => 10, 'safe' => 0, 'offset' => $offset]);
var_dump(empty($res_with_offset));
?>
--EXPECT--
bool(true)
