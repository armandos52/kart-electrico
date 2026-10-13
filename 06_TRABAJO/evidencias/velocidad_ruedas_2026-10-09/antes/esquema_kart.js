// Planta dibujada a partir de DOC-0044, foto superior 15.42.32.
// Proporciones visuales, no proyección métrica. Componentes ocultos: tarjetas sin posición asignada.
import {$} from './nucleo.js';

export const steeringPivots={left:[261,196],right:[579,196]};
const part=(id,label,content)=>`<g class="part" data-part="${id}" role="button" tabindex="0" aria-label="${label}">${content}</g>`;
const card=(x,y,title,valueId,note,width=174)=>`<g class="plan-card"><rect class="plan-card-bg" x="${x}" y="${y}" width="${width}" height="64" rx="7"/><text class="plan-card-title" x="${x+12}" y="${y+19}">${title}</text><text class="plan-card-value" id="${valueId}" x="${x+12}" y="${y+38}">—</text><text class="plan-card-note" x="${x+12}" y="${y+54}">${note}</text></g>`;
const leader=d=>`<path class="plan-leader" d="${d}"/>`;

function wheel(side,x,y,w,h,front){
  const id={fl:'front_left',fr:'front_right',rl:'rear_left',rr:'rear_right'}[side];
  const label=`Rueda ${front?'delantera':'trasera'} ${side.endsWith('l')?'izquierda':'derecha'}`;
  const tread=Array.from({length:11},(_,i)=>{
    const yy=y-22+i*18;
    return front?`<path d="M${x+5} ${yy}l12 6m3-6 12 6m-28 2 12 6m3-6 12 6"/>`:`<path d="M${x+3} ${yy}l${w/2-3} 12 ${w/2-3}-12"/>`;
  }).join('');
  const tire=`<rect class="plan-tire" x="${x}" y="${y}" width="${w}" height="${h}" rx="${front?14:17}"/>
    <rect class="plan-sidewall" x="${x+3}" y="${y+8}" width="${w-6}" height="${h-16}" rx="11"/>
    <g clip-path="url(#plan-clip-${side})"><g id="rim-${side}" class="plan-tread ${front?'offroad':'road'}">${tread}</g></g>
    <path class="plan-rim-edge" d="M${side.endsWith('l')?x+w-3:x+3} ${y+18}v${h-36}"/>
    <rect class="selection-ring" x="${x-4}" y="${y-4}" width="${w+8}" height="${h+8}" rx="17"/>`;
  return `<defs><clipPath id="plan-clip-${side}"><rect x="${x+5}" y="${y+6}" width="${w-10}" height="${h-12}" rx="11"/></clipPath></defs>`+part(id,label,
    (front?`<g id="steer-${side==='fl'?'left':'right'}">${tire}</g>`:tire)+
    `<text class="plan-wheel-label" x="${x+w/2}" y="${y+h+18}" text-anchor="middle">${front?'DEL.':'TRAS.'} ${side.endsWith('l')?'I':'D'}</text><text class="plan-rpm" id="rpm-${side}" x="${x+w/2}" y="${y+h+35}" text-anchor="middle">— rpm</text>`);
}

function suspension(x,y,sign){
  return `<g class="plan-suspension"><path d="M${x} ${y}l${sign*51} 24 ${-sign*5} 32 ${-sign*46} -18 M${x+sign*46} ${y+56}l${sign*7} -36" class="plan-tube-thin"/><rect x="${x+sign*20-5}" y="${y-1}" width="10" height="48" rx="4" fill="url(#plan-metal)"/>${Array.from({length:7},(_,i)=>`<path d="M${x+sign*20-8} ${y+5+i*5}h16" class="plan-spring"/>`).join('')}</g>`;
}

