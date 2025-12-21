<?php

	$req_method = $_SERVER["REQUEST_METHOD"] ?? "GET";
	$req_params = $_GET;

	if ($req_method != "GET") {
		header("Status: 405 Method Not Allowed");
		header("Content-Length: 0");
		echo "\n";
		exit;
	}

	header("Status: 200 OK");
	header("Content-Type: text/html");
	foreach ($req_params as $name => $value) {
		echo "<li>$name = $value</li>\n";
	}
?>
