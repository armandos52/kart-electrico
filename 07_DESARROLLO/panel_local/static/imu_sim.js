// Simulación de IMU y dinámica en planta para la demostración: datos SINTÉTICOS, nunca mezclados con lecturas reales.
// Modelo de bicicleta con neumáticos saturables y tracción trasera diferencial. Masa, inercia, rigideces y fuerza por
// rueda son supuestos didácticos; geometría y ESC copiados de esp32_trasero/lib/kart_control (v1.3, sin validar en pista).
// Guía: 04_CONOCIMIENTO/software_y_telemetria/GUIA_PID_E_IMU.md
const G=9.81, D2R=Math.PI/180, R2D=180/Math.PI;
const clamp=(x,a,b)=>Math.min(b,Math.max(a,x));
export const GEOMETRIA={L:1.3,a:0.65,b:0.65,track:1.0,diam:0.48,rpmMax:570};
const MASA={m:150,Iz:35,Cf:9000,Cr:11000};
const DT_CONTROL=0.010, H=0.001; // GYRO_PERIOD_MS = 10 ms; física a 1 ms

function gauss(){let u=0,v=0;while(!u)u=Math.random();while(!v)v=Math.random();return Math.sqrt(-2*Math.log(u))*Math.cos(2*Math.PI*v);}
function tire(C,alpha,Fmax){return Fmax>1?Fmax*Math.tanh(C*alpha/Fmax):0;}

