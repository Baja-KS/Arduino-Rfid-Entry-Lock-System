<?php
try {
    $pdo=new PDO('mysql:host=localhost;dbname=rfidDB','root','');
} catch (PDOException $e) {
    echo $e->getmessage();
}
?>