export function showEvents() {
    fetch(`/api/events${window.location.search}`).then(r => r.json())
      .then(events => {
        document.getElementById("events").innerHTML = events.map(e => `
          <a href="/events/${e.id}">
            <div class="cell">
                <figure class="image is-3by4">
                  <img src="${e.img_path}" alt="${e.title}">
                </figure>
                <p>${e.title}</p>
                <p>${e.price.toLocaleString('bg-BG', { style: 'currency', currency: 'BGN' })}</p>
            </div>
          </a>`).join('');
      });
}