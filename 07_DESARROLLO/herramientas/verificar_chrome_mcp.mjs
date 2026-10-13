// Prueba real de la configuración guardada: MCP, Chrome, clic, consola y red.
import { spawn, execFileSync } from 'node:child_process';
import { createServer } from 'node:http';
import { createInterface } from 'node:readline';
import { writeFile } from 'node:fs/promises';
import { once } from 'node:events';

const config = JSON.parse(execFileSync(
  'C:/Users/nandi/AppData/Local/Programs/OpenAI/Codex/bin/codex.exe',
  ['mcp', 'get', 'chrome-devtools', '--json'], { encoding: 'utf8' }
));
const server = createServer((req, res) => {
  if (req.url === '/dato') {
    res.setHeader('Content-Type', 'application/json');
    res.end('{"estado":"OK_MCP"}');
  } else {
    res.setHeader('Content-Type', 'text/html; charset=utf-8');
    res.end(`<!doctype html><html lang="es"><meta charset="utf-8">
      <title>Prueba Chrome MCP</title><h1>Prueba de conexión</h1>
      <button id="probar">Probar conexión</button><p id="resultado">Pendiente</p>
      <script>document.querySelector('#probar').onclick = async () => {
        const dato = await (await fetch('/dato')).json();
        document.querySelector('#resultado').textContent = dato.estado;
        console.log('CONSOLA_MCP_OK');
      };</script></html>`);
  }
});
server.listen(0, '127.0.0.1');
await once(server, 'listening');
const url = `http://127.0.0.1:${server.address().port}/`;
const child = spawn(config.transport.command, [...config.transport.args, '--headless'], {
  stdio: ['pipe', 'pipe', 'pipe'], windowsHide: true
});
let stderr = '';
child.stderr.on('data', chunk => { stderr += chunk; });
const pending = new Map();
let sequence = 0;
const lines = createInterface({ input: child.stdout });
lines.on('line', line => {
  let response;
  try { response = JSON.parse(line); } catch { return; }
  const waiter = pending.get(response.id);
  if (!waiter) return;
  clearTimeout(waiter.timer);
  pending.delete(response.id);
  if (response.error) waiter.reject(new Error(JSON.stringify(response.error)));
  else waiter.resolve(response.result);
});
child.on('error', error => {
  for (const waiter of pending.values()) { clearTimeout(waiter.timer); waiter.reject(error); }
  pending.clear();
});
function request(method, params) {
  return new Promise((resolve, reject) => {
    const id = ++sequence;
    const timer = setTimeout(() => {
      pending.delete(id);
      reject(new Error(`Tiempo agotado: ${method}. ${stderr.slice(-2000)}`));
    }, 55000);
    pending.set(id, { resolve, reject, timer });
    child.stdin.write(JSON.stringify({ jsonrpc: '2.0', id, method, params }) + '\n');
  });
}
const textContent = result => (result.content ?? []).filter(x => x.type === 'text').map(x => x.text).join('\n');
async function call(name, args = {}) {
  const result = await request('tools/call', { name, arguments: args });
  if (result.isError) throw new Error(`${name}: ${textContent(result)}`);
  return result;
}
let pageId;
try {
  const initialized = await request('initialize', {
    protocolVersion: '2024-11-05', capabilities: {},
    clientInfo: { name: 'kart-verificacion-mcp', version: '1.0.0' }
  });
  child.stdin.write(JSON.stringify({ jsonrpc: '2.0', method: 'notifications/initialized' }) + '\n');
  const { tools } = await request('tools/list', {});
  console.log(`Servidor: ${initialized.serverInfo.name} ${initialized.serverInfo.version}; herramientas: ${tools.length}`);
  const opened = await call('new_page', { url });
  const pages = textContent(await call('list_pages'));
  const pageLine = pages.split('\n').find(line => line.includes(url));
  const idMatch = pageLine?.match(/^\s*(\d+):/);
  if (!idMatch) throw new Error(`No se identifica la página: ${pages}\n${textContent(opened)}`);
  pageId = Number(idMatch[1]);
  const pageArgs = tools.find(x => x.name === 'take_snapshot').inputSchema.properties.pageId ? { pageId } : {};
  const snapshot = textContent(await call('take_snapshot', pageArgs));
  const uid = snapshot.match(/uid=([^\s]+).*button "Probar conexión"/)?.[1];
  if (!uid) throw new Error(`No se identifica el botón: ${snapshot}`);
  await call('click', { ...pageArgs, uid });
  await call('wait_for', { ...pageArgs, text: ['OK_MCP'], timeout: 10000 });
  const evaluation = textContent(await call('evaluate_script', {
    ...pageArgs, function: '() => ({titulo: document.title, resultado: document.querySelector("#resultado").textContent})'
  }));
  if (!evaluation.includes('OK_MCP')) throw new Error(`Clic fallido: ${evaluation}`);
  const consoleMessages = textContent(await call('list_console_messages', pageArgs));
  if (!consoleMessages.includes('CONSOLA_MCP_OK')) throw new Error(`No se lee la consola: ${consoleMessages}`);
  const network = textContent(await call('list_network_requests', pageArgs));
  if (!network.includes('/dato') || !network.includes('200')) throw new Error(`No se verifica la red: ${network}`);
  const report = {
    fechaUTC: new Date().toISOString(), servidor: initialized.serverInfo,
    chrome: '154.0.8037.98', modoPrueba: 'headless; configuración habitual visible',
    herramientas: tools.length,
    resultados: { conexionMCP: true, navegador: true, clic: true, javascript: true, consola: true, redHTTP200: true },
    paginaPrueba: 'Página sintética local; no se ha ensayado todavía una aplicación del kart',
    evidencia: { evaluation, consoleMessages, network }
  };
  await writeFile(new URL('./chrome_mcp_verificacion.json', import.meta.url), JSON.stringify(report, null, 2) + '\n');
  console.log('OK: conexión MCP, Chrome, clic, JavaScript, consola y petición HTTP 200.');
} catch (error) {
  console.error(error.message);
  process.exitCode = 1;
} finally {
  if (pageId !== undefined) {
    try { await call('close_page', { pageId }); } catch { /* El cierre de transporte finaliza la sesión. */ }
  }
  child.stdin.end();
  server.close();
  const killTimer = setTimeout(() => child.kill(), 3000);
  killTimer.unref();
  if (child.exitCode === null) await once(child, 'exit');
  clearTimeout(killTimer);
  lines.close();
}
