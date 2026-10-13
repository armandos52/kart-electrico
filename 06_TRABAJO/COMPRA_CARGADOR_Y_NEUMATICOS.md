# Compra de cargador y neumáticos

Fecha: 2026-10-01. Búsqueda preliminar; ninguna compra ni compatibilidad aprobada.

## Actualización del usuario y revisión focalizada

**13-10-2026:** 16 celdas y 3,2–3,3 V en reposo declarados. [Tutorial](../04_CONOCIMIENTO/bateria_y_bms/TUTORIAL_CELDAS_Y_CARGA_16S.md): ficha TSWB-LYP60AHA imprime 70 Ah, informe 60 Ah; identidad física pendiente. Techo prudente propuesto 3,65 V/celda = 58,4 V, condicionado a BMS/cargador y supervisión individual; el total no basta. Sin aprobación/conexión de cargador. [PROB-0010](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0010_identificacion_y_carga_bateria_16s.md).

**07-10-2026:** el usuario reconfirma 16 celdas en la [sesión de estudio](SESION_ESTUDIO_2026-10-07.md). La cantidad declarada deja de ser pregunta abierta; siguen pendientes conexión 16S activa, 16 lecturas y parámetros del BMS. Sin nueva revisión de candidatos, precios o disponibilidad en esta sesión.

El 01-10 el usuario confirma 16 celdas funcionales y el BMS de los archivos finales; compra en Portugal, preferencia Leiria. Se toma como declaración del montaje actual, no como verificación física. Queda descartada la rama de compra 15S para esta necesidad.

Revisión visual de DOC-0042, página PDF 70 (impresa 56), tabla 19: configuración histórica de **15 celdas**, carga 10 A, capacidad 60 Ah, tensión máxima del pack 57 V, mínima 42 V, máxima por celda 3,8 V, mínima 2,8 V, descarga 150 A y CAN Pylontech. El texto identifica DALY 8–17S pero menciona 17 celdas, inconsistente con la tabla. No aplicar automáticamente esa tabla al montaje actual de 16 celdas. Comprobar en SmartBMS que se configuraron las 16 celdas y sus protecciones; el límite de protección no es una consigna de carga.

Candidato actual: [Greenboatsolutions 16S LiFePO4, 58,4 V / 10 A / 600 W](https://greenboatsolutions.com/shop/charger/battery-charger/48V-10A-LiFePO4%20battery%20charger). Web muestra 159 € con IVA alemán 19 % y disponible; transporte, IVA final y plazo para Leiria pendientes. La página tiene errores de descripción (50 A y peso); pedir ficha exacta, perfil CC/CV y conector antes de comprar. PowerTech muestra 276 € y sin existencias, por lo que no resuelve compra inmediata.

Compra local de neumáticos: [First Stop Superpneus de Leiria](https://rede.firststop.pt/oficina/ps003472/first-stop-superpneus-de-leiria), con servicio de neumáticos de moto; [Botequim com Pneus, Av. do Lis 19 A, Leiria](https://www.pirelli.com/tyres/pt-pt/moto/loja-de-pneu/portugal/leiria/pt0006911640), distribuidor registrado por Pirelli. No hay existencias confirmadas de las medidas. [Anova Portugal 4.10/3.50-6 4PR con cámara](https://www.anovaloja.pt/producto/neumatico-4-10-3-50-6-4pr-con-camara-55-7572) coincide en medida pero está vendido para maquinaria: no aprobar para el kart sin carga/velocidad admitidas.

## Necesidad declarada por el usuario

Prioridad: cargador de 600 W, 60 V, 10 A. Sustituir neumáticos 130/60-13 y 4.10/3.50-6. Medidas declaradas, sin fotos recibidas en esta consulta; cantidades, destino de envío y presupuesto pendientes. Aclarar si necesita neumáticos o ruedas completas.

## Evidencia documental y problema pendiente

Consulta focalizada mediante buscar_documentos.py, texto extraído de [DOC-0042](../03_CATALOGO/fichas/DOC-0042.md), configuración 2025/26, sin nueva revisión visual: página PDF 69 describe 16 celdas Winston LiFePO4; página PDF 85 (impresa 71) describe 15 celdas activas tras avería de una y sustitución pendiente. No inferir la configuración actual. Página PDF 77 (impresa 63): cargador utilizado de 4 A y circuito declarado preparado para 10 A; esto no acredita el estado actual del cableado/BMS. Página PDF 76 (impresa 62): dos pines auxiliares unidos en la ficha accionan el bloqueo de marcha durante carga. Preservar esa función al adaptar el conector, tras verificar pinout y polaridad.

Antes de seleccionar: comprobar número de celdas activas, modelo de celda, perfil/tensión final admitidos, límite de carga del BMS, etiqueta del cargador anterior, conector y montaje actual. La expresión comercial «60 V» no determina compatibilidad. No conectar un cargador 20S de 73 V a la batería descrita de 15/16 celdas.

## Candidatos encontrados en web

Consultados el 01-10-2026; precio, existencias, envío y conformidad pendientes. Son candidatos, no recomendaciones de conexión.

- [OHRIJA 54,75 V / 10 A, anunciado para LiFePO4 15S](https://ohrija.com/product/ohrija-54-75v-10a-ebike-battery-charger-48v/): estudiar solo si se confirman 15 celdas y perfil admitido.
- [PowerTech 600 W / 10 A, LiFePO4 16S](https://www.mylithiumbattery.com/shop/chargers-for-lithium-ion-batteries/chargers-for-48v-lithium-ion-batteries/600w-48v-10a-charger-for-lithium-ion-48v-batteries/): ficha anuncia máximo 58,4 V; estudiar solo si se confirma configuración/perfil compatible.
- [Keysun 73 V / 10 A, LiFePO4 20S](https://keysunpower.com/products/73v10a): no corresponde a las 15/16 celdas descritas en DOC-0042.
- [Norauto Michelin Power Pure SC 130/60-13 60P TL](https://www.norauto.pt/p/pneu-scooter-michelin-power-pure-sc-130-60-13-60p-tl-dianteiro-traseiro-665046.html): resultado de búsqueda; apertura web no disponible. Confirmar llanta y montaje TL/TT, carga y velocidad.
- [Worten Impac IS307 4.10/3.50-6](https://www.worten.pt/produtos/pneu-4-10-3-50-6-is307-cinzentoimpac-mrkean-8053329965965): coincidencia de medida; uso, carga, velocidad y cámara pendientes. La medida sola no acredita aptitud para el kart.

Siguiente paso: recibir identificación física y destino de compra; seleccionar modelo y pedir precio total con transporte, plazo y conector compatible. No se ha contactado a vendedores.
