<?php
    try {
        $pdo=new PDO('mysql:host=localhost;dbname=rfidDB','root','');
    } catch (PDOException $e) {
        echo $e->getmessage();
    }

    $valid=0;

    $rfid=$_GET['rfid'];


    $query=$pdo->prepare('select * from Users where rfid=:rfid limit 1');
   $query->bindParam("rfid",$rfid);

   $query->execute();

    $row=$query->fetch(PDO::FETCH_ASSOC);

    $row ? $valid=1 : $valid=0;

    echo "#".$valid;

?>