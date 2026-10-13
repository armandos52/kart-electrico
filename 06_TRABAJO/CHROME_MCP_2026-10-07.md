---
estado: configurado; pendiente de recarga del cliente
actualizado: 2026-10-07
resumen: Chrome DevTools MCP configurado para depuración y pruebas web desde Codex.
---

# Chrome DevTools MCP · 07-10-2026

Armando solicita configurar el MCP de Chrome para depurar y realizar pruebas.

## Configuración comprobada

- Ajuste general añadido a `C:\Users\nandi\.codex\config.toml`, servidor `chrome-devtools`; `codex mcp get chrome-devtools --json` confirma que está habilitado.
- Copia del archivo anterior: `C:\Users\nandi\.codex\config.toml.antes-chrome-20261007-154211.bak`. Es una copia de ese ajuste, no una copia de seguridad del proyecto.
- Node.js `24.15.0`, npm `11.12.1`, Chrome `154.0.8037.98`; paquete probado `chrome_devtools 1.10.1`, con 30 herramientas anunciadas. La configuración usa `@latest`, por lo que puede actualizarse en futuros arranques.
- Inicio mediante Node y `npx-cli.js` con rutas absolutas de Windows; Chrome localizado explícitamente, perfil temporal `--isolated`, estadísticas del MCP y consultas CrUX desactivadas. Arranque: 60 s; herramientas: 120 s.
- Chrome se abre cuando una herramienta lo necesita. El uso habitual será con ventana visible; la comprobación se realizó en modo `headless`.

## Evidencia y alcance

La [verificación guardada](../07_DESARROLLO/herramientas/chrome_mcp_verificacion.json) acredita conexión MCP, apertura de una página sintética local, clic, evaluación JavaScript, lectura del mensaje de consola y petición `/dato` con respuesta HTTP 200. No acredita pruebas del panel del kart ni conexión al hardware.

Scripts: [configuración](../07_DESARROLLO/herramientas/configurar_chrome_mcp.ps1) y [verificación](../07_DESARROLLO/herramientas/verificar_chrome_mcp.mjs). El primero se detiene si ya existe la entrada, para evitar duplicarla. El segundo prueba los argumentos guardados, añadiendo `--headless`, y cierra su página y servidor local.

## Siguiente paso

1. Cerrar y volver a abrir Codex para que cargue el nuevo servidor y su catálogo de herramientas. En este chat no se ha comprobado todavía la incorporación del MCP al catálogo del cliente.
2. Indicar la URL o archivo de la interfaz que se desea probar y sus casos de prueba.
3. Usar las herramientas de Chrome DevTools para probar la interfaz y revisar consola/red; documentar resultados. Las pruebas web no sustituyen los ensayos del kart.

Fuentes de configuración: [MCP de Codex](https://developers.openai.com/codex/mcp), [Chrome DevTools MCP](https://github.com/ChromeDevTools/chrome-devtools-mcp) y su [guía de clientes](https://github.com/ChromeDevTools/chrome-devtools-mcp/blob/main/docs/client-configurations.md). La ejecución local confirma los datos anteriores.
