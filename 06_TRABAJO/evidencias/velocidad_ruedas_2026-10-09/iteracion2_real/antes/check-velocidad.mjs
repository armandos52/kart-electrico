// Contrato visual: crecimiento, independencia entre ruedas y tiempo de animación.
// Solo datos sintéticos; no carga el servidor ni abre puertos del kart.
import fs from 'node:fs';
global.window={matchMedia:()=>({matches:false})};
const source=process.argv[2] || new URL('../static/nucleo.js',import.meta.url);
const {wheelMotion}=await import('data:text/javascript;base64,'+Buffer.from(fs.readFileSync(source,'utf8')).toString('base64'));
const checks=[], rows=[];
const check=(name,ok)=>checks.push({name,pass:!!ok});
const signal=value=>({value,quality:'fresh'});
const fixture=(rpm,gear='D')=>({gear:signal(gear),...Object.fromEntries(['fl','fr','rl','rr'].map(side=>['rpm_'+side,signal(rpm)]))});
const rate=(s,dt=1/60)=>wheelMotion(s,dt).rl/dt;
const close=(a,b)=>Math.abs(a-b)<1e-9;
for(const gear of ['D','R']){
  const rates=[30,45,60,80,100].map(pedal=>{
    const rpm=pedal*4, radPerSecond=rate(fixture(rpm,gear));
    rows.push({gear,pedal_pct:pedal,rpm,visual_rad_s:radPerSecond});
    return Math.abs(radPerSecond);
  });
  check(gear+': cada incremento de 30 a 100 % acelera la animación',rates.every((v,i)=>!i||v>rates[i-1]));
  check(gear+': duplicar RPM duplica velocidad visual',close(rates[2],rates[0]*2));
}
for(const rpm of [0,1,120,400,1200,4000])check('D/R opuestos a '+rpm+' RPM',close(rate(fixture(rpm)), -rate(fixture(rpm,'R'))));
check('Cero RPM: cero movimiento',rate(fixture(0))===0);
const isolated=fixture(240), independent=rate(isolated);isolated.rpm_rr=signal(1600);
check('Acelerar rueda D no ralentiza rueda I',close(rate(isolated),independent));
check('Reparto I/D conserva proporción 240/1600',close(wheelMotion(isolated,1/60).rl/wheelMotion(isolated,1/60).rr,240/1600));
const elapsed=fixture(400), reference=rate(elapsed);
for(const fps of [10,20,30,60,120])check('Mismo giro por segundo a '+fps+' fps',close(rate(elapsed,1/fps),reference));
for(const bad of [null,undefined,NaN,Infinity,'400']){
  const s=fixture(240);s.rpm_rl=signal(bad);
  check('RPM inválida '+String(bad)+': sin movimiento ni NaN',wheelMotion(s,1/60).rl===0&&Number.isFinite(wheelMotion(s,1/60).rr));
}
for(const quality of ['stale','missing']){
  const s=fixture(240);s.rpm_rl.quality=quality;
  check('RPM '+quality+': rueda quieta',wheelMotion(s,1/60).rl===0);
}
const applied=fixture(240);applied.drive_direction=signal('R');
check('La marcha aplicada R tiene prioridad sobre solicitada D',rate(applied)>0);
check('Paso temporal negativo no mueve ruedas',rate(fixture(240),-1)===0);
const result={scope:'Prueba automática sintética del panel; sin hardware',source:String(source),status:checks.every(c=>c.pass)?'PASS':'FAIL',rows,checks};
if(process.argv[3])fs.writeFileSync(process.argv[3],JSON.stringify(result,null,2));
console.log(JSON.stringify(result,null,2));
if(result.status==='FAIL')process.exitCode=1;
