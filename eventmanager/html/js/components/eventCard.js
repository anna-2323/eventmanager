import { toDatetime, toPrice } from '../core/dom.js';

export function getEventCard(e) {
  return `
    <a href="/events/${e.id}">
      <div class="cell">
          <figure class="image is-3by4">
            <img src="${e.img_path}" alt="${e.title}">
          </figure>
          <div class="event-caption">
            <p class="subtitle"><b>${e.title}</b></p>
            <p class="subtitle">${toPrice(e.price)}</p>
            <p class="subtitle"><i class="fas fa-location-dot"></i> ${e.venue_name}, ${e.city}</p>
            <p class="subtitle"><i class="fas fa-clock"></i> ${toDatetime(e.begins_at)}</p>
          </div>
      </div>
    </a>`;
}
