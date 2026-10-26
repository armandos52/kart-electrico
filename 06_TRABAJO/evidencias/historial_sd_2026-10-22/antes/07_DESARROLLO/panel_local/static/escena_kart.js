import * as T from './vendor/three.module.min.js';
import {OrbitControls} from './vendor/OrbitControls.js';
import {buildKart} from './modelo_kart.js';
import {$,app,activeSignals,steeringPresentation,reducedMotion,f,wheelMotion,wheelRates,wheelBlur,t} from './nucleo.js';
import {onIdioma} from './i18n.js';

export function createKartScene(onPick) {
  const host=$('escena-3d'), scene=new T.Scene();
  const renderer=new T.WebGLRenderer({antialias:true,alpha:true});
  renderer.setPixelRatio(Math.min(devicePixelRatio,1.75));renderer.shadowMap.enabled=true;
  renderer.shadowMap.type=T.PCFSoftShadowMap;renderer.toneMapping=T.ACESFilmicToneMapping;renderer.toneMappingExposure=1.35;
  const canvas=renderer.domElement;canvas.tabIndex=0;canvas.setAttribute('role','img');
  canvas.setAttribute('aria-label',t('scene.aria'));onIdioma(()=>canvas.setAttribute('aria-label',t('scene.aria')));
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
  // En planta, frontal arriba e izquierda del conductor a la izquierda de pantalla.
  const cameras={perspectiva:[3.5,2.5,-4.5],lateral:[5,.65,0],frontal:[0,1,-5],trasera:[0,1,5],superior:[0,5,.02]};
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
  canvas.addEventListener('webglcontextlost',e=>{e.preventDefault();lost=true;$('modelo-status').textContent=t('scene.perdida');});
  canvas.addEventListener('webglcontextrestored',()=>{lost=false;dirty=true;});
  function select(part){selected=part;dirty=true;statusClock=1;}
  const locationNotes=['esp32','battery','motor_left','motor_right','can','imu','power','stm32'];
  function update(dt){
    if($('modelo-3d').hidden||document.hidden||lost)return;
    accumulated+=dt;statusClock+=dt;if(accumulated<1/30)return;dt=Math.min(accumulated,.12);accumulated=0;
    const signals=activeSignals(),value=key=>signals[key]?.quality==='fresh'?signals[key].value:null;
    const steering=steeringPresentation(),angle=steering.mapAngle??steering.angle??0;
    const radians=T.MathUtils.degToRad(-angle);
    for(const id of ['front_left','front_right'])if(kart.wheels[id].hub.rotation.y!==radians){kart.wheels[id].hub.rotation.y=radians;dirty=true;}
    if(kart.steering.rotation.z!==radians*2){kart.steering.rotation.z=radians*2;dirty=true;}
    const motion=wheelMotion(signals,dt), rates=wheelRates(signals), moving=!reducedMotion.matches&&!app.motionPaused;
    for(const [id,side] of Object.entries({front_left:'fl',front_right:'fr',rear_left:'rl',rear_right:'rr'})){
      const wheel=kart.wheels[id], rate=moving?rates[side]:0, blur=wheelBlur(rate);
      if(motion[side]&&moving){wheel.spin.rotation.x=(wheel.spin.rotation.x+motion[side])%(Math.PI*2);dirty=true;}
      // Con la animación en pausa o movimiento reducido, la rueda se ve nítida y sin estela.
      if(wheel.setBlur(blur.detail,blur.arc,Math.sign(rate)))dirty=true;
    }
    if(statusClock>.2){
      statusClock=0;
      const rpmText=key=>value(key)===null?'—':f(value(key),0);
      $('modelo-rpm').textContent=t('m3d.rpm_valor',{i:rpmText('rpm_rl'),d:rpmText('rpm_rr')});
      $('modelo-status').textContent=(app.demo?'DEMO · ':'' )+(locationNotes.includes(selected)?t('loc.'+selected):t('scene.ruedas'));
      $('modelo-steering').textContent=steering.outside?t('m3d.dir_fuera'):angle===0&&steering.angle===null&&!steering.calibrated?t('m3d.dir_sin_vigente'):steering.calibrated?t('m3d.dir_recorrido',{pct:f(steering.percent)}):t(steering.estimated?'m3d.dir_estimados':'m3d.dir_reportados',{grados:f(steering.angle)});
      const target=kart.groups[selected];helper.visible=!!target?.children.length;
      if(helper.visible)helper.box.setFromObject(target);
    }
    if(dirty){renderer.render(scene,camera);dirty=false;}
  }
  cameraView('perspectiva');renderer.render(scene,camera);canvas.dataset.ready='true';
  return {select,resize,update};
}
