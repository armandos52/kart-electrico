---
estado: vigente
actualizado: 2026-09-30
resumen: Entradas a los proyectos de código, diseños y registro de datos de la entrega.
---

# Datos y proyectos originales

Entradas localizadas por inventario. El código, los diseños y sus dependencias se conservan completos; no se han compilado. Estructura, versiones y mejoras del código: [[04_CONOCIMIENTO/software_y_telemetria/INVENTARIO_CODIGO|inventario del código]] (30-09-2026).

## Entradas principales

| Archivo | Contexto de la carpeta | Abrir |
|---|---|---|
| main.c | 24/25 | [Original](../02_ORIGINALES/entrega_2026-09-28/Relat%C3%B3rios%20de%20anos%20anteriores/PEEC_2425_10_Smart_Eletric_Kart_Cross_Anexos/PEEC_2425_10_Smart_Eletric_Kart_Cross_Anexos/KartProjetoFinal/Core/Src/main.c) |
| KartProjetoFinal.ioc | 24/25 | [Original](../02_ORIGINALES/entrega_2026-09-28/Relat%C3%B3rios%20de%20anos%20anteriores/PEEC_2425_10_Smart_Eletric_Kart_Cross_Anexos/PEEC_2425_10_Smart_Eletric_Kart_Cross_Anexos/KartProjetoFinal/KartProjetoFinal.ioc) |
| CodigoKart.ino | 2023/24 | [Original](../02_ORIGINALES/entrega_2026-09-28/Relat%C3%B3rios%20de%20anos%20anteriores/ProjetoKart2324/ProjetoKart2324/Arduino/CodigoKart/CodigoKart.ino) |
| Part1.SLDDRW | 2023/24 | [Original](../02_ORIGINALES/entrega_2026-09-28/Relat%C3%B3rios%20de%20anos%20anteriores/ProjetoKart2324/ProjetoKart2324/CaixaBaterias/Part1.SLDDRW) |
| Part1.SLDPRT | 2023/24 | [Original](../02_ORIGINALES/entrega_2026-09-28/Relat%C3%B3rios%20de%20anos%20anteriores/ProjetoKart2324/ProjetoKart2324/CaixaBaterias/Part1.SLDPRT) |
| Part3.SLDPRT | 2023/24 | [Original](../02_ORIGINALES/entrega_2026-09-28/Relat%C3%B3rios%20de%20anos%20anteriores/ProjetoKart2324/ProjetoKart2324/CaixaBaterias/Part3.SLDPRT) |
| KART_LOG.CSV | Paquete 2025/26 | [Original](../02_ORIGINALES/entrega_2026-09-28/Smart%20Eletric%20Kart%20Cross-2223406-2223403/Smart%20Eletric%20Kart%20Cross-2223406-2223403/Dados%20Cart%C3%A3o%20SD/KART_LOG.CSV) |
| Eletronica Kart.pdsprj | Paquete 2025/26 | [Original](../02_ORIGINALES/entrega_2026-09-28/Smart%20Eletric%20Kart%20Cross-2223406-2223403/Smart%20Eletric%20Kart%20Cross-2223406-2223403/Eletronica%20Kart/Eletronica%20Kart.pdsprj) |
| ESP32-final.ino | Paquete 2025/26 | [Original](../02_ORIGINALES/entrega_2026-09-28/Smart%20Eletric%20Kart%20Cross-2223406-2223403/Smart%20Eletric%20Kart%20Cross-2223406-2223403/ESP32-final/ESP32-final/ESP32-final.ino) |
| main.c | Paquete 2025/26 | [Original](../02_ORIGINALES/entrega_2026-09-28/Smart%20Eletric%20Kart%20Cross-2223406-2223403/Smart%20Eletric%20Kart%20Cross-2223406-2223403/STM32-final/Core/Src/main.c) |
| STM32F746G_DISCO.ioc | Paquete 2025/26 | [Original](../02_ORIGINALES/entrega_2026-09-28/Smart%20Eletric%20Kart%20Cross-2223406-2223403/Smart%20Eletric%20Kart%20Cross-2223406-2223403/STM32-final/STM32F746G_DISCO.ioc) |

## DAT-0001 · Registro de tarjeta SD

KART_LOG.CSV contiene 801 filas de datos y 13 columnas, separadas por punto y coma. Encabezados: Tick, Accel, Steer, Brake, ModeD, ModeR, Vel, DiEsq, DiDir, TrEsq, TrDir, ModoDif, Yaw. Se comprobó la estructura de todas las filas; no se han interpretado unidades, modos, calibraciones ni resultados. Las unidades y el significado deben contrastarse con el firmware y el informe antes de graficar o calcular conclusiones.

## Uso eficiente

Empezar por el código propio y la configuración de cada proyecto. Los directorios de bibliotecas, controladores del fabricante y compilación solo se consultan cuando una pregunta concreta lo necesita. Las fechas indicadas arriba identifican el paquete de procedencia, no demuestran la versión cargada en el hardware.
