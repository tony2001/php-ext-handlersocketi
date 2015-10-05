--TEST--
openIndex() and find() - 2
--FILE--
<?php
include(__DIR__."/common.inc");

if (!class_exists('handlersocketi')) dl('handlersocketi.so');

$options = ['timeout' => 1, 'persistent' => true];
$hs = new \HandlerSocketi($config['server'], $config['write_port'], $options);
$open_options = ['id' => 1, 'index' => 'PRIMARY', 'filter' => "user_id,ts,data"];
$index = $hs->openIndex('test', 'HandlerSocketTestTbl', array('user_id', 'ts', 'data'),$open_options);
$insert_values = [10,1010110];

$already_exist = $index->find(['=' => $insert_values[0]], ['safe' => 0]);
if (!empty($already_exist)) {
    $index->remove(['=' => $insert_values[0]]);
}

$index->insert($insert_values);
$user_id = $insert_values[0];
$inserted_values = $index->find(['=' => $user_id], ['safe' => 0]);
var_dump($insert_values[0] == $inserted_values[0][0]);
var_dump($insert_values[1] == $inserted_values[0][1]);
var_dump(empty($inserted_values[0][2]));

?>
--EXPECT--
bool(true)
bool(true)
bool(true)
