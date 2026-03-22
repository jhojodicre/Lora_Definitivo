# Protocolo CHISMOSO

## Objetivo

Este documento describe el protocolo CHISMOSO tal como esta implementado hoy en el codigo. No propone todavia una version nueva; primero fija el comportamiento real del Maestro y del Nodo para que la siguiente fase de mejora parta de una base verificable.

## Alcance del analisis

El comportamiento documentado aqui fue reconstruido a partir de la implementacion actual en:

- [Master.cpp](Master.cpp)
- [Master.h](Master.h)
- [Lora.cpp](Lora.cpp)
- [Lora.h](Lora.h)
- [Lora_Definitivo.ino](Lora_Definitivo.ino)
- [Flujo del Codigo](Flujo%20del%20Codigo)

## Resumen ejecutivo

CHISMOSO es un protocolo LoRa orientado a dos roles:

- Maestro: consulta nodos, recibe respuestas, administra timeouts, reintentos y una cola de mensajes provenientes del servidor web.
- Nodo: monitorea entradas locales, genera alertas, atiende mensajes del Maestro, ejecuta funciones y responde con su estado.

En la implementacion actual, el protocolo no usa una sola semantica de trama. La misma trama de 8 caracteres cambia de significado segun la direccion del mensaje:

- Maestro -> Nodo: los bytes 3 a 7 son comando y parametros.
- Nodo -> Maestro: los bytes 3 a 7 son estados de zonas, salidas y fuente.

Eso funciona, pero mezcla dos protocolos dentro del mismo frame y es una de las principales razones para documentarlo antes de optimizarlo.

## Configuracion observada en el firmware actual

