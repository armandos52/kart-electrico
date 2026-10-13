const {chromium}=require('C:/Users/nandi/.cache/codex-runtimes/codex-primary-runtime/dependencies/node/node_modules/playwright');
const path=require('node:path');
(async()=>{
 const browser=await chromium.launch({headless:true,executablePath:'C:/Program Files/Google/Chrome/Application/chrome.exe'});
 try{
  const page=await browser.newPage({viewport:{width:1440,height:1000}});const errors=[];
  page.on('pageerror',e=>errors.push(e.message));page.on('console',m=>{if(m.type()==='error')errors.push(m.text());});
  await page.goto('http://127.0.0.1:8766/');
  await page.waitForFunction(()=>document.querySelector('#escena-3d canvas')?.dataset.ready||document.querySelector('#ver-3d').disabled);
  await page.waitForTimeout(1000);
  await page.screenshot({path:path.join(process.argv[2],'modelo-primer-render.png'),fullPage:true});
  await page.locator('[data-camera="lateral"]').click();await page.waitForTimeout(150);
  await page.locator('.lamina').screenshot({path:path.join(process.argv[2],'modelo-lateral.png')});
  console.log(JSON.stringify({errors,status:await page.locator('#modelo-status').textContent(),size:await page.locator('.lamina').boundingBox()}));
 }finally{await browser.close();}
})().catch(e=>{console.error(e);process.exit(1);});
