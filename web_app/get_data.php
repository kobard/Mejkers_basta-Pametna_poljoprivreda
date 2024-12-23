<?php
// Pristupni podaci za bazu podataka
$host = "adresa_mysql_hosta";
$user = "korisnicko_ime_mysql_baze";
$password = "lozinka_za_mysql_korisnika";
$database = "ime_baze_na_mysql_serveru";

// Kreiranje konekcije prema bazi podataka
$conn = new mysqli($host, $user, $password, $database);

// Provera konekcije
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}

// Upit koji preuzima poslednji upis iz tabele
$sql = "SELECT vlaznost, temperatura, ec, ph, n, p, k FROM zemlja ORDER BY vreme DESC LIMIT 1";
$result = $conn->query($sql);

if ($result->num_rows > 0) {
    $data = $result->fetch_assoc();
    echo json_encode($data); // Vraca podatke u JSON formi
} else {
    echo json_encode(["error" => "Greska: nema podataka."]);
}

$conn->close();
?>