<?php
/*
 *   zemlja_input.php
 *   - Unos podataka pristiglih sa esp8266 u MySQL bazu podataka
*/

$podaci7u1 = $_POST["podaci7u1"]; // podaci koje salje ESP preko http-post zahteva

// Input string
// $input = "'11.1', '22.2', '333', '3.3', '444', '555', '666', CURRENT_TIMESTAMP, NULL";
$input = $podaci7u1;

// Step 1: Remove extra characters (like quotes) for parsing
$cleaned = str_replace(["'", "CURRENT_TIMESTAMP", "NULL"], "", $input);

// Step 2: Split the string into parts
$values = explode(',', $cleaned);

// Step 3: Trim each value to remove extra spaces
$values = array_map('trim', $values);

// Step 4: Assign to variables
$vlaznost = $values[0];
$temperatura = $values[1];
$ec = $values[2];
$ph = $values[3];
$n  = $values[4];
$p  = $values[5];
$k  = $values[6];

$red_za_csv = $values[0] . "," . $values[1] . "," . $values[2] . "," . $values[3] . "," . $values[4] . "," . $values[5] . "," . $values[6] . "\r\n";

// Append the data
if (file_put_contents("7u1ocitavanja.csv", $red_za_csv, FILE_APPEND | LOCK_EX)) {
    echo "Data successfully appended to the file.";
} else {
    echo "Failed to append data.";
}

// Database connection credentials
//$servername   = "localhost";
$servername   = "adresa_mysql_hosta";
$username     = "korisnicko_ime_mysql_baze";
$password     = "lozinka_za_mysql_korisnika";
$dbname       = "ime_baze_na_mysql_serveru";

// Create a new MySQLi connection
$conn = new mysqli($servername, $username, $password, $dbname);

// Check connection
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}

// Prepare an SQL statement
$sql = "INSERT INTO zemlja (vlaznost, temperatura, ec, ph, n, p, k, vreme, ID) VALUES (" . $podaci7u1 . ")";

if (mysqli_query($conn, $sql)) {
    echo "New record created successfully";
} else {
    echo "Error: " . $sql . "<br>" . mysqli_error($conn);
}

// Close the statement and connection

$conn->close();