export function createImuSim(){
  const cfg={mode:'fw',mu:0.9,fmax:600,cmax:0.10,kp:0.006,ki:0.02,kd:0,aw:true,cap:false,ramp:true,
    gyroNoise:true,steerNoise:false,weak:false,bias:0.8,range:250};
  const vmax=GEOMETRIA.rpmMax*Math.PI*GEOMETRIA.diam/60;
  const Fzf=MASA.m*G*GEOMETRIA.b/GEOMETRIA.L, Fzr=MASA.m*G*GEOMETRIA.a/GEOMETRIA.L;
  let st,pid,acc=0;
  const trail=[];
  function reset(){
    st={X:0,Y:0,psi:0,vx:0,vy:0,r:0,ax:0,ay:0,steer:0,effL:0,effR:0,FL:0,FR:0,Fyf:0,Fyr:0,
      offset:cfg.bias,gyro:0,ideal:0,err:0,corr:0,head:0,sat:false,t:0};
    pid={i:0,prevMeas:0,prevErr:0,dF:0,P:0,I:0,D:0};
    trail.length=0;acc=0;
  }
  function physics(h,brake){
    const {L,a,b,track}=GEOMETRIA, d=st.steer*D2R, vxs=Math.max(st.vx,1), low=clamp(st.vx,0,1);
    const af=d-Math.atan2(st.vy+a*st.r,vxs), ar=-Math.atan2(st.vy-b*st.r,vxs);
    const vL=st.vx+st.r*track/2, vR=st.vx-st.r*track/2;
    let FL=cfg.fmax*st.effL*clamp(1-vL/vmax,0,1)*(cfg.weak?0.8:1), FR=cfg.fmax*st.effR*clamp(1-vR/vmax,0,1);
    const FxMax=cfg.mu*Fzr;
    if(FL+FR>FxMax){const s=FxMax/(FL+FR);FL*=s;FR*=s;}
    const FyrMax=Math.sqrt(Math.max(0,FxMax*FxMax-(FL+FR)**2)); // círculo de adherencia trasero
    const Fyf=tire(MASA.Cf,af,cfg.mu*Fzf)*low, Fyr=tire(MASA.Cr,ar,FyrMax)*low;
    let Fres=st.vx>0.02?0.015*MASA.m*G+0.5*1.2*0.5*st.vx*st.vx:0;
    if(brake&&st.vx>0.02)Fres+=0.8*cfg.mu*MASA.m*G;
    const Fx=FL+FR-Fres-Fyf*Math.sin(d), Fy=Fyf*Math.cos(d)+Fyr;
    const Mz=a*Fyf*Math.cos(d)-b*Fyr+(FL-FR)*track/2;
    st.vx+=h*(Fx/MASA.m+st.vy*st.r);st.vy+=h*(Fy/MASA.m-st.vx*st.r);st.r+=h*(Mz/MASA.Iz);
    if(st.vx<0)st.vx=0;
    if(st.vx<0.3){st.vy*=0.98;st.r*=0.98;}
    st.psi+=h*st.r;
    st.X+=h*(st.vx*Math.cos(st.psi)-st.vy*Math.sin(st.psi));st.Y+=h*(st.vx*Math.sin(st.psi)+st.vy*Math.cos(st.psi));
    // La IMU mide fuerza específica (fuerza total / masa).
    st.ax=Fx/MASA.m;st.ay=Fy/MASA.m;st.FL=FL;st.FR=FR;st.Fyf=Fyf;st.Fyr=Fyr;
  }
  function control(dt,inp){
    st.steer=inp.steer;
    const steerMeas=inp.steer+(cfg.steerNoise?gauss()*1.2:0);
    let g=st.r*R2D+cfg.bias-st.offset+(cfg.gyroNoise?gauss()*0.4:0);
    const lsb=32768/cfg.range;g=Math.round(g*lsb)/lsb;           // cuantización del rango elegido
    st.sat=Math.abs(g)>=cfg.range;g=clamp(g,-cfg.range,cfg.range);
    st.gyro=g;st.head+=g*dt;
    const base=inp.brake?0:inp.throttle, ang=steerMeas*D2R, {L,track}=GEOMETRIA;
    let left=base,right=base;
    if(cfg.mode!=='equal'){const split=(track*0.5/L)*Math.tan(ang);left=base*(1+split);right=base*(1-split);}
    let ideal=st.vx*Math.tan(ang)/L*R2D;
    if(cfg.cap&&st.vx>0.5){const lim=0.85*cfg.mu*G/st.vx*R2D;ideal=clamp(ideal,-lim,lim);}
    const err=ideal-g;let corr=0;pid.P=pid.I=pid.D=0;
    if(cfg.mode==='fw'){
      // Igual que kart_control.cpp: zona muerta 2 °/s, ganancia 1/200 por °/s, límite y escala por esfuerzo base.
      const u=Math.abs(err)>2?clamp(err/200,-cfg.cmax,cfg.cmax):0;pid.P=u;corr=u*base;pid.i=0;
    }else if(cfg.mode==='pid'){
      if(base<=0.001){pid.i=0;pid.dF=0;}
      else{
        const P=cfg.kp*err;pid.i+=err*dt;
        pid.dF+=(-(g-pid.prevMeas)/dt-pid.dF)*0.15;               // derivada sobre la medida, filtrada
        const D=cfg.kd*pid.dF;let u=P+cfg.ki*pid.i+D;
        if(u>cfg.cmax){u=cfg.cmax;if(cfg.aw)pid.i-=err*dt;}
        else if(u<-cfg.cmax){u=-cfg.cmax;if(cfg.aw)pid.i-=err*dt;}
        pid.P=P;pid.I=cfg.ki*pid.i;pid.D=D;corr=u*base;
      }
    }
    pid.prevErr=err;pid.prevMeas=g;
    left=Math.max(0,left+corr);right=Math.max(0,right-corr);
    const mx=Math.max(1,left,right);left/=mx;right/=mx;
    if(cfg.ramp){const dl=(300/GEOMETRIA.rpmMax)*dt;st.effL=Math.min(left,st.effL+dl);st.effR=Math.min(right,st.effR+dl);}
    else{st.effL=left;st.effR=right;}
    st.ideal=ideal;st.err=err;st.corr=corr;
  }
  // inp: {throttle 0..1, steer ° (+ derecha), brake}
  function step(dt,inp){
    acc+=Math.min(dt,0.1);
    while(acc>=DT_CONTROL){
      control(DT_CONTROL,inp);
      for(let i=0;i<DT_CONTROL/H;i++)physics(H,inp.brake);
      st.t+=DT_CONTROL;acc-=DT_CONTROL;
      if(Math.round(st.t*100)%5===0){trail.push([st.X,st.Y]);if(trail.length>500)trail.shift();}
    }
  }
  const rpm=v=>Math.max(0,v)*60/(Math.PI*GEOMETRIA.diam);
  // Señales con los mismos nombres que la telemetría real cuando existen; las nuevas llevan prefijo de simulación.
  function signals(){
    const half=GEOMETRIA.track/2;
    return {yaw_dps:st.gyro,sim_yaw_ideal_dps:st.ideal,sim_lat_g:st.ay/G,sim_long_g:st.ax/G,
      sim_esc_corr_pct:st.corr*100,sim_speed_kph:st.vx*3.6,mpu_ok:1,
      rpm_rl:rpm(st.vx+st.r*half),rpm_rr:rpm(st.vx-st.r*half),rpm_fl:rpm(st.vx),rpm_fr:rpm(st.vx),
      pwm_left_raw:Math.round(255-st.effL*255),pwm_right_raw:Math.round(255-st.effR*255)};
  }
  function calibrate(){if(st.vx>0.05)return false;st.offset=cfg.bias;st.head=st.psi*R2D;return true;}
  reset();
  return {cfg,reset,step,signals,calibrate,kick:()=>{st.r+=40*D2R;},get st(){return st;},get pid(){return pid;},trail};
}

