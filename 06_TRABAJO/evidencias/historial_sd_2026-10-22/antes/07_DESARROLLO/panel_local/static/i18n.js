// Idiomas de la interfaz: español (por defecto), portugués de Portugal e inglés. Selector de un clic en la cabecera.
// La preferencia se guarda en el navegador (localStorage); el español es el idioma si no hay ninguna guardada.
import {idioma, t, saved, save, cargarTextos} from './nucleo.js';
import {TEXTOS, SERVIDOR, PATRONES} from './textos.js';

export const IDIOMAS = {es:{locale:'es-ES', html:'es'}, pt:{locale:'pt-PT', html:'pt-PT'}, en:{locale:'en-GB', html:'en'}};
const ATRIBUTOS = {aria:'aria-label', title:'title', ph:'placeholder', alt:'alt'};
const oyentes = [];

cargarTextos({TEXTOS, SERVIDOR, PATRONES});
const guardado = saved('idioma', 'es');
fijar(Object.hasOwn(IDIOMAS, guardado) ? guardado : 'es');

function fijar(code) {
  idioma.code = code;
  idioma.locale = IDIOMAS[code].locale;
  document.documentElement.lang = IDIOMAS[code].html;
  document.title = t('titulo');
}
// Rellena los elementos marcados con data-i18n (texto) y data-i18n-aria / -title / -ph / -alt (atributos).
export function applyDom(root = document) {
  for (const el of root.querySelectorAll('[data-i18n]')) el.textContent = t(el.dataset.i18n);
  for (const [name, attribute] of Object.entries(ATRIBUTOS))
    for (const el of root.querySelectorAll(`[data-i18n-${name}]`)) el.setAttribute(attribute, t(el.getAttribute(`data-i18n-${name}`)));
}
// Lo que se dibuja por código (gráficas, listas, estado) se reconstruye en estos oyentes tras cada cambio de idioma.
export function onIdioma(callback) { oyentes.push(callback); }
export function setIdioma(code) {
  if (!Object.hasOwn(IDIOMAS, code) || code === idioma.code) return;
  fijar(code);
  save('idioma', code);
  marcarSelector();
  applyDom();
  for (const callback of oyentes) callback(code);
}
function marcarSelector() {
  for (const button of document.querySelectorAll('[data-lang]')) button.setAttribute('aria-pressed', String(button.dataset.lang === idioma.code));
}
export function initIdioma() {
  for (const button of document.querySelectorAll('[data-lang]')) button.addEventListener('click', () => setIdioma(button.dataset.lang));
  marcarSelector();
  applyDom();
}
