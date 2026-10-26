(() => {
const errors=[];window.addEventListener('error',e=>errors.push(e.message));
const root=document.getElementById('kart-frontal-propuesta'),results=[];
const devices=[...root.querySelectorAll('.kf-device')];
for(const d of devices){
 const section=d.closest('[data-variant]');section.hidden=false;
 for(const page of ['Resumen','Entradas','CAN','SD','BMS']){
  d.querySelector('[data-page="'+page+'"]').click();
  const content=d.querySelector('.kf-content');
  results.push({design:d.dataset.design,page,selected:d.querySelector('[data-page="'+page+'"]').getAttribute('aria-pressed')==='true',text:content.innerText,overflow:[...d.querySelectorAll('*')].filter(e=>e.clientWidth>0&&e.scrollWidth>e.clientWidth+1).map(e=>e.className)});
 }
 d.querySelector('[data-page="SD"]').click();d.querySelector('[data-retry]').click();
 results.push({design:d.dataset.design,action:'registro',opening:d.querySelector('.kf-content').innerText.includes('ABRIENDO'),errorKept:d.querySelector('.kf-content').innerText.includes('último error: 1')});
 d.querySelector('[data-retry]').click();results.push({design:d.dataset.design,action:'pausa',requested:d.querySelector('.kf-content').innerText.includes('PAUSA PEDIDA')});
 d.querySelector('[data-page="Resumen"]').click();
 if(d.dataset.design==='mapa'){d.querySelector('[data-wheel="1"]').click();results.push({design:'mapa',action:'rueda',selected:d.querySelector('.kf-map-detail').innerText.includes('Delantera derecha')});}
 const base=d.getBoundingClientRect();results.push({design:d.dataset.design,width:base.width,height:base.height,usable:[...d.children].filter(e=>!e.classList.contains('kf-reserved')).reduce((sum,e)=>sum+e.getBoundingClientRect().height,0),reserved:d.querySelector('.kf-reserved').getBoundingClientRect().height});
 section.hidden=true;
}
for(const scenario of ['Pedal no libre','CAN caducado','Modo confirmado']){
 window.dispatchEvent(new CustomEvent('openai:set_globals',{detail:{globals:{widgetState:{privateContent:{estado:{scenario,page:'Resumen',wheel:2}}}}}}));
 const d=devices[0];d.closest('[data-variant]').hidden=false;
 results.push({scenario,bannerFits:d.querySelector('.kf-banner').scrollHeight<=d.querySelector('.kf-banner').clientHeight+1,text:d.querySelector('.kf-content').innerText,cause:d.querySelector('.kf-banner').innerText});
 d.closest('[data-variant]').hidden=true;
}
window.dispatchEvent(new CustomEvent('openai:set_globals',{detail:{globals:{widgetState:{privateContent:{estado:{scenario:'Sin estado ESP',page:'Resumen',wheel:2}}}}}}));
devices[0].closest('[data-variant]').hidden=false;
parent.postMessage({kind:'kart-qa',width:innerWidth,errors,results},'*');
})();
