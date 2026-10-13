// Contrato visual: giro real 1:1, crecimiento, independencia entre ruedas, tiempo de animación y desenfoque.
// Solo datos sintéticos; no carga el servidor ni abre puertos del kart.
import fs from 'node:fs';
global.window={matchMedia:()=>({matches:false})};
const source=process.argv[2] || new URL('../static/nucleo.js',import.meta.url);
const api=await import('data:text/javascript;base64,'+Buffer.from(fs.readFileSync(source,'utf8')).toString('base64'));
const {wheelMotion,wheelRates,wheelBlur,app}=api;
const checks=[], rows=[];
const check=(name,ok)=>checks.push({name,pass:!!ok});
const signal=value=>({value,quality:'fresh'});
const fixture=(rpm,gear='D')=>({gear:signal(gear),...Object.fromEntries(['fl','fr','rl','rr'].map(side=>['rpm_'+side,signal(rpm)]))});
const rate=(s,dt=1/60)=>wheelMotion(s,dt).rl/dt;
const close=(a,b,tol=1e-9)=>Math.abs(a-b)<=tol*Math.max(1,Math.abs(a),Math.abs(b));
const REV=2*Math.PI, RPM_RAD=REV/60;
// Referencia del firmware: Ø 0,48 m. 50 km/h ≈ 552,6 RPM de rueda.
const RPM_50KMH=50/3.6/(Math.PI*0.48)*60;

for(const gear of ['D','R']){
  const rates=[30,45,60,80,100].map(pedal=>{
    const rpm=pedal*5.5, radPerSecond=rate(fixture(rpm,gear));
    rows.push({gear,pedal_pct:pedal,rpm,visual_rad_s:radPerSecond});
    return Math.abs(radPerSecond);
  });
  check(gear+': cada incremento de 30 a 100 % acelera la animación',rates.every((v,i)=>!i||v>rates[i-1]));
  check(gear+': duplicar RPM duplica velocidad visual',close(rate(fixture(100,gear))*2,rate(fixture(200,gear))));
}
// Giro real: ninguna escala artificial.
check('Giro real: 60 RPM = una vuelta por segundo (D)',close(rate(fixture(60)),-REV));
check('Giro real: 60 RPM = una vuelta por segundo (R)',close(rate(fixture(60,'R')),REV));
check('Giro real: 50 km/h (Ø 0,48 m) = RPM·2π/60 rad/s',close(-rate(fixture(RPM_50KMH)),RPM_50KMH*RPM_RAD,1e-9)&&close(RPM_50KMH,552.6,1e-3));
check('Giro real: 50 km/h supera 9 vueltas por segundo',Math.abs(rate(fixture(RPM_50KMH)))/REV>9);
check('wheelRates entrega rad/s con signo sin depender de dt',typeof wheelRates==='function'&&close(wheelRates(fixture(120)).rl,-120*RPM_RAD)&&close(wheelRates(fixture(120,'R')).rr,120*RPM_RAD));
check('wheelMotion = rad/s × dt',typeof wheelRates==='function'&&close(wheelMotion(fixture(300),.02).fl,wheelRates(fixture(300)).fl*.02));

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

// Desenfoque de movimiento: tacos y pernos se desvanecen; la marca deja una estela proporcional al giro.
const blurOK=typeof wheelBlur==='function';
check('wheelBlur disponible',blurOK);
if(blurOK){
  const blur=(rpm)=>wheelBlur(rpm*RPM_RAD);
  check('Parado y a 20 RPM los detalles se ven nítidos',blur(0).detail===1&&blur(0).arc===0&&blur(20).detail===1);
  check('A partir de 120 RPM (≈ 11 km/h) los detalles desaparecen',blur(120).detail===0&&blur(RPM_50KMH).detail===0&&blur(4000).detail===0);
  const details=[0,10,20,30,40,60,80,100,120].map(rpm=>blur(rpm).detail);
  check('La nitidez baja de forma monótona entre 0 y 120 RPM',details.every((v,i)=>!i||v<=details[i-1])&&details.some(v=>v>0&&v<1));
  check('Estela = giro en 1/30 s (a 50 km/h ≈ 110°)',close(blur(RPM_50KMH).arc,RPM_50KMH*RPM_RAD/30,1e-9)&&Math.abs(blur(RPM_50KMH).arc*180/Math.PI-110.4)<1);
  const arcs=[30,120,240,400,RPM_50KMH,800].map(rpm=>blur(rpm).arc);
  check('La estela crece con la velocidad (se distinguen los escalones)',arcs.every((v,i)=>!i||v>arcs[i-1]));
  check('La estela nunca supera una vuelta completa',blur(100000).arc<REV&&blur(100000).arc>REV*.9);
  check('El desenfoque es simétrico respecto al sentido',close(wheelBlur(-300).arc,wheelBlur(300).arc)&&wheelBlur(-300).detail===wheelBlur(300).detail);
  for(const bad of [NaN,Infinity,null,undefined])check('wheelBlur con '+String(bad)+' no produce NaN',Number.isFinite(wheelBlur(bad).arc)&&Number.isFinite(wheelBlur(bad).detail)&&wheelBlur(bad).arc===0);
}
// Modo lento (3 %): desactivable para comparar; con él la animación vuelve a ser nítida.
if(app&&wheelRates){
  app.wheelScale=.03;
  check('Modo lento: 3 % de la velocidad real',close(rate(fixture(400)),-400*RPM_RAD*.03));
  check('Modo lento: sin desenfoque a 400 RPM',blurOK&&wheelBlur(wheelRates(fixture(400)).rl).detail===1);
  app.wheelScale=1;
  check('Modo real restablecido',close(rate(fixture(400)),-400*RPM_RAD));
}else check('Modo lento seleccionable (app.wheelScale)',false);
const result={scope:'Prueba automática sintética del panel; sin hardware',source:String(source),status:checks.every(c=>c.pass)?'PASS':'FAIL',rows,checks};
if(process.argv[3])fs.writeFileSync(process.argv[3],JSON.stringify(result,null,2));
console.log(JSON.stringify(result,null,2));
if(result.status==='FAIL')process.exitCode=1;
