import { $ } from "../core/dom.js";

export function search() {
  $(".navbar .button").addEventListener("click", function () {
    const query = $('.navbar input[type="search"]').value.trim();
    if (query)
      window.location.href = `/events?search=${encodeURIComponent(query)}`;
    else window.location.href = `/events`;
  });

  // може да се потвърди и с Enter
  $('.navbar input[type="search"]').addEventListener("keydown", function (e) {
    if (e.key === "Enter") {
      const query = this.value.trim();
      if (query)
        window.location.href = `/events?search=${encodeURIComponent(query)}`;
      else window.location.href = `/events`;
    }
  });
}
