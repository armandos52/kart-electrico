// Reconstrucción visual de las tres fotos del 08-10-2026, no CAD ni geometría calibrada.
// Coordenadas sin unidad física: Y arriba, frente -Z, izquierda del conductor -X.
import * as T from './vendor/three.module.min.js';

// Estela de la marca de rodadura: la marca deja detrás un arco que se desvanece (desenfoque de
// movimiento). El ángulo se mide en el marco de la rueda (Y arriba, marca en +Y) y la estela
// queda siempre detrás del sentido de giro. uArc = giro de la marca durante la exposición.
const trailVertex=`varying vec2 vYZ;
void main(){vYZ=position.yz;gl_Position=projectionMatrix*modelViewMatrix*vec4(position,1.0);}`;
const trailFragment=`uniform float uArc;uniform float uDir;uniform float uPeak;uniform vec3 uColor;varying vec2 vYZ;
void main(){
  if(uArc<=0.0)discard;
  float behind=mod(-uDir*atan(vYZ.y,vYZ.x),6.28318530718);
  float a=1.0-behind/uArc;
  if(a<=0.0)discard;
  gl_FragColor=vec4(uColor,pow(a,1.4)*uPeak);
}`;

export function buildKart() {
  const root = new T.Group(), wheels = {}, pickables = [], groups = {}, covers = [];
  const mat = (color, metalness=.1, roughness=.55) => new T.MeshStandardMaterial({color,metalness,roughness});
  const red=mat('#8e172b',.58,.3), yellow=mat('#e7bd13',.18,.4), black=mat('#171c21',.15,.55);
  const rubber=mat('#25282a',0,.9), tread=mat('#303336',0,.86), silver=mat('#acb3b7',.78,.36);
  const rotationMarker=mat('#e4f4ec',0,.65);
  const steel=mat('#45494d',.8,.37), floor=mat('#949ea3',.68,.57), wood=mat('#be9c63',0,.88);
  const glass=new T.MeshPhysicalMaterial({color:'#c1dfeb',metalness:0,roughness:.18,transparent:true,opacity:.24,depthWrite:false,side:T.DoubleSide});
  const pcb=mat('#276951',.12,.7), screen=mat('#334955',.28,.28);
  function group(id, parent=root) {const g=new T.Group();parent.add(g);if(id){g.userData.part=id;groups[id]=g;}return g;}
  function mesh(geo, material, position, parent=root) {
    const m=new T.Mesh(geo,material);if(position)m.position.set(...position);parent.add(m);m.castShadow=true;m.receiveShadow=true;return m;
  }
  function box(size,position,material,parent=root){return mesh(new T.BoxGeometry(...size),material,position,parent);}
  function rod(a,b,r=.024,material=red,parent=root){
    const start=new T.Vector3(...a),end=new T.Vector3(...b),delta=end.clone().sub(start);
    const m=mesh(new T.CylinderGeometry(r,r,delta.length(),12),material,start.add(end).multiplyScalar(.5).toArray(),parent);
    m.quaternion.setFromUnitVectors(new T.Vector3(0,1,0),delta.normalize());return m;
  }
  function tube(points,r=.026,material=red,parent=root,closed=false){
    const curve=new T.CatmullRomCurve3(points.map(p=>new T.Vector3(...p)),closed,'centripetal');
    return mesh(new T.TubeGeometry(curve,Math.max(28,points.length*10),r,10,closed),material,null,parent);
  }
  function cyl(r,length,position,material,parent=root){const m=mesh(new T.CylinderGeometry(r,r,length,40),material,position,parent);m.rotation.z=Math.PI/2;return m;}
  function torus(r,thickness,position,material,parent=root){const m=mesh(new T.TorusGeometry(r,thickness,10,48),material,position,parent);m.rotation.y=Math.PI/2;return m;}
  function bolt(position,parent=root){return cyl(.012,.009,position,silver,parent);}

  // Largueros, travesaños, suelo de chapa y defensas.
  for(const s of [-1,1]) {
    tube([[s*.22,.31,-1.65],[s*.34,.3,-1.52],[s*.39,.3,-.65],[s*.4,.3,.35],[s*.5,.36,1.36]],.033);
    tube([[s*.26,.38,-1.55],[s*.35,.66,-.76],[s*.37,.69,-.45],[s*.36,.53,-.06],[s*.4,.52,.46]],.026);
    tube([[s*.4,.32,.2],[s*.63,.36,.28],[s*.63,.38,.72],[s*.46,.4,.84]],.027);
    rod([s*.4,.31,.24],[s*.33,.83,.6],.026);
  }
  for(const z of [-1.42,-.72,.06,.6,1.3])rod([-.37,.31,z],[.37,.31,z],.028);
  tube([[-.28,.33,-1.57],[-.2,.32,-1.7],[.2,.32,-1.7],[.28,.33,-1.57]],.034);
  tube([[-.53,.45,1.34],[-.53,.78,1.42],[-.42,.82,1.5],[.42,.82,1.5],[.53,.78,1.42],[.53,.45,1.34]],.033);
  rod([-.52,.48,1.43],[.52,.48,1.43],.03);
  box([.65,.016,1.52],[0,.32,-.66],floor);
  for(const x of [-.28,.28])for(const z of [-1.25,-.7,-.15])mesh(new T.SphereGeometry(.009,8,6),silver,[x,.333,z]);

  // Arco alto posterior y tubos longitudinales inclinados observados en la foto lateral.
  tube([[-.4,.36,.51],[-.34,1.15,.67],[-.26,2.05,.81],[-.18,2.16,.81],[.18,2.16,.81],[.26,2.05,.81],[.34,1.15,.67],[.4,.36,.51]],.034);
  for(const s of [-1,1]) {
    tube([[s*.27,.35,-1.54],[s*.3,.84,-.91],[s*.28,1.69,-.34],[s*.25,1.91,-.04],[s*.22,2.1,.8]],.031);
    rod([s*.37,.37,-.85],[s*.33,.95,-.62],.024);
    rod([s*.35,.72,.6],[s*.53,.41,1.3],.028);
    // Dos bandas claras que identifican el tubo sin simular una alarma.
    rod([s*.297,1.07,-.765],[s*.296,1.12,-.73],.033,yellow);
  }
  rod([-.32,1.34,.705],[.32,1.34,.705],.024);
  rod([-.28,1.86,.785],[.28,1.86,.785],.027);
  rod([-.34,.84,-.67],[.34,.84,-.67],.028);
  const chain=[];for(let i=0;i<=20;i++){const x=-.26+i*.026;chain.push([x,1.64+.28*(x/.26)**2,.81]);}tube(chain,.008,steel);

  // Asiento de cubeta amarillo: perfil longitudinal y curvatura transversal.
  const seat=group();
  const profile=[[-.38,.44,.25],[-.3,.42,.28],[-.1,.4,.29],[.12,.44,.27],[.26,.56,.25],[.35,.79,.25],[.43,1.04,.28],[.46,1.13,.26]];
  const vertices=[],indices=[],n=16;
  for(const [z,y,w] of profile)for(let j=0;j<=n;j++){const u=j/n*2-1;vertices.push(u*w,y+.09*Math.pow(Math.abs(u),4),z);}
  for(let i=0;i<profile.length-1;i++)for(let j=0;j<n;j++){const a=i*(n+1)+j,b=a+n+1;indices.push(a,b,a+1,b,b+1,a+1);}
  const geo=new T.BufferGeometry();geo.setAttribute('position',new T.Float32BufferAttribute(vertices,3));geo.setIndex(indices);geo.computeVertexNormals();
  const shell=yellow.clone();shell.side=T.DoubleSide;mesh(geo,shell,null,seat);
  const backing=black.clone();backing.side=T.BackSide;const back=mesh(geo,backing,[0,-.008,.009],seat);
  for(const s of [-1,1])tube(profile.map(([z,y,w])=>[s*w,y+.09,z]),.009,yellow,seat);
  tube([[-.26,1.22,.46],[0,1.14,.46],[.26,1.22,.46]],.008,yellow,seat);
  for(const s of [-1,1]){rod([s*.22,.32,-.1],[s*.23,.41,-.1],.018,steel);rod([s*.21,.33,.29],[s*.22,.6,.3],.018,steel);}

  // Volante, columna, salpicadero y pedales; pantalla frontal seleccionable.
  rod([0,.39,-1.12],[0,.94,-.36],.022,steel);
  const steering=group();steering.position.set(0,.99,-.39);steering.rotation.x=-.68;
  const ring=mesh(new T.TorusGeometry(.21,.018,12,48),black,[0,0,0],steering);
  for(let i=0;i<3;i++){const a=i*Math.PI*2/3;rod([0,0,0],[Math.cos(a)*.198,Math.sin(a)*.198,0],.013,steel,steering);}
  mesh(new T.CylinderGeometry(.044,.044,.045,20),steel,[0,0,0],steering).rotation.x=Math.PI/2;
  const dash=group('stm32');dash.position.set(0,.88,-.76);dash.rotation.x=-.46;
  box([.55,.3,.11],[0,0,0],black,dash);box([.23,.132,.009],[0,.016,.06],silver,dash);box([.205,.112,.01],[0,.016,.069],screen,dash);
  for(const x of [-.245,.245])for(const y of [-.12,.12])mesh(new T.SphereGeometry(.01,8,6),silver,[x,y,.06],dash);
  for(const s of [-1,1]){const pedal=box([.09,.14,.024],[s*.19,.43,-1.16],black);pedal.rotation.x=-.35;rod([s*.19,.33,-1.25],[s*.19,.48,-1.15],.012,steel);}

  function shock(a,b,coilColor=silver) {
    rod(a,b,.033,steel);const start=new T.Vector3(...a),end=new T.Vector3(...b),delta=end.clone().sub(start);
    const g=group();g.position.copy(start);g.quaternion.setFromUnitVectors(new T.Vector3(0,1,0),delta.clone().normalize());
    const len=delta.length(),points=[];
    for(let i=0;i<=168;i++){const t=i/168,angle=t*Math.PI*2*9;points.push([Math.cos(angle)*.051,.07+t*(len-.14),Math.sin(angle)*.051]);}
    tube(points,.0085,coilColor,g);mesh(new T.CylinderGeometry(.06,.06,.022,20),silver,[0,.045,0],g);mesh(new T.CylinderGeometry(.06,.06,.022,20),silver,[0,len-.045,0],g);
  }
  for(const s of [-1,1]){
    // Brazos triangulados y muelles. Mecanismo esquemático, no cinemática resuelta.
    for(const y of [.31,.44]){
      rod([s*.32,y,-1.35],[s*.7,.36,-1.04],.023);
      rod([s*.36,y,-.65],[s*.7,.36,-1.04],.023);
    }
    rod([s*.12,.39,-1.03],[s*.68,.39,-.95],.011,steel);
    shock([s*.63,.34,-1.02],[s*.39,.72,-.94]);
    rod([s*.43,.4,.3],[s*.77,.43,1.05],.031);
    shock([s*.59,.4,1.13],[s*.36,1.07,.65],black);
    box([.053,.38,.1],[s*.8,.66,1.13],red);
    rod([s*.48,.86,1.1],[s*.8,.84,1.13],.031);
  }

  function wheel(id,x,z,r,width,front) {
    const hub=group(id);hub.position.set(x,r,z);const spin=group(null,hub);
    // Tacos, pernos y radios: detalle repetitivo que a velocidad real parpadea; se desvanece con el giro.
    const detail=group(null,spin), fading=[];
    const fadeMat=m=>{const c=m.clone();fading.push(c);return c;};
    const lugMat=fadeMat(front?tread:black), boltMat=fadeMat(steel), spokeMat=fadeMat(silver);
    const markerMat=rotationMarker.clone();   // más tenue con el giro rápido: que se lea como difuminado, no como destello
    // Perfil redondeado del neumático; eje de giro transversal X.
    const profilePoints=[[-width*.52,r*.64],[-width*.52,r*.84],[-width*.45,r*.96],[-width*.3,r],[width*.3,r],[width*.45,r*.96],[width*.52,r*.84],[width*.52,r*.64]];
    const tireGeo=new T.LatheGeometry(profilePoints.map(([ax,rad])=>new T.Vector2(rad,ax)),48);
    const tire=mesh(tireGeo,rubber,null,spin);tire.rotation.z=Math.PI/2;
    cyl(r*.64,width*.86,[0,0,0],silver,spin);
    const outward=Math.sign(x);
    for(const side of [-1,1]){
      const ax=side*width*.48;
      torus(r*.64,.015,[ax,0,0],silver,spin);
      cyl(r*(front?.45:.57),.009,[ax,0,0],front?floor:black,spin);
      for(let k=0;k<(front?4:10);k++){
        const angle=k*Math.PI*2/(front?4:10),rr=r*(front?.28:.44);
        cyl(front?.025:.014,.014,[ax+side*.01,Math.cos(angle)*rr,Math.sin(angle)*rr],boltMat,detail);
      }
      cyl(.062,.026,[ax+side*.015,0,0],silver,spin);
    }
    if(!front){
      const side=outward*width*.53;torus(r*.54,.006,[side,0,0],silver,spin);
      for(let k=0;k<28;k++){const a=k*Math.PI*2/28;rod([side,Math.cos(a)*r*.47,Math.sin(a)*r*.47],[side,Math.cos(a)*r*.55,Math.sin(a)*r*.55],.0035,spokeMat,detail);}
      const brake=box([.085,.13,.075],[-outward*.07,-.06,.28],red,hub);
    }
    // Dibujo de tacos delante; canales oblicuos de carretera detrás.
    const count=front?28:36;
    for(let k=0;k<count;k++)for(const row of [-1,0,1]){
      const a=k*Math.PI*2/count+(row===0?.045:0),radius=r+(front?.006:.001);
      const lug=box([width*(front?.25:.23),front?.019:.002,front?.066:.01],[row*width*.31,Math.cos(a)*radius,Math.sin(a)*radius],lugMat,detail);
      lug.rotation.x=a;lug.rotation.y=(row<0?-1:1)*.32;
    }
    // Una marca clara por vuelta, en banda y flancos; ayuda visual, no pieza del kart.
    const marker=group(null,spin);marker.name='marca-rodadura';
    box([width*.72,.014,.065],[0,r+.012,0],markerMat,marker);
    for(const side of [-1,1])box([.012,r*.22,.04],[side*width*.53,r*.77,0],markerMat,marker);
    // Estela de la marca (banda y flancos). Un solo material por rueda; no se puede seleccionar ni da sombra.
    const trailMat=new T.ShaderMaterial({uniforms:{uArc:{value:0},uDir:{value:0},uPeak:{value:.92},uColor:{value:new T.Vector3(.894,.957,.925)}},
      vertexShader:trailVertex,fragmentShader:trailFragment,transparent:true,depthWrite:false,side:T.DoubleSide,toneMapped:false});
    const trail=group(null,spin);trail.visible=false;
    const band=new T.CylinderGeometry(r+.014,r+.014,width*.74,72,1,true);band.rotateZ(Math.PI/2);
    const parts=[mesh(band,trailMat,null,trail)];
    for(const side of [-1,1]){
      const ring=new T.RingGeometry(r*.66,r*.9,72,1);ring.rotateY(Math.PI/2);ring.translate(side*width*.535,0,0);
      parts.push(mesh(ring,trailMat,null,trail));
    }
    for(const part of parts){part.castShadow=false;part.receiveShadow=false;part.userData.noPick=true;}
    // Devuelve true si cambia algo que haya que volver a dibujar.
    const state={detail:1,arc:0,dir:0};
    const setBlur=(detailLevel,arc,dir)=>{
      if(detailLevel===state.detail&&arc===state.arc&&dir===state.dir)return false;
      if(detailLevel!==state.detail){
        const faded=detailLevel<.999;
        for(const m of fading){if(m.transparent!==faded){m.transparent=faded;m.needsUpdate=true;}m.opacity=detailLevel;}
        detail.visible=detailLevel>.02;
        const strength=.6+.4*detailLevel;
        markerMat.transparent=strength<.999;markerMat.needsUpdate=true;markerMat.opacity=strength;
        trailMat.uniforms.uPeak.value=.92*strength;
      }
      trailMat.uniforms.uArc.value=arc;trailMat.uniforms.uDir.value=dir;trail.visible=arc>.01;
      Object.assign(state,{detail:detailLevel,arc,dir});return true;
    };
    wheels[id]={hub,spin,setBlur,blur:state,trail:trailMat,detail,trailGroup:trail};
    return hub;
  }
  wheel('front_left',-.77,-1.07,.355,.31,true);wheel('front_right',.77,-1.07,.355,.31,true);
  wheel('rear_left',-.79,1.06,.43,.235,false);wheel('rear_right',.79,1.06,.43,.235,false);

  // Plataforma negra y electrónica aproximada. La foto no identifica placas/cableado oculto.
  const battery=group('battery');box([.56,.31,.48],[0,.48,.82],black,battery);
  const deck=box([1.07,.022,.78],[0,.8,1.02],black);covers.push(deck);
  for(const s of [-1,1]){
    const controller=group(s===-1?'motor_left':'motor_right');
    box([.2,.14,.32],[s*.39,.6,1.04],silver,controller);
    for(let i=0;i<8;i++)box([.01,.04,.3],[s*.39-.087+i*.025,.689,1.04],silver,controller);
    tube([[s*.4,.56,1.12],[s*.53,.49,1.22],[s*.66,.43,1.08]],.012,black);
  }
  const electronics=group('esp32');electronics.position.set(0,.82,1.16);electronics.rotation.x=.15;
  box([.36,.24,.3],[0,.12,0],glass,electronics);
  box([.24,.012,.19],[0,.04,0],pcb,electronics);box([.07,.016,.08],[.028,.06,0],steel,electronics);
  for(const x of [-.18,.18])for(const z of [-.15,.15])rod([x,0,z],[x,.24,z],.006,silver,electronics);
  for(const y of [0,.24])tube([[-.18,y,-.15],[.18,y,-.15],[.18,y,.15],[-.18,y,.15]],.005,silver,electronics,true);
  const power=group('power');box([.27,.25,.29],[-.59,.57,.58],wood,power);box([.27,.012,.29],[-.59,.702,.58],silver,power);
  box([.09,.042,.1],[-.64,.73,.61],black,power);
  const stop=mesh(new T.CylinderGeometry(.039,.03,.052,20),red,[-.64,.775,.61],power);
  // Estas dos agrupaciones solo se seleccionan en el inspector: sin anclaje físico inventado.
  group('can');group('imu');
  root.traverse(o=>{if(o.isMesh&&!o.userData.noPick)pickables.push(o);});
  return {root,wheels,groups,pickables,steering,covers};
}
