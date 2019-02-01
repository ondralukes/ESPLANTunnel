<?php
error_reporting(0);
clearstatcache();
$miso = "miso";
$mosi = "mosi";
$inputraw = file_get_contents('php://input');
$input = substr($inputraw, 1);
if($inputraw[0] == 'D'){
	$wpath = $mosi;
}  else if($inputraw[0] == 'C'){
	$wpath = $mosi."ctl";
	$input .= ';';
}
$maxsize = 4096;
//////////////////////////////////////////WRITE
if(strlen($input) >0){
while(file_exists($wpath.".lock"));
fclose(fopen($wpath.".lock","w"));
$handle = fopen($wpath,"a");
fwrite($handle,$input);
fclose($handle);
if(file_exists($wpath.".lock"))unlink($wpath.".lock");
}
//////////////////////////////////////READ
if(filesize($miso)==0){
	//////////////////////////////////////////////////CTL
while(file_exists($miso."ctl.lock"));
fclose(fopen($miso."ctl.lock","w"));

$data = "";
echo("^");                              /////startingchar
if(filesize($miso."ctl") > 0){
echo("C");
$handle = fopen($miso."ctl","r");
$data = fread($handle,filesize($miso."ctl"));
$end = strpos($data,";");
echo(substr($data,0,$end));
$data = substr($data,$end+1);
fclose($handle);
}
if(strlen($data) ==1) $data="";
$handle = fopen($miso."ctl","w");
fwrite($handle,$data);
fclose($handle);
if(file_exists($miso."ctl.lock"))unlink($miso."ctl.lock");
} else {//////////////////////////////////////////////////DATA
while(file_exists($miso.".lock"));
fclose(fopen($miso.".lock","w"));
$handle = fopen($miso."","r");
$data = "";
echo("^");                              /////startingchar
if(filesize($miso) > 0){
echo("D");
$data = fread($handle,filesize($miso));
if(strlen($data) > $maxsize){
	echo(substr($data,0,$maxsize));
	$data = substr($data,$maxsize);
} else{
	echo($data);
	$data = "";
}
}
fclose($handle);
$handle = fopen($miso,"w");
fwrite($handle,$data);
fclose($handle);
if(file_exists($miso.".lock"))unlink($miso.".lock");
}
?>
