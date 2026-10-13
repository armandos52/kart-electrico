# Verificación de consulta y gráfico · 13-10-2026

Sin hardware conectado ni carga realizada. Datos de tensión física: declaración del usuario (16 celdas, 3,2–3,3 V en reposo), no adquisición instrumental.

- DOC-0030 PDF 1 y DOC-0042 PDF 69 recuperados con buscar_documentos.py, renderizados con Poppler y revisados visualmente. Originales intactos; hashes en SHA256.csv.
- Aritmética decimal: 16 × 2,8 = 44,8; ×3,0 = 48,0; ×3,2 = 51,2; ×3,3 = 52,8; ×3,45 = 55,2; ×3,55 = 56,8; ×3,65 = 58,4; ×3,8 = 60,8 V. Ejemplo sintético: 3,8 + 15 × 3,64 = 58,4 V.
- Gráfico abierto en Chrome como vista previa; D3 y curva visibles, ejes y etiquetas legibles en vista amplia y compacta. Selector cambia celda → pack y escala 3,65 → 58,4 V. No se ha calibrado voltaje/SOC ni se han ensayado celdas.
- La previsualización usa iframe aislado: lectura directa contentDocument devuelve null; inspección visual y árbol accesible disponibles. No es fallo de la gráfica.
- Regeneración de vista previa exigió `--force` por existir el archivo creado en esta sesión; resuelto sobre ese derivado y hashes recalculados. Originales no modificados.
- Prueba solicitada de ancho 360 px limitada por ventana Chrome a aproximadamente 500 px; no afirmar QA físico a 320/360 px. El fragmento calcula el ancho disponible y reduce etiquetas.

La gráfica usa puntos sintéticos para explicar la meseta; no publicar como curva medida del kart. Referencias y propuestas condicionadas en el tutorial.
