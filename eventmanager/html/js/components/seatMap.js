import { api } from "../core/api.js";
import { $, show } from "../core/dom.js";

export async function loadSeatMap(eventId) {
  const data = await api.events.getSeatMap(eventId);

  // Ако залата няма разделение по сектори, тоест има само един сектор:
  if (!data.has_sectors) {
    $("#sector-id-input").dataset.sectorId = data.no_sector_id;
    return;
  }

  show($("#event-layout-field"));

  const svg = $("#seat-map");
  svg.setAttribute("viewBox", data.viewbox);
  svg.innerHTML = data.background_svg || "";

  data.sectors.forEach((sector) => {
    const path = document.createElementNS("http://www.w3.org/2000/svg", "path");
    path.setAttribute("d", sector.svg_path);
    path.setAttribute("fill", sector.available > 0 ? sector.color : "#ccc");
    path.setAttribute("stroke", "#333");
    path.dataset.sectorId = sector.id;
    path.classList.add("sector");
    if (sector.available === 0) path.classList.add("sold-out");
    else path.addEventListener("click", () => selectSector(sector, path));
    svg.appendChild(path);

    const label = document.createElementNS(
      "http://www.w3.org/2000/svg",
      "text",
    );
    // Центриране на текст
    const label_coords = getPathCenter(sector.svg_path);
    label.setAttribute("x", label_coords.x);
    label.setAttribute("y", label_coords.y);
    label.setAttribute("text-anchor", "middle");
    label.textContent = `${sector.name}`;
    label.classList.add("sector-label");
    svg.appendChild(label);
  });
}

function selectSector(sector, sectorPath) {
  document
    .querySelectorAll(".sector.selected")
    .forEach((el) => el.classList.remove("selected"));
  sectorPath.classList.add("selected");

  $("#sector-id-input").dataset.sectorId = sector.id;

  $("#summary-name").textContent = sector.name;
  $("#summary-price").textContent =
    `${sector.price.toLocaleString("bg-BG", { style: "currency", currency: "EUR" })}, `;
  $("#summary-available").textContent = sector.available;
  $("#sector-summary").style.visibility = "visible";
}

// Помощна функция за центриране на текст
function getPathCenter(pathData) {
  const svgNS = "http://www.w3.org/2000/svg";
  const svg = document.createElementNS(svgNS, "svg");
  const path = document.createElementNS(svgNS, "path");
  path.setAttribute("d", pathData);
  svg.appendChild(path);
  document.body.appendChild(svg);

  const box = path.getBBox();
  document.body.removeChild(svg);

  return {
    x: box.x + box.width / 2,
    y: box.y + box.height / 2,
  };
}
