import { $, $$ } from '../core/dom.js';

export function input(label, id, type = "text") {
    return `
        <div class="field">
            <label class="label">${label}</label>

            <div class="control">
                <input class="input"
                       type="${type}"
                       id="${id}">
            </div>
        </div>
    `;
}

export function editSection(title, id, body) {
    return `
        <div class="box mb-5">
            <div class="change-toggle is-flex is-justify-content-space-between is-align-items-center"
                 style="cursor:pointer">
                <h3 class="title is-5 mb-0">${title}</h3>

                <span class="icon">
                    <i class="change-icon fas fa-chevron-down"></i>
                </span>
            </div>

            <div class="change-form" id="${id}" style="display:none">
                <hr>
                ${body}
            </div>
        </div>
    `;
}

export function activateToggles() {
  var toggle_array = [...$$(".change-toggle")]; // NodeList -> Array
  var form_array = [...$$(".change-form")];
  var icon_array = [...$$(".change-icon")];
  toggle_array.forEach((toggle, i) => {
    toggle.addEventListener("click", function () {
      const open = form_array[i].style.display === "none";
      form_array[i].style.display = open ? "block" : "none";
      icon_array[i].classList.toggle("fa-chevron-down", !open);
      icon_array[i].classList.toggle("fa-chevron-up", open);
    });
  });
}
