<?php

	$req_method = $_SERVER["REQUEST_METHOD"] ?? "GET";
	$req_params = $_GET;
	$req_content_type = $_SERVER["CONTENT_TYPE"] ?? "";
	$req_content_length = isset($_SERVER["CONTENT_LENGTH"]) ? intval($_SERVER["CONTENT_LENGTH"]) : null;

	$req_content = file_get_contents("php://input", length: $req_content_length);

	if ($req_method != "POST") {
		header("Status: 405 Method Not Allowed");
		header("Content-Length: 0");
		echo "\n";
		exit;
	}

	if ($req_content_type != "text/plain") {
		header("Status: 400 Bad Request");
		header("Content-Type: text/html");
		echo "<span>Expected content to be in plain text</span>";
		exit ;
	}

	header("Status: 200 OK");
	header("Content-Type: text/html");
	$req_lines = explode("\n", $req_content);
	foreach ($req_lines as $req_line) {
		echo "<li>$req_line<li>\n";
	}
?>
