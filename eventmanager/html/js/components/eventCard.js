export function getEventCard(e) {
  return `
    <a href="/events/${e.id}">
      <div class="cell">
          <figure class="image is-3by4">
            <img src="${e.img_path}" alt="${e.title}">
          </figure>
          <div class="event-caption">
            <p class="subtitle"><b>${e.title}</b></p>
            <p class="subtitle">${e.price.toLocaleString("bg-BG", { style: "currency", currency: "EUR" })}</p>
            <p class="subtitle"><i class="fas fa-location-dot"></i> ${e.venue_name}, ${e.city}</p>
            <p class="subtitle"><i class="fas fa-clock"></i> ${new Date(
              e.begins_at,
            ).toLocaleDateString("bg-BG", {
              day: "2-digit",
              month: "2-digit",
              year: "numeric",
            })}</p>
          </div>
      </div>
    </a>`;
}
