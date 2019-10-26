<?php
    $DB_SERVER = '';
    $DB_USERNAME = '';
	$DB_PASSWORD = '';
    $DB_NAME = '';

	   /* Attempt to connect to MySQL database */

	   $link = mysqli_connect($DB_SERVER, $DB_USERNAME, $DB_PASSWORD, $DB_NAME);

	   // Check connection
	   if($link === false){
		   die("Erreur de connexion à la base de données. " . mysqli_connect_error());
	   }

	$mytime=(int)time();
	$temp1=(float)$_POST["temp1"];
	$temp2=(float)$_POST["temp2"];
	$temp3=(float)$_POST["temp3"];
	$humi=(float)$_POST["humi"];
	$pres=(float)$_POST["pres"];
	$relpres=(float)$_POST["relpres"];
	$pm25=(float)$_POST["pm25"];

	$sql = "INSERT INTO meteo_data (mytime, temp1, temp2, temp3, humi, pres, relpres, pm25) 
		VALUES (?, ?, ?, ?, ?, ?, ?, ?)"; 

if($stmt = mysqli_prepare($link, $sql)){

	// Bind variables to the prepared statement as parameters
	mysqli_stmt_bind_param($stmt, "iddddddd", $param_time, $param_temp1,$param_temp2,$param_temp3,$param_humi,$param_pres,$param_relpres,$param_pm25);

	// Set parameters
	$param_time = $mytime;
	$param_temp1 = $temp1;
	$param_temp2 = $temp2;
	$param_temp3 = $temp3;
	$param_humi = $humi;
	$param_pres = $pres;
	$param_relpres = $relpres;
	$param_pm25 = $pm25;

	// Attempt to execute the prepared statement
	if(mysqli_stmt_execute($stmt)){
		
		echo 'Success !';
} else {
	echo 'Echec !';
}
}

$link->close();   

?>
