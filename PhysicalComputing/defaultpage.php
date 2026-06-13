<?php
/**
 * defaultpage.php — Hosting-Standardseite (Infomaniak/Hostpoint); leitet zur Hauptdomain weiter.
 */

$domain = strtr( $_SERVER["HTTP_HOST"], array("www." => ""));

$sprache = substr($_SERVER['HTTP_ACCEPT_LANGUAGE'], 0, 2);



if ($sprache != "de" AND $sprache != "fr" AND $sprache != "it" AND $sprache != "en")

{ 

	$sprache = "de";

}



$ch = curl_init("http://defaultpage.hostpoint.ch/$sprache/index.php?domain=$domain");



curl_exec($ch);



curl_close($ch);



?>