export function initPlan(){
  const svg=$('kart');svg.classList.add('foto-superior');svg.setAttribute('viewBox','0 0 840 640');
  svg.innerHTML=`<title id="kart-title">Esquema superior del kart basado en la foto real</title>
    <desc id="kart-desc">Frontal hacia arriba. Bastidor rojo, volante, asiento amarillo, cubierta trasera en T y ruedas de distinto perfil. Selecciona una pieza para ver sus señales. Ubicación de componentes ocultos pendiente, proporciones aproximadas.</desc>
    <defs>
      <linearGradient id="plan-red" x1="0" y1="0" x2="1" y2="0"><stop stop-color="#671226"/><stop offset=".43" stop-color="#c23e4f"/><stop offset=".65" stop-color="#a22338"/><stop offset="1" stop-color="#6f1628"/></linearGradient>
      <linearGradient id="plan-metal"><stop stop-color="#9ba6aa"/><stop offset=".42" stop-color="#e0e5e5"/><stop offset="1" stop-color="#859299"/></linearGradient>
      <linearGradient id="plan-yellow" x1="0" y1="0" x2="1" y2=".2"><stop stop-color="#c89b0b"/><stop offset=".4" stop-color="#f5d543"/><stop offset=".75" stop-color="#eac22b"/><stop offset="1" stop-color="#ad840c"/></linearGradient>
      <linearGradient id="plan-cover" x1="0" y1="0" x2="1" y2="1"><stop stop-color="#41484c"/><stop offset=".4" stop-color="#20282c"/><stop offset=".7" stop-color="#323b40"/><stop offset="1" stop-color="#1d2529"/></linearGradient>
      <pattern id="plan-grid" width="24" height="24" patternUnits="userSpaceOnUse"><path d="M24 0H0V24" fill="none" stroke="#b9c5ca" stroke-opacity=".18" stroke-width=".7"/></pattern>
    </defs>
    <rect width="840" height="640" fill="url(#plan-grid)"/>
    <g class="plan-heading"><text x="24" y="30" class="plan-eyebrow">REFERENCIA FOTOGRÁFICA / 04</text><text x="24" y="53" class="plan-heading-title">El kart desde arriba.</text><text x="24" y="71" class="plan-card-note">Bastidor, asiento y cubierta del montaje real</text></g>
    <path d="M420 23v-12m-5 5 5-5 5 5" class="plan-orientation"/><text x="420" y="40" class="plan-wheel-label" text-anchor="middle">FRONTAL</text>
    <text id="steering-map" x="420" y="64" class="plan-steering" text-anchor="middle">DIRECCIÓN SIN DATOS</text>
    <g aria-hidden="true">
      <ellipse cx="420" cy="347" rx="151" ry="245" fill="#a7b3b9" opacity=".13"/>
      <path d="M366 96Q420 84 474 96L491 360 505 544H335L349 360Z" fill="url(#plan-metal)" stroke="#9ea9af"/>
      <path class="plan-tube" d="M356 99Q420 76 484 99L504 483Q507 531 485 553H355Q333 531 336 483Z"/>
      <path class="plan-tube-thin" d="M356 145L326 198Q318 215 349 239 M484 145L514 198Q522 215 491 239 M348 280Q311 290 310 327L325 416 M492 280Q529 290 530 327L515 416 M349 365L307 416 M491 365L533 416 M350 285H490"/>
      <path class="plan-tube-thin" d="M336 471L294 497H328 M504 471L546 497H512 M309 470V522 M531 470V522"/>
      <path class="plan-tube" d="M332 540L329 563Q329 578 355 578H485Q511 578 511 563L508 540 M334 549H506"/>
      <path class="plan-linkage" d="M278 195H562 M278 187L350 179 M562 187L490 179"/>
      ${suspension(290,158,1)}${suspension(550,158,-1)}
      <path d="M407 184V263" class="plan-column"/>
      <path d="M370 263h16v-28h-12z M454 263h16l-4-28h-12z" fill="#3a4247"/>
      <path d="M373 284Q420 273 467 284L474 332Q473 351 457 362Q420 375 383 362Q367 351 366 332Z" fill="url(#plan-yellow)" stroke="#b99218" stroke-width="2"/>
      <path d="M381 292Q420 284 459 292L462 325Q456 343 420 344Q384 343 378 325Z" fill="#f4d342" stroke="#d4ad22"/>
      <path d="M373 347Q420 366 467 347L467 361Q420 387 373 361Z" fill="#23292c"/>
      <path d="M350 114L357 342 M490 114L483 342 M357 340Q420 331 483 340" class="plan-cage-shadow"/>
      <path d="M354 106L359 337 M486 106L481 337 M359 337Q420 327 481 337" class="plan-cage"/>
      <path d="M354 106H486 M360 360H480" class="plan-tube"/>
      <g transform="translate(420 226)"><ellipse rx="41" ry="35" class="plan-steering-rim"/><path d="M-37-2-7 5-3 30H3L7 5 37-2 M0 0v-7" stroke="#555e64" stroke-width="9" fill="none"/><circle r="7" fill="#a7b0b5"/><circle r="2" fill="#333b3f"/></g>
      ${[-1,1].map(s=>`<g transform="translate(${420+s*84} 393)"><rect x="-8" y="-34" width="16" height="73" rx="6" fill="url(#plan-metal)"/>${Array.from({length:10},(_,i)=>`<path d="M-11 ${-24+i*5.5}h22" class="plan-rear-spring"/>`).join('')}</g>`).join('')}
      <path d="M356 387H484V442H542V540H447V551H393V540H298V442H356Z" fill="url(#plan-cover)" stroke="#566169" stroke-width="2"/>
      <path d="M363 395H477 M308 450H352 M488 450H532" stroke="#7a8489" opacity=".55"/>
      ${[[367,397],[420,397],[473,397],[308,452],[532,452],[308,529],[532,529],[364,478],[476,478]].map(([x,y])=>`<circle cx="${x}" cy="${y}" r="2.4" fill="#9aa5aa"/><circle cx="${x}" cy="${y}" r=".8" fill="#3a4247"/>`).join('')}
      <text x="420" y="476" class="plan-cover-label" text-anchor="middle">CUBIERTA TRASERA</text><text x="420" y="493" class="plan-cover-note" text-anchor="middle">Componentes ocultos</text>
    </g>
    ${wheel('fl',237,151,48,90,true)}${wheel('fr',555,151,48,90,true)}
    ${wheel('rl',249,430,39,109,false)}${wheel('rr',552,430,39,109,false)}
    ${part('stm32','Pantalla frontal y STM32',`<rect x="377" y="129" width="86" height="57" rx="5" fill="#242d32"/><rect x="391" y="140" width="58" height="31" rx="2" fill="#7b8e96"/><rect x="395" y="144" width="50" height="23" rx="1" fill="#465d66"/><rect class="selection-ring" x="372" y="124" width="96" height="67" rx="6"/>${leader('M377 147H335V126H196')}${card(22,94,'PANTALLA / STM32','val-stm32','Unidad frontal · asociación')}`)}
    ${part('power','Caja lateral visible; relés y contactor por confirmar',`<rect x="291" y="375" width="40" height="57" rx="3" fill="#d0b875"/><rect x="300" y="375" width="31" height="42" fill="#d6dde0"/><rect x="273" y="377" width="30" height="30" rx="2" fill="#d4bc79"/><rect x="278" y="382" width="20" height="19" rx="3" fill="#343d41"/><circle cx="288" cy="391" r="7" fill="#ad263d" stroke="#e76672" stroke-width="2"/><rect class="selection-ring" x="269" y="371" width="67" height="66" rx="5"/>${leader('M272 391H218V356H196')}${card(22,324,'CAJA LATERAL / RELÉS','val-power','Contenido por confirmar')}`)}
    ${part('esp32','Caja trasera y ESP32; asociación por confirmar',`<rect x="391" y="532" width="58" height="53" rx="3" fill="#c1dae2" fill-opacity=".6" stroke="#889fa9"/><rect x="400" y="539" width="41" height="26" fill="#4d8564"/><rect x="406" y="544" width="13" height="10" fill="#40514b"/><path d="M424 544h9m-9 6h9m-27 9h27" stroke="#d3b778" stroke-width="3"/><rect x="398" y="569" width="44" height="10" fill="#d8e3e7" fill-opacity=".8"/><circle cx="407" cy="574" r="3" fill="#51656f"/><circle cx="433" cy="574" r="2" fill="#51656f"/><rect class="selection-ring" x="386" y="527" width="68" height="63" rx="5"/>${leader('M449 558H622V555H644')}${card(644,528,'CAJA TRASERA / ESP32','val-esp32','Asociación por confirmar')}`)}
    <g class="plan-heading"><text x="656" y="102" class="plan-eyebrow">SEÑALES DEL SISTEMA</text><text x="656" y="119" class="plan-card-note">Sin posición física asignada</text></g>
    ${part('can','Bus CAN, sin ubicación física asignada',card(644,132,'BUS CAN','val-can','Comunicación lógica'))}
    ${part('imu','Giroscopio, ubicación pendiente',card(644,207,'GIROSCOPIO','val-imu','Ubicación pendiente'))}
    ${part('battery','Batería y BMS, ubicación pendiente',card(644,324,'BATERÍA / BMS','battery-value','Ubicación pendiente'))}
    ${part('motor_left','Controlador Kelly izquierdo, ubicación pendiente',card(22,445,'KELLY IZQUIERDO','val-kelly-l','Orden PWM · ubicación pendiente'))}
    ${part('motor_right','Controlador Kelly derecho, ubicación pendiente',card(644,445,'KELLY DERECHO','val-kelly-r','Orden PWM · ubicación pendiente'))}
    <text id="travel-map" x="420" y="613" class="plan-steering" text-anchor="middle">MARCHA SIN DATOS</text>
    <text x="420" y="632" class="plan-footnote" text-anchor="middle">Proporciones aproximadas · giro y rodadura ilustrativos</text>`;
  // Mantener el kart grande en superficies estrechas; las señales siguen en el inspector.
  function resize(){const compact=svg.parentElement.clientWidth<680;svg.classList.toggle('plan-compact',compact);svg.setAttribute('viewBox',compact?'208 40 424 600':'0 0 840 640');}
  new ResizeObserver(resize).observe(svg.parentElement);resize();
}
