// Gráficas en canvas: minigráficas de la tira y gráficas apiladas con cursor de tiempo común.
import {f, t} from './nucleo.js';

const DASH = {medida:[], orden:[5,3], estimacion:[1.5,3]};
let palette=null;
function colors() {
  if(palette)return palette;
  const css=getComputedStyle(document.documentElement), v=name=>css.getPropertyValue(name).trim();
  return palette={series:[v('--serie-1'),v('--serie-2')],grid:v('--rejilla'),axis:v('--texto-3'),spark:v('--texto-2'),cursor:v('--cursor')};
}
function context(canvas) {
  const w=canvas.clientWidth, h=canvas.clientHeight, dpr=window.devicePixelRatio||1;
  if(!w||!h)return null;
  if(canvas.width!==Math.round(w*dpr)||canvas.height!==Math.round(h*dpr)){canvas.width=Math.round(w*dpr);canvas.height=Math.round(h*dpr);}
  const ctx=canvas.getContext('2d');ctx.setTransform(dpr,0,0,dpr,0,0);ctx.clearRect(0,0,w,h);
  return {ctx,w,h};
}
function path(ctx, points, x, y) {
  ctx.beginPath();let pen=false;
  for(const p of points){if(p.v===null){pen=false;continue;}if(!pen){ctx.moveTo(x(p.t),y(p.v));pen=true;}else ctx.lineTo(x(p.t),y(p.v));}
  ctx.stroke();
}
function range(values) {
  let min=Math.min(...values), max=Math.max(...values);
  const pad=max-min>1e-9?(max-min)*.12:Math.max(Math.abs(max)*.1,1);
  return [min-pad,max+pad];
}

export function sparkline(canvas, points, kind='medida', windowMs=30000) {
  canvas.hidden=false;
  const c=context(canvas);if(!c)return;
  const {ctx,w,h}=c, now=Date.now(), visible=points.filter(p=>p.t>=now-windowMs), values=visible.filter(p=>p.v!==null).map(p=>p.v);
  if(!values.length)return;
  const [min,max]=range(values), x=t=>(t-(now-windowMs))/windowMs*w, y=v=>2+(max-v)/(max-min)*(h-4);
  ctx.strokeStyle=colors().spark;ctx.lineWidth=1.2;ctx.setLineDash(DASH[kind]||[]);path(ctx,visible,x,y);
}

// charts: [{title, keys, canvas, legend}]; tools: {history, nature, shortName, signal, windowMs}
let charts=[], tools=null, cursorT=null;
export function buildCharts(container, specs, helpers) {
  tools=helpers;container.replaceChildren();charts=[];
  for(const spec of specs) {
    const box=document.createElement('div');box.className='grafica'+(spec.free?' libre':'');
    box.innerHTML='<div class="g-cab"><span class="g-titulo"></span><span class="g-leyenda"></span></div><div class="lienzo"><canvas role="img"></canvas></div>';
    box.querySelector('.g-titulo').textContent=spec.title;
    const canvas=box.querySelector('canvas');canvas.setAttribute('aria-label',t('graf.aria',{titulo:spec.title}));
    canvas.addEventListener('pointermove',event=>{const r=canvas.getBoundingClientRect(), left=46, pw=r.width-left-8, frac=(event.clientX-r.left-left)/pw;cursorT=frac<0||frac>1?null:Date.now()-tools.windowMs*(1-frac);drawCharts();});
    canvas.addEventListener('pointerleave',()=>{cursorT=null;drawCharts();});
    container.appendChild(box);
    charts.push({...spec,canvas,legend:box.querySelector('.g-leyenda')});
  }
}
function valueAt(points, t) {
  if(t===null){const last=points.at(-1);return last?last.v:null;}
  let best=null, distance=Infinity;
  for(const p of points){const d=Math.abs(p.t-t);if(d<distance){distance=d;best=p;}}
  return best&&distance<1500?best.v:null;
}
export function drawCharts(now=Date.now()) {
  const pal=colors(), windowMs=tools?.windowMs||60000;
  for(const chart of charts) {
    const signals=chart.keys.map((key,i)=>{const s=tools.signal(key);return {key,s,kind:tools.nature(key,s),color:pal.series[i%2],points:tools.history(key).filter(p=>p.t>=now-windowMs)};});
    chart.legend.replaceChildren();
    for(const item of signals) {
      const value=valueAt(item.points,cursorT), span=document.createElement('span');
      span.className='g-serie '+item.kind+(item.color===pal.series[1]?' s2':'');
      span.textContent=`${tools.shortName(item.key,item.s)} ${value===null?'—':f(value)}${item.s?.unit&&value!==null?' '+item.s.unit:''}`;
      chart.legend.appendChild(span);
    }
    const c=context(chart.canvas);if(!c)continue;
    const {ctx,w,h}=c, left=46, right=8, top=6, bottom=16, pw=Math.max(1,w-left-right), ph=Math.max(1,h-top-bottom);
    const x=t=>left+(t-(now-windowMs))/windowMs*pw;
    ctx.font='11px Bahnschrift, "Segoe UI", sans-serif';ctx.lineWidth=1;
    const values=signals.flatMap(item=>item.points.filter(p=>p.v!==null).map(p=>p.v));
    ctx.strokeStyle=pal.grid;ctx.strokeRect(left+.5,top+.5,pw-1,ph-1);
    ctx.fillStyle=pal.axis;
    ['−60 s','−30 s',t('graf.ahora')].forEach((label,i)=>{ctx.textAlign=i===0?'left':i===1?'center':'right';ctx.fillText(label,left+i/2*pw,h-3);});
    if(!values.length){ctx.textAlign='center';ctx.fillText(t('graf.sin_muestras'),left+pw/2,top+ph/2+4);continue;}
    const [min,max]=range(values), y=v=>top+(max-v)/(max-min)*ph, digits=max-min<10?1:0;
    ctx.textAlign='right';
    for(let i=0;i<=2;i++){const v=max-i/2*(max-min), yy=top+i/2*ph;ctx.strokeStyle=pal.grid;ctx.setLineDash([]);ctx.beginPath();ctx.moveTo(left,yy+.5);ctx.lineTo(left+pw,yy+.5);ctx.stroke();ctx.fillText(f(v,digits),left-5,yy+(i===0?9:i===2?-2:4));}
    ctx.save();ctx.beginPath();ctx.rect(left,top,pw,ph);ctx.clip();
    for(const item of signals){ctx.strokeStyle=item.color;ctx.lineWidth=1.6;ctx.setLineDash(DASH[item.kind]||[]);path(ctx,item.points,x,y);}
    ctx.restore();
    if(cursorT!==null){ctx.strokeStyle=pal.cursor;ctx.setLineDash([]);ctx.beginPath();ctx.moveTo(Math.round(x(cursorT))+.5,top);ctx.lineTo(Math.round(x(cursorT))+.5,top+ph);ctx.stroke();}
  }
}
