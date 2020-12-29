<?php
    try {
        $pdo=new PDO('mysql:host=localhost;dbname=rfidDB','root','');
    } catch (PDOException $e) {
        echo $e->getmessage();
    }

    $mode=$_GET['mode'];
    $rfid=$_GET['rfid'];


    $query=$pdo->prepare('insert into Entries(mode,rfid) values(:mode,:rfid)');
   $query->bindParam("mode",$mode);
   $query->bindParam("rfid",$rfid);

   $query->execute();
    echo "#".true;

?>