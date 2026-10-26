// Reloj monotónico de reproducción. Módulo puro, sin acceso al kart ni al DOM.
export class ReplayClock {
  constructor(duration=0){this.duration=duration;this.position=0;this.speed=1;this.playing=false;this.last=null;}
  tick(now){if(this.last!==null&&this.playing)this.position=Math.min(this.duration,this.position+Math.max(0,now-this.last)*this.speed);this.last=now;if(this.position>=this.duration)this.playing=false;return this.position;}
  seek(ms){this.position=Math.max(0,Math.min(this.duration,Number(ms)||0));return this.position;}
  rate(value){if(![.25,.5,1,2,4].includes(value))throw new Error('Velocidad no admitida');this.speed=value;}
}
export function historicSignals(signals,at){
  return Object.fromEntries(Object.entries(signals).map(([key,s])=>{
    const age=Math.max(0,at-s.sample_t);return [key,{...s,age_ms:age,quality:s.quality==='fresh'&&age>s.stale_after_ms?'stale':s.quality}];
  }));
export function seriesFromFrames(frames,epoch){
  const output=new Map(),last=new Map();
  for(const frame of frames){
    for(const [key,s] of Object.entries(frame.signals)){
      if(typeof s.value!=='number')continue;
      const series=output.get(key)||[],age=frame.t-s.sample_t,valid=s.quality==='fresh'&&age<=s.stale_after_ms;
      if(valid && last.get(key)!==s.sample_t){
        const prev=series.at(-1);if(prev && epoch+s.sample_t-prev.t>s.stale_after_ms)series.push({t:prev.t+s.stale_after_ms,v:null});
        series.push({t:epoch+s.sample_t,v:s.value});last.set(key,s.sample_t);
      }else if(!valid&&series.length&&series.at(-1).v!==null)series.push({t:epoch+frame.t,v:null});
      output.set(key,series);
    }
  }
  return output;
}
