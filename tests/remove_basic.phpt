--TEST--
openIndex() and remove()
--FILE--
<?php
include(__DIR__."/common.inc");

if (!class_exists('handlersocketi')) dl('handlersocketi.so');

$options = ['timeout' => 1, 'persistent' => true];
$hs = new \HandlerSocketi($config['server'], $config['write_port'], $options);
$open_options = ['id' => 1, 'index' => 'PRIMARY', 'filter' => "user_id,ts,data"];
$index = $hs->openIndex('test', 'HandlerSocketTestTbl', array('user_id', 'ts', 'data'),$open_options);
$user_id = 10;

$is_exist = $index->find(['=' => $user_id], ['safe' => 0]);
if (empty($is_exist)) {
    $insert_values = [10,101010,10];
    $index->insert($insert_values);
}

$index->remove(['=' => $user_id]);
$is_exist = $index->find(['=' => $user_id], ['safe' => 0]);
var_dump(empty($is_exist));

?>
--EXPECT--
bool(true)
