# Script para compilar el proyecto Lora_Definitivo

# Verificar si el comando arduino-cli está disponible
try {
    arduino-cli version
} catch {
    Write-Host "arduino-cli no está instalado o no está en el PATH. Por favor, instálelo primero."
    exit 1
}

Write-Host "Compilando Lora_Definitivo..."

# Compilar el proyecto
$result = arduino-cli compile --fqbn esp32:esp32:heltec_wifi_lora_32_V2 .

if ($LASTEXITCODE -eq 0) {
    Write-Host "Compilación exitosa!" -ForegroundColor Green
} else {
    Write-Host "Error en la compilación" -ForegroundColor Red
}
