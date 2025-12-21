<?php

	$req_method = $_SERVER["REQUEST_METHOD"] ?? "GET";
	$req_content_type = $_SERVER["CONTENT_TYPE"] ?? "text/plain";

	if ($req_method != "POST") {
		header("Status: 405 Method Not Allowed");
		echo "\n";
		exit;
	}
	if ($req_content_type != "application/x-www-form-urlencoded") {
		header("Status: 400 Bad Request");
		echo "\n";
		exit;
	}

	$req_params = $_POST;

	header("Status: 200 OK");
	header("Content-Type: text/html");
	foreach ($req_params as $name => $value) {
		echo "<li>$name = $value</li>\n";
	}
?>
