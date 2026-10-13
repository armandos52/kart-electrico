import * as T from './vendor/three.module.min.js';
import {OrbitControls} from './vendor/OrbitControls.js';
import {buildKart} from './modelo_kart.js';
import {$,app,activeSignals,steeringPresentation,reducedMotion,f} from './nucleo.js';

export function createKartScene(onPick) {
  const host=$('escena-3d'), scene=new T.Scene();
  const renderer=new T.WebGLRenderer({antialias:true,alpha:true});
  renderer.setPixelRatio(Math.min(devicePixelRatio,1.75));renderer.shadowMap.enabled=true;
  renderer.shadowMap.type=T.PCFSoftShadowMap;renderer.toneMapping=T.ACESFilmicToneMapping;renderer.toneMappingExposure=1.35;
  const canvas=renderer.domElement;canvas.tabIndex=0;canvas.setAttribute('role','img');
  canvas.setAttribute('aria-label','Kart 3D. Arrastra para girar; usa las cámaras y el selector de componentes con teclado.');
  host.appendChild(canvas);
  const camera=new T.PerspectiveCamera(39,1,.1,60);
  const controls=new OrbitControls(camera,canvas);controls.target.set(0,.99,-.06);
  controls.enableDamping=false;controls.enablePan=false;controls.minDistance=3;controls.maxDistance=10;
  controls.minPolarAngle=.025;controls.maxPolarAngle=Math.PI*.49;
  const ambient=new T.HemisphereLight('#f4f7ff','#8f8b83',3);scene.add(ambient);
  const key=new T.DirectionalLight('#fff8ec',4);key.position.set(3,6,-4);key.castShadow=true;
  key.shadow.mapSize.set(2048,2048);Object.assign(key.shadow.camera,{left:-3,right:3,top:3,bottom:-3,near:.5,far:15});key.shadow.bias=-.00035;key.shadow.normalBias=.025;scene.add(key);
  const rim=new T.DirectionalLight('#d4e9ff',2.5);rim.position.set(-3,3,4);scene.add(rim);
  const ground=new T.Mesh(new T.PlaneGeometry(200,200),new T.ShadowMaterial({opacity:.15}));ground.rotation.x=-Math.PI/2;ground.position.y=-.012;ground.receiveShadow=true;scene.add(ground);
  const grid=new T.GridHelper(8,32,'#c1c9cc','#dae0e2');grid.position.y=-.02;grid.material.transparent=true;grid.material.opacity=.42;scene.add(grid);
  const kart=buildKart();scene.add(kart.root);
  const helper=new T.Box3Helper(new T.Box3(),new T.Color('#367baf'));helper.visible=false;scene.add(helper);
  const cameras={perspectiva:[3.5,2.5,-4.5],lateral:[5,.65,0],frontal:[0,1,-5],trasera:[0,1,5],superior:[0,5,-.02]};
  let preset='perspectiva',selected='esp32',dirty=true,accumulated=0,statusClock=1,lost=false;
  function cameraView(name){
    preset=name;const direction=new T.Vector3(...cameras[name]).normalize();
    const narrow=Math.max(1,.95/Math.max(.1,camera.aspect));
    camera.position.copy(controls.target).add(direction.multiplyScalar(4.95*narrow));controls.update();dirty=true;
    document.querySelectorAll('[data-camera]').forEach(b=>b.setAttribute('aria-pressed',String(b.dataset.camera===name)));
  }
  controls.addEventListener('change',()=>{dirty=true;});
  controls.addEventListener('start',()=>{preset=null;document.querySelectorAll('[data-camera]').forEach(b=>b.setAttribute('aria-pressed','false'));});
  for(const b of document.querySelectorAll('[data-camera]'))b.addEventListener('click',()=>cameraView(b.dataset.camera));
  function resize(){
    const {width,height}=host.getBoundingClientRect();if(!width||!height)return;
    renderer.setSize(width,height,false);camera.aspect=width/height;camera.updateProjectionMatrix();
    if(preset)cameraView(preset);dirty=true;
  }
  new ResizeObserver(resize).observe(host);resize();
  const ray=new T.Raycaster(),pointer=new T.Vector2();let down=null;
  canvas.addEventListener('pointerdown',e=>{down={x:e.clientX,y:e.clientY};});
  canvas.addEventListener('pointerup',e=>{
    if(!down||Math.hypot(e.clientX-down.x,e.clientY-down.y)>5)return;down=null;
    const b=canvas.getBoundingClientRect();pointer.set((e.clientX-b.left)/b.width*2-1,-(e.clientY-b.top)/b.height*2+1);ray.setFromCamera(pointer,camera);
    // Respetar la primera superficie visible; no seleccionar una pieza atravesando el bastidor.
    const hit=ray.intersectObjects(kart.pickables,false).find(h=>h.object.visible);if(!hit)return;
    let object=hit.object;while(object&&!object.userData.part)object=object.parent;
    if(object?.userData.part)onPick(object.userData.part);
  });
  canvas.addEventListener('pointercancel',()=>{down=null;});
  canvas.addEventListener('webglcontextlost',e=>{e.preventDefault();lost=true;$('modelo-status').textContent='Vista 3D interrumpida. Usa Esquema o recarga la página.';});
  canvas.addEventListener('webglcontextrestored',()=>{lost=false;dirty=true;});
  function select(part){selected=part;dirty=true;statusClock=1;}
  const locationNotes={esp32:'Unidad trasera · asociación visual por confirmar',battery:'Batería / BMS · volumen y ubicación orientativos',motor_left:'Kelly izquierdo · ubicación orientativa bajo cubierta',motor_right:'Kelly derecho · ubicación orientativa bajo cubierta',can:'CAN · sin ubicación física asignada',imu:'Giroscopio · sin ubicación física asignada',power:'Caja lateral visible · contenido por confirmar',stm32:'Pantalla frontal · unidad STM32 de referencia'};
  function update(dt){
    if($('modelo-3d').hidden||document.hidden||lost)return;
    accumulated+=dt;statusClock+=dt;if(accumulated<1/30)return;dt=Math.min(accumulated,.12);accumulated=0;
    const signals=activeSignals(),value=key=>signals[key]?.quality==='fresh'?signals[key].value:null;
    const steering=steeringPresentation(),angle=steering.mapAngle??steering.angle??0;
    const radians=T.MathUtils.degToRad(-angle);
    for(const id of ['front_left','front_right'])if(kart.wheels[id].hub.rotation.y!==radians){kart.wheels[id].hub.rotation.y=radians;dirty=true;}
    if(kart.steering.rotation.z!==radians*2){kart.steering.rotation.z=radians*2;dirty=true;}
    const direction=value('drive_direction')||value('gear');
    for(const [id,key] of Object.entries({front_left:'rpm_fl',front_right:'rpm_fr',rear_left:'rpm_rl',rear_right:'rpm_rr'})){
      const rpm=value(key);if(rpm!==null&&rpm!==0&&!reducedMotion.matches&&!app.motionPaused){
        // Ralentizado y limitado para legibilidad; sentido ilustrativo según marcha ordenada.
        kart.wheels[id].spin.rotation.x=(kart.wheels[id].spin.rotation.x+(direction==='R'?-1:1)*Math.min(Math.abs(rpm)*.018,3)*dt)%(Math.PI*2);dirty=true;
      }
    }
    if(statusClock>.2){
      statusClock=0;
      const rpmText=key=>value(key)===null?'—':f(value(key),0);
      $('modelo-rpm').textContent=`I ${rpmText('rpm_rl')} · D ${rpmText('rpm_rr')}`;
      $('modelo-status').textContent=(app.demo?'DEMO · ':'' )+(locationNotes[selected]||'Ruedas · rodadura ralentizada según RPM vigentes');
      $('modelo-steering').textContent=steering.outside?'Dirección fuera de rango · pose neutra de referencia':angle===0&&steering.angle===null&&!steering.calibrated?'Sin dirección vigente · pose neutra de referencia':steering.calibrated?`${f(steering.percent)} % de recorrido · giro esquemático`:`${f(steering.angle)}° ${steering.estimated?'estimados del mando':'reportados'} · giro esquemático`;
      const target=kart.groups[selected];helper.visible=!!target?.children.length;
      if(helper.visible)helper.box.setFromObject(target);
    }
    if(dirty){renderer.render(scene,camera);dirty=false;}
  }
  cameraView('perspectiva');renderer.render(scene,camera);canvas.dataset.ready='true';
  return {select,resize,update};
}
