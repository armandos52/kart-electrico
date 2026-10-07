// Captura de la interfaz existente. No abre puertos ni conecta o modifica hardware.
const {chromium}=require('C:/Users/nandi/.cache/codex-runtimes/codex-primary-runtime/dependencies/node/node_modules/playwright');
const fs=require('node:fs'),path=require('node:path');
const url=process.argv[2] || 'http://127.0.0.1:8766/';
const destination=process.argv[3];
if(!destination)throw new Error('Indica carpeta de evidencias');
(async()=>{
  const browser=await chromium.launch({headless:true,executablePath:'C:/Program Files/Google/Chrome/Application/chrome.exe'});
  try{
    const page=await browser.newPage({viewport:{width:1280,height:960}});
    await page.goto(url);
    await page.waitForFunction(()=>document.querySelector('#firmware-info').textContent.includes('Firmware:'));
    await page.screenshot({path:path.join(destination,'panel_real_can_ausente.png'),fullPage:true});
    const state=await (await page.request.get(url+'api/state')).json();
    const summary={status:state.status,port:state.port,stats:state.stats,nodes:state.nodes,
      diagnostic:await page.locator('#diagnostic').innerText(),
      control_state:state.signals.control_state.value,
      rpm_rl_quality:state.signals.rpm_rl.quality,
      pwm_left_quality:state.signals.pwm_left_raw.quality};
    if(summary.control_state!=='can_wait' || summary.rpm_rl_quality!=='missing' || summary.pwm_left_quality!=='missing')throw new Error('No corresponde al ensayo con CAN ausente');
    fs.writeFileSync(path.join(destination,'panel_real_resumen.json'),JSON.stringify(summary,null,2));
    console.log(JSON.stringify(summary,null,2));
  }finally{await browser.close();}
})().catch(error=>{console.error(error);process.exitCode=1;});