// ---------- Dibujo ----------
function tokens(){const s=getComputedStyle(document.documentElement),v=n=>s.getPropertyValue(n).trim();
  return {fondo:v('--panel-2')||'#f1f5f5',rejilla:v('--rejilla')||'#c5c8cb',texto:v('--texto-2')||'#454b51',suave:v('--texto-3')||'#646a70',borde:v('--borde')||'#d6dfe0'};}
const COL={traccion:'#2e7d32',lateral:'#d97706',imu:'#c62828',vel:'#1c5ea6',giro:'#7b1fa2',real:'#c62828',integ:'#7b1fa2',estela:'rgba(28,94,166,.35)'};
function fit(c){const r=c.getBoundingClientRect(),p=devicePixelRatio||1;
  if(!r.width||!r.height)return null;
  if(c.width!==Math.round(r.width*p)||c.height!==Math.round(r.height*p)){c.width=Math.round(r.width*p);c.height=Math.round(r.height*p);}
  const x=c.getContext('2d');x.setTransform(p,0,0,p,0,0);return [x,r.width,r.height];}
function arrow(x,x0,y0,dx,dy,col,w=2.5){
  const len=Math.hypot(dx,dy);if(len<2)return;
  x.strokeStyle=x.fillStyle=col;x.lineWidth=w;x.beginPath();x.moveTo(x0,y0);x.lineTo(x0+dx,y0+dy);x.stroke();
  const a=Math.atan2(dy,dx),s=Math.min(9,len*.4);
  x.beginPath();x.moveTo(x0+dx,y0+dy);x.lineTo(x0+dx-s*Math.cos(a-.45),y0+dy-s*Math.sin(a-.45));x.lineTo(x0+dx-s*Math.cos(a+.45),y0+dy-s*Math.sin(a+.45));x.closePath();x.fill();
}
export function drawTrack(canvas,sim){
  const k=fit(canvas);if(!k)return;const [x,w,h]=k,st=sim.st,T=tokens(),S=46,{a,b,track}=GEOMETRIA;
  x.fillStyle=T.fondo;x.fillRect(0,0,w,h);x.save();x.translate(w/2,h/2);
  x.strokeStyle=T.rejilla;x.lineWidth=1;const g=2*S,ox=-(st.X*S)%g,oy=-(st.Y*S)%g;
  for(let i=-Math.ceil(w/g)-1;i<=Math.ceil(w/g)+1;i++){x.beginPath();x.moveTo(ox+i*g,-h);x.lineTo(ox+i*g,h);x.stroke();}
  for(let i=-Math.ceil(h/g)-1;i<=Math.ceil(h/g)+1;i++){x.beginPath();x.moveTo(-w,oy+i*g);x.lineTo(w,oy+i*g);x.stroke();}
  x.strokeStyle=COL.estela;x.lineWidth=2;x.beginPath();
  sim.trail.forEach((p,i)=>{const px=(p[0]-st.X)*S,py=(p[1]-st.Y)*S;i?x.lineTo(px,py):x.moveTo(px,py);});x.stroke();
  x.rotate(st.psi);
  const half=track/2*S,fa=a*S,rb=-b*S,d=st.steer*D2R,fs=.12;
  x.fillStyle='#b3261e';x.strokeStyle='#7f1d17';x.lineWidth=1.5;x.beginPath();x.roundRect(rb-12,-half+6,fa-rb+30,2*half-12,8);x.fill();x.stroke();
  x.fillStyle='#f2c230';x.beginPath();x.arc(0,0,9,0,2*Math.PI);x.fill();
  const wheel=(px,py,ang)=>{x.save();x.translate(px,py);x.rotate(ang);x.fillStyle='#1b1f23';x.fillRect(-9,-5,18,10);x.restore();};
  wheel(fa,-half,d);wheel(fa,half,d);wheel(rb,-half,0);wheel(rb,half,0);
  arrow(x,rb,-half,st.FL*fs,0,COL.traccion);arrow(x,rb,half,st.FR*fs,0,COL.traccion);
  arrow(x,fa,0,-Math.sin(d)*st.Fyf*fs,Math.cos(d)*st.Fyf*fs,COL.lateral);arrow(x,rb,0,0,st.Fyr*fs,COL.lateral);
  arrow(x,0,0,st.ax*9,st.ay*9,COL.imu,3);arrow(x,0,0,st.vx*6,st.vy*6,COL.vel,2);
  const rr=st.r*R2D;
  if(Math.abs(rr)>1){const ang=clamp(rr/120,-1.6,1.6),e=-Math.PI/2+ang;x.strokeStyle=COL.giro;x.lineWidth=3;x.beginPath();x.arc(0,0,26,-Math.PI/2,e,ang<0);x.stroke();
    arrow(x,26*Math.cos(e),26*Math.sin(e),(ang>0?-1:1)*Math.sin(e)*6,(ang>0?1:-1)*Math.cos(e)*6,COL.giro,3);}
  x.restore();
}
export function drawGG(canvas,sim,titulo){
  const k=fit(canvas);if(!k)return;const [x,w,h]=k,st=sim.st,T=tokens(),cx=w/2,cy=h/2+6,R=Math.min(w,h)/2-18;
  x.clearRect(0,0,w,h);x.fillStyle=T.suave;x.font='11px system-ui';x.fillText(titulo,8,14);
  x.strokeStyle=T.rejilla;x.lineWidth=1;for(const q of [.5,1]){x.beginPath();x.arc(cx,cy,R*q,0,2*Math.PI);x.stroke();}
  x.beginPath();x.moveTo(cx-R-4,cy);x.lineTo(cx+R+4,cy);x.moveTo(cx,cy-R-4);x.lineTo(cx,cy+R+4);x.stroke();
  x.strokeStyle=COL.imu;x.setLineDash([4,3]);x.beginPath();x.arc(cx,cy,R*sim.cfg.mu,0,2*Math.PI);x.stroke();x.setLineDash([]);
  x.fillStyle=COL.imu;x.beginPath();x.arc(cx+clamp(st.ay/G,-1.3,1.3)*R,cy-clamp(st.ax/G,-1.3,1.3)*R,5,0,2*Math.PI);x.fill();
}
export function drawCompass(canvas,sim,titulo){
  const k=fit(canvas);if(!k)return;const [x,w,h]=k,st=sim.st,T=tokens(),cx=w/2,cy=h/2+6,R=Math.min(w,h)/2-18;
  x.clearRect(0,0,w,h);x.fillStyle=T.suave;x.font='11px system-ui';x.fillText(titulo,8,14);
  x.strokeStyle=T.rejilla;x.beginPath();x.arc(cx,cy,R,0,2*Math.PI);x.stroke();
  const needle=(ang,col,lw)=>{x.strokeStyle=col;x.lineWidth=lw;x.beginPath();x.moveTo(cx,cy);x.lineTo(cx+Math.cos(ang)*R*.9,cy+Math.sin(ang)*R*.9);x.stroke();};
  needle(st.psi,COL.real,4);needle(st.head*D2R,COL.integ,2);
}
export const COLORES=COL;
