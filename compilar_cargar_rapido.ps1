# Script PowerShell para compilar y cargar más rápido
# Ajusta el puerto COM según tu configuración

# Puerto COM - ajustar según sea necesario
$PUERTO_COM = "COM8"  

# Compilar el proyecto con optimizaciones
Write-Host "Compilando con optimizaciones..."
arduino-cli compile --fqbn esp32:esp32:heltec_wifi_lora_32_V2 Lora_Definitivo --build-property "build.code_debug=1" --build-property "compiler.cpp.extra_flags=-DRELEASE_BUILD"

# Verificar si la compilación fue exitosa
if ($LASTEXITCODE -eq 0) {
    Write-Host "Cargando a la placa..."
    # Cargar a la máxima velocidad disponible
    arduino-cli upload -p $PUERTO_COM --fqbn esp32:esp32:heltec_wifi_lora_32_V2 Lora_Definitivo --verify --upload-speed 921600
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host "¡Carga exitosa!" -ForegroundColor Green
    } else {
        Write-Host "Error durante la carga" -ForegroundColor Red
    }
} else {
    Write-Host "Error durante la compilación" -ForegroundColor Red
}