En [Lora_Definitivo.ino#L65](Lora_Definitivo.ino#L65) y [Lora_Definitivo.ino#L66](Lora_Definitivo.ino#L66) se instancia:

- `Lora Node(false,5,'1');`
- `Master Chismoso(false, 5, '3');`

Interpretacion de esa configuracion:

- El objeto de radio `Node` tiene direccion local `'1'`.
- El objeto `Chismoso` esta creado en modo Nodo porque el primer parametro es `false`.
- El `NodeAddress` del protocolo en `Chismoso` es `'3'`.

Esto sugiere que hoy existen al menos dos conceptos de direccion local en memoria y no necesariamente estan sincronizados. Esa observacion es importante para cualquier rediseño futuro.

## Componentes involucrados

### Capa de radio e IO

La clase `Lora` se encarga de:

- Inicializar el radio LoRa.
- Transmitir y recibir paquetes.
- Mantener banderas de recepcion.
- Leer zonas, salidas y bateria.
- Exponer estados que luego el protocolo empaqueta.

Funciones clave:

- [Lora.cpp#L176](Lora.cpp#L176): `Lora::Lora_TX`
- [Lora.cpp#L207](Lora.cpp#L207): `Lora::Lora_RX`
- [Lora.cpp#L472](Lora.cpp#L472): `Lora::Lora_IO_Battery`
- [Lora.cpp#L525](Lora.cpp#L525): `Lora::Lora_IO_Zones`

### Capa de protocolo

La clase `Master` implementa ambos roles logicos:

- Protocolo del Maestro.
- Protocolo del Nodo.

Funciones clave del Nodo:

- [Master.cpp#L359](Master.cpp#L359): `Master::Node_Decodificar`
- [Master.cpp#L428](Master.cpp#L428): `Master::Node_Protocol`

Funciones clave del Maestro:

- [Master.cpp#L626](Master.cpp#L626): `Master::Nodo_REQUEST`
- [Master.cpp#L702](Master.cpp#L702): `Master::MasterMessage`
- [Master.cpp#L809](Master.cpp#L809): `Master::MasterDecodificar`
- [Master.cpp#L890](Master.cpp#L890): `Master::Master_Protocol`

### Orquestacion principal

El firmware arranca en [Lora_Definitivo.ino#L85](Lora_Definitivo.ino#L85) y ejecuta el ciclo principal en [Lora_Definitivo.ino#L106](Lora_Definitivo.ino#L106). El `loop()` llama a `Chismoso.Preguntar()`, y eso deriva en:

- `Master_Protocol()` si el equipo esta en modo Maestro.
- `Node_Protocol()` si el equipo esta en modo Nodo.
- `Calibration_Protocol()` si la calibracion esta activa.

## Formato de trama actual

### Longitud base

La implementacion opera sobre una trama de 8 caracteres:

| Posicion | Campo generico |
|---|---|
| 0 | Remitente |
| 1 | Destinatario |
| 2 | Tipo de mensaje |
| 3 | Campo 1 |
| 4 | Campo 2 |
| 5 | Campo 3 |
| 6 | Campo 4 |
| 7 | Campo 5 |

### Semantica Maestro -> Nodo

Cuando el Maestro envia una trama, la interpretacion usada por `Node_Decodificar()` es:

| Posicion | Significado |
|---|---|
| 0 | Remitente |
| 1 | Destinatario |
| 2 | Tipo de mensaje |
| 3 | Modo de funcion |
| 4 | Numero de funcion |
| 5 | Parametro 1 |
| 6 | Parametro 2 |
| 7 | Parametro 3 |

Referencia: [Master.cpp#L359](Master.cpp#L359).

### Semantica Nodo -> Maestro

Cuando el Nodo responde, `Node_Message()` construye esta trama:

| Posicion | Significado |
|---|---|
| 0 | Direccion del nodo |
| 1 | Direccion del maestro |
| 2 | Tipo de mensaje |
| 3 | Estado zona A |
| 4 | Estado zona B |
| 5 | Estado salida 1 |
| 6 | Estado salida 2 |
| 7 | Estado de fuente |

Referencia: [Master.cpp#L319](Master.cpp#L319).

## Tipos de mensaje observados

Los siguientes tipos aparecen en el codigo actual:

| Tipo | Uso observado |
|---|---|
| `.` | Consulta regular del Maestro al Nodo |
| `E` | Emergencia del Nodo al Maestro, pero tambien se usa para marcar ejecucion en el Nodo |
| `P` | Respuesta del Nodo al Maestro |
| `O` | Acuse relacionado con nodo escuchado o sincronizacion |
| `A` | En algunos lugares significa alerta; en otros solo dispara contador |
| `M` | Mensaje especial desde servidor o Maestro, pero en el Nodo hoy no ejecuta logica |
| `F` | Caso especial para empaquetar contador en `Node_Message()` |
| `R` | Reset detectado por el Maestro al decodificar |

Conclusion: la tabla de tipos no esta cerrada ni consistentemente aplicada en todos los flujos.

## Que hace el Maestro

## 1. Inicializacion

En modo Maestro, `Iniciar()` crea el temporizador principal de encuesta (`timer_master`) y arranca el ciclo de consulta periodica. Referencia: [Master.cpp#L70](Master.cpp#L70).

## 2. Arbitraje de transmision

En [Master.cpp#L890](Master.cpp#L890), el Maestro ejecuta este orden:

1. Leer radio con `nodeRef->Lora_RX()`.
2. Revisar timeout de una TX en curso.
3. Si no hay TX en curso, priorizar cola del servidor.
4. Si no hay servidor pendiente o se alcanzo el burst maximo, consultar al siguiente nodo.
5. Procesar timeout o respuesta recibida.
6. Ejecutar comandos locales provenientes del servidor si aplica.

## 3. Seleccion del siguiente nodo

`Nodo_REQUEST()` hace lo siguiente:

1. Si hay reintento pendiente, vuelve a consultar ese nodo.
2. Si no, avanza secuencialmente del primero al ultimo y vuelve a empezar.
3. Incrementa contador de intentos del nodo consultado.
4. Arma un temporizador de no respuesta.
5. Si no llega respuesta dentro del timeout, marca reintento 1 o 2.

Referencia: [Master.cpp#L626](Master.cpp#L626).

## 4. Construccion de consulta regular

`MasterMessage()` arma la trama del Maestro con este patron:

- remitente = `Master_Address` (`X`)
- destinatario = `Nodo_Proximo`
- tipo = `.`
- resto de campos = `tx_funct_*`

Referencia: [Master.cpp#L702](Master.cpp#L702).

## 5. Recepcion y decodificacion de respuestas

`MasterDecodificar()`:

1. Parsea la trama recibida.
2. Si el remitente coincide con el nodo esperado, libera el canal TX.
3. Marca `nodeResponde = true`.
4. Actualiza el arreglo `estadosNodos[]`.
5. Si el tipo recibido es `A`, lo interpreta como alerta.
6. Si el remitente no era el esperado, lo toma como mensaje inesperado o alerta espontanea.
7. Si el tipo es `E`, encola un acuse `O` hacia ese nodo.

Referencia: [Master.cpp#L809](Master.cpp#L809).

## 6. Integracion con servidor

El Maestro puede recibir solicitudes desde el servidor web:

- Si el mensaje comienza con `M`, se ejecuta localmente en el Maestro.
- Si no, se convierte en una trama LoRa y se mete en cola para ser enviada a un nodo.

La cola del servidor usa `SERVER_QUEUE_SIZE = 10` y `SERVER_BURST_MAX = 2`. Referencia: [Master.h](Master.h) y [Master.cpp#L482](Master.cpp#L482).

## 7. Persistencia del estado

Cuando el Maestro recibe o pierde un nodo, `NodeStatusUpdate()` actualiza:

- `Node_Status_str`
- `Node_Num_str`
- campos de la ultima trama
- `jsonString`
- bandera `F_ServerUpdate`

Eso permite que el `loop()` principal publique datos al servidor. Referencia: [Master.cpp#L732](Master.cpp#L732) y [Lora_Definitivo.ino#L130](Lora_Definitivo.ino#L130).

## Que hace el Nodo

## 1. Leer radio y estados locales

`Node_Protocol()` arranca cada iteracion con:

1. `Lora_RX()` para ver si llego una trama.
2. `Lora_IO_Zones()` para actualizar entradas, salidas y bateria.

Referencia: [Master.cpp#L428](Master.cpp#L428).

## 2. Detectar eventos locales

Si `nodeRef->F_IO_Event_Enable` esta activa, el Nodo:

1. Llama `Node_Alerta()`.
2. Cambia `message_type = "E"`.
3. Marca alerta pendiente de transmision.
4. Desactiva temporalmente la bandera de IO.

Referencia: [Master.cpp#L303](Master.cpp#L303) y [Master.cpp#L440](Master.cpp#L440).

## 3. Decodificar mensajes entrantes

Si `F_Recibido` esta en `true`, `Node_Decodificar()`:

1. Parte la trama en 8 campos.
2. Verifica si el destinatario coincide con `NodeAddress`.
3. Si el mensaje es para el nodo, activa respuesta.
4. Si `rx_mensaje == "E"`, habilita ejecucion de funcion.
5. Si `rx_mensaje == "O"`, considera que el nodo fue escuchado y desactiva alerta pendiente.
6. Si el nodo esta en alerta y detecta trafico al Maestro, puede sincronizar su envio diferido.

Referencia: [Master.cpp#L359](Master.cpp#L359).

## 4. Ejecutar comandos

Si `F_Node_Excecute` esta activa, el Nodo:

1. Concatena `rx_funct_mode + rx_funct_num + rx_funct_parameter1 + rx_funct_parameter2 + rx_funct_parameter3`.
2. Llama `Functions_Request()`.
3. Llama `Functions_Run()`.
4. Refresca IO con `Lora_IO_Zones()`.
5. Prepara respuesta al Maestro.

Referencia: [Master.cpp#L459](Master.cpp#L459).

## 5. Transmitir estado

Si `F_Responder` esta activa, el Nodo:

1. Construye la trama con `Node_Message()`.
2. Usa `Lora_TX()` para enviarla.
3. Resetea `F_Responder`.

Referencia: [Master.cpp#L479](Master.cpp#L479).

## Reglas actuales del protocolo CHISMOSO

Estas son las reglas efectivas que hoy se desprenden del codigo:

1. La red opera por encuesta ciclica del Maestro hacia los nodos.
2. El Maestro mantiene a lo sumo una TX logica en curso con timeout asociado.
3. Un nodo puede responder por consulta regular o por evento local.
4. El Nodo usa una unica trama de estado para informar zonas, salidas y fuente.
5. El Maestro acepta mensajes inesperados y los trata como posible alerta espontanea.
6. El servidor web puede inyectar comandos a la cola del Maestro.
7. Hay prioridad para mensajes del servidor, pero con limitacion de burst para no bloquear la encuesta automatica.
8. Los reintentos por no respuesta son maximo dos.
9. La recepcion LoRa se basa en ISR (`rxFlag`) mas procesamiento diferido en `loop()`.
10. El mismo campo `tipo de mensaje` esta sobrecargado con significados distintos segun contexto.

## Hallazgos tecnicos importantes

## 1. El frame cambia de significado segun el sentido

Eso complica validacion, versionado, CRC, extensibilidad y depuracion.

## 2. Los tipos de mensaje no estan normalizados

Ejemplos:

- El Nodo pone `message_type = "E"` para alerta local.
- El Maestro interpreta `A` como alerta en `MasterDecodificar()`.
- El Nodo ejecuta funciones cuando recibe `E`, no cuando recibe `M`.
- `M` existe conceptualmente como mensaje especial, pero en `Node_Decodificar()` no dispara logica.

## 3. El nombre de campos no refleja una capa unica

`rx_master_lora_*` se usa tambien dentro del rol Nodo, lo que mezcla semanticas y vuelve mas dificil el mantenimiento.

## 4. Hay direccionamiento duplicado

La direccion local del radio (`local_Address`) y la del protocolo (`NodeAddress`) no necesariamente son la misma.

## 5. Hay mezcla entre transporte, sesion y aplicacion

La clase `Master` hace al mismo tiempo:

- arbitraje de canal
- secuenciacion
- parseo de tramas
- logica de alertas
- integracion con servidor
- ejecucion de comandos

Eso aumenta el acoplamiento.

## 6. No hay control estructural de integridad

Hoy no se observa en las tramas:

- version de protocolo
- longitud declarada
- checksum o CRC de aplicacion
- numero de secuencia estable por mensaje
- tipo de payload desacoplado del tipo de frame

LoRa/Radiolib protege a nivel fisico, pero no resuelve por si sola la coherencia del protocolo de aplicacion.

## 7. El flujo de alertas esta parcialmente sincronizado, pero no completamente definido

Existe una mecanica de alerta diferida y de acuse `O`, pero no hay una especificacion cerrada sobre:

- cuando una alerta se considera entregada
- si debe repetirse
- cuantas veces
- que hacer ante perdida del acuse

## Oportunidades claras para la siguiente fase

Sin diseñar todavia la solucion final, el codigo actual muestra estas lineas de mejora:

1. Separar claramente protocolo Maestro y protocolo Nodo.
2. Definir un frame unico con campos estables y payload versionado.
3. Crear una tabla cerrada de tipos de mensaje y de estados.
4. Unificar direccion local y direccion logica.
5. Separar consulta regular, evento, comando y acuse como operaciones distintas.
6. Formalizar timeout, reintento, prioridad y acuse.
7. Mantener compatibilidad con servidor web pero desacoplando su cola del parser de radio.

## Estado de este documento

Este documento describe el protocolo CHISMOSO actual, no el protocolo ideal. La siguiente etapa recomendada es convertir estas observaciones en:

1. una especificacion v2 del frame
2. una maquina de estados Maestro
3. una maquina de estados Nodo
4. una politica de retransmision y acuses
5. un plan de migracion sin romper el hardware actual

## Backlog priorizado para mejorar CHISMOSO

Esta lista esta ordenada por impacto en confiabilidad y por riesgo operativo.

### Prioridad 0 (critico)

#### 0.1 Unificar direccion local de nodo

- Problema: hoy hay dos fuentes de verdad para la direccion (`local_Address` en `Lora` y `NodeAddress` en `Master`) y pueden divergir.
- Impacto: mensajes ignorados o aceptados por el nodo equivocado.
- Evidencia: [Lora_Definitivo.ino#L65](Lora_Definitivo.ino#L65) y [Lora_Definitivo.ino#L66](Lora_Definitivo.ino#L66).
- Accion recomendada: exponer una sola direccion canonica y usarla en parseo, armado y logs.
- Criterio de salida: no existe ningun flujo que compare contra una direccion distinta de la canonica.

#### 0.2 Normalizar tabla de tipos de mensaje

- Problema: `E`, `A`, `M`, `O`, `P`, `F`, `R`, `.` tienen usos cruzados y en algunos casos contradictorios.
- Impacto: comportamiento ambiguo y bugs de estado dificilmente reproducibles.
- Evidencia: [Master.cpp#L359](Master.cpp#L359), [Master.cpp#L809](Master.cpp#L809), [Master.cpp#L890](Master.cpp#L890).
- Accion recomendada: crear enum/constantes con un significado unico por tipo.
- Criterio de salida: cada tipo se interpreta igual en Maestro y Nodo.

### Prioridad 1 (alta)

#### 1.1 Separar frame de comando y frame de estado

- Problema: la misma trama de 8 bytes cambia semantica segun sentido.
- Impacto: parser fragil, dificil de extender y validar.
- Accion recomendada: mantener 8 bytes si se quiere compatibilidad, pero con `kind` explicito y campos estables; alternativa: frame variable con cabecera fija.
- Criterio de salida: parser unico que no dependa del sentido para interpretar posiciones.

#### 1.2 Definir contrato de ACK y retransmision

- Problema: la alerta diferida y el acuse `O` no tienen reglas cerradas de entrega.
- Impacto: posible perdida silenciosa de alertas o reintentos innecesarios.
- Accion recomendada: fijar politica de reintento, timeout por tipo y maximo de retransmisiones.
- Criterio de salida: cada mensaje critico tiene estado `pendiente`, `confirmado` o `expirado`.

#### 1.3 Incorporar numero de secuencia de aplicacion

- Problema: no hay identificador robusto de mensaje para deduplicar/reintentar.
- Impacto: duplicados y dificultad para correlacionar TX/RX.
- Accion recomendada: agregar `seq` por remitente y validacion de repetidos.
- Criterio de salida: el receptor detecta y descarta duplicados de forma deterministica.

### Prioridad 2 (media)

#### 2.1 Separar responsabilidades de `Master`

- Problema: mezcla transporte, scheduler, parser, negocio, servidor y ejecucion.
- Impacto: alto acoplamiento y baja testabilidad.
- Accion recomendada: dividir en modulos: `LinkLayer`, `ProtocolCodec`, `NodeScheduler`, `ServerBridge`.
- Criterio de salida: cada modulo tiene una responsabilidad clara y API corta.

#### 2.2 Formalizar maquina de estados Maestro/Nodo

- Problema: hoy hay muchas banderas con transiciones implícitas.
- Impacto: condiciones de carrera logicas y estados invalidados por orden de ejecucion.
- Accion recomendada: modelar estados explicitos y transiciones con eventos.
- Criterio de salida: diagrama y codigo alineados con transiciones verificables.

#### 2.3 Endurecer validacion de trama

- Problema: no hay version ni longitud declarada ni checksum de aplicacion.
- Impacto: tramas parcialmente validas pueden activar acciones no deseadas.
- Accion recomendada: agregar `ver`, `len` y checksum simple de payload.
- Criterio de salida: cualquier trama invalida se descarta con motivo de error trazable.

### Prioridad 3 (mejora continua)

#### 3.1 Telemetria de protocolo

- Problema: no existe metrica consolidada de PER, latencia, reintentos y colisiones logicas.
- Impacto: se optimiza a ciegas.
- Accion recomendada: exponer contadores y percentiles basicos via JSON/serial.
- Criterio de salida: dashboard o log con KPIs minimos del enlace.

#### 3.2 Compatibilidad evolutiva

- Problema: cambios de protocolo pueden romper nodos desplegados.
- Impacto: riesgo operativo en campo.
- Accion recomendada: estrategia de versionado y modo dual temporal v1/v2.
- Criterio de salida: migracion gradual sin downtime de red.

## Orden sugerido de ejecucion

1. P0.1 Direccion canonica.
2. P0.2 Tabla unica de tipos.
3. P1.1 Frame unificado (manteniendo compatibilidad).
4. P1.2 ACK/retransmision.
5. P1.3 Secuencia y deduplicacion.
6. P2.2 Maquinas de estados.
7. P2.1 Refactor modular.
8. P2.3 Validacion estructural.
9. P3.1 Telemetria.
10. P3.2 Migracion v1/v2.

## Entregable para la siguiente sesion

Si seguimos con la optimizacion, el siguiente entregable recomendado es:

1. especificacion minima v2 de trama (campos, tipos y ejemplos)
2. tabla final de tipos de mensaje
3. matriz de compatibilidad v1/v